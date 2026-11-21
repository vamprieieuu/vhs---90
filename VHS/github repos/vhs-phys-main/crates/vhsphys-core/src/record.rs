//! Record side of the deck: baseband video -> luma FM + colour-under -> record
//! current -> saturating tape magnetisation (two-state relay model).
//!
//! The record head field crosses the tape coercivity where the head current
//! crosses +/-`relay_threshold`; the tape keeps the last state it was driven to.
//! Sub-sample crossing times are retained so that transition timing is not
//! quantised to the 43 MS/s grid (a one-sample error would cap the carrier to
//! noise ratio near 15 dB). Because the sum of the FM carrier and the (much
//! smaller) colour-under signal is what crosses the threshold, the chroma is
//! recorded as a duty-cycle modulation of the FM carrier: the FM luma acts as
//! its bias, exactly as in the VHS standard.

use serde::{Deserialize, Serialize};
use super::consts::*;
use crate::dsp::*;
use super::emphasis::SubPreemphasis;
use super::video::Baseband;
use crate::standard::TvStandard;
use std::f64::consts::PI;
use std::sync::OnceLock;

/// RF samples the record stage takes through all its RF steps at a time: three lines, a
/// multiple of `OS`, so that the piece's buffers stay in the cache.
const RECORD_PIECE_RF: usize = 3 * RF_PER_LINE;

/// Newton iterations used to refine the Catmull-Rom crossing time of a magnetisation reversal.
const NEWTON_STEPS: usize = 2;

/// One magnetisation reversal on tape.
#[derive(Clone, Copy, Debug)]
pub struct Transition {
    /// Position in RF samples from the start of the track window (fractional).
    pub t: f64,
    /// +1: rising reversal (state -> +1), -1: falling.
    pub s: i8,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(default)]
pub struct RecordParams {
    /// Input (record) luma bandwidth.
    pub luma_lpf_hz: f64,
    /// Emphasis time constant and boost ratio (standard: 1.3 us, 5x).
    pub emph_tau: f64,
    pub emph_ratio: f64,
    pub white_clip_ire: f32,
    pub dark_clip_ire: f32,
    /// Modulator carrier frequency error (fraction) and gain error.
    pub fm_freq_error: f64,
    pub fm_deviation_scale: f64,
    /// FM high-pass before the head (>17 dB down at 629 kHz).
    pub fm_hpf_hz: f64,
    /// Per-component chroma bandwidth (record band-pass, +/-0.5 MHz).
    pub chroma_lpf_hz: f64,
    /// Colour-under amplitude relative to the FM carrier for |C| = 100 IRE.
    pub chroma_level: f64,
    pub burst_boost: f32,
    /// The sub-preemphasis of the LP and EP modes, before the main emphasis; none at SP.
    pub sub_emphasis: Option<super::emphasis::SubEmphasisParams>,
    /// How far the channel-1 head's carrier sits above the channel-2 head's, Hz (fH/2 at LP and EP).
    pub carrier_interleave_hz: f64,
    /// Head current threshold (relative to FM amplitude 1) = tape coercivity.
    pub relay_threshold: f64,
    /// Asymmetry of the FM drive (DC offset of the record current): a failing
    /// modulator gives an asymmetrical, distorted FM carrier.
    pub drive_offset: f64,
}

impl Default for RecordParams {
    fn default() -> Self {
        Self {
            luma_lpf_hz: 4.2e6,
            emph_tau: PRE_EMPH_TAU,
            emph_ratio: PRE_EMPH_RATIO,
            white_clip_ire: WHITE_CLIP_IRE,
            dark_clip_ire: DARK_CLIP_IRE,
            fm_freq_error: 0.0,
            fm_deviation_scale: 1.0,
            fm_hpf_hz: 1.6e6,
            chroma_lpf_hz: 0.65e6,
            chroma_level: 0.30,
            burst_boost: 2.0,
            sub_emphasis: None,
            carrier_interleave_hz: 0.0,
            relay_threshold: 0.30,
            drive_offset: 0.0,
        }
    }
}

/// Whole carrier cycles per line. The per-sample carrier table holds only these, so it is the
/// same on every line; the fraction of a cycle the carrier gains per line (1/8 in the 625-line
/// format) goes into `line_rotation` with the head's own rotation.
pub const CU_WHOLE_CYCLES_PER_LINE: f64 = CU_CYCLES_PER_LINE as i64 as f64;

/// Eighths of a turn the carrier gains per line beyond the whole cycles (0 at 525 lines, 1 at 625).
const CU_EIGHTHS_PER_LINE: i64 = ((CU_CYCLES_PER_LINE - CU_WHOLE_CYCLES_PER_LINE) * 8.0) as i64;

/// cos/sin of the colour-under carrier phase across one line (`CU_WHOLE_CYCLES_PER_LINE` cycles).
fn carrier_table() -> &'static (Vec<f32>, Vec<f32>) {
    static T: OnceLock<(Vec<f32>, Vec<f32>)> = OnceLock::new();
    T.get_or_init(|| {
        let c = (0..RF_PER_LINE).map(|r| (2.0 * PI * CU_WHOLE_CYCLES_PER_LINE * r as f64 / RF_PER_LINE as f64).cos() as f32).collect();
        let s = (0..RF_PER_LINE).map(|r| (2.0 * PI * CU_WHOLE_CYCLES_PER_LINE * r as f64 / RF_PER_LINE as f64).sin() as f32).collect();
        (c, s)
    })
}

/// Phase of the colour-under carrier at the start of `line`, relative to the line-periodic
/// carrier table: `2 pi (frac(CU_CYCLES_PER_LINE) + CU_ROTATION_QUARTERS[parity] / 4) * line`.
/// At 525 lines the carrier is line locked and only the head's rotation counts (parity 0
/// advances 90 deg per line, parity 1 retards 90 deg); at 625 lines the carrier also gains an
/// eighth of a cycle per line. Every term is a whole number of eighths of a turn, so the
/// phase is counted in eighths modulo 8 and stays exact however large `line` is.
/// Returns the unit vector (cos, sin) of the rotation.
pub fn line_rotation(parity: usize, line: i64) -> (f32, f32) {
    const H: f32 = std::f32::consts::FRAC_1_SQRT_2;
    const EIGHTHS: [(f32, f32); 8] =
        [(1.0, 0.0), (H, H), (0.0, 1.0), (-H, H), (-1.0, 0.0), (-H, -H), (0.0, -1.0), (H, -H)];
    let per_line = CU_EIGHTHS_PER_LINE + 2 * CU_ROTATION_QUARTERS[parity];
    EIGHTHS[(per_line * line.rem_euclid(8)).rem_euclid(8) as usize]
}

/// Whether the V axis of `line` (global line index) is inverted. PAL switches V on alternate
/// lines; the sequence follows the global line count, and as a frame has an odd number of
/// lines it runs on from one frame into the next. Never in the 525-line engine.
pub fn v_switched(line: i64) -> bool {
    STANDARD == TvStandard::Pal && line.rem_euclid(2) == 1
}

/// Whether a multi-standard deck inverts V on `line` (global line index) of its 4.43 MHz output
/// when it plays an NTSC tape into a PAL television (PAL-60). The tape itself is not switched
/// (`v_switched` stays false at 525 lines): the deck does it after its own colour processing,
/// on every odd line, so an NTSC tape's phase error comes off the deck mirrored with V.
pub fn pal60_switched(line: i64) -> bool {
    line.rem_euclid(2) == 1
}

/// Source of baseband video: concatenated frames addressed by baseband sample
/// index. Out-of-range regions are blank (sync, no picture).
pub trait VideoSource: Sync {
    fn fill(&self, bb_start: i64, y: &mut [f32], cu: &mut [f32], cv: &mut [f32]);
}

/// A fixed list of encoded frames, blank outside.
pub struct FrameStore {
    pub frames: Vec<Baseband>,
    blank: Baseband,
}

impl FrameStore {
    pub fn new(frames: Vec<Baseband>) -> Self {
        Self { frames, blank: Baseband::blank() }
    }
}

impl VideoSource for FrameStore {
    fn fill(&self, bb_start: i64, y: &mut [f32], cu: &mut [f32], cv: &mut [f32]) {
        let n = y.len();
        let mut i = 0;
        while i < n {
            let g = bb_start + i as i64;
            let fr = g.div_euclid(FRAME_BB as i64);
            let off = g.rem_euclid(FRAME_BB as i64) as usize;
            let take = (FRAME_BB - off).min(n - i);
            let src = if fr >= 0 && (fr as usize) < self.frames.len() { &self.frames[fr as usize] } else { &self.blank };
            y[i..i + take].copy_from_slice(&src.y[off..off + take]);
            cu[i..i + take].copy_from_slice(&src.cu[off..off + take]);
            cv[i..i + take].copy_from_slice(&src.cv[off..off + take]);
            i += take;
        }
    }
}

/// Luma FM modulation of an RF-rate luma signal (IRE) -> carrier in [-1, 1].
/// `offset_hz` is how far this head's carrier sits above the format's, Hz
/// (the channel-1 head's share of the LP/EP carrier interleave).
pub fn fm_modulate(y_rf: &[f32], p: &RecordParams, offset_hz: f64, out: &mut Vec<f32>) {
    out.clear();
    out.reserve(y_rf.len());
    let mut phase = 0.0f64;
    let k = 2.0 * PI / FS_RF;
    let f0 = (F_SYNC_TIP + offset_hz) * (1.0 + p.fm_freq_error);
    for &y in y_rf {
        let f = f0 + (y as f64 - SYNC_IRE as f64) * HZ_PER_IRE * p.fm_deviation_scale;
        phase += k * f;
        if phase > PI {
            phase -= 2.0 * PI;
        }
        out.push(phase.sin() as f32);
    }
}

/// Upsample `x[k0..k1]` by `OS` with cubic interpolation into `out`, clamping indices against
/// the full `x` slice so piece boundaries match whole-array `upsample`.
fn upsample_range(x: &[f32], k0: usize, k1: usize, out: &mut Vec<f32>) {
    out.clear();
    out.reserve(k1.saturating_sub(k0) * OS);
    let n = x.len();
    for k in k0..k1 {
        let g = |j: i64| x[j.clamp(0, n as i64 - 1) as usize];
        let (p0, p1, p2, p3) = (g(k as i64 - 1), x[k], g(k as i64 + 1), g(k as i64 + 2));
        out.push(p1);
        for j in 1..OS {
            out.push(cubic(p0, p1, p2, p3, j as f32 / OS as f32));
        }
    }
}

/// Refine where the record current crosses `level` between `p1` and `p2` from the linear
/// estimate, using the Catmull-Rom cubic through `p0..p3` = `i[n-2], i[n-1], i[n], i[n+1]`.
#[inline]
fn cubic_crossing(p0: f64, p1: f64, p2: f64, p3: f64, level: f64) -> f64 {
    // Catmull-Rom cubic through p0..p3 = i[n-2], i[n-1], i[n], i[n+1] on [0, 1] from i[n-1] to i[n];
    // solve cubic(x) = level for x by Newton's method from the straight-line estimate, in f64.
    let a3 = -0.5 * p0 + 1.5 * p1 - 1.5 * p2 + 0.5 * p3;
    let b3 = p0 - 2.5 * p1 + 2.0 * p2 - 0.5 * p3;
    let c3 = -0.5 * p0 + 0.5 * p2;
    let mut x = ((level - p1) / (p2 - p1)).clamp(0.0, 1.0);
    for _ in 0..NEWTON_STEPS {
        let f = ((a3 * x + b3) * x + c3) * x + p1 - level;
        let d = (3.0 * a3 * x + 2.0 * b3) * x + c3;
        if d.abs() < 1e-12 {
            break;
        }
        let step = f / d;
        x = (x - step).clamp(0.0, 1.0);
        if step.abs() < 1e-9 {
            break;
        }
    }
    x
}

/// Run the record chain for one track and return the magnetisation reversals.
///
/// `g0` is the global RF index of the first window sample (multiple of `OS`);
/// `n_rf` the window length (multiple of `OS`); `parity` selects head/track 1 or 2.
pub fn record_track(src: &dyn VideoSource, p: &RecordParams, parity: usize, g0: i64, n_rf: usize) -> Vec<Transition> {
    assert!(g0 % OS as i64 == 0 && n_rf % OS == 0);
    const PAD: usize = 2 * BB_PER_LINE; // filter warm-up lines each side
    let n_bb = n_rf / OS;
    let bb0 = g0 / OS as i64 - PAD as i64;
    let tot = n_bb + 2 * PAD;
    let mut y = vec![0.0f32; tot];
    let mut cu = vec![0.0f32; tot];
    let mut cv = vec![0.0f32; tot];
    src.fill(bb0, &mut y, &mut cu, &mut cv);

    // ---- chroma: PAL line switch, burst boost ----
    // The source is the camera's unswitched (U, V); a PAL coder inverts V, burst included, on
    // alternate lines before the deck ever sees the chroma, so it is done here ahead of the
    // deck's band-pass. The switch falls in the sync pulse, clear of the burst and the picture.
    if STANDARD == TvStandard::Pal {
        let mut line = bb0.div_euclid(BB_PER_LINE as i64);
        let mut col = bb0.rem_euclid(BB_PER_LINE as i64) as usize;
        let mut i = 0;
        while i < tot {
            let take = (BB_PER_LINE - col).min(tot - i);
            if v_switched(line) {
                for v in &mut cv[i..i + take] {
                    *v = -*v;
                }
            }
            i += take;
            line += 1;
            col = 0;
        }
    }
    let mut col = bb0.rem_euclid(BB_PER_LINE as i64) as usize;
    for i in 0..tot {
        if (BURST_START..BURST_START + BURST_LEN).contains(&col) {
            cu[i] *= p.burst_boost;
            cv[i] *= p.burst_boost;
        }
        col += 1;
        if col == BB_PER_LINE {
            col = 0;
        }
    }

    // ---- baseband low-pass (chroma and luma) and upsample ----
    // The three second-order Butterworth filters are independent, so run them in one pass
    // over the baseband buffers before upsampling.
    let mut fu = butter_lp(2, p.chroma_lpf_hz, FS_BB);
    fu.settle(0.0);
    let mut fv = butter_lp(2, p.chroma_lpf_hz, FS_BB);
    fv.settle(0.0);
    let mut fy = butter_lp(2, p.luma_lpf_hz, FS_BB);
    fy.settle(0.0);
    for i in 0..tot {
        cu[i] = fu.tick(cu[i] as f64) as f32;
        cv[i] = fv.tick(cv[i] as f64) as f32;
        y[i] = fy.tick(y[i] as f64) as f32;
    }
    // ---- RF chain: state carried across pieces ----
    // LP and EP: the level-dependent boost of fine detail comes first, so the main emphasis
    // sees the boosted signal and the playback can undo the two in the reverse order
    let mut sub = p.sub_emphasis.as_ref().map(|se| {
        let mut s = SubPreemphasis::new(se, FS_RF);
        s.settle(0.0);
        s
    });
    let tau = p.emph_tau;
    let mut emph = Biquad::first_order_pz(1.0 / tau, p.emph_ratio / tau, FS_RF);
    emph.settle(0.0);
    // the channel-1 head (parity 1) records its carrier fH/2 above the channel-2 head's at LP and EP
    let offset_hz = if parity == 1 { p.carrier_interleave_hz } else { 0.0 };
    let mut phase = 0.0f64;
    let k = 2.0 * PI / FS_RF;
    let f0 = (F_SYNC_TIP + offset_hz) * (1.0 + p.fm_freq_error);
    let mut hpf = butter_hp(2, p.fm_hpf_hz, FS_RF);

    let (tc, ts) = carrier_table();
    let a = (p.chroma_level / 100.0) as f32;
    let off = PAD * OS;
    let thr = p.relay_threshold as f32;
    let mut out: Vec<Transition> = Vec::with_capacity(n_rf / 5);
    let mut state: i8 = 0;
    let mut prev_i = 0.0f32;
    let mut prev2_i = 0.0f32;
    // A crossing found between n-1 and n waits one sample for i[n+1] so the cubic has four points.
    let mut pending: Option<(usize, f64, f64, f64, f64, i8)> = None;
    let mut line = g0.div_euclid(RF_PER_LINE as i64);
    let mut r = g0.rem_euclid(RF_PER_LINE as i64) as usize;
    let (mut rc, mut rs) = line_rotation(parity, line);

    let total_rf = OS * tot;
    let piece_cap = RECORD_PIECE_RF.min(total_rf);
    let mut cu_p = Vec::with_capacity(piece_cap);
    let mut cv_p = Vec::with_capacity(piece_cap);
    let mut y_p = Vec::with_capacity(piece_cap);
    let mut fm_p = Vec::with_capacity(piece_cap);

    let mut r0 = 0usize;
    while r0 < total_rf {
        let r1 = (r0 + RECORD_PIECE_RF).min(total_rf);
        let (k0, k1) = (r0 / OS, r1 / OS);

        upsample_range(&cu, k0, k1, &mut cu_p);
        upsample_range(&cv, k0, k1, &mut cv_p);
        upsample_range(&y, k0, k1, &mut y_p);

        // luma: sub-emphasis, emphasis, clip
        if let Some(sub) = sub.as_mut() {
            sub.process(&mut y_p);
        }
        emph.process(&mut y_p);
        for v in y_p.iter_mut() {
            *v = v.clamp(p.dark_clip_ire, p.white_clip_ire);
        }

        // FM carrier, record high-pass
        fm_p.clear();
        for &yv in &y_p {
            let f = f0 + (yv as f64 - SYNC_IRE as f64) * HZ_PER_IRE * p.fm_deviation_scale;
            phase += k * f;
            if phase > PI {
                phase -= 2.0 * PI;
            }
            fm_p.push(phase.sin() as f32);
        }
        hpf.process(&mut fm_p);

        // colour-under + sum + relay over the active window [off, off + n_rf)
        let j0 = off.saturating_sub(r0);
        let j1 = (off + n_rf).saturating_sub(r0).min(fm_p.len());
        for j in j0..j1 {
            let n = r0 + j - off;
            // carrier phasor e^{j(theta + rho)}
            let (cr, ci) = (tc[r] * rc - ts[r] * rs, tc[r] * rs + ts[r] * rc);
            let u = a * (cu_p[j] * cr - cv_p[j] * ci);
            let i_rec = fm_p[j] + u + p.drive_offset as f32;
            if let Some((cn, p0, p1, p2, level, s)) = pending.take() {
                let t = (cn - 1) as f64 + cubic_crossing(p0, p1, p2, i_rec as f64, level);
                out.push(Transition { t, s });
            }
            if n == 0 {
                state = if fm_p[j] > 0.0 { 1 } else { -1 };
                prev_i = i_rec;
                prev2_i = i_rec;
            } else {
                if state < 0 && i_rec > thr {
                    pending = Some((n, prev2_i as f64, prev_i as f64, i_rec as f64, thr as f64, 1));
                    state = 1;
                } else if state > 0 && i_rec < -thr {
                    pending = Some((n, prev2_i as f64, prev_i as f64, i_rec as f64, -(thr as f64), -1));
                    state = -1;
                }
                prev2_i = prev_i;
                prev_i = i_rec;
            }
            r += 1;
            if r == RF_PER_LINE {
                r = 0;
                line += 1;
                (rc, rs) = line_rotation(parity, line);
            }
        }

        r0 = r1;
    }
    if let Some((cn, p0, p1, p2, level, s)) = pending.take() {
        let t = (cn - 1) as f64 + cubic_crossing(p0, p1, p2, p2, level);
        out.push(Transition { t, s });
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::standard::TvStandard;

    /// These tests pin the figures of the 525-line format; the other engine runs the same
    /// file against its own constants, so they return early there.
    const NTSC: bool = matches!(STANDARD, TvStandard::Ntsc);

    /// Constant-luma source (sync-free) for carrier-frequency checks.
    struct Flat(f32);
    impl VideoSource for Flat {
        fn fill(&self, _s: i64, y: &mut [f32], cu: &mut [f32], cv: &mut [f32]) {
            y.fill(self.0);
            cu.fill(0.0);
            cv.fill(0.0);
        }
    }

    fn mean_freq(tr: &[Transition]) -> f64 {
        let n = tr.len();
        let span = tr[n - 1].t - tr[0].t;
        (n as f64 - 1.0) / 2.0 / span * FS_RF
    }

    #[test]
    fn carrier_frequency_follows_luma_level() {
        if !NTSC {
            return;
        }
        let p = RecordParams { luma_lpf_hz: 20e6, emph_ratio: 1.0, ..Default::default() };
        for (ire, want) in [(-40.0f32, 3.4e6), (0.0, 3.4e6 + 40.0 * HZ_PER_IRE), (100.0, 4.4e6)] {
            // emphasis ratio 1 (flat): the clip must not engage for these levels
            let tr = record_track(&Flat(ire), &p, 0, 0, 30_000);
            let f = mean_freq(&tr);
            assert!((f - want).abs() < 0.01e6, "ire {ire}: {f} vs {want}");
        }
    }

    #[test]
    fn transitions_alternate_and_are_sub_sample() {
        let p = RecordParams { emph_ratio: 1.0, ..Default::default() };
        let tr = record_track(&Flat(50.0), &p, 1, 0, 30_000);
        for w in tr.windows(2) {
            assert_eq!(w[0].s, -w[1].s);
            assert!(w[1].t > w[0].t + 3.0);
        }
        let frac: Vec<f64> = tr.iter().map(|t| t.t.fract()).collect();
        let distinct = frac.iter().filter(|f| **f > 0.05 && **f < 0.95).count();
        assert!(distinct > tr.len() / 2, "positions should not be on the sample grid");
    }

    #[test]
    fn clip_bounds_carrier_frequency() {
        if !NTSC {
            return;
        }
        // a huge luma step is clipped to the 3.0 .. 5.0 MHz deviation limits
        let p = RecordParams { luma_lpf_hz: 20e6, emph_ratio: 1.0, ..Default::default() };
        let hi = mean_freq(&record_track(&Flat(500.0), &p, 0, 0, 30_000));
        let lo = mean_freq(&record_track(&Flat(-500.0), &p, 0, 0, 30_000));
        assert!((hi - 5.0e6).abs() < 0.02e6, "{hi}");
        assert!((lo - 3.0e6).abs() < 0.02e6, "{lo}");
    }

    #[test]
    fn rotation_alternates_by_head() {
        if !NTSC {
            return;
        }
        assert_eq!(line_rotation(0, 1), (0.0, 1.0));
        assert_eq!(line_rotation(1, 1), (0.0, -1.0));
        assert_eq!(line_rotation(0, 4), (1.0, 0.0));
        assert_eq!(line_rotation(1, 2), (-1.0, 0.0));
    }

    /// Constant-chroma source for the duty-cycle test.
    struct FlatChroma(f32, f32, f32);
    impl VideoSource for FlatChroma {
        fn fill(&self, _s: i64, y: &mut [f32], cu: &mut [f32], cv: &mut [f32]) {
            y.fill(self.0);
            cu.fill(self.1);
            cv.fill(self.2);
        }
    }

    /// The chroma is recorded as a duty-cycle modulation of the FM carrier. With
    /// threshold h and amplitude A the low-frequency magnetisation is
    /// dM/dc = 2 / (pi A sqrt(1 - (h/A)^2)) per unit of chroma current c.
    #[test]
    fn chroma_is_recorded_as_duty_cycle_of_the_fm_carrier() {
        let p = RecordParams { luma_lpf_hz: 20e6, emph_ratio: 1.0, burst_boost: 1.0, chroma_level: 0.30, ..Default::default() };
        let n = 20 * RF_PER_LINE;
        let tr = record_track(&FlatChroma(20.0, 100.0, 0.0), &p, 0, 0, n);
        // render the two-state magnetisation at RF resolution (box-averaged)
        let mut m = vec![0.0f64; n];
        let mut state = -tr[0].s as f64;
        let mut last = 0.0f64;
        for t in &tr {
            let i = t.t.floor() as usize;
            if i >= n { break; }
            // fill whole samples up to i with the old state, then split sample i
            for k in last as usize..i { m[k] = state; }
            let frac = t.t - i as f64;
            m[i] = state * frac + (t.s as f64) * (1.0 - frac);
            state = t.s as f64;
            last = (i + 1) as f64;
        }
        // amplitude of the 40-cycles-per-line component, averaged over lines
        let mut amps = vec![];
        for line in 4..16 {
            let seg = &m[line * RF_PER_LINE..(line + 1) * RF_PER_LINE];
            let (mut re, mut im) = (0.0, 0.0);
            for (r, v) in seg.iter().enumerate() {
                let a = 2.0 * PI * 40.0 * r as f64 / RF_PER_LINE as f64;
                re += v * a.cos();
                im += v * a.sin();
            }
            amps.push(2.0 * (re * re + im * im).sqrt() / RF_PER_LINE as f64);
        }
        let amp = amps.iter().sum::<f64>() / amps.len() as f64;
        let (a, h, c) = (1.0f64, 0.30f64, 0.30f64);
        // exact duty cycle for this c, evaluated at the peak of the chroma carrier
        let exact = ((-(h + c) / a).acos() + ((h - c) / a).acos()) / PI - 1.0;
        let small = 2.0 / (PI * a * (1.0 - (h / a).powi(2)).sqrt()) * c;
        assert!((amp - small).abs() < 0.03, "measured {amp}, small-signal {small}, exact {exact}");
    }

    #[test]
    fn upsample_range_matches_whole_array() {
        let mut rng = 0x1234_5678u32;
        let x: Vec<f32> = (0..2000)
            .map(|_| {
                rng = rng.wrapping_mul(1_664_525).wrapping_add(1_013_904_223);
                (rng as i32 as f32) / i32::MAX as f32
            })
            .collect();
        let mut want = Vec::new();
        upsample(&x, OS, &mut want);
        let want_bits: Vec<u32> = want.iter().map(|v| v.to_bits()).collect();
        if OS == REFERENCE_OS {
            let mut want3 = Vec::new();
            upsample3(&x, &mut want3);
            assert_eq!(want_bits, want3.iter().map(|v| v.to_bits()).collect::<Vec<_>>());
        }
        for step in [1usize, 7, 910] {
            let mut got = Vec::new();
            let mut piece = Vec::new();
            let mut k0 = 0;
            while k0 < x.len() {
                let k1 = (k0 + step).min(x.len());
                upsample_range(&x, k0, k1, &mut piece);
                got.extend_from_slice(&piece);
                k0 = k1;
            }
            let got_bits: Vec<u32> = got.iter().map(|v| v.to_bits()).collect();
            assert_eq!(got_bits, want_bits, "step={step}");
        }
    }
}
