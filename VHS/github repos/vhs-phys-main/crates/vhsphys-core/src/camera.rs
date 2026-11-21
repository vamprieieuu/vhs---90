//! The camera stage: the video that goes into the recorder is what a three-tube Plumbicon
//! camera made of the scene, with the lag, comet tails and finite beam aperture of its pickup
//! tubes (`tube`) followed by its head amplifier and processing amplifier (`camera_processing`).
//!
//! Each channel's target is simulated on a sub-row grid (`SUB_ROWS = 4` per active line)
//! charged continuously in time by the held input pictures and read line by line, field by
//! field, at each element's own read time (`read_time_s`). The beam has a vertical Gaussian
//! profile across sub-rows and a horizontal Gaussian spot along the line signal, producing
//! the continuous signal current in nA. That line signal then passes through the head
//! amplifier (shot, feedback and rising FET noise, Butterworth band) and processing amplifier
//! (clamp, gain, knee, gamma, aperture correction from delay lines with coring, clip) before
//! baseband encoding.
//!
//! The stage is off by default and then costs nothing: the session does not even build it.

use super::camera_processing::{CameraVideo, HeadAmplifier, Processing, VideoProcessor};
use super::consts::{ACTIVE_H, ACTIVE_START, ACTIVE_W, FH, FRAME_RATE, FS_BB};
use super::stripe::{optical_lowpass, target_light, StripeDecoder, STRIPE_WHITE_LIGHT};
use super::video::{encode_from_rgb, row_to_line, Baseband, Frame};
use crate::geometry::{
    black_shading_factor, displacement_deriv_x, displacement_ph, focus_factor, white_shading_factor,
    GeometrySpec, OpticsPlan, Registration, Shading, BLACK_SQUARE_PH, X_EDGE_PH,
};
use crate::spectral::{colour_matrix, taking_matrix};
use crate::tube::{
    oetf_inverse, photo_current, scan, Dbc, TubeChannel, TubeKind, ACT_BEAM_NA, ACT_FLYBACK_S,
    ACT_SPAN_PH, BEAM_RATIO_DEFAULT, BEND_RADIUS_MAX_LINES, DBC_TOLERANCE_NA, MAX_TRAPS,
    TARGET_ASPECT, V_BEND_ONSET_V,
};
use rayon::prelude::*;
use serde::{Deserialize, Serialize};

/// Sub-rows per active frame row on each target, so the beam's vertical profile overlaps the
/// other field's lines.
pub const SUB_ROWS: usize = 4;

/// Dark scanning time before frame 0; exceeds 30 times the slowest dark settling constant.
const DARK_SETTLE_S: f64 = 10.0;
/// Shortest held-charge sub-step, s: an overloaded element never takes more sub-steps than this grid.
pub const RETENTION_STEP_S: f64 = 0.5e-3;
/// Layer swing per held-charge sub-step, V.
pub const HELD_STEP_V: f64 = 0.5;
const STATE_LEN: usize = MAX_TRAPS + 2;
const HELD_IDX: usize = MAX_TRAPS;
const BURN_IDX: usize = MAX_TRAPS + 1;

thread_local! {
    /// Held-charge sub-steps taken on this thread; read by `UniformColumn::held_steps`.
    static HELD_STEPS: std::cell::Cell<u64> = const { std::cell::Cell::new(0) };
}

fn held_steps_now() -> u64 {
    HELD_STEPS.with(|c| c.get())
}

fn count_held_steps(n: u64) {
    HELD_STEPS.with(|c| c.set(c.get() + n));
}
const IDENTITY_MATRIX: [[f64; 3]; 3] = [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]];

#[inline]
fn needs_element_state(ch: &TubeChannel) -> bool {
    ch.traps.iter().any(|t| t.capture > 0.0)
        || !ch.collection.is_full()
        || ch.burn.rate_per_s > 0.0
}

/// Signal currents read from the targets in one frame: per active frame row, per channel, one
/// value per baseband sample of the active line, nA (before the head amplifier).
#[derive(Clone, Debug, PartialEq)]
pub struct TargetSignal {
    pub rows: Vec<[Vec<f32>; 3]>,
}

/// Charge offered to each element of the ACT flyback span per flyback, nA s:
/// `ACT_BEAM_NA * ACT_FLYBACK_S / ACT_SPAN_PH`.
pub fn act_flyback_nas() -> f64 {
    ACT_BEAM_NA * ACT_FLYBACK_S / ACT_SPAN_PH
}

/// Horizontal sample pitch in frame-line pitches: `TARGET_ASPECT * ACTIVE_H / ACTIVE_W`.
pub fn sample_pitch_lines() -> f64 {
    TARGET_ASPECT * ACTIVE_H as f64 / ACTIVE_W as f64
}

/// `(x, y)` in PH from the picture centre of fractional sample `s` of active row `r`.
#[inline]
pub fn picture_position_ph(r: usize, s: f64) -> (f64, f64) {
    (
        ((s + 0.5) / ACTIVE_W as f64 - 0.5) * TARGET_ASPECT,
        (r as f64 + 0.5) / ACTIVE_H as f64 - 0.5,
    )
}

/// Seconds from the session start at which the beam centre reads fractional sample `s` of active
/// row `r` in frame `n`.
#[inline]
fn read_time_s_f64(n: i64, r: usize, s: f64) -> f64 {
    let (line, _) = row_to_line(r);
    n as f64 / FRAME_RATE + line as f64 / FH + (ACTIVE_START as f64 + s) / FS_BB
}

/// Seconds from the session start at which the beam centre reads sample `x` of active row `r`
/// in frame `n`.
pub fn read_time_s(n: i64, r: usize, x: usize) -> f64 {
    read_time_s_f64(n, r, x as f64)
}

/// Seconds from the session start at which the ACT flyback runs before pass `r` in frame `n`.
fn flyback_time_s(n: i64, r: usize) -> f64 {
    let (line, _) = row_to_line(r);
    n as f64 / FRAME_RATE + line as f64 / FH
}

/// How the camera's tubes see colour (config `layout`: `three-tube`, `stripe`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "kebab-case")]
pub enum Layout {
    #[default]
    ThreeTube,
    Stripe,
}

/// The slot of `TargetSignal` that carries the stripe camera's one target.
pub const STRIPE_SLOT: usize = 1;
/// Frames of white card read before the alignment measures it.
pub const ALIGN_FRAMES: usize = 8;
/// Head-amplifier frame indices of the alignment's white card and capped lens.
pub const ALIGN_WHITE_FRAME: i64 = -1;
pub const ALIGN_CAPPED_FRAME: i64 = -2;

/// The stripe camera's alignment: the decoder's carrier gain, and per channel `[R, G, B]` the decoded
/// level of a capped lens (`floor`) and of a white card above it (`white`).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct StripeAlignment {
    pub carrier_gain: f64,
    pub floor: [f64; 3],
    pub white: [f64; 3],
}

/// The camera: which pickup tubes it has and how it is set up.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
#[serde(default)]
pub struct CameraParams {
    /// The pickup tube (`off`, `plumbicon`, `plumbicon-bias`, `plumbicon-act`, `plumbicon-dbc`,
    /// `vidicon`, `newvicon`, `saticon`, `chalnicon`).
    pub tube: TubeKind,
    /// How the camera's tubes see colour (`three-tube`, `stripe`).
    pub layout: Layout,
    /// Exposure of the scene on the tube targets, in stops (powers of two of the light).
    /// Positive values overexpose: highlights then carry more than the beam can take off.
    pub iris_stops: f64,
    /// Beam current as a multiple of what peak white needs (the data sheet sets 2.0).
    pub beam_ratio: f64,
    /// Beam-bending strength: how strongly an element above the 10 V onset bends the
    /// approaching beam towards itself (`0.0` = off, the default; negative values count as `0`).
    pub blooming: f64,
    /// Camera electronics profile (`studio`, `consumer`).
    pub processing: Processing,
    /// Target temperature in degrees Celsius (default 25.0).
    pub temperature_c: f64,
    /// Faceplate temperature at switch-on, deg C (`None` = already at `temperature_c`).
    pub cold_start_c: Option<f64>,
    /// Lens-capped time with the beam on before the first frame, s (rounded to whole frames; 0 = none).
    pub idle_s: f64,
    /// Time the beam is held off at the start of the shot with the lens open, s (rounded to whole
    /// frames; 0 = on from the start).
    pub beam_off_s: f64,
    /// Spectral taking through the prism, IR-cut and material responses, with the processing
    /// amplifier's colour matrix when the profile carries one (default `false`: ideal taking).
    pub spectral: bool,
    /// Deflection, registration, black/white shading, optical flare and corner focus, plus the
    /// profile's correctors (default `false`: ideal geometry).
    pub geometry: bool,
}

impl Default for CameraParams {
    fn default() -> Self {
        Self {
            tube: TubeKind::Off,
            layout: Layout::default(),
            iris_stops: 0.0,
            beam_ratio: BEAM_RATIO_DEFAULT,
            blooming: 0.0,
            processing: Processing::default(),
            temperature_c: 25.0,
            cold_start_c: None,
            idle_s: 0.0,
            beam_off_s: 0.0,
            spectral: false,
            geometry: false,
        }
    }
}

/// Precomputed vertical Gaussian beam shares `g(y_k - c_r) / S_k` and ACT flyback span for
/// one active row `r`.
#[derive(Clone)]
struct RowWeights {
    k_start: usize,
    shares: Vec<f64>,
    fb_range: std::ops::Range<usize>,
}

/// Sub-row range `|y_k - c_r| <= h_act` covered by a pass centred at `c_r` lines on a grid
/// shifted by `pad_y` top sub-rows.
fn act_flyback_range_at(
    c_r: f64,
    pad_y: usize,
    h_sub_tgt: usize,
    h_act: f64,
) -> std::ops::Range<usize> {
    let lo = (((c_r - h_act) * SUB_ROWS as f64 - 0.5).floor() as isize + pad_y as isize).max(0)
        as usize;
    let hi = (((c_r + h_act) * SUB_ROWS as f64 - 0.5).ceil() as isize + pad_y as isize)
        .min((h_sub_tgt - 1) as isize);
    if hi < lo as isize {
        return 0..0;
    }
    let mut start = None;
    let mut end = 0;
    for ks in lo..=hi as usize {
        let k_signed = ks as isize - pad_y as isize;
        let y_k = (k_signed as f64 + 0.5) / SUB_ROWS as f64;
        if (y_k - c_r).abs() <= h_act {
            if start.is_none() {
                start = Some(ks);
            }
            end = ks + 1;
        }
    }
    match start {
        Some(s) => s..end,
        None => 0..0,
    }
}

/// Sub-row range `|y_k - c_r| <= ACT_SPAN_PH * ACTIVE_H / 2` covered by row `r`'s ACT flyback.
fn act_flyback_range(r: usize) -> std::ops::Range<usize> {
    act_flyback_range_at(
        r as f64 + 0.5,
        0,
        SUB_ROWS * ACTIVE_H,
        ACT_SPAN_PH * ACTIVE_H as f64 / 2.0,
    )
}

/// Build the vertical beam shares for every active row, truncated at `3 * sigma_v` and
/// normalised by `S_k` (the sum of `g` over every pass of a frame at sub-row `k`).
fn build_vertical_weights(sigma_v: f64) -> Vec<RowWeights> {
    let n_sub = SUB_ROWS * ACTIVE_H;
    let cutoff = 3.0 * sigma_v;
    let mut raw: Vec<(usize, Vec<f64>)> = Vec::with_capacity(ACTIVE_H);
    let mut s_k = vec![0.0; n_sub];
    for r in 0..ACTIVE_H {
        let c_r = r as f64 + 0.5;
        let k_min = (((c_r - cutoff) * SUB_ROWS as f64 - 0.5).ceil() as isize).max(0) as usize;
        let k_max =
            (((c_r + cutoff) * SUB_ROWS as f64 - 0.5).floor() as isize).min((n_sub - 1) as isize);
        if k_max < k_min as isize {
            raw.push((0, Vec::new()));
            continue;
        }
        let k_max = k_max as usize;
        let mut gs = Vec::with_capacity(k_max - k_min + 1);
        for k in k_min..=k_max {
            let y_k = (k as f64 + 0.5) / SUB_ROWS as f64;
            let d = (y_k - c_r) / sigma_v;
            let g = (-0.5 * d * d).exp();
            s_k[k] += g;
            gs.push(g);
        }
        raw.push((k_min, gs));
    }
    raw.into_iter()
        .enumerate()
        .map(|(r, (k_start, mut gs))| {
            for (j, g) in gs.iter_mut().enumerate() {
                *g /= s_k[k_start + j];
            }
            RowWeights { k_start, shares: gs, fb_range: act_flyback_range(r) }
        })
        .collect()
}

/// Precompute the horizontal Gaussian spot kernel in baseband sample steps.
fn build_horizontal_kernel(sigma_h_s: f64) -> (usize, Vec<f64>) {
    let sigma_samples = sigma_h_s * FS_BB;
    if sigma_samples <= 0.0 {
        return (0, vec![1.0]);
    }
    let radius = (4.0 * sigma_samples).ceil() as usize;
    let kernel = (-(radius as isize)..=(radius as isize))
        .map(|d| (-0.5 * (d as f64 / sigma_samples).powi(2)).exp())
        .collect();
    (radius, kernel)
}

/// Convolve one active row's signal with the horizontal spot kernel, renormalising at the line
/// ends inside `[0, raw.len())`.
fn convolve_horizontal(raw: &[f64], radius: usize, kernel: &[f64], out: &mut [f32]) {
    let w = raw.len();
    if radius == 0 || w == 0 {
        for (o, &v) in out.iter_mut().zip(raw.iter()) {
            *o = v as f32;
        }
        return;
    }
    let full_sum: f64 = kernel.iter().sum();
    let r_edge = radius.min(w);
    for x in 0..r_edge {
        out[x] = convolve_edge(raw, x, radius, kernel) as f32;
    }
    if w > 2 * radius {
        for x in radius..w - radius {
            let mut acc = 0.0;
            let src = &raw[x - radius..=x + radius];
            for (a, b) in src.iter().zip(kernel.iter()) {
                acc += a * b;
            }
            out[x] = (acc / full_sum) as f32;
        }
    }
    let right_start = r_edge.max(w.saturating_sub(radius));
    for x in right_start..w {
        out[x] = convolve_edge(raw, x, radius, kernel) as f32;
    }
}

fn convolve_edge(raw: &[f64], x: usize, radius: usize, kernel: &[f64]) -> f64 {
    let x0 = x.saturating_sub(radius);
    let x1 = (x + radius + 1).min(raw.len());
    let mut num = 0.0;
    let mut den = 0.0;
    for xi in x0..x1 {
        let w = kernel[xi + radius - x];
        num += w * raw[xi];
        den += w;
    }
    num / den
}

/// Precomputed vertical shares, flyback range and beam read-sample coordinate for one row of
/// one target column under geometry distortion.
#[derive(Clone)]
struct ColRowSpan {
    k_start: usize,
    share_start: usize,
    share_end: usize,
    fb_range: std::ops::Range<usize>,
    s_sample: f64,
}

/// All row passes for a single target column with geometry distortion.
#[derive(Clone)]
struct ColWeights {
    spans: Vec<ColRowSpan>,
    shares: Vec<f64>,
}

/// Padded target dimensions and per-column pass weights for one channel with geometry active.
#[derive(Clone)]
struct GeomScan {
    pad_x: usize,
    pad_y: usize,
    w_tgt: usize,
    h_sub_tgt: usize,
    max_band: usize,
    cols: Vec<ColWeights>,
}

trait PassLookup<'a> {
    fn pass(&self, r: usize) -> (usize, &'a [f64], std::ops::Range<usize>, f64);
}

struct UniformPasses<'a> {
    weights: &'a [RowWeights],
    x: usize,
}

impl<'a> PassLookup<'a> for UniformPasses<'a> {
    #[inline]
    fn pass(&self, r: usize) -> (usize, &'a [f64], std::ops::Range<usize>, f64) {
        let weights = self.weights;
        let rw = &weights[r];
        (rw.k_start, &rw.shares, rw.fb_range.clone(), self.x as f64)
    }
}

struct GeomColPasses<'a> {
    col: &'a ColWeights,
}

impl<'a> PassLookup<'a> for GeomColPasses<'a> {
    #[inline]
    fn pass(&self, r: usize) -> (usize, &'a [f64], std::ops::Range<usize>, f64) {
        let col = self.col;
        let sp = &col.spans[r];
        (
            sp.k_start,
            &col.shares[sp.share_start..sp.share_end],
            sp.fb_range.clone(),
            sp.s_sample,
        )
    }
}

/// Solve `x(s) + dx_ch(x(s), y_r) = x_target_ph` for nominal sample `s` via Newton-Raphson.
fn solve_landing_sample(
    geom: &GeometrySpec,
    ch_idx: usize,
    y_r: f64,
    x_target_ph: f64,
    s_guess: f64,
) -> f64 {
    let mut s = s_guess;
    let samples_per_ph = ACTIVE_W as f64 / TARGET_ASPECT;
    for _ in 0..16 {
        let x_s = ((s + 0.5) / ACTIVE_W as f64 - 0.5) * TARGET_ASPECT;
        let (dx, _) = displacement_ph(geom, ch_idx, x_s, y_r);
        let err_samples = (x_s + dx - x_target_ph) * samples_per_ph;
        if err_samples.abs() < 1e-10 {
            break;
        }
        let deriv = (1.0 + displacement_deriv_x(geom, ch_idx, x_s, y_r)).max(0.1);
        s -= err_samples / deriv;
    }
    s
}

/// Build the padded target grid and per-column vertical shares for channel `ch_idx`.
fn build_geom_scan(ch_idx: usize, ch: &TubeChannel, geom: &GeometrySpec) -> GeomScan {
    let mut max_dx = 0.0f64;
    let mut max_dy = 0.0f64;
    let mut max_f = 1.0f64;
    for iy in 0..=16 {
        let y = -0.6 + 1.2 * (iy as f64 / 16.0);
        for ix in 0..=16 {
            let x = -1.2 * X_EDGE_PH + 2.4 * X_EDGE_PH * (ix as f64 / 16.0);
            let (dx, dy) = displacement_ph(geom, ch_idx, x, y);
            max_dx = max_dx.max(dx.abs());
            max_dy = max_dy.max(dy.abs());
            max_f = max_f.max(focus_factor(geom.corner_focus[ch_idx], x, y));
        }
    }
    let pad_x = (max_dx * 1.25 * ACTIVE_W as f64 / TARGET_ASPECT).ceil() as usize
        + (4.0 * ch.sigma_h_s * FS_BB * max_f).ceil() as usize
        + 4;
    let pad_y_lines = (max_dy * 1.25 * ACTIVE_H as f64).ceil() as usize
        + (3.0 * ch.sigma_v_lines * max_f).ceil() as usize
        + 2;
    let pad_y = pad_y_lines * SUB_ROWS;
    let w_tgt = ACTIVE_W + 2 * pad_x;
    let h_sub_tgt = SUB_ROWS * ACTIVE_H + 2 * pad_y;
    let h_act = ACT_SPAN_PH * ACTIVE_H as f64 / 2.0;

    let cols: Vec<ColWeights> = (0..w_tgt)
        .into_par_iter()
        .map(|jx| {
            let j_signed = jx as isize - pad_x as isize;
            let x_target = ((j_signed as f64 + 0.5) / ACTIVE_W as f64 - 0.5) * TARGET_ASPECT;
            let mut s_k = vec![0.0f64; h_sub_tgt];
            let mut spans = Vec::with_capacity(ACTIVE_H);
            let mut shares = Vec::with_capacity(ACTIVE_H * 12);
            for r in 0..ACTIVE_H {
                let y_r = (r as f64 + 0.5) / ACTIVE_H as f64 - 0.5;
                let s_sample = solve_landing_sample(geom, ch_idx, y_r, x_target, j_signed as f64);
                let (x_nom, _) = picture_position_ph(r, s_sample);
                let (_, dy) = displacement_ph(geom, ch_idx, x_nom, y_r);
                let f = focus_factor(geom.corner_focus[ch_idx], x_nom, y_r);
                let c_rj = r as f64 + 0.5 + dy * ACTIVE_H as f64;
                let sigma_v = ch.sigma_v_lines * f;
                let fb_range = act_flyback_range_at(c_rj, pad_y, h_sub_tgt, h_act);
                if sigma_v <= 0.0 {
                    let cur = shares.len();
                    spans.push(ColRowSpan {
                        k_start: 0,
                        share_start: cur,
                        share_end: cur,
                        fb_range,
                        s_sample,
                    });
                    continue;
                }
                let cutoff = 3.0 * sigma_v;
                let k_min = (((c_rj - cutoff) * SUB_ROWS as f64 - 0.5).ceil() as isize
                    + pad_y as isize)
                    .max(0) as usize;
                let k_max = (((c_rj + cutoff) * SUB_ROWS as f64 - 0.5).floor() as isize
                    + pad_y as isize)
                    .min(h_sub_tgt as isize - 1);
                if k_max < k_min as isize {
                    let cur = shares.len();
                    spans.push(ColRowSpan {
                        k_start: 0,
                        share_start: cur,
                        share_end: cur,
                        fb_range,
                        s_sample,
                    });
                    continue;
                }
                let k_max = k_max as usize;
                let share_start = shares.len();
                for ks in k_min..=k_max {
                    let k_signed = ks as isize - pad_y as isize;
                    let y_k = (k_signed as f64 + 0.5) / SUB_ROWS as f64;
                    let d = (y_k - c_rj) / sigma_v;
                    let g = (-0.5 * d * d).exp();
                    s_k[ks] += g;
                    shares.push(g);
                }
                spans.push(ColRowSpan {
                    k_start: k_min,
                    share_start,
                    share_end: shares.len(),
                    fb_range,
                    s_sample,
                });
            }
            for sp in &spans {
                for (off, val) in shares[sp.share_start..sp.share_end].iter_mut().enumerate() {
                    *val /= s_k[sp.k_start + off];
                }
            }
            ColWeights { spans, shares }
        })
        .collect();

    let max_band = cols
        .iter()
        .flat_map(|c| c.spans.iter())
        .map(|sp| sp.share_end - sp.share_start)
        .max()
        .unwrap_or(0);
    GeomScan { pad_x, pad_y, w_tgt, h_sub_tgt, max_band, cols }
}

/// Evaluate the moving horizontal spot across active row `r` with deflection, scan speed gain and
/// corner focus on the padded target columns `raw`.
fn convolve_horizontal_geom(
    raw: &[f64],
    r: usize,
    pad_x: usize,
    ch_idx: usize,
    sigma_h_s: f64,
    geom: &GeometrySpec,
    out: &mut [f32],
) {
    let w_tgt = raw.len();
    let y_r = (r as f64 + 0.5) / ACTIVE_H as f64 - 0.5;
    let focus = geom.corner_focus[ch_idx];
    let scale = ACTIVE_W as f64 / TARGET_ASPECT;
    for (s, dst) in out.iter_mut().enumerate().take(ACTIVE_W) {
        let x_s = ((s as f64 + 0.5) / ACTIVE_W as f64 - 0.5) * TARGET_ASPECT;
        let (dx, _) = displacement_ph(geom, ch_idx, x_s, y_r);
        let x_pos = s as f64 + dx * scale;
        let x_prime = 1.0 + displacement_deriv_x(geom, ch_idx, x_s, y_r);
        let sigma_x = sigma_h_s * FS_BB * focus_factor(focus, x_s, y_r);
        if sigma_x <= 0.0 {
            let jx = (x_pos.round() as isize + pad_x as isize).clamp(0, w_tgt as isize - 1) as usize;
            *dst = (x_prime * raw[jx]) as f32;
            continue;
        }
        let cutoff = 4.0 * sigma_x;
        let j_lo = ((x_pos - cutoff).ceil() as isize + pad_x as isize).max(0) as usize;
        let j_hi =
            ((x_pos + cutoff).floor() as isize + pad_x as isize).min(w_tgt as isize - 1) as usize;
        if j_hi < j_lo {
            let jx = (x_pos.round() as isize + pad_x as isize).clamp(0, w_tgt as isize - 1) as usize;
            *dst = (x_prime * raw[jx]) as f32;
            continue;
        }
        let mut num = 0.0;
        let mut den = 0.0;
        for (jx, &val) in raw.iter().enumerate().take(j_hi + 1).skip(j_lo) {
            let d = x_pos - (jx as isize - pad_x as isize) as f64;
            let w = (-0.5 * (d / sigma_x).powi(2)).exp();
            num += w * val;
            den += w;
        }
        *dst = (x_prime * num / den) as f32;
    }
}

/// Integrate one sub-element `(x, k)` from its last-landed timestamp to `t_now`, updating
/// stored charge, timestamp and slow states (traps, held charge, burn) in place, and return the
/// integrated charge capped at `ch.plate_nas`.
#[allow(clippy::too_many_arguments)]
#[inline]
fn integrate_sub_row<F>(
    ch_idx: usize,
    ch: &TubeChannel,
    x_pic: usize,
    k: usize,
    pad_y: usize,
    black_shading: Option<(&Shading, f64)>,
    t_now: f64,
    q: &mut f64,
    t_prev_ref: &mut f64,
    mut trap_occ: Option<&mut [f64; STATE_LEN]>,
    photo: &F,
    field_s: f64,
) -> f64
where
    F: Fn(usize, usize, usize, f64) -> f64 + ?Sized,
{
    let t_prev = *t_prev_ref;
    if t_now <= t_prev {
        return *q;
    }
    let shade = match black_shading {
        Some((sh, x_elem)) => {
            let k_signed = k as isize - pad_y as isize;
            let y_elem = (k_signed as f64 + 0.5) / (SUB_ROWS * ACTIVE_H) as f64 - 0.5;
            1.0 + black_shading_factor(sh, x_elem, y_elem)
        }
        None => 1.0,
    };
    let r_pic = (k as isize - pad_y as isize)
        .div_euclid(SUB_ROWS as isize)
        .clamp(0, (ACTIVE_H - 1) as isize) as usize;
    let bias_na = ch.bias_na * shade;
    let plate_nas = ch.plate_nas;

    if ch.collection.is_full() && ch.burn.rate_per_s <= 0.0 && ch.cold_offset_c == 0.0 {
        let dark_plus_bias = (ch.dark_na + ch.bias_na) * shade;
        let mut t0 = t_prev;
        let mut m = (t0 / field_s).floor() as i64;
        let mut photo_nas = 0.0;
        let mut trap_dn = 0.0;
        while t0 < t_now {
            let t_end = ((m + 1) as f64 * field_s).min(t_now);
            let dt_piece = t_end - t0;
            if dt_piece > 0.0 {
                let t_mid = 0.5 * (t0 + t_end);
                let p_na = photo(ch_idx, r_pic, x_pic, t_mid).max(0.0);
                photo_nas += p_na * dt_piece;
                if let Some(occ) = trap_occ.as_deref_mut() {
                    let j_light = p_na + bias_na;
                    for (p, trap) in ch.traps.iter().enumerate() {
                        if trap.capture > 0.0 {
                            let b =
                                trap.capture * j_light / trap.capacity_nas + 1.0 / trap.release_s;
                            let n_inf = trap.capture * j_light / b;
                            let n_0 = occ[p];
                            let n_1 = n_inf + (n_0 - n_inf) * (-b * dt_piece).exp();
                            trap_dn += n_1 - n_0;
                            occ[p] = n_1;
                        }
                    }
                }
            }
            t0 = t_end;
            m += 1;
        }
        let dt = t_now - t_prev;
        let before = (*q + photo_nas + dark_plus_bias * dt - trap_dn).min(plate_nas);
        *q = before;
        *t_prev_ref = t_now;
        return before;
    }

    let has_traps = trap_occ.is_some() && ch.traps.iter().any(|t| t.capture > 0.0);
    let has_collection = !ch.collection.is_full();
    let has_burn = ch.burn.rate_per_s > 0.0;
    let q_knee = if has_collection {
        ch.capacitance_f * (ch.target_voltage_v - ch.collection.knee_v) * 1e9
    } else {
        f64::INFINITY
    };

    let c_nas_per_v = ch.capacitance_f * 1e9;
    let mut q_cur = *q;
    let mut t0 = t_prev;
    let mut m = (t0 / field_s).floor() as i64;
    let mut steps: u64 = 0;
    while t0 < t_now {
        let t_end = ((m + 1) as f64 * field_s).min(t_now);
        let dt_piece = t_end - t0;
        if dt_piece > 0.0 {
            let t_mid = 0.5 * (t0 + t_end);
            let p_na = photo(ch_idx, r_pic, x_pic, t_mid).max(0.0);

            let (h_cur, b_cur) = match trap_occ.as_deref() {
                Some(st) => (st[HELD_IDX], st[BURN_IDX]),
                None => (0.0, 0.0),
            };
            let s_est = if has_burn { 1.0 - b_cur } else { 1.0 };
            let j_est = (p_na + bias_na) * s_est;
            let dark_est = ch.dark_na_at(t_mid)
                * shade
                * if has_burn { 1.0 + ch.burn.dark_gain * b_cur } else { 1.0 };
            let n_sub = if ch.retention_step_s > 0.0 {
                (dt_piece / ch.retention_step_s).ceil().max(1.0) as usize
            } else if has_collection
                && (h_cur > 0.0 || q_cur + (j_est + dark_est) * dt_piece > q_knee)
            {
                let per_swing = ((j_est + dark_est) * dt_piece / (c_nas_per_v * HELD_STEP_V)).ceil();
                let grid = (dt_piece / RETENTION_STEP_S).ceil();
                per_swing.min(grid).max(1.0) as usize
            } else {
                1
            };
            steps += n_sub as u64;
            let ds = dt_piece / n_sub as f64;

            for sub in 0..n_sub {
                let t_sub_mid = t0 + (sub as f64 + 0.5) * ds;
                let (s, dark_burn) = if has_burn {
                    let st = trap_occ.as_deref_mut().expect("burn state");
                    let drive = ch.burn.rate_per_s * (p_na / ch.peak_signal_na);
                    let beta = drive + 1.0 / ch.burn.recovery_s;
                    let b_inf = drive / beta;
                    let b_0 = st[BURN_IDX];
                    let b_1 = b_inf + (b_0 - b_inf) * (-beta * ds).exp();
                    st[BURN_IDX] = b_1;
                    let b_m = 0.5 * (b_0 + b_1);
                    (1.0 - b_m, 1.0 + ch.burn.dark_gain * b_m)
                } else {
                    (1.0, 1.0)
                };
                let j_light = (p_na + bias_na) * s;
                let dark_na = ch.dark_na_at(t_sub_mid) * shade * dark_burn;

                let mut trap_dn = 0.0;
                let mut trap_dn_half = 0.0;
                if has_traps {
                    let st = trap_occ.as_deref_mut().expect("trap state");
                    for (p, trap) in ch.traps.iter().enumerate() {
                        if trap.capture > 0.0 {
                            let b =
                                trap.capture * j_light / trap.capacity_nas + 1.0 / trap.release_s;
                            let n_inf = trap.capture * j_light / b;
                            let n_0 = st[p];
                            let e_half = (-0.5 * b * ds).exp();
                            trap_dn_half += (n_inf - n_0) * (1.0 - e_half);
                            let n_1 = n_inf + (n_0 - n_inf) * (e_half * e_half);
                            trap_dn += n_1 - n_0;
                            st[p] = n_1;
                        }
                    }
                }

                let held_dn = if has_collection {
                    let st = trap_occ.as_deref_mut().expect("held state");
                    let h_0 = st[HELD_IDX];
                    let h_1 = held_midpoint(ch, q_cur, h_0, j_light, dark_na, trap_dn_half, ds);
                    st[HELD_IDX] = h_1;
                    h_1 - h_0
                } else {
                    0.0
                };

                q_cur = (q_cur + j_light * ds - trap_dn - held_dn + dark_na * ds).min(plate_nas);
            }
        }
        t0 = t_end;
        m += 1;
    }
    count_held_steps(steps);
    *q = q_cur;
    *t_prev_ref = t_now;
    q_cur
}

/// One held-charge sub-step of `ds` from charge `q` and held charge `h_0`, with light `j` and dark
/// current `d` constant over it and `trap_dn_half` the traps' change to its middle: the held charge
/// advanced exactly with the collection taken at the charge predicted for the middle of the step.
#[inline]
fn held_midpoint(
    ch: &TubeChannel,
    q: f64,
    h_0: f64,
    j: f64,
    d: f64,
    trap_dn_half: f64,
    ds: f64,
) -> f64 {
    let col = &ch.collection;
    if j == 0.0 {
        // nothing is captured, whatever the collection: the held charge only decays
        return h_0 * (-ds / col.release_s).exp();
    }
    let eta_0 = col.efficiency(ch.layer_voltage_v(q));
    let a_0 = (1.0 - eta_0) * j;
    let beta_0 = a_0 / col.capacity_nas + 1.0 / col.release_s;
    let h_inf_0 = a_0 / beta_0;
    let h_half = h_inf_0 + (h_0 - h_inf_0) * (-0.5 * beta_0 * ds).exp();
    let q_half = (q + (j + d) * (0.5 * ds) - trap_dn_half - (h_half - h_0)).min(ch.plate_nas);
    let eta = col.efficiency(ch.layer_voltage_v(q_half));
    let a = (1.0 - eta) * j;
    let beta = a / col.capacity_nas + 1.0 / col.release_s;
    let h_inf = a / beta;
    h_inf + (h_0 - h_inf) * (-beta * ds).exp()
}

/// Step 1 (ACT only): integrate elements in `fb_range` to `t_fb` and discharge those above
/// `q_act` with flyback beam `b_fb`.
#[allow(clippy::too_many_arguments)]
#[inline]
fn act_flyback_step<F>(
    ch_idx: usize,
    ch: &TubeChannel,
    x_pic: usize,
    pad_y: usize,
    black_shading: Option<(&Shading, f64)>,
    fb_range: std::ops::Range<usize>,
    t_fb: f64,
    q_col: &mut [f64],
    t_col: &mut [f64],
    traps_col: &mut [[f64; STATE_LEN]],
    photo: &F,
    field_s: f64,
    b_fb: f64,
    q_act: f64,
) where
    F: Fn(usize, usize, usize, f64) -> f64 + ?Sized,
{
    let has_traps = !traps_col.is_empty();
    let k_nas = ch.k_nas;
    for k in fb_range {
        let trap_occ = if has_traps { Some(&mut traps_col[k]) } else { None };
        let q = integrate_sub_row(
            ch_idx,
            ch,
            x_pic,
            k,
            pad_y,
            black_shading,
            t_fb,
            &mut q_col[k],
            &mut t_col[k],
            trap_occ,
            photo,
            field_s,
        );
        if q > q_act {
            q_col[k] = q_act + scan(b_fb, k_nas, q - q_act);
        }
    }
}

/// Step 3 (DBC only): Ampex beam law per column by bisection on the band's integrated charges.
fn dbc_beam(
    ch: &TubeChannel,
    dbc: Dbc,
    q_band: &[f64],
    shares: &[f64],
    frame_s: f64,
) -> f64 {
    let k_nas = ch.k_nas;
    let denom = SUB_ROWS as f64 * frame_s;
    let head = dbc.headroom * ch.beam_na;
    let g = |beam_i: f64| -> f64 {
        let mut landed = 0.0;
        let b_frame = beam_i * frame_s;
        for (&q_k, &share) in q_band.iter().zip(shares.iter()) {
            landed += q_k - scan(b_frame * share, k_nas, q_k);
        }
        beam_i - head - landed / denom
    };
    let mut lo = ch.beam_na;
    if g(lo) >= 0.0 {
        return lo;
    }
    let mut hi = (dbc.max_ratio * ch.peak_signal_na).max(lo);
    if g(hi) <= 0.0 {
        return hi;
    }
    while hi - lo > DBC_TOLERANCE_NA {
        let mid = 0.5 * (lo + hi);
        if g(mid) >= 0.0 {
            hi = mid;
        } else {
            lo = mid;
        }
    }
    hi
}

/// Steps 1 to 3 for one column `x` on pass `r`: flyback, integration over `int_start..int_end`,
/// and column beam current `I_b(x)`. Returns `(i_b, has_hot)` where `has_hot` is true if any
/// integrated element exceeds `V_BEND_ONSET_V`.
#[allow(clippy::too_many_arguments)]
#[inline]
fn prepare_column<F>(
    ch_idx: usize,
    ch: &TubeChannel,
    k_start: usize,
    shares: &[f64],
    fb_range: std::ops::Range<usize>,
    s_sample: f64,
    frame: i64,
    r: usize,
    x_pic: usize,
    pad_y: usize,
    black_shading: Option<(&Shading, f64)>,
    int_start: usize,
    int_end: usize,
    q_col: &mut [f64],
    t_col: &mut [f64],
    traps_col: &mut [[f64; STATE_LEN]],
    photo: &F,
    frame_s: f64,
    field_s: f64,
    b_fb: f64,
    q_act: f64,
    beam_on: bool,
) -> (f64, bool)
where
    F: Fn(usize, usize, usize, f64) -> f64 + ?Sized,
{
    let has_traps = !traps_col.is_empty();
    if ch.act && beam_on {
        let t_fb = flyback_time_s(frame, r);
        act_flyback_step(
            ch_idx,
            ch,
            x_pic,
            pad_y,
            black_shading,
            fb_range,
            t_fb,
            q_col,
            t_col,
            traps_col,
            photo,
            field_s,
            b_fb,
            q_act,
        );
    }
    let t_now = read_time_s_f64(frame, r, s_sample);
    let check_hot = beam_on && ch.bending > 0.0;
    let mut hot = false;
    for k in int_start..int_end {
        let trap_occ = if has_traps { Some(&mut traps_col[k]) } else { None };
        let q = integrate_sub_row(
            ch_idx,
            ch,
            x_pic,
            k,
            pad_y,
            black_shading,
            t_now,
            &mut q_col[k],
            &mut t_col[k],
            trap_occ,
            photo,
            field_s,
        );
        if check_hot && ch.potential_v(q) > V_BEND_ONSET_V {
            hot = true;
        }
    }
    let band_end = k_start + shares.len();
    let i_b = if !beam_on {
        0.0
    } else {
        match ch.dbc {
            Some(dbc) => dbc_beam(ch, dbc, &q_col[k_start..band_end], shares, frame_s),
            None => ch.beam_na,
        }
    };
    (i_b, hot)
}

/// Step 5 (bending on): find the captor `(x', k')` for aim `(x, k)` among elements above
/// `V_BEND_ONSET_V` within their radius `rho(V)`, nearest first, breaking ties by higher `V`,
/// then smaller `x'`, then smaller `k'`.
#[allow(clippy::too_many_arguments)]
fn find_captor(
    ch: &TubeChannel,
    q_all: &[f64],
    col_stride: usize,
    col_hot: &[bool],
    x_lo: usize,
    x_hi: usize,
    x: usize,
    k: usize,
    int_start: usize,
    int_end: usize,
    dk_max: usize,
    p_h: f64,
) -> (usize, usize) {
    if ch.potential_v(q_all[x * col_stride + k]) > V_BEND_ONSET_V {
        return (x, k);
    }
    if !col_hot[x_lo..=x_hi].iter().any(|&h| h) {
        return (x, k);
    }
    let k_lo = k.saturating_sub(dk_max).max(int_start);
    let k_hi = (k + dk_max).min(int_end - 1);
    let mut best: Option<(f64, f64, usize, usize)> = None;
    for xp in x_lo..=x_hi {
        if !col_hot[xp] {
            continue;
        }
        let dx = (xp as f64 - x as f64) * p_h;
        let dx2 = dx * dx;
        let col_q = &q_all[xp * col_stride..];
        for kp in k_lo..=k_hi {
            let v = ch.potential_v(col_q[kp]);
            if v <= V_BEND_ONSET_V {
                continue;
            }
            let rho = ch.bend_radius_lines(v);
            let dk = (kp as f64 - k as f64) / SUB_ROWS as f64;
            let d = (dx2 + dk * dk).sqrt();
            if d <= rho {
                let better = match best {
                    None => true,
                    Some((bd, bv, bx, bk)) => {
                        d < bd
                            || (d == bd
                                && (v > bv || (v == bv && (xp < bx || (xp == bx && kp < bk)))))
                    }
                };
                if better {
                    best = Some((d, v, xp, kp));
                }
            }
        }
    }
    match best {
        Some((_, _, xp, kp)) => (xp, kp),
        None => (x, k),
    }
}

/// Scan all active rows of one frame for a single column and channel `ch_idx`, updating the
/// column's stored sub-row charges `q_col`, last-landed times `t_col`, and (when the channel has
/// traps) trap occupancies `traps_col` in place.
#[allow(clippy::too_many_arguments)]
fn scan_column<'a, F, P: PassLookup<'a>>(
    ch_idx: usize,
    ch: &TubeChannel,
    passes: &P,
    frame: i64,
    x_pic: usize,
    pad_y: usize,
    h_sub_tgt: usize,
    black_shading: Option<(&Shading, f64)>,
    q_col: &mut [f64],
    t_col: &mut [f64],
    traps_col: &mut [[f64; STATE_LEN]],
    raw_col: &mut [f64],
    photo: &F,
    beam_on: bool,
) where
    F: Fn(usize, usize, usize, f64) -> f64 + ?Sized,
{
    let frame_s = 1.0 / FRAME_RATE;
    let field_s = 0.5 * frame_s;
    let k_nas = ch.k_nas;
    let b_fb = act_flyback_nas();
    let q_act = ch.act_level_nas();
    let bending = ch.bending > 0.0;
    let pad = if bending { (BEND_RADIUS_MAX_LINES * SUB_ROWS as f64).ceil() as usize } else { 0 };
    let dk_max = pad;
    let p_h = sample_pitch_lines();

    let max_band = if beam_on && bending {
        (0..ACTIVE_H).map(|r| passes.pass(r).1.len()).max().unwrap_or(0)
    } else {
        0
    };
    let max_int = max_band + 2 * pad;
    let mut captors = if beam_on && bending { vec![0usize; max_band] } else { Vec::new() };
    let mut b_received = if beam_on && bending { vec![0.0; max_int] } else { Vec::new() };
    let mut landed_elem = if beam_on && bending { vec![0.0; max_int] } else { Vec::new() };

    for field in 0..2 {
        for r in (field..ACTIVE_H).step_by(2) {
            let (band_start, shares, fb_range, s_sample) = passes.pass(r);
            let band_len = shares.len();
            if band_len == 0 {
                raw_col[r] = 0.0;
                continue;
            }
            let band_end = band_start + band_len;
            let int_start = band_start.saturating_sub(pad);
            let int_end = (band_end + pad).min(h_sub_tgt);
            let (i_b, hot) = prepare_column(
                ch_idx,
                ch,
                band_start,
                shares,
                fb_range,
                s_sample,
                frame,
                r,
                x_pic,
                pad_y,
                black_shading,
                int_start,
                int_end,
                q_col,
                t_col,
                traps_col,
                photo,
                frame_s,
                field_s,
                b_fb,
                q_act,
                beam_on,
            );
            if !beam_on {
                raw_col[r] = 0.0;
                continue;
            }
            let beam_frame_nas = i_b * frame_s;
            if !hot {
                let mut landed_sum = 0.0;
                for (j, &share) in shares.iter().enumerate() {
                    let k = band_start + j;
                    let before = q_col[k];
                    let after = scan(beam_frame_nas * share, k_nas, before);
                    landed_sum += before - after;
                    q_col[k] = after;
                }
                raw_col[r] = landed_sum / (SUB_ROWS as f64 * frame_s);
            } else {
                let int_len = int_end - int_start;
                b_received[..int_len].fill(0.0);
                for (j, _) in shares.iter().enumerate() {
                    let k = band_start + j;
                    let (_, ck) = find_captor(
                        ch,
                        q_col,
                        h_sub_tgt,
                        &[true],
                        0,
                        0,
                        0,
                        k,
                        int_start,
                        int_end,
                        dk_max,
                        p_h,
                    );
                    let off = ck - int_start;
                    captors[j] = off;
                    b_received[off] += beam_frame_nas * shares[j];
                }
                for offset in 0..int_len {
                    let b_e = b_received[offset];
                    if b_e > 0.0 {
                        let ck = int_start + offset;
                        let before = q_col[ck];
                        let after = scan(b_e, k_nas, before);
                        q_col[ck] = after;
                        landed_elem[offset] = before - after;
                    } else {
                        landed_elem[offset] = 0.0;
                    }
                }
                let mut landed_sum = 0.0;
                for (j, &share) in shares.iter().enumerate() {
                    let idx = captors[j];
                    let b_e = b_received[idx];
                    if b_e > 0.0 {
                        let b = beam_frame_nas * share;
                        landed_sum += landed_elem[idx] * (b / b_e);
                    }
                }
                raw_col[r] = landed_sum / (SUB_ROWS as f64 * frame_s);
            }
        }
    }
}

/// Scan all active rows of one frame row by row for channel `ch_idx` when beam bending is active,
/// parallelising across columns within each pass.
#[allow(clippy::too_many_arguments)]
fn scan_channel_row_by_row<F>(
    ch_idx: usize,
    ch: &TubeChannel,
    weights: &[RowWeights],
    geom_scan: Option<&GeomScan>,
    shading: &Shading,
    frame: i64,
    q_all: &mut [f64],
    t_all: &mut [f64],
    traps_all: &mut [[f64; STATE_LEN]],
    raw_all: &mut [f64],
    photo: &F,
) where
    F: Fn(usize, usize, usize, f64) -> f64 + Sync + ?Sized,
{
    let w_tgt = geom_scan.map_or(ACTIVE_W, |gs| gs.w_tgt);
    let h_sub_tgt = geom_scan.map_or(SUB_ROWS * ACTIVE_H, |gs| gs.h_sub_tgt);
    let pad_x = geom_scan.map_or(0, |gs| gs.pad_x);
    let pad_y = geom_scan.map_or(0, |gs| gs.pad_y);
    let use_black = *shading != Shading::NONE && geom_scan.is_some();

    let frame_s = 1.0 / FRAME_RATE;
    let field_s = 0.5 * frame_s;
    let k_nas = ch.k_nas;
    let b_fb = act_flyback_nas();
    let q_act = ch.act_level_nas();
    let pad = (BEND_RADIUS_MAX_LINES * SUB_ROWS as f64).ceil() as usize;
    let col_extra = if geom_scan.is_some() { pad + 2 } else { pad };
    let p_h = sample_pitch_lines();
    let dx_max = (BEND_RADIUS_MAX_LINES / p_h).floor() as usize;
    let dk_max = pad;

    let max_band = geom_scan
        .map_or_else(|| weights.iter().map(|rw| rw.shares.len()).max().unwrap_or(0), |gs| gs.max_band);
    let dy_margin = if geom_scan.is_some() { h_sub_tgt } else { max_band + 2 * pad };
    let _ = dy_margin;
    let mut i_b = vec![0.0f64; w_tgt];
    let mut col_hot = vec![false; w_tgt];
    let mut captors = vec![(0usize, 0usize); w_tgt * max_band];
    let mut b_received: Vec<f64> = Vec::new();
    let mut landed_elem: Vec<f64> = Vec::new();

    for field in 0..2 {
        for r in (field..ACTIVE_H).step_by(2) {
            let (r_int_start, r_int_end, r_band_len) = match geom_scan {
                None => {
                    let rw = &weights[r];
                    let band_len = rw.shares.len();
                    if band_len == 0 {
                        raw_all.par_chunks_mut(ACTIVE_H).for_each(|col| col[r] = 0.0);
                        continue;
                    }
                    let int_start = rw.k_start.saturating_sub(pad);
                    let int_end = (rw.k_start + band_len + pad).min(h_sub_tgt);
                    (int_start, int_end, band_len)
                }
                Some(gs) => {
                    let mut lo = usize::MAX;
                    let mut hi = 0usize;
                    let mut mb = 0usize;
                    for col in &gs.cols {
                        let sp = &col.spans[r];
                        let len = sp.share_end - sp.share_start;
                        if len > 0 {
                            lo = lo.min(sp.k_start.saturating_sub(col_extra));
                            hi = hi.max((sp.k_start + len + col_extra).min(h_sub_tgt));
                            mb = mb.max(len);
                        }
                    }
                    if mb == 0 {
                        raw_all.par_chunks_mut(ACTIVE_H).for_each(|col| col[r] = 0.0);
                        continue;
                    }
                    (lo, hi, mb)
                }
            };
            let int_len = r_int_end - r_int_start;

            let prepare_col = |jx: usize,
                               q_col: &mut [f64],
                               t_col: &mut [f64],
                               trap_col: &mut [[f64; STATE_LEN]]|
             -> (f64, bool) {
                let (k_start, shares, fb_range, s_sample) = match geom_scan {
                    None => UniformPasses { weights, x: jx }.pass(r),
                    Some(gs) => GeomColPasses { col: &gs.cols[jx] }.pass(r),
                };
                let band_len = shares.len();
                let c_int_start = k_start.saturating_sub(col_extra);
                let c_int_end = (k_start + band_len + col_extra).min(h_sub_tgt);
                let j_signed = jx as isize - pad_x as isize;
                let x_pic = j_signed.clamp(0, (ACTIVE_W - 1) as isize) as usize;
                let black_shade = use_black.then(|| {
                    let x_elem = ((j_signed as f64 + 0.5) / ACTIVE_W as f64 - 0.5) * TARGET_ASPECT;
                    (shading, x_elem)
                });
                prepare_column(
                    ch_idx,
                    ch,
                    k_start,
                    shares,
                    fb_range,
                    s_sample,
                    frame,
                    r,
                    x_pic,
                    pad_y,
                    black_shade,
                    c_int_start,
                    c_int_end,
                    q_col,
                    t_col,
                    trap_col,
                    photo,
                    frame_s,
                    field_s,
                    b_fb,
                    q_act,
                    true,
                )
            };

            if traps_all.is_empty() {
                q_all
                    .par_chunks_mut(h_sub_tgt)
                    .zip(t_all.par_chunks_mut(h_sub_tgt))
                    .zip(i_b.par_iter_mut())
                    .zip(col_hot.par_iter_mut())
                    .enumerate()
                    .for_each(|(jx, (((q_col, t_col), ib_out), hot_out))| {
                        let (ib, hot) = prepare_col(jx, q_col, t_col, &mut []);
                        *ib_out = ib;
                        *hot_out = hot;
                    });
            } else {
                q_all
                    .par_chunks_mut(h_sub_tgt)
                    .zip(t_all.par_chunks_mut(h_sub_tgt))
                    .zip(traps_all.par_chunks_mut(h_sub_tgt))
                    .zip(i_b.par_iter_mut())
                    .zip(col_hot.par_iter_mut())
                    .enumerate()
                    .for_each(|(jx, ((((q_col, t_col), trap_col), ib_out), hot_out))| {
                        let (ib, hot) = prepare_col(jx, q_col, t_col, trap_col);
                        *ib_out = ib;
                        *hot_out = hot;
                    });
            }

            if !col_hot.iter().any(|&h| h) {
                q_all
                    .par_chunks_mut(h_sub_tgt)
                    .zip(raw_all.par_chunks_mut(ACTIVE_H))
                    .zip(i_b.par_iter())
                    .enumerate()
                    .for_each(|(jx, ((q_col, raw_col), &ib))| {
                        let (band_start, shares, _, _) = match geom_scan {
                            None => UniformPasses { weights, x: jx }.pass(r),
                            Some(gs) => GeomColPasses { col: &gs.cols[jx] }.pass(r),
                        };
                        let beam_frame_nas = ib * frame_s;
                        let mut landed_sum = 0.0;
                        for (j, &share) in shares.iter().enumerate() {
                            let k = band_start + j;
                            let before = q_col[k];
                            let after = scan(beam_frame_nas * share, k_nas, before);
                            landed_sum += before - after;
                            q_col[k] = after;
                        }
                        raw_col[r] = landed_sum / (SUB_ROWS as f64 * frame_s);
                    });
                continue;
            }

            // Step 5: capture across columns (read-only on q_all).
            {
                let q_ro: &[f64] = q_all;
                let hot_ro: &[bool] = &col_hot;
                captors[..w_tgt * r_band_len]
                    .par_chunks_mut(r_band_len)
                    .enumerate()
                    .for_each(|(jx, cap_col)| {
                        let (band_start, shares, _, _) = match geom_scan {
                            None => UniformPasses { weights, x: jx }.pass(r),
                            Some(gs) => GeomColPasses { col: &gs.cols[jx] }.pass(r),
                        };
                        let x_lo = jx.saturating_sub(dx_max);
                        let x_hi = (jx + dx_max).min(w_tgt - 1);
                        for j in 0..shares.len() {
                            cap_col[j] = find_captor(
                                ch,
                                q_ro,
                                h_sub_tgt,
                                hot_ro,
                                x_lo,
                                x_hi,
                                jx,
                                band_start + j,
                                r_int_start,
                                r_int_end,
                                dk_max,
                                p_h,
                            );
                        }
                    });
            }

            // Step 6: accumulate captured beam per element, then land once per element.
            let need = w_tgt * int_len;
            if b_received.len() < need {
                b_received.resize(need, 0.0);
                landed_elem.resize(need, 0.0);
            }
            b_received[..need].fill(0.0);
            for jx in 0..w_tgt {
                let (_, shares, _, _) = match geom_scan {
                    None => UniformPasses { weights, x: jx }.pass(r),
                    Some(gs) => GeomColPasses { col: &gs.cols[jx] }.pass(r),
                };
                let beam_frame_nas = i_b[jx] * frame_s;
                let cap_col = &captors[jx * r_band_len..(jx + 1) * r_band_len];
                for (j, &share) in shares.iter().enumerate() {
                    let (cx, ck) = cap_col[j];
                    b_received[cx * int_len + (ck - r_int_start)] += beam_frame_nas * share;
                }
            }

            q_all
                .par_chunks_mut(h_sub_tgt)
                .zip(b_received[..need].par_chunks(int_len))
                .zip(landed_elem[..need].par_chunks_mut(int_len))
                .for_each(|((q_col, b_col), land_col)| {
                    for offset in 0..int_len {
                        let b_e = b_col[offset];
                        if b_e > 0.0 {
                            let ck = r_int_start + offset;
                            let before = q_col[ck];
                            let after = scan(b_e, k_nas, before);
                            q_col[ck] = after;
                            land_col[offset] = before - after;
                        } else {
                            land_col[offset] = 0.0;
                        }
                    }
                });

            // Step 7: share landed charge back to each aim's column.
            let b_ro = &b_received[..need];
            let land_ro = &landed_elem[..need];
            raw_all
                .par_chunks_mut(ACTIVE_H)
                .zip(captors[..w_tgt * r_band_len].par_chunks(r_band_len))
                .zip(i_b.par_iter())
                .enumerate()
                .for_each(|(jx, ((raw_col, cap_col), &ib))| {
                    let (_, shares, _, _) = match geom_scan {
                        None => UniformPasses { weights, x: jx }.pass(r),
                        Some(gs) => GeomColPasses { col: &gs.cols[jx] }.pass(r),
                    };
                    let beam_frame_nas = ib * frame_s;
                    let mut landed_sum = 0.0;
                    for (j, &share) in shares.iter().enumerate() {
                        let (cx, ck) = cap_col[j];
                        let idx = cx * int_len + (ck - r_int_start);
                        let b_e = b_ro[idx];
                        if b_e > 0.0 {
                            let b = beam_frame_nas * share;
                            landed_sum += land_ro[idx] * (b / b_e);
                        }
                    }
                    raw_col[r] = landed_sum / (SUB_ROWS as f64 * frame_s);
                });
        }
    }
}

fn idle_slow_states_beam_on(
    ch: &TubeChannel,
    dt: f64,
    black_shading: Option<&Shading>,
    pad_y: usize,
    x_elem: f64,
    state_col: &mut [[f64; STATE_LEN]],
) {
    let has_traps = ch.traps.iter().any(|t| t.capture > 0.0);
    let has_collection = !ch.collection.is_full();
    let has_burn = ch.burn.rate_per_s > 0.0;
    let h_decay = if has_collection { (-dt / ch.collection.release_s).exp() } else { 1.0 };
    let b_decay = if has_burn { (-dt / ch.burn.recovery_s).exp() } else { 1.0 };
    for (k, st) in state_col.iter_mut().enumerate() {
        let shade = match black_shading {
            Some(sh) => {
                let k_signed = k as isize - pad_y as isize;
                let y_elem = (k_signed as f64 + 0.5) / (SUB_ROWS * ACTIVE_H) as f64 - 0.5;
                1.0 + black_shading_factor(sh, x_elem, y_elem)
            }
            None => 1.0,
        };
        let bias_na = ch.bias_na * shade;
        if has_traps {
            for (p, trap) in ch.traps.iter().enumerate() {
                if trap.capture > 0.0 {
                    if bias_na == 0.0 {
                        st[p] *= (-dt / trap.release_s).exp();
                    } else {
                        let b = trap.capture * bias_na / trap.capacity_nas + 1.0 / trap.release_s;
                        let n_inf = trap.capture * bias_na / b;
                        st[p] = n_inf + (st[p] - n_inf) * (-b * dt).exp();
                    }
                }
            }
        }
        if has_collection {
            st[HELD_IDX] *= h_decay;
        }
        if has_burn {
            st[BURN_IDX] *= b_decay;
        }
    }
}

fn idle_dark_element(
    ch: &TubeChannel,
    shade: f64,
    dt: f64,
    q: &mut f64,
    t_prev_ref: &mut f64,
    mut state: Option<&mut [f64; STATE_LEN]>,
) {
    if dt <= 0.0 {
        return;
    }
    let t_prev = *t_prev_ref;
    let has_traps = state.is_some() && ch.traps.iter().any(|t| t.capture > 0.0);
    let has_collection = !ch.collection.is_full();
    let has_burn = ch.burn.rate_per_s > 0.0;
    let warming = ch.cold_offset_c != 0.0 && !ch.dark_doubling_c.is_infinite();
    let bias_na = ch.bias_na * shade;
    let plate_nas = ch.plate_nas;

    let q_knee = if has_collection {
        ch.capacitance_f * (ch.target_voltage_v - ch.collection.knee_v) * 1e9
    } else {
        f64::INFINITY
    };
    let h_0 = state.as_deref().map_or(0.0, |st| st[HELD_IDX]);
    let b_0 = state.as_deref().map_or(0.0, |st| st[BURN_IDX]);
    let held_regime = has_collection
        && bias_na > 0.0
        && (h_0 > 0.0 || *q + (bias_na + ch.dark_na_at(t_prev) * shade) * dt > q_knee);
    let n_steps = if ch.retention_step_s > 0.0 {
        (dt / ch.retention_step_s).ceil().max(1.0) as usize
    } else {
        let n_held = if held_regime {
            ((bias_na + ch.dark_na_at(t_prev) * shade) * dt
                / (ch.capacitance_f * 1e9 * HELD_STEP_V))
                .ceil()
                .min((dt / RETENTION_STEP_S).ceil())
        } else {
            1.0
        };
        let n_slow = if warming || (has_burn && b_0 > 0.0) { dt.ceil() } else { 1.0 };
        n_held.max(n_slow).max(1.0) as usize
    };
    let ds = dt / n_steps as f64;
    let mut q_cur = *q;
    for step in 0..n_steps {
        if !warming && !has_burn && q_cur >= plate_nas {
            let rem_dt = (n_steps - step) as f64 * ds;
            if has_traps {
                let st = state.as_deref_mut().expect("trap state");
                for (p, trap) in ch.traps.iter().enumerate() {
                    if trap.capture > 0.0 {
                        let n_0 = st[p];
                        st[p] = if bias_na == 0.0 {
                            n_0 * (-rem_dt / trap.release_s).exp()
                        } else {
                            let b = trap.capture * bias_na / trap.capacity_nas + 1.0 / trap.release_s;
                            let n_inf = trap.capture * bias_na / b;
                            n_inf + (n_0 - n_inf) * (-b * rem_dt).exp()
                        };
                    }
                }
            }
            if has_collection {
                let eta = ch.collection.efficiency(ch.layer_voltage_v(plate_nas));
                let a = (1.0 - eta) * bias_na;
                let st = state.as_deref_mut().expect("held state");
                let h_cur = st[HELD_IDX];
                st[HELD_IDX] = if a == 0.0 {
                    h_cur * (-rem_dt / ch.collection.release_s).exp()
                } else {
                    let beta = a / ch.collection.capacity_nas + 1.0 / ch.collection.release_s;
                    let h_inf = a / beta;
                    h_inf + (h_cur - h_inf) * (-beta * rem_dt).exp()
                };
            }
            break;
        }
        let t_mid = t_prev + (step as f64 + 0.5) * ds;
        let (s, dark_burn) = if has_burn {
            let st = state.as_deref_mut().expect("burn state");
            let b0 = st[BURN_IDX];
            let b1 = b0 * (-ds / ch.burn.recovery_s).exp();
            st[BURN_IDX] = b1;
            let b_m = 0.5 * (b0 + b1);
            (1.0 - b_m, 1.0 + ch.burn.dark_gain * b_m)
        } else {
            (1.0, 1.0)
        };
        let j_light = bias_na * s;
        let dark_na = ch.dark_na_at(t_mid) * shade * dark_burn;

        let mut trap_dn = 0.0;
        let mut trap_dn_half = 0.0;
        if has_traps {
            let st = state.as_deref_mut().expect("trap state");
            for (p, trap) in ch.traps.iter().enumerate() {
                if trap.capture > 0.0 {
                    let n_0 = st[p];
                    let n_1 = if j_light == 0.0 {
                        let e_half = (-0.5 * ds / trap.release_s).exp();
                        trap_dn_half += -n_0 * (1.0 - e_half);
                        n_0 * e_half * e_half
                    } else {
                        let b = trap.capture * j_light / trap.capacity_nas + 1.0 / trap.release_s;
                        let n_inf = trap.capture * j_light / b;
                        let e_half = (-0.5 * b * ds).exp();
                        trap_dn_half += (n_inf - n_0) * (1.0 - e_half);
                        n_inf + (n_0 - n_inf) * (e_half * e_half)
                    };
                    trap_dn += n_1 - n_0;
                    st[p] = n_1;
                }
            }
        }

        let held_dn = if has_collection {
            let st = state.as_deref_mut().expect("held state");
            let h_cur = st[HELD_IDX];
            let h_1 = held_midpoint(ch, q_cur, h_cur, j_light, dark_na, trap_dn_half, ds);
            st[HELD_IDX] = h_1;
            h_1 - h_cur
        } else {
            0.0
        };

        q_cur = (q_cur + j_light * ds - trap_dn - held_dn + dark_na * ds).min(plate_nas);
    }
    *q = q_cur;
    *t_prev_ref = t_prev + dt;
}

#[allow(clippy::too_many_arguments)]
fn idle_dark_column(
    ch: &TubeChannel,
    dt: f64,
    black_shading: Option<&Shading>,
    pad_y: usize,
    x_elem: f64,
    q_col: &mut [f64],
    t_col: &mut [f64],
    state_col: &mut [[f64; STATE_LEN]],
) {
    let has_state = !state_col.is_empty();
    for k in 0..q_col.len() {
        let shade = match black_shading {
            Some(sh) => {
                let k_signed = k as isize - pad_y as isize;
                let y_elem = (k_signed as f64 + 0.5) / (SUB_ROWS * ACTIVE_H) as f64 - 0.5;
                1.0 + black_shading_factor(sh, x_elem, y_elem)
            }
            None => 1.0,
        };
        let state_opt = if has_state { Some(&mut state_col[k]) } else { None };
        idle_dark_element(ch, shade, dt, &mut q_col[k], &mut t_col[k], state_opt);
    }
}

fn reset_geom_charge_and_time(
    ch: &TubeChannel,
    gs: &GeomScan,
    frame: i64,
    settled_q: &[f64],
    charge_k: &mut [f64],
    last_t_k: &mut [f64],
) {
    let n_sub = SUB_ROWS * ACTIVE_H;
    let h_sub_tgt = gs.h_sub_tgt;
    let pad_y = gs.pad_y as isize;
    let bend_pad = if ch.bending > 0.0 {
        (BEND_RADIUS_MAX_LINES * SUB_ROWS as f64).ceil() as usize + 2
    } else {
        0
    };
    let f_prev = frame - 1;
    let t_shift = frame as f64 / FRAME_RATE;
    last_t_k.fill(-DARK_SETTLE_S + t_shift);
    for (jx, col) in gs.cols.iter().enumerate() {
        let off = jx * h_sub_tgt;
        for ks in 0..h_sub_tgt {
            let k_pic = (ks as isize - pad_y).clamp(0, n_sub as isize - 1) as usize;
            charge_k[off + ks] = settled_q[k_pic];
        }
        for field in 0..2 {
            for r in (field..ACTIVE_H).step_by(2) {
                let sp = &col.spans[r];
                if ch.act {
                    let t_fb = flyback_time_s(f_prev, r);
                    for k in sp.fb_range.clone() {
                        last_t_k[off + k] = t_fb;
                    }
                }
                let t_now = read_time_s_f64(f_prev, r, sp.s_sample);
                let band_len = sp.share_end - sp.share_start;
                if band_len > 0 {
                    let int_start = sp.k_start.saturating_sub(bend_pad);
                    let int_end = (sp.k_start + band_len + bend_pad).min(h_sub_tgt);
                    for k in int_start..int_end {
                        last_t_k[off + k] = t_now;
                    }
                }
            }
        }
    }
}

/// State of one sub-element of a `UniformColumn`; 0 for the states its channel does not carry.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct ElementState {
    pub charge_nas: f64,
    pub traps_nas: [f64; MAX_TRAPS],
    pub held_nas: f64,
    pub burn: f64,
}

/// One uniformly lit target column scanned by the same row passes as the full camera.
pub struct UniformColumn {
    ch: TubeChannel,
    weights: Vec<RowWeights>,
    charge: Vec<f64>,
    last_t: Vec<f64>,
    traps: Vec<[f64; STATE_LEN]>,
    settled_charge: Vec<f64>,
    settled_last_t: Vec<f64>,
    dark: Vec<f64>,
    frame: i64,
    beam_on: bool,
    held_steps: u64,
}

impl UniformColumn {
    /// Start at zero charge and scan the column dark until its state settles.
    pub fn new(ch: &TubeChannel) -> Self {
        let weights = build_vertical_weights(ch.sigma_v_lines);
        let n_sub = SUB_ROWS * ACTIVE_H;
        let mut charge = vec![0.0; n_sub];
        let mut last_t = vec![-DARK_SETTLE_S; n_sub];
        let has_state = needs_element_state(ch);
        let mut traps = if has_state { vec![[0.0; STATE_LEN]; n_sub] } else { Vec::new() };
        let mut raw = vec![0.0; ACTIVE_H];
        let dark = |_: usize, _: usize, _: usize, _: f64| 0.0;
        let settle_frames = (DARK_SETTLE_S * FRAME_RATE).ceil() as i64;
        for frame in -settle_frames..0 {
            scan_column(
                0,
                ch,
                &UniformPasses { weights: &weights, x: ACTIVE_W / 2 },
                frame,
                ACTIVE_W / 2,
                0,
                n_sub,
                None,
                &mut charge,
                &mut last_t,
                &mut traps,
                &mut raw,
                &dark,
                true,
            );
        }
        let settled_charge = charge.clone();
        let settled_last_t = last_t.clone();
        Self {
            ch: *ch,
            weights,
            charge,
            last_t,
            traps,
            settled_charge,
            settled_last_t,
            dark: raw,
            frame: 0,
            beam_on: true,
            held_steps: 0,
        }
    }

    /// Index of the next frame to be read.
    pub fn frame(&self) -> i64 {
        self.frame
    }

    pub fn set_beam_on(&mut self, on: bool) {
        self.beam_on = on;
    }

    /// State of sub-row `k` (`0..SUB_ROWS * ACTIVE_H`) as last left by a read or idle.
    pub fn element(&self, k: usize) -> ElementState {
        let st = self.traps.get(k).copied().unwrap_or([0.0; STATE_LEN]);
        let mut traps_nas = [0.0; MAX_TRAPS];
        traps_nas.copy_from_slice(&st[..MAX_TRAPS]);
        ElementState {
            charge_nas: self.charge[k],
            traps_nas,
            held_nas: st[HELD_IDX],
            burn: st[BURN_IDX],
        }
    }

    /// Sub-steps taken by this column's reads in the pieces that leave the merged rule, since the
    /// dark settle.
    pub fn held_steps(&self) -> u64 {
        self.held_steps
    }

    /// Lens capped, beam as set, for `seconds` (rounded to whole frames) without reading.
    pub fn idle(&mut self, seconds: f64) {
        let n = (seconds * FRAME_RATE).round().max(0.0) as i64;
        if n <= 0 {
            return;
        }
        let dt = n as f64 / FRAME_RATE;
        if self.beam_on {
            if !self.traps.is_empty() {
                idle_slow_states_beam_on(&self.ch, dt, None, 0, 0.0, &mut self.traps);
            }
            self.frame += n;
            let t_shift = self.frame as f64 / FRAME_RATE;
            self.charge.copy_from_slice(&self.settled_charge);
            for (dst, &t0) in self.last_t.iter_mut().zip(&self.settled_last_t) {
                *dst = t0 + t_shift;
            }
        } else {
            idle_dark_column(
                &self.ch,
                dt,
                None,
                0,
                0.0,
                &mut self.charge,
                &mut self.last_t,
                &mut self.traps,
            );
            self.frame += n;
        }
    }

    /// Read the next frame with photocurrent `photo(r, t)`, nA, the light of frame row `r`
    /// (each sub-row takes its row's light, as `CameraTube`), held within each field's interval.
    pub fn read_rows(&mut self, photo: &dyn Fn(usize, f64) -> f64) -> Vec<f64> {
        let mut raw = vec![0.0; ACTIVE_H];
        let current = |_: usize, r: usize, _: usize, t: f64| photo(r, t);
        let steps_before = held_steps_now();
        scan_column(
            0,
            &self.ch,
            &UniformPasses { weights: &self.weights, x: ACTIVE_W / 2 },
            self.frame,
            ACTIVE_W / 2,
            0,
            SUB_ROWS * ACTIVE_H,
            None,
            &mut self.charge,
            &mut self.last_t,
            &mut self.traps,
            &mut raw,
            &current,
            self.beam_on,
        );
        self.held_steps += held_steps_now() - steps_before;
        self.frame += 1;
        raw
    }

    /// Scan one frame with a uniform photocurrent over the target, returning each row's signal in nA.
    pub fn read(&mut self, photo: &dyn Fn(f64) -> f64) -> Vec<f64> {
        self.read_rows(&|_, t| photo(t))
    }
}

#[allow(clippy::too_many_arguments)]
fn scan_channel_dispatch<F>(
    ch_idx: usize,
    ch: &TubeChannel,
    w_ch: &[RowWeights],
    geom_scan: Option<&GeomScan>,
    shading: &Shading,
    frame: i64,
    q_all: &mut [f64],
    t_all: &mut [f64],
    traps_all: &mut [[f64; STATE_LEN]],
    raw_all: &mut [f64],
    photo: &F,
    beam_on: bool,
) where
    F: Fn(usize, usize, usize, f64) -> f64 + Sync + ?Sized,
{
    if ch.bending > 0.0 && beam_on {
        scan_channel_row_by_row(
            ch_idx,
            ch,
            w_ch,
            geom_scan,
            shading,
            frame,
            q_all,
            t_all,
            traps_all,
            raw_all,
            photo,
        );
        return;
    }

    if let Some(gs) = geom_scan {
        let h_sub_tgt = gs.h_sub_tgt;
        let pad_x = gs.pad_x;
        let pad_y = gs.pad_y;
        let use_black = *shading != Shading::NONE;
        if traps_all.is_empty() {
            q_all
                .par_chunks_mut(h_sub_tgt)
                .zip(t_all.par_chunks_mut(h_sub_tgt))
                .zip(raw_all.par_chunks_mut(ACTIVE_H))
                .enumerate()
                .for_each(|(jx, ((q_col, t_col), raw_col))| {
                    let j_signed = jx as isize - pad_x as isize;
                    let x_pic = j_signed.clamp(0, (ACTIVE_W - 1) as isize) as usize;
                    let black_shade = use_black.then(|| {
                        let x_elem =
                            ((j_signed as f64 + 0.5) / ACTIVE_W as f64 - 0.5) * TARGET_ASPECT;
                        (shading, x_elem)
                    });
                    scan_column(
                        ch_idx,
                        ch,
                        &GeomColPasses { col: &gs.cols[jx] },
                        frame,
                        x_pic,
                        pad_y,
                        h_sub_tgt,
                        black_shade,
                        q_col,
                        t_col,
                        &mut [],
                        raw_col,
                        photo,
                        beam_on,
                    );
                });
        } else {
            q_all
                .par_chunks_mut(h_sub_tgt)
                .zip(t_all.par_chunks_mut(h_sub_tgt))
                .zip(traps_all.par_chunks_mut(h_sub_tgt))
                .zip(raw_all.par_chunks_mut(ACTIVE_H))
                .enumerate()
                .for_each(|(jx, (((q_col, t_col), trap_col), raw_col))| {
                    let j_signed = jx as isize - pad_x as isize;
                    let x_pic = j_signed.clamp(0, (ACTIVE_W - 1) as isize) as usize;
                    let black_shade = use_black.then(|| {
                        let x_elem =
                            ((j_signed as f64 + 0.5) / ACTIVE_W as f64 - 0.5) * TARGET_ASPECT;
                        (shading, x_elem)
                    });
                    scan_column(
                        ch_idx,
                        ch,
                        &GeomColPasses { col: &gs.cols[jx] },
                        frame,
                        x_pic,
                        pad_y,
                        h_sub_tgt,
                        black_shade,
                        q_col,
                        t_col,
                        trap_col,
                        raw_col,
                        photo,
                        beam_on,
                    );
                });
        }
        return;
    }

    let n_sub = SUB_ROWS * ACTIVE_H;
    if traps_all.is_empty() {
        q_all
            .par_chunks_mut(n_sub)
            .zip(t_all.par_chunks_mut(n_sub))
            .zip(raw_all.par_chunks_mut(ACTIVE_H))
            .enumerate()
            .for_each(|(x, ((q_col, t_col), raw_col))| {
                scan_column(
                    ch_idx,
                    ch,
                    &UniformPasses { weights: w_ch, x },
                    frame,
                    x,
                    0,
                    n_sub,
                    None,
                    q_col,
                    t_col,
                    &mut [],
                    raw_col,
                    photo,
                    beam_on,
                );
            });
    } else {
        q_all
            .par_chunks_mut(n_sub)
            .zip(t_all.par_chunks_mut(n_sub))
            .zip(traps_all.par_chunks_mut(n_sub))
            .zip(raw_all.par_chunks_mut(ACTIVE_H))
            .enumerate()
            .for_each(|(x, (((q_col, t_col), trap_col), raw_col))| {
                scan_column(
                    ch_idx,
                    ch,
                    &UniformPasses { weights: w_ch, x },
                    frame,
                    x,
                    0,
                    n_sub,
                    None,
                    q_col,
                    t_col,
                    trap_col,
                    raw_col,
                    photo,
                    beam_on,
                );
            });
    }
}

/// Subtract dark current and scale slot `STRIPE_SLOT` to the decoder scale where white averages `STRIPE_WHITE_LIGHT`.
fn clamp_and_scale(sig: &TargetSignal, black: f64, peak_na: f64) -> Vec<Vec<f32>> {
    let scale = STRIPE_WHITE_LIGHT / peak_na;
    sig.rows
        .iter()
        .map(|chans| {
            chans[STRIPE_SLOT]
                .iter()
                .map(|&i| ((i as f64 - black) * scale) as f32)
                .collect()
        })
        .collect()
}

/// Mean of each decoded channel over the middle half of the rows and of the line.
fn centre_mean(rows: &[[Vec<f32>; 3]]) -> [f64; 3] {
    let mut sum = [0.0; 3];
    let mut n = 0.0;
    for row in &rows[ACTIVE_H / 4..3 * ACTIVE_H / 4] {
        for x in ACTIVE_W / 4..3 * ACTIVE_W / 4 {
            for k in 0..3 {
                sum[k] += row[k][x] as f64;
            }
            n += 1.0;
        }
    }
    sum.map(|s| s / n)
}

/// Align the stripe camera on a white card and a capped lens without modifying `base`.
fn align_stripe(base: &CameraTube) -> (StripeAlignment, StripeDecoder) {
    let mut card = base.clone();
    card.set_iris_stops(0.0);
    let mut white_frame = Frame::new(ACTIVE_W, ACTIVE_H);
    white_frame.rgb.fill(255);
    let mut white_sig = card.read(&white_frame, &white_frame);
    for _ in 1..ALIGN_FRAMES {
        white_sig = card.read(&white_frame, &white_frame);
    }
    card.head_amp.amplify(&mut white_sig, ALIGN_WHITE_FRAME);

    let dark_val = base.black[STRIPE_SLOT] as f32;
    let mut capped_sig = TargetSignal {
        rows: (0..ACTIVE_H)
            .map(|_| {
                let mut chans = [Vec::new(), Vec::new(), Vec::new()];
                chans[STRIPE_SLOT] = vec![dark_val; ACTIVE_W];
                chans
            })
            .collect(),
    };
    base.head_amp.amplify(&mut capped_sig, ALIGN_CAPPED_FRAME);

    let peak_na = base.channels[STRIPE_SLOT].peak_signal_na;
    let white_s = clamp_and_scale(&white_sig, base.black[STRIPE_SLOT], peak_na);
    let capped_s = clamp_and_scale(&capped_sig, base.black[STRIPE_SLOT], peak_na);

    let dec1 = StripeDecoder::new(1.0);
    let w1 = centre_mean(&dec1.decode(&white_s));
    let c1 = centre_mean(&dec1.decode(&capped_s));
    let a = 0.5 * ((w1[0] - c1[0]) + (w1[2] - c1[2]));
    let carrier_gain = 1.0 / a;

    let decoder = StripeDecoder::new(carrier_gain);
    let w = centre_mean(&decoder.decode(&white_s));
    let c = centre_mean(&decoder.decode(&capped_s));
    let floor = c;
    let white = [w[0] - c[0], w[1] - c[1], w[2] - c[2]];
    (StripeAlignment { carrier_gain, floor, white }, decoder)
}

/// The pickup tubes of the camera with the charge and last-read time on every sub-element of
/// their targets.
#[derive(Clone)]
pub struct CameraTube {
    layout: Layout,
    spectral: bool,
    taking: [[f64; 3]; 3],
    geom: GeometrySpec,
    optics: Option<OpticsPlan>,
    has_white_shading: bool,
    geom_scans: [Option<GeomScan>; 3],
    channels: [TubeChannel; 3],
    /// Signal of a dark target at equilibrium, nA: the level the camera clamps to black.
    black: [f64; 3],
    /// Linear scene light (exposure included) for each 8-bit input level.
    linear_light: [f64; 256],
    /// Photo current for each input level, nA, per channel: the iris, the inverse gamma
    /// corrector and the target's transfer exponent, evaluated once for the 256 levels.
    photo: [[f64; 256]; 3],
    /// Precomputed vertical beam weights per channel and active row.
    weights: [Vec<RowWeights>; 3],
    /// Precomputed horizontal beam spot kernel per channel: `(radius, kernel)`.
    horiz: [(usize, Vec<f64>); 3],
    /// Stored charge per sub-element in nA s, column-major (`x * stride + k`).
    charge: [Vec<f64>; 3],
    /// Last-landed timestamp per sub-element in seconds, column-major.
    last_t: [Vec<f64>; 3],
    /// Slow states (trap occupancies, held charge, burn) per sub-element, column-major; empty when
    /// the channel needs none.
    traps: [Vec<[f64; STATE_LEN]>; 3],
    /// Dark-settled column charge and timestamps per channel, used to reset target after beam-on `idle`.
    settled_col_q: [Vec<f64>; 3],
    settled_col_t: [Vec<f64>; 3],
    /// Pre-spot signal scratch per channel, column-major (`x * ACTIVE_H + r`).
    raw_cols: [Vec<f64>; 3],
    /// Held photocurrent map of the previous frame's field 1 (`ACTIVE_H x ACTIVE_W x 3` for
    /// `ThreeTube`, `ACTIVE_H x ACTIVE_W` for `Stripe`), nA. Empty before the first `read`.
    prev_f1_photo: Vec<f64>,
    frame: i64,
    beam_on: bool,
    /// Frame index at which a pending switch-on turns the beam on.
    beam_on_at: Option<i64>,
    head_amp: HeadAmplifier,
    processor: VideoProcessor,
    alignment: Option<StripeAlignment>,
    decoder: Option<StripeDecoder>,
}

impl CameraTube {
    /// The stage with an explicit geometry spec and deterministic noise `seed`, or `None` when
    /// the camera has no tube.
    pub fn seeded_geometry(
        params: &CameraParams,
        seed: u64,
        geometry: &GeometrySpec,
    ) -> Option<Self> {
        let mut channels = params.tube.channels_at(params.beam_ratio, params.temperature_c)?;
        let materials = params.tube.materials()?;
        let bending = params.blooming.max(0.0);
        let cold_offset_c = params.cold_start_c.map_or(0.0, |c| c - params.temperature_c);
        for ch in &mut channels {
            ch.bending = bending;
            ch.cold_offset_c = cold_offset_c;
        }
        if params.layout == Layout::Stripe {
            channels = [channels[STRIPE_SLOT]; 3];
        }
        let mut geom = *geometry;
        if params.layout == Layout::Stripe {
            geom.registration = [Registration::NONE; 3];
        }
        let has_geom = geom != GeometrySpec::NONE;
        let optics = if has_geom { OpticsPlan::new(&geom) } else { None };
        let has_white_shading = has_geom && geom.white_fall.iter().any(|&f| f != 0.0);

        let spectral = params.spectral && params.layout == Layout::ThreeTube;
        let taking = if spectral { taking_matrix(materials) } else { IDENTITY_MATRIX };
        let exposure = params.iris_stops.exp2();
        let linear_light: [f64; 256] =
            std::array::from_fn(|v| oetf_inverse(v as f64 / 255.0) * exposure);
        let n_sub = SUB_ROWS * ACTIVE_H;

        let (weights, horiz) = match params.layout {
            Layout::ThreeTube => (
                std::array::from_fn(|ch| build_vertical_weights(channels[ch].sigma_v_lines)),
                std::array::from_fn(|ch| build_horizontal_kernel(channels[ch].sigma_h_s)),
            ),
            Layout::Stripe => {
                let mut weights: [Vec<RowWeights>; 3] = [Vec::new(), Vec::new(), Vec::new()];
                let mut horiz: [(usize, Vec<f64>); 3] =
                    [(0, Vec::new()), (0, Vec::new()), (0, Vec::new())];
                weights[STRIPE_SLOT] = build_vertical_weights(channels[STRIPE_SLOT].sigma_v_lines);
                horiz[STRIPE_SLOT] = build_horizontal_kernel(channels[STRIPE_SLOT].sigma_h_s);
                (weights, horiz)
            }
        };

        let geom_scans: [Option<GeomScan>; 3] = if !has_geom {
            [None, None, None]
        } else {
            match params.layout {
                Layout::ThreeTube => {
                    std::array::from_fn(|ch| Some(build_geom_scan(ch, &channels[ch], &geom)))
                }
                Layout::Stripe => [
                    None,
                    Some(build_geom_scan(STRIPE_SLOT, &channels[STRIPE_SLOT], &geom)),
                    None,
                ],
            }
        };

        let mut black = [0.0; 3];
        let mut photo = [[0.0; 256]; 3];
        let mut charge: [Vec<f64>; 3] = [Vec::new(), Vec::new(), Vec::new()];
        let mut last_t: [Vec<f64>; 3] = [Vec::new(), Vec::new(), Vec::new()];
        let mut traps: [Vec<[f64; STATE_LEN]>; 3] = [Vec::new(), Vec::new(), Vec::new()];
        let mut settled_col_q: [Vec<f64>; 3] = [Vec::new(), Vec::new(), Vec::new()];
        let mut settled_col_t: [Vec<f64>; 3] = [Vec::new(), Vec::new(), Vec::new()];
        let mut raw_cols: [Vec<f64>; 3] = [Vec::new(), Vec::new(), Vec::new()];

        let init_slot = |ch: &TubeChannel,
                         gs_opt: Option<&GeomScan>,
                         charge_k: &mut Vec<f64>,
                         last_t_k: &mut Vec<f64>,
                         traps_k: &mut Vec<[f64; STATE_LEN]>,
                         settled_q_k: &mut Vec<f64>,
                         settled_t_k: &mut Vec<f64>,
                         raw_k: &mut Vec<f64>|
         -> f64 {
            let settled = UniformColumn::new(ch);
            let q_col = settled.charge;
            let t_col = settled.last_t;
            let tr_col = settled.traps;
            let dark = settled.dark.iter().sum::<f64>() / ACTIVE_H as f64;
            match gs_opt {
                None => {
                    *charge_k = vec![0.0; ACTIVE_W * n_sub];
                    *last_t_k = vec![0.0; ACTIVE_W * n_sub];
                    if !tr_col.is_empty() {
                        *traps_k = vec![[0.0; STATE_LEN]; ACTIVE_W * n_sub];
                    }
                    *raw_k = vec![0.0; ACTIVE_W * ACTIVE_H];
                    for x in 0..ACTIVE_W {
                        let off = x * n_sub;
                        charge_k[off..off + n_sub].copy_from_slice(&q_col);
                        let dx_s = (x as f64 - (ACTIVE_W / 2) as f64) / FS_BB;
                        for i in 0..n_sub {
                            last_t_k[off + i] = t_col[i] + dx_s;
                        }
                        if !tr_col.is_empty() {
                            traps_k[off..off + n_sub].copy_from_slice(&tr_col);
                        }
                    }
                }
                Some(gs) => {
                    let w_tgt = gs.w_tgt;
                    let h_sub_tgt = gs.h_sub_tgt;
                    let pad_y = gs.pad_y as isize;
                    *charge_k = vec![0.0; w_tgt * h_sub_tgt];
                    *last_t_k = vec![-DARK_SETTLE_S; w_tgt * h_sub_tgt];
                    if !tr_col.is_empty() {
                        *traps_k = vec![[0.0; STATE_LEN]; w_tgt * h_sub_tgt];
                        for jx in 0..w_tgt {
                            let off = jx * h_sub_tgt;
                            for ks in 0..h_sub_tgt {
                                let k_pic =
                                    (ks as isize - pad_y).clamp(0, n_sub as isize - 1) as usize;
                                traps_k[off + ks] = tr_col[k_pic];
                            }
                        }
                    }
                    *raw_k = vec![0.0; w_tgt * ACTIVE_H];
                    reset_geom_charge_and_time(ch, gs, 0, &q_col, charge_k, last_t_k);
                }
            }
            *settled_q_k = q_col;
            *settled_t_k = t_col;
            dark
        };

        match params.layout {
            Layout::ThreeTube => {
                for k in 0..3 {
                    black[k] = init_slot(
                        &channels[k],
                        geom_scans[k].as_ref(),
                        &mut charge[k],
                        &mut last_t[k],
                        &mut traps[k],
                        &mut settled_col_q[k],
                        &mut settled_col_t[k],
                        &mut raw_cols[k],
                    );
                }
            }
            Layout::Stripe => {
                let dark = init_slot(
                    &channels[STRIPE_SLOT],
                    geom_scans[STRIPE_SLOT].as_ref(),
                    &mut charge[STRIPE_SLOT],
                    &mut last_t[STRIPE_SLOT],
                    &mut traps[STRIPE_SLOT],
                    &mut settled_col_q[STRIPE_SLOT],
                    &mut settled_col_t[STRIPE_SLOT],
                    &mut raw_cols[STRIPE_SLOT],
                );
                black = [dark; 3];
            }
        }
        for k in 0..3 {
            for (v, p) in photo[k].iter_mut().enumerate() {
                *p = photo_current(&channels[k], linear_light[v]);
            }
        }

        let prev_f1_photo = Vec::new();
        let spec = params.processing.spec();
        let head_amp = HeadAmplifier::new(&spec.head_amp, seed);
        let mut processor = VideoProcessor::new(params.processing, &channels, black);
        if spectral && spec.matrix {
            processor = processor.with_matrix(colour_matrix(materials));
        }

        let mut cam = Self {
            layout: params.layout,
            spectral,
            taking,
            geom,
            optics,
            has_white_shading,
            geom_scans,
            channels,
            black,
            linear_light,
            photo,
            weights,
            horiz,
            charge,
            last_t,
            traps,
            settled_col_q,
            settled_col_t,
            raw_cols,
            prev_f1_photo,
            frame: 0,
            beam_on: true,
            beam_on_at: None,
            head_amp,
            processor,
            alignment: None,
            decoder: None,
        };

        let flare_reading = if spec.corrections.flare && cam.optics.is_some() {
            cam.measure_flare_reading()
        } else {
            [0.0; 3]
        };
        if has_geom && params.layout == Layout::ThreeTube {
            cam.processor = cam.processor.with_geometry(&cam.geom, flare_reading);
        }
        if params.layout == Layout::Stripe {
            let (alignment, decoder) = align_stripe(&cam);
            let mut proc = VideoProcessor::with_levels(
                params.processing,
                &cam.channels,
                alignment.floor,
                alignment.white,
            );
            if has_geom {
                let mut proc_geom = cam.geom;
                let scale = STRIPE_WHITE_LIGHT / cam.channels[STRIPE_SLOT].peak_signal_na;
                for r_na in &mut proc_geom.black_reference_na {
                    *r_na *= scale;
                }
                proc = proc.with_geometry(&proc_geom, flare_reading);
            }
            cam.processor = proc;
            cam.alignment = Some(alignment);
            cam.decoder = Some(decoder);
        }
        // The switch-on sequence: the lens cap, then the beam held off with the lens open.
        let idle_s = if params.idle_s.is_finite() { params.idle_s.max(0.0) } else { 0.0 };
        if idle_s > 0.0 {
            cam.idle(idle_s);
        }
        let off_s = if params.beam_off_s.is_finite() { params.beam_off_s.max(0.0) } else { 0.0 };
        let off_frames = (off_s * FRAME_RATE).round() as i64;
        if off_frames > 0 {
            cam.beam_on = false;
            cam.beam_on_at = Some(cam.frame + off_frames);
        }
        Some(cam)
    }

    /// Measure the centre black-square chart photocurrent as a fraction of peak white through the
    /// camera's own optics at unit exposure (`L = 1`, `iris_stops = 0`).
    fn measure_flare_reading(&self) -> [f64; 3] {
        let mut planes: [Vec<f64>; 3] = std::array::from_fn(|_| vec![1.0; ACTIVE_W * ACTIVE_H]);
        let half_sq = 0.5 * BLACK_SQUARE_PH;
        for r in 0..ACTIVE_H {
            for x in 0..ACTIVE_W {
                let (px, py) = picture_position_ph(r, x as f64);
                if px.abs() <= half_sq && py.abs() <= half_sq {
                    let idx = r * ACTIVE_W + x;
                    planes[0][idx] = 0.0;
                    planes[1][idx] = 0.0;
                    planes[2][idx] = 0.0;
                }
            }
        }
        if self.layout == Layout::Stripe {
            let mono = self.stripe_photo_from_planes(planes);
            let r = ACTIVE_H / 2;
            let cx = ACTIVE_W / 2;
            let mean: f64 = ((cx - 1)..=(cx + 1)).map(|x| mono[r * ACTIVE_W + x]).sum::<f64>() / 3.0;
            return [mean / self.channels[STRIPE_SLOT].peak_signal_na; 3];
        }
        let photo_vec = self.three_tube_photo_from_planes(planes);
        let idx = (ACTIVE_H / 2) * ACTIVE_W + (ACTIVE_W / 2);
        std::array::from_fn(|ch| photo_vec[idx * 3 + ch] / self.channels[ch].peak_signal_na)
    }

    /// The stage for `params` with deterministic noise `seed`, or `None` when the camera has no
    /// tube. Every sub-element starts at dark equilibrium, found by scanning a dark target until
    /// the reading settles.
    pub fn seeded(params: &CameraParams, seed: u64) -> Option<Self> {
        let geom = if params.geometry {
            params.processing.geometry(params.tube)
        } else {
            GeometrySpec::NONE
        };
        Self::seeded_geometry(params, seed, &geom)
    }

    /// The stage for `params` with `seed = 0`, or `None` when the camera has no tube.
    pub fn new(params: &CameraParams) -> Option<Self> {
        Self::seeded(params, 0)
    }

    /// The camera's geometry specification (`GeometrySpec::NONE` when off).
    pub fn geometry(&self) -> &GeometrySpec {
        &self.geom
    }

    /// The processing amplifier of this camera.
    pub fn processor(&self) -> &VideoProcessor {
        &self.processor
    }

    /// How the camera's tubes see colour.
    pub fn layout(&self) -> Layout {
        self.layout
    }

    /// The stripe camera's alignment, or `None` with three tubes.
    pub fn stripe_alignment(&self) -> Option<StripeAlignment> {
        self.alignment
    }

    /// Settled dark-equilibrium reading per channel, nA (the camera's black clamp).
    pub fn black(&self) -> [f64; 3] {
        self.black
    }

    /// Switch the electron beam on or off, cancelling any pending switch-on from `beam_off_s`.
    pub fn set_beam_on(&mut self, on: bool) {
        self.beam_on = on;
        self.beam_on_at = None;
    }

    /// Lens capped, beam as set, for `seconds` (rounded to whole frames) without reading.
    pub fn idle(&mut self, seconds: f64) {
        let n = (seconds * FRAME_RATE).round().max(0.0) as i64;
        if n <= 0 {
            return;
        }
        let dt = n as f64 / FRAME_RATE;
        let active_slots: &[usize] = match self.layout {
            Layout::ThreeTube => &[0, 1, 2],
            Layout::Stripe => &[STRIPE_SLOT],
        };
        let n_sub = SUB_ROWS * ACTIVE_H;
        let shading = &self.geom.black_shading;
        let use_black = *shading != Shading::NONE;

        if self.beam_on {
            self.frame += n;
            let frame = self.frame;
            let t_shift = frame as f64 / FRAME_RATE;
            for &slot in active_slots {
                let ch = &self.channels[slot];
                let gs_opt = self.geom_scans[slot].as_ref();
                let sq = &self.settled_col_q[slot];
                let st = &self.settled_col_t[slot];
                match gs_opt {
                    None => {
                        if !self.traps[slot].is_empty() {
                            self.traps[slot]
                                .par_chunks_mut(n_sub)
                                .for_each(|state_col| {
                                    idle_slow_states_beam_on(ch, dt, None, 0, 0.0, state_col);
                                });
                        }
                        self.charge[slot]
                            .par_chunks_mut(n_sub)
                            .zip(self.last_t[slot].par_chunks_mut(n_sub))
                            .enumerate()
                            .for_each(|(x, (q_col, t_col))| {
                                q_col.copy_from_slice(sq);
                                let dx_s = (x as f64 - (ACTIVE_W / 2) as f64) / FS_BB;
                                for i in 0..n_sub {
                                    t_col[i] = st[i] + dx_s + t_shift;
                                }
                            });
                    }
                    Some(gs) => {
                        let h_sub_tgt = gs.h_sub_tgt;
                        let pad_x = gs.pad_x;
                        let pad_y = gs.pad_y;
                        if !self.traps[slot].is_empty() {
                            let sh_opt = use_black.then_some(shading);
                            self.traps[slot]
                                .par_chunks_mut(h_sub_tgt)
                                .enumerate()
                                .for_each(|(jx, state_col)| {
                                    let j_signed = jx as isize - pad_x as isize;
                                    let x_elem = ((j_signed as f64 + 0.5) / ACTIVE_W as f64 - 0.5)
                                        * TARGET_ASPECT;
                                    idle_slow_states_beam_on(
                                        ch, dt, sh_opt, pad_y, x_elem, state_col,
                                    );
                                });
                        }
                        reset_geom_charge_and_time(
                            ch,
                            gs,
                            frame,
                            sq,
                            &mut self.charge[slot],
                            &mut self.last_t[slot],
                        );
                    }
                }
            }
        } else {
            for &slot in active_slots {
                let ch = &self.channels[slot];
                let gs_opt = self.geom_scans[slot].as_ref();
                match gs_opt {
                    None => {
                        if self.traps[slot].is_empty() {
                            self.charge[slot]
                                .par_chunks_mut(n_sub)
                                .zip(self.last_t[slot].par_chunks_mut(n_sub))
                                .for_each(|(q_col, t_col)| {
                                    idle_dark_column(ch, dt, None, 0, 0.0, q_col, t_col, &mut []);
                                });
                        } else {
                            self.charge[slot]
                                .par_chunks_mut(n_sub)
                                .zip(self.last_t[slot].par_chunks_mut(n_sub))
                                .zip(self.traps[slot].par_chunks_mut(n_sub))
                                .for_each(|((q_col, t_col), state_col)| {
                                    idle_dark_column(
                                        ch, dt, None, 0, 0.0, q_col, t_col, state_col,
                                    );
                                });
                        }
                    }
                    Some(gs) => {
                        let h_sub_tgt = gs.h_sub_tgt;
                        let pad_x = gs.pad_x;
                        let pad_y = gs.pad_y;
                        let sh_opt = use_black.then_some(shading);
                        if self.traps[slot].is_empty() {
                            self.charge[slot]
                                .par_chunks_mut(h_sub_tgt)
                                .zip(self.last_t[slot].par_chunks_mut(h_sub_tgt))
                                .enumerate()
                                .for_each(|(jx, (q_col, t_col))| {
                                    let j_signed = jx as isize - pad_x as isize;
                                    let x_elem = ((j_signed as f64 + 0.5) / ACTIVE_W as f64 - 0.5)
                                        * TARGET_ASPECT;
                                    idle_dark_column(
                                        ch, dt, sh_opt, pad_y, x_elem, q_col, t_col, &mut [],
                                    );
                                });
                        } else {
                            self.charge[slot]
                                .par_chunks_mut(h_sub_tgt)
                                .zip(self.last_t[slot].par_chunks_mut(h_sub_tgt))
                                .zip(self.traps[slot].par_chunks_mut(h_sub_tgt))
                                .enumerate()
                                .for_each(|(jx, ((q_col, t_col), state_col))| {
                                    let j_signed = jx as isize - pad_x as isize;
                                    let x_elem = ((j_signed as f64 + 0.5) / ACTIVE_W as f64 - 0.5)
                                        * TARGET_ASPECT;
                                    idle_dark_column(
                                        ch, dt, sh_opt, pad_y, x_elem, q_col, t_col, state_col,
                                    );
                                });
                        }
                    }
                }
            }
            self.frame += n;
        }
        if !self.prev_f1_photo.is_empty() {
            self.prev_f1_photo.fill(0.0);
        }
    }

    /// Update the iris exposure setting for subsequent frames.
    pub fn set_iris_stops(&mut self, iris_stops: f64) {
        let exposure = iris_stops.exp2();
        for (v, l) in self.linear_light.iter_mut().enumerate() {
            *l = oetf_inverse(v as f64 / 255.0) * exposure;
        }
        for k in 0..3 {
            for (v, p) in self.photo[k].iter_mut().enumerate() {
                *p = photo_current(&self.channels[k], self.linear_light[v]);
            }
        }
    }

    /// The channels as built (operating point, `K`, spot widths).
    pub fn channels(&self) -> &[TubeChannel; 3] {
        &self.channels
    }

    /// The taking matrix in use: `taking_matrix(kind.materials())` with `spectral`, else the identity.
    pub fn taking(&self) -> [[f64; 3]; 3] {
        self.taking
    }

    /// The processor's colour matrix: `colour_matrix(..)` with `spectral` and a profile that has one,
    /// else the identity.
    pub fn colour_matrix(&self) -> [[f64; 3]; 3] {
        self.processor.matrix()
    }

    /// The photocurrent the picture `f` drives on each target element, nA, indexed
    /// `(r * ACTIVE_W + x) * 3 + ch`: the light through the taking, then the target's sensitivity
    /// and transfer. What `read` integrates for a picture held over a field.
    pub fn photo_currents(&self, f: &Frame) -> Vec<f64> {
        if self.layout == Layout::Stripe {
            let mono = self.stripe_frame_photo(f);
            let mut out = vec![0.0; ACTIVE_W * ACTIVE_H * 3];
            for (i, &p) in mono.iter().enumerate() {
                out[i * 3 + STRIPE_SLOT] = p;
            }
            return out;
        }
        self.frame_photo(f)
    }

    /// Read one frame from photocurrents: `photo(ch, r, x, t)` is the target-equivalent photo
    /// current density, nA, at element (row r, sample x) at time t. It is constant over each
    /// field's interval (the pictures are held), so the engine integrates it in pieces split at
    /// the field boundaries, evaluating it once inside each piece. Advances the frame counter.
    pub fn read_currents(
        &mut self,
        photo: &(dyn Fn(usize, usize, usize, f64) -> f64 + Sync),
    ) -> TargetSignal {
        self.read_currents_impl(photo)
    }

    fn read_currents_impl<F>(&mut self, photo: &F) -> TargetSignal
    where
        F: Fn(usize, usize, usize, f64) -> f64 + Sync + ?Sized,
    {
        let frame = self.frame;
        let beam_on = self.beam_on;
        let channels = &self.channels;
        let weights = &self.weights;
        let horiz = &self.horiz;
        let geom = &self.geom;
        let geom_scans = &self.geom_scans;
        let raw_cols = &mut self.raw_cols;

        if self.layout == Layout::Stripe {
            let gs_opt = geom_scans[STRIPE_SLOT].as_ref();
            scan_channel_dispatch(
                STRIPE_SLOT,
                &channels[STRIPE_SLOT],
                &weights[STRIPE_SLOT],
                gs_opt,
                &geom.black_shading,
                frame,
                &mut self.charge[STRIPE_SLOT],
                &mut self.last_t[STRIPE_SLOT],
                &mut self.traps[STRIPE_SLOT],
                &mut raw_cols[STRIPE_SLOT],
                photo,
                beam_on,
            );
            let (radius, ref kernel) = horiz[STRIPE_SLOT];
            let raw_ch = &raw_cols[STRIPE_SLOT];
            let w_tgt = gs_opt.map_or(ACTIVE_W, |g| g.w_tgt);
            let rows: Vec<[Vec<f32>; 3]> = (0..ACTIVE_H)
                .into_par_iter()
                .map_init(
                    || vec![0.0; w_tgt],
                    |scratch, r| {
                        let mut out = vec![0.0f32; ACTIVE_W];
                        if beam_on {
                            for x in 0..w_tgt {
                                scratch[x] = raw_ch[x * ACTIVE_H + r];
                            }
                            match gs_opt {
                                Some(gs) => convolve_horizontal_geom(
                                    scratch,
                                    r,
                                    gs.pad_x,
                                    STRIPE_SLOT,
                                    channels[STRIPE_SLOT].sigma_h_s,
                                    geom,
                                    &mut out,
                                ),
                                None => convolve_horizontal(scratch, radius, kernel, &mut out),
                            }
                        }
                        let mut chans = [Vec::new(), Vec::new(), Vec::new()];
                        chans[STRIPE_SLOT] = out;
                        chans
                    },
                )
                .collect();
            self.frame += 1;
            return TargetSignal { rows };
        }

        self.charge
            .par_iter_mut()
            .zip(self.last_t.par_iter_mut())
            .zip(self.traps.par_iter_mut())
            .zip(raw_cols.par_iter_mut())
            .enumerate()
            .for_each(|(ch_idx, (((q_all, t_all), traps_all), raw_all))| {
                scan_channel_dispatch(
                    ch_idx,
                    &channels[ch_idx],
                    &weights[ch_idx],
                    geom_scans[ch_idx].as_ref(),
                    &geom.black_shading,
                    frame,
                    q_all,
                    t_all,
                    traps_all,
                    raw_all,
                    photo,
                    beam_on,
                );
            });

        let scratch_len = geom_scans
            .iter()
            .map(|o| o.as_ref().map_or(ACTIVE_W, |g| g.w_tgt))
            .max()
            .unwrap_or(ACTIVE_W);
        let rows: Vec<[Vec<f32>; 3]> = (0..ACTIVE_H)
            .into_par_iter()
            .map_init(
                || vec![0.0; scratch_len],
                |scratch, r| {
                    std::array::from_fn(|ch| {
                        let mut out = vec![0.0f32; ACTIVE_W];
                        if !beam_on {
                            return out;
                        }
                        let raw_ch = &raw_cols[ch];
                        match geom_scans[ch].as_ref() {
                            Some(gs) => {
                                let w_tgt = gs.w_tgt;
                                for x in 0..w_tgt {
                                    scratch[x] = raw_ch[x * ACTIVE_H + r];
                                }
                                convolve_horizontal_geom(
                                    &scratch[..w_tgt],
                                    r,
                                    gs.pad_x,
                                    ch,
                                    channels[ch].sigma_h_s,
                                    geom,
                                    &mut out,
                                );
                            }
                            None => {
                                for x in 0..ACTIVE_W {
                                    scratch[x] = raw_ch[x * ACTIVE_H + r];
                                }
                                let (radius, ref kernel) = horiz[ch];
                                convolve_horizontal(&scratch[..ACTIVE_W], radius, kernel, &mut out);
                            }
                        }
                        out
                    })
                },
            )
            .collect();

        self.frame += 1;
        TargetSignal { rows }
    }

    /// Apply veil, halation, white shading and tube transfer to 3-channel linear light planes.
    fn three_tube_photo_from_planes(&self, mut planes: [Vec<f64>; 3]) -> Vec<f64> {
        if let Some(ref optics) = self.optics {
            optics.apply_three(&mut planes, ACTIVE_W, ACTIVE_H);
        }
        let mut out = vec![0.0; ACTIVE_W * ACTIVE_H * 3];
        let row_len = ACTIVE_W * 3;
        let chs = &self.channels;
        let fall = self.geom.white_fall;
        let has_white = self.has_white_shading;
        out.par_chunks_mut(row_len).enumerate().for_each(|(r, dst)| {
            let row_off = r * ACTIVE_W;
            for x in 0..ACTIVE_W {
                let idx = row_off + x;
                let (px, py) = if has_white { picture_position_ph(r, x as f64) } else { (0.0, 0.0) };
                for ch in 0..3 {
                    let mut l = planes[ch][idx];
                    if has_white && fall[ch] != 0.0 {
                        l *= white_shading_factor(fall[ch], px, py);
                    }
                    dst[x * 3 + ch] = photo_current(&chs[ch], l);
                }
            }
        });
        out
    }

    /// Convert an RGB frame into per-element photocurrents (nA) for three-tube mode.
    fn frame_photo(&self, frame: &Frame) -> Vec<f64> {
        let expected = ACTIVE_W * ACTIVE_H * 3;
        assert_eq!(frame.rgb.len(), expected, "the camera stage takes frames of the engine's active size");
        if self.optics.is_none() && !self.has_white_shading {
            let mut out = vec![0.0; expected];
            let row_len = ACTIVE_W * 3;
            if !self.spectral {
                let photo = &self.photo;
                out.par_chunks_mut(row_len)
                    .zip(frame.rgb.par_chunks(row_len))
                    .for_each(|(dst, src)| {
                        for (i, &v) in src.iter().enumerate() {
                            dst[i] = photo[i % 3][v as usize];
                        }
                    });
                return out;
            }
            let linear = &self.linear_light;
            let t = &self.taking;
            let chs = &self.channels;
            out.par_chunks_mut(row_len)
                .zip(frame.rgb.par_chunks(row_len))
                .for_each(|(dst, src)| {
                    for x in 0..ACTIVE_W {
                        let p = x * 3;
                        let l_in = [
                            linear[src[p] as usize],
                            linear[src[p + 1] as usize],
                            linear[src[p + 2] as usize],
                        ];
                        for ch in 0..3 {
                            let l_ch = t[ch][0] * l_in[0] + t[ch][1] * l_in[1] + t[ch][2] * l_in[2];
                            dst[p + ch] = photo_current(&chs[ch], l_ch);
                        }
                    }
                });
            return out;
        }
        let n_px = ACTIVE_W * ACTIVE_H;
        let linear = &self.linear_light;
        let mut planes: [Vec<f64>; 3] = std::array::from_fn(|_| Vec::with_capacity(n_px));
        if !self.spectral {
            for p in 0..n_px {
                let base = p * 3;
                for ch in 0..3 {
                    planes[ch].push(linear[frame.rgb[base + ch] as usize]);
                }
            }
        } else {
            let t = &self.taking;
            for p in 0..n_px {
                let base = p * 3;
                let l_in = [
                    linear[frame.rgb[base] as usize],
                    linear[frame.rgb[base + 1] as usize],
                    linear[frame.rgb[base + 2] as usize],
                ];
                for ch in 0..3 {
                    planes[ch].push(t[ch][0] * l_in[0] + t[ch][1] * l_in[1] + t[ch][2] * l_in[2]);
                }
            }
        }
        self.three_tube_photo_from_planes(planes)
    }

    /// Apply scene veil, optical low-pass, stripe filter, halation, white shading and transfer.
    fn stripe_photo_from_planes(&self, mut rgb_planes: [Vec<f64>; 3]) -> Vec<f64> {
        if let Some(ref optics) = self.optics {
            for plane in &mut rgb_planes {
                optics.apply_veil_one(STRIPE_SLOT, plane, ACTIVE_W, ACTIVE_H);
            }
        }
        let mut target_l = vec![0.0; ACTIVE_W * ACTIVE_H];
        target_l
            .par_chunks_mut(ACTIVE_W)
            .enumerate()
            .for_each(|(r, dst)| {
                let off = r * ACTIVE_W;
                let lp: [Vec<f64>; 3] = std::array::from_fn(|c| {
                    optical_lowpass(&rgb_planes[c][off..off + ACTIVE_W])
                });
                for x in 0..ACTIVE_W {
                    dst[x] = target_light(r, x, [lp[0][x], lp[1][x], lp[2][x]]) / STRIPE_WHITE_LIGHT;
                }
            });
        if let Some(ref optics) = self.optics {
            optics.apply_halation_one(STRIPE_SLOT, &mut target_l, ACTIVE_W, ACTIVE_H);
        }
        let ch = &self.channels[STRIPE_SLOT];
        let fall = self.geom.white_fall[STRIPE_SLOT];
        let has_white = self.has_white_shading && fall != 0.0;
        target_l
            .par_chunks_mut(ACTIVE_W)
            .enumerate()
            .for_each(|(r, row)| {
                for (x, v) in row.iter_mut().enumerate() {
                    let mut l = *v;
                    if has_white {
                        let (px, py) = picture_position_ph(r, x as f64);
                        l *= white_shading_factor(fall, px, py);
                    }
                    *v = photo_current(ch, l);
                }
            });
        target_l
    }

    /// Convert an RGB frame into per-element photocurrents (nA) through the optical low-pass and stripe filter.
    fn stripe_frame_photo(&self, frame: &Frame) -> Vec<f64> {
        let expected = ACTIVE_W * ACTIVE_H * 3;
        assert_eq!(frame.rgb.len(), expected, "the camera stage takes frames of the engine's active size");
        if self.optics.is_none() && !self.has_white_shading {
            let mut out = vec![0.0; ACTIVE_W * ACTIVE_H];
            let linear = &self.linear_light;
            let ch = &self.channels[STRIPE_SLOT];
            let row_rgb_len = ACTIVE_W * 3;
            out.par_chunks_mut(ACTIVE_W)
                .zip(frame.rgb.par_chunks(row_rgb_len))
                .enumerate()
                .for_each(|(r, (dst, src))| {
                    let lp: [Vec<f64>; 3] = std::array::from_fn(|c| {
                        let row: Vec<f64> =
                            (0..ACTIVE_W).map(|x| linear[src[x * 3 + c] as usize]).collect();
                        optical_lowpass(&row)
                    });
                    for x in 0..ACTIVE_W {
                        let light = target_light(r, x, [lp[0][x], lp[1][x], lp[2][x]]);
                        dst[x] = photo_current(ch, light / STRIPE_WHITE_LIGHT);
                    }
                });
            return out;
        }
        let n_px = ACTIVE_W * ACTIVE_H;
        let linear = &self.linear_light;
        let mut rgb_planes: [Vec<f64>; 3] = std::array::from_fn(|_| Vec::with_capacity(n_px));
        for p in 0..n_px {
            let base = p * 3;
            for c in 0..3 {
                rgb_planes[c].push(linear[frame.rgb[base + c] as usize]);
            }
        }
        self.stripe_photo_from_planes(rgb_planes)
    }

    /// Read one frame from pictures: field 0's interval sees `f0`, field 1's sees `f1`.
    pub fn read(&mut self, f0: &Frame, f1: &Frame) -> TargetSignal {
        let t_f0 = self.frame as f64 / FRAME_RATE;
        let t_f1 = (self.frame as f64 + 0.5) / FRAME_RATE;
        if self.layout == Layout::Stripe {
            let cur_f0 = self.stripe_frame_photo(f0);
            let cur_f1 = if std::ptr::eq(f0, f1) { None } else { Some(self.stripe_frame_photo(f1)) };
            let f1_ref: &[f64] = cur_f1.as_deref().unwrap_or(&cur_f0);
            let prev_f1 = std::mem::take(&mut self.prev_f1_photo);
            let sig = self.read_currents_impl(&|_ch, r, x, t| {
                let idx = r * ACTIVE_W + x;
                if t < t_f0 && !prev_f1.is_empty() {
                    prev_f1[idx]
                } else if t < t_f1 {
                    cur_f0[idx]
                } else {
                    f1_ref[idx]
                }
            });
            self.prev_f1_photo = cur_f1.unwrap_or(cur_f0);
            return sig;
        }
        let cur_f0 = self.frame_photo(f0);
        let cur_f1 = if std::ptr::eq(f0, f1) { None } else { Some(self.frame_photo(f1)) };
        let f1_ref: &[f64] = cur_f1.as_deref().unwrap_or(&cur_f0);
        let prev_f1 = std::mem::take(&mut self.prev_f1_photo);
        let sig = self.read_currents_impl(&|ch, r, x, t| {
            let idx = (r * ACTIVE_W + x) * 3 + ch;
            if t < t_f0 && !prev_f1.is_empty() {
                prev_f1[idx]
            } else if t < t_f1 {
                cur_f0[idx]
            } else {
                f1_ref[idx]
            }
        });
        self.prev_f1_photo = cur_f1.unwrap_or(cur_f0);
        sig
    }

    fn stripe_decode_raw(&mut self, f0: &Frame, f1: &Frame) -> Vec<[Vec<f32>; 3]> {
        let frame_idx = self.frame;
        let mut sig = self.read(f0, f1);
        self.head_amp.amplify(&mut sig, frame_idx);
        let s = clamp_and_scale(&sig, self.black[STRIPE_SLOT], self.channels[STRIPE_SLOT].peak_signal_na);
        self.decoder.as_ref().expect("stripe decoder").decode(&s)
    }

    /// The stripe camera's colour of one frame: read, head amplifier, clamp and scale, decoder, then
    /// `(v - floor) / white` per channel. That is linear light with white 1.0 (in the tube's own
    /// transfer for a tube that linearises), before linearise, knee, gamma, aperture correction and clip.
    /// Advances the frame as `video` does. `None` with three tubes, which then reads nothing.
    pub fn stripe_colour(&mut self, f0: &Frame, f1: &Frame) -> Option<Vec<[Vec<f32>; 3]>> {
        if self.layout != Layout::Stripe {
            return None;
        }
        let alignment = self.alignment?;
        let mut rows = self.stripe_decode_raw(f0, f1);
        for row in &mut rows {
            for k in 0..3 {
                let floor = alignment.floor[k];
                let white = alignment.white[k];
                for v in &mut row[k] {
                    *v = ((*v as f64 - floor) / white) as f32;
                }
            }
        }
        Some(rows)
    }

    /// Scan one frame from pictures (`f0` for field 0, `f1` for field 1), amplify and process
    /// the line signal through the camera electronics.
    pub fn video(&mut self, f0: &Frame, f1: &Frame) -> CameraVideo {
        if let Some(at) = self.beam_on_at {
            if self.frame >= at {
                self.beam_on = true;
                self.beam_on_at = None;
            }
        }
        if self.layout == Layout::Stripe {
            let rows = self.stripe_decode_raw(f0, f1);
            return self.processor.process(&TargetSignal { rows });
        }
        let frame_idx = self.frame;
        let mut sig = self.read(f0, f1);
        self.head_amp.amplify(&mut sig, frame_idx);
        self.processor.process(&sig)
    }

    /// The picture path: `read`, then the camera electronics, then the encoder.
    pub fn encode_fields(&mut self, f0: &Frame, f1: &Frame) -> Baseband {
        let vid = self.video(f0, f1);
        encode_from_rgb(|row, x| {
            let r = &vid.rows[row];
            [r[0][x], r[1][x], r[2][x]]
        })
    }

    /// Expose the targets to `frame` for both fields, scan them, and encode the continuous
    /// camera output signal directly to baseband Y / C without writing an intermediate `u8`
    /// video frame.
    pub fn encode_baseband(&mut self, frame: &Frame) -> Baseband {
        self.encode_fields(frame, frame)
    }

    /// Expose the targets to `frame` for both fields, scan them, and return what the
    /// camera outputs. The target state is kept for the next frame.
    pub fn process_frame(&mut self, frame: &Frame) -> Frame {
        let vid = self.video(frame, frame);
        let mut out = Frame::new(ACTIVE_W, ACTIVE_H);
        for (row, r) in vid.rows.iter().enumerate() {
            for x in 0..ACTIVE_W {
                let p = (row * ACTIVE_W + x) * 3;
                for k in 0..3 {
                    out.rgb[p + k] = (255.0f32 * r[k][x]).round().clamp(0.0, 255.0) as u8;
                }
            }
        }
        out
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn flat(v: u8) -> Frame {
        let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
        f.rgb.fill(v);
        f
    }

    #[test]
    fn the_default_camera_has_no_tube() {
        let p = CameraParams::default();
        assert_eq!(p.tube, TubeKind::Off);
        assert_eq!(p.iris_stops, 0.0);
        assert_eq!(p.beam_ratio, BEAM_RATIO_DEFAULT);
        assert!(CameraTube::new(&p).is_none());
    }

    #[test]
    fn the_tube_is_named_in_the_config_as_the_data_sheet_types_are() {
        let p: CameraParams = toml::from_str("tube = \"plumbicon-act\"\niris_stops = 1.5").unwrap();
        assert_eq!(p.tube, TubeKind::PlumbiconAct);
        assert_eq!(p.iris_stops, 1.5);
        assert_eq!(p.beam_ratio, BEAM_RATIO_DEFAULT);
    }

    #[test]
    fn a_light_that_goes_out_leaves_a_fading_trace_and_the_charge_is_kept_between_frames() {
        let p = CameraParams { tube: TubeKind::Plumbicon, ..CameraParams::default() };
        let mut cam = CameraTube::new(&p).unwrap();
        for _ in 0..40 {
            cam.process_frame(&flat(128));
        }
        // Check field 1 (row 1), which integrates the dark half-frame before it is read.
        let row1_green = |f: Frame| {
            let margin = 16;
            let sum: f64 =
                (margin..ACTIVE_W - margin).map(|x| f.rgb[(ACTIVE_W + x) * 3 + 1] as f64).sum();
            sum / (ACTIVE_W - 2 * margin) as f64
        };
        let first = row1_green(cam.process_frame(&flat(0)));
        let second = row1_green(cam.process_frame(&flat(0)));
        let third = row1_green(cam.process_frame(&flat(0)));
        assert!(first > second && second >= third, "{first} {second} {third}");
        assert!(first > 0.0 && first < 64.0, "{first}");
    }
}
