//! Common playback electronics after the head switch: equaliser, dropout
//! compensator, limiter, delay-line FM discriminator, de-emphasis, luma
//! low-pass/aperture corrector, colour-under extraction.
//!
//! Every stage is an explicit circuit model so that component drift (R, C,
//! delay-line detuning) and supply faults change the picture the way they do in
//! a real deck. Fixed "alignment" constants (e.g. the discriminator's assumed
//! delay) come from the deck's nominal design; faults perturb the *actual*
//! values only.

use super::consts::*;
use super::emphasis::SubDeemphasis;
use crate::dsp::*;
use serde::{Deserialize, Serialize};

/// Nominal discriminator delay assumed by the downstream scaling (samples).
pub const DEMOD_DELAY_NOMINAL: f64 = 3.0 * RF_SCALE;

/// RF samples processed per piece by the streaming deck: a multiple of `OS` around
/// 16 000 so that a piece's intermediate buffers stay in the second-level cache.
const DECK_PIECE_RF: usize = 6 * RF_PER_LINE;

/// Number of equalised samples used to initialise the AGC.
const AGC_INIT_RF: usize = rf_samples(4000);

/// First piece size: long enough for both the AGC start-up mean and the coupling
/// high-pass settling mean (`min(n, RF_PER_LINE)` de-emphasised samples).
const DECK_STARTUP_RF: usize = if AGC_INIT_RF > RF_PER_LINE {
    AGC_INIT_RF
} else {
    RF_PER_LINE
};

/// Baseband FIR outputs computed side by side in the unclamped interior.
const FIR_OUTPUT_BLOCK: usize = 16;

#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(default)]
pub struct DeckParams {
    /// Playback equaliser peaks: (centre Hz, Q, gain dB).
    pub eq_peaks: Vec<(f64, f64, f64)>,
    /// Gain applied after the equaliser (alignment: nominal carrier -> 1.0).
    pub fm_gain: f64,
    /// Slow RF level control ahead of the compensator and limiter: absorbs head
    /// wear and tape output differences (time constant `agc_ms`, gain range
    /// 0.25 .. `agc_max_gain`) so that only *fast* dips trip the compensator.
    pub agc_enabled: bool,
    pub agc_ms: f64,
    pub agc_target: f64,
    pub agc_max_gain: f64,
    pub doc_enabled: bool,
    /// Dropout compensator trip level and release level, fraction of the AGC
    /// reference level (the release level gives the hysteresis).
    pub doc_trip: f64,
    pub doc_release: f64,
    /// Gain per recirculation of the line delay (glass ~0.85, CCD 1.0).
    pub doc_recirculation: f64,
    /// Limiter comparator hysteresis as a fraction of the nominal carrier
    /// amplitude (finite limiter gain / input-referred offset).
    pub limiter_threshold: f64,
    /// FM-path high-pass ahead of the limiter: keeps the colour-under (and the
    /// FM audio band) out of the discriminator.
    pub fm_hpf_hz: f64,
    /// Actual discriminator delay (samples); nominal is `DEMOD_DELAY_NOMINAL`.
    pub demod_delay: f64,
    /// Discriminator output low-pass at RF rate (first stage against the
    /// 2 x carrier product) ...
    pub demod_lpf_hz: f64,
    pub demod_lpf_order: usize,
    /// ... and the linear-phase video low-pass after decimation (the deck's
    /// ~3 MHz luminance bandwidth).
    pub luma_lpf_hz: f64,
    pub luma_lpf_transition_hz: f64,
    /// Scale factors on the de-emphasis network's time constant and ratio.
    pub deemph_tau_scale: f64,
    /// The sub-deemphasis of the LP and EP modes, after the main de-emphasis; none at SP.
    pub sub_deemphasis: Option<super::emphasis::SubEmphasisParams>,
    pub deemph_ratio_scale: f64,
    /// Aperture corrector ("sharpness"): centre, Q, gain dB.
    pub sharp_freq: f64,
    pub sharp_q: f64,
    pub sharp_db: f64,
    /// Colour-under extraction low-pass.
    pub chroma_lpf_hz: f64,
    /// Additive disturbances (IRE) injected after the discriminator, keyed by
    /// global RF sample index: supplies, hum, switching ripple.
    pub supply: SupplyParams,
    /// Coupling capacitor in the luma path: low-frequency cutoff (Hz). Aged
    /// electrolytics (lower C, higher ESR) raise it and cause tilt/streaking.
    pub coupling_hpf_hz: f64,
    /// DC offset error of the discriminator (IRE), e.g. a detuned tank.
    pub dc_error_ire: f64,
}

/// Power-supply related disturbances reaching the video path.
#[derive(Clone, Debug, Default, Serialize, Deserialize)]
#[serde(default)]
pub struct SupplyParams {
    /// Peak-to-peak hum at the discriminator output, IRE.
    pub hum_ire: f64,
    /// Mains frequency (Hz) and ripple harmonic content (0 = sine at 2 x mains
    /// only, 1 = strongly peaked charging pulses).
    pub mains_hz: f64,
    pub ripple_peakiness: f64,
    /// Switch-mode supply: frequency (Hz) and amplitude (IRE p-p) of the
    /// switching ripple leaking into the video path.
    pub smps_hz: f64,
    pub smps_ire: f64,
    /// Random phase for the supply waveforms (derived from the seed).
    pub phase: f64,
}

impl Default for DeckParams {
    fn default() -> Self {
        let carrier_scale = (F_SYNC_TIP + 0.5e6) / 3.9e6;
        Self {
            eq_peaks: vec![
                (5.4e6 * carrier_scale, 0.9, 6.0),
                (7.2e6 * carrier_scale, 1.2, 3.0),
            ],
            fm_gain: 1.0,
            agc_enabled: true,
            agc_ms: 6.0,
            agc_target: 0.6,
            agc_max_gain: 6.0,
            doc_enabled: true,
            doc_trip: 0.30,
            doc_release: 0.45,
            doc_recirculation: 1.0,
            limiter_threshold: 0.03,
            fm_hpf_hz: 1.8e6,
            demod_delay: DEMOD_DELAY_NOMINAL,
            demod_lpf_hz: 4.5e6,
            demod_lpf_order: 6,
            luma_lpf_hz: 3.0e6,
            luma_lpf_transition_hz: 2.0e6,
            deemph_tau_scale: 1.0,
            sub_deemphasis: None,
            deemph_ratio_scale: 1.0,
            sharp_freq: 2.2e6,
            sharp_q: 0.7,
            sharp_db: 3.0,
            chroma_lpf_hz: 1.2e6,
            supply: SupplyParams::default(),
            coupling_hpf_hz: 15.0,
            dc_error_ire: 0.0,
        }
    }
}

/// What a segment of playback produced (all at the baseband rate, 4 x fsc).
pub struct SegmentOut {
    pub luma: Vec<f32>,
    pub cunder: Vec<f32>,
    pub dropout: Vec<u8>,
}

/// Supply disturbance at global RF sample `g`: `(offset_ire, gain_modulation)`.
///
/// The supply rail feeds the video amplifiers, so ripple both shifts the level
/// (which the TV's back-porch clamp removes again) and modulates the gain of the
/// stages (which it cannot: whites move while black stays put). The gain term is
/// what makes hum bars visible. Hum is the 2 x mains ripple (a full-wave
/// rectified waveform, sharper as the filter capacitor's ESR grows); a switch
/// mode supply adds a fixed-frequency ripple that beats with the line rate.
pub fn supply_disturbance(s: &SupplyParams, g: i64) -> (f32, f32) {
    let t = g as f64 / FS_RF;
    let (mut add, mut gain) = (0.0, 0.0);
    if s.hum_ire != 0.0 {
        let w = 2.0 * std::f64::consts::PI * 2.0 * s.mains_hz * t + s.phase;
        let saw = (w.sin()
            + 0.35 * s.ripple_peakiness * (2.0 * w).sin()
            + 0.2 * s.ripple_peakiness * (3.0 * w).sin())
            / (1.0 + 0.35 * s.ripple_peakiness);
        add += 0.5 * s.hum_ire * saw;
        gain += 0.5 * s.hum_ire / 100.0 * 0.8 * saw;
    }
    if s.smps_ire != 0.0 {
        let w = 2.0 * std::f64::consts::PI * s.smps_hz * t + 1.7 * s.phase;
        add += 0.5 * s.smps_ire * w.sin();
    }
    (add as f32, gain as f32)
}

/// Streaming playback chain: carries filter, detector and delay state across
/// short pieces of RF so intermediate buffers stay in the second-level cache.
struct DeckStream<'a> {
    p: &'a DeckParams,
    g0: i64,
    n: usize,
    eq: Vec<Biquad>,
    gain: f32,
    reference: f32,
    agc_alpha: f32,
    agc_gmin: f32,
    agc_gmax: f32,
    agc_avg: f32,
    startup_done: bool,
    doc_decay: f32,
    doc_trip: f32,
    doc_rel: f32,
    doc_rec: f32,
    doc_env: f32,
    doc_active: bool,
    doc_hold: usize,
    doc_line: Vec<f32>,
    doc_pos: usize,
    rf_idx: usize,
    cu_rf_lpf: Cascade,
    cu_samples: [f32; OS],
    cu_phase: usize,
    doc_group: u8,
    clpf: Cascade,
    fm_hp: Cascade,
    disc: DiscStream,
    lpf: Cascade,
    tau: f64,
    de: Biquad,
    sub_de: Option<SubDeemphasis>,
    y_settled: bool,
    cpl: Biquad,
    cpl_settled: bool,
    mid: f32,
    y_phase: usize,
    pending_lb: f32,
    fir: FirStream,
    sh: Option<Biquad>,
    have_supply: bool,
    dc_error: f32,
    out_k: usize,
    finished: bool,
    x_buf: Vec<f32>,
    prod_buf: Vec<f32>,
    lb_buf: Vec<f32>,
    fir_buf: Vec<f32>,
}

impl<'a> DeckStream<'a> {
    fn new(p: &'a DeckParams, g0: i64, n: usize) -> Self {
        let eq: Vec<Biquad> = p
            .eq_peaks
            .iter()
            .map(|&(f, q, g)| Biquad::peaking(f, q, g, FS_RF))
            .collect();
        let gain = p.fm_gain as f32;
        let reference = if p.agc_enabled {
            p.agc_target as f32
        } else {
            1.0
        };
        let agc_alpha = (1.0 - (-1.0 / (p.agc_ms * 1e-3 * FS_RF)).exp()) as f32;
        let (agc_gmin, agc_gmax) = (0.25f32, p.agc_max_gain as f32);
        let doc_decay = (-1.0 / (0.35e-6 * FS_RF)).exp() as f32;
        let (doc_trip, doc_rel) = (
            p.doc_trip as f32 * reference,
            p.doc_release as f32 * reference,
        );
        let doc_rec = p.doc_recirculation as f32;
        let doc_line = if p.doc_enabled {
            vec![0.0f32; RF_PER_LINE]
        } else {
            Vec::new()
        };
        let cu_rf_lpf = butter_lp(4, p.chroma_lpf_hz, FS_RF);
        let clpf = butter_lp(4, p.chroma_lpf_hz, FS_BB);
        let fm_hp = butter_hp(4, p.fm_hpf_hz, FS_RF);
        let disc = DiscStream::new(p.limiter_threshold as f32, p.demod_delay);
        let mut lpf = butter_lp(p.demod_lpf_order, p.demod_lpf_hz, FS_RF);
        // the discriminator idles at the sync-tip... settle on the 0 IRE level
        lpf.settle(r_of_freq(F_SYNC_TIP - SYNC_IRE as f64 * HZ_PER_IRE) as f64);
        let tau = DEMOD_DELAY_NOMINAL / FS_RF;
        let t = PRE_EMPH_TAU * p.deemph_tau_scale;
        let ratio = PRE_EMPH_RATIO * p.deemph_ratio_scale;
        let de = Biquad::first_order_pz(ratio / t, 1.0 / t, FS_RF);
        let sub_de = p
            .sub_deemphasis
            .as_ref()
            .map(|se| SubDeemphasis::new(se, FS_RF));
        let cpl = Biquad::one_pole_hp(p.coupling_hpf_hz, FS_RF);
        let fir_taps = kaiser_lowpass(p.luma_lpf_hz, p.luma_lpf_transition_hz, 55.0, FS_BB);
        let fir = FirStream::new(fir_taps);
        let sh = if p.sharp_db != 0.0 {
            Some(Biquad::peaking(p.sharp_freq, p.sharp_q, p.sharp_db, FS_BB))
        } else {
            None
        };
        let have_supply = p.supply.hum_ire != 0.0 || p.supply.smps_ire != 0.0;
        let dc_error = p.dc_error_ire as f32;
        let piece_cap = DECK_PIECE_RF.max(DECK_STARTUP_RF);
        let bb_cap = piece_cap / OS + 4;
        Self {
            p,
            g0,
            n,
            eq,
            gain,
            reference,
            agc_alpha,
            agc_gmin,
            agc_gmax,
            agc_avg: 0.0,
            startup_done: false,
            doc_decay,
            doc_trip,
            doc_rel,
            doc_rec,
            doc_env: reference,
            doc_active: false,
            doc_hold: 0,
            doc_line,
            doc_pos: 0,
            rf_idx: 0,
            cu_rf_lpf,
            cu_samples: [0.0; OS],
            cu_phase: 0,
            doc_group: 0,
            clpf,
            fm_hp,
            disc,
            lpf,
            tau,
            de,
            sub_de,
            y_settled: false,
            cpl,
            cpl_settled: false,
            mid: 0.0,
            y_phase: 0,
            pending_lb: 0.0,
            fir,
            sh,
            have_supply,
            dc_error,
            out_k: 0,
            finished: false,
            x_buf: Vec::with_capacity(piece_cap),
            prod_buf: Vec::with_capacity(piece_cap + 4),
            lb_buf: Vec::with_capacity(bb_cap),
            fir_buf: Vec::with_capacity(bb_cap),
        }
    }

    #[inline(always)]
    fn step_agc_doc_cu(&mut self, mut v: f32, out: &mut SegmentOut) -> f32 {
        if self.p.agc_enabled {
            let a = v.abs();
            let g = (self.reference / (self.agc_avg * std::f32::consts::FRAC_PI_2).max(1e-6))
                .clamp(self.agc_gmin, self.agc_gmax);
            self.agc_avg += self.agc_alpha * (a - self.agc_avg);
            v *= g;
        }
        let mut doc_flag = 0u8;
        if self.p.doc_enabled {
            self.doc_env = v.abs().max(self.doc_env * self.doc_decay);
            if !self.doc_active && self.doc_env < self.doc_trip {
                self.doc_active = true;
            } else if self.doc_active && self.doc_env > self.doc_rel {
                if self.doc_hold == 0 {
                    self.doc_hold = rf_samples(6);
                }
                self.doc_hold -= 1;
                if self.doc_hold == 0 {
                    self.doc_active = false;
                }
            } else if self.doc_active {
                self.doc_hold = 0;
            }
            if self.doc_active && self.rf_idx >= RF_PER_LINE {
                v = self.doc_line[self.doc_pos] * self.doc_rec;
                doc_flag = 1;
            }
            self.doc_line[self.doc_pos] = v;
            self.doc_pos += 1;
            if self.doc_pos == RF_PER_LINE {
                self.doc_pos = 0;
            }
        }
        self.rf_idx += 1;

        // Low-pass the colour-under tap at the RF rate before averaging onto the baseband grid,
        // so FM carrier harmonics above the baseband Nyquist do not fold into the colour band.
        self.cu_samples[self.cu_phase] = self.cu_rf_lpf.tick(v as f64) as f32;
        self.doc_group |= doc_flag;
        self.cu_phase += 1;
        if self.cu_phase == OS {
            self.cu_phase = 0;
            // the mean of the RF samples of one baseband sample, summed in order
            let mut sum = self.cu_samples[0];
            for v in &self.cu_samples[1..] {
                sum += *v;
            }
            let raw = sum / OS as f32;
            let filtered = self.clpf.tick(raw as f64) as f32;
            let d = if self.doc_group != 0 { 1 } else { 0 };
            out.cunder.push(if d == 1 { 0.0 } else { filtered });
            out.dropout.push(d);
            self.doc_group = 0;
        }
        v
    }

    fn push(&mut self, piece: &[f32], out: &mut SegmentOut) {
        if piece.is_empty() {
            return;
        }
        self.x_buf.clear();
        if !self.startup_done {
            self.startup_done = true;
            // First piece holds at least `min(n, DECK_STARTUP_RF)` samples so both the
            // AGC mean and `mid` (for settling `cpl` and `sh`) are known before moving on.
            for &sample in piece {
                let mut v = sample as f64;
                for b in &mut self.eq {
                    v = b.tick(v) as f32 as f64;
                }
                self.x_buf.push(v as f32 * self.gain);
            }
            if self.p.agc_enabled {
                let k = self.n.min(AGC_INIT_RF);
                self.agc_avg = self.x_buf[..k].iter().map(|v| v.abs()).sum::<f32>() / k as f32;
            }
            for i in 0..self.x_buf.len() {
                let v = self.step_agc_doc_cu(self.x_buf[i], out);
                self.x_buf[i] = self.fm_hp.tick(v as f64) as f32;
            }
        } else {
            for &sample in piece {
                let mut v = sample as f64;
                for b in &mut self.eq {
                    v = b.tick(v) as f32 as f64;
                }
                let v = self.step_agc_doc_cu(v as f32 * self.gain, out);
                self.x_buf.push(self.fm_hp.tick(v as f64) as f32);
            }
        }

        self.prod_buf.clear();
        self.disc.push(&self.x_buf, &mut self.prod_buf);
        if self.rf_idx == self.n {
            // Short inputs (`n <= DECK_STARTUP_RF`) need the tail of `y` before `mid` can be settled.
            self.disc.finish(&mut self.prod_buf);
            self.finished = true;
        }
        self.process_prod(out, self.finished);
    }

    fn process_prod(&mut self, out: &mut SegmentOut, at_end: bool) {
        if !self.prod_buf.is_empty() {
            for (i, v) in self.prod_buf.iter_mut().enumerate() {
                let lp = self.lpf.tick(*v as f64) as f32;
                let f = (1.0 - lp as f64) / (4.0 * self.tau);
                let y = ((f - F_SYNC_TIP) / HZ_PER_IRE + SYNC_IRE as f64) as f32;
                if !self.y_settled && i == 0 {
                    self.de.settle(y as f64);
                }
                *v = self.de.tick(y as f64) as f32;
            }
            if let Some(sub) = &mut self.sub_de {
                if !self.y_settled {
                    sub.settle(self.prod_buf[0] as f64);
                }
                sub.process(&mut self.prod_buf);
            }
            self.y_settled = true;

            if !self.cpl_settled {
                let m = self.n.min(RF_PER_LINE);
                let mid = self.prod_buf[..m].iter().sum::<f32>() / m as f32;
                self.cpl.settle(mid as f64);
                if let Some(sh) = &mut self.sh {
                    sh.settle(mid as f64);
                }
                self.mid = mid;
                self.cpl_settled = true;
            }

            self.lb_buf.clear();
            for &y in &self.prod_buf {
                let v = (self.cpl.tick(y as f64) as f32) + self.mid;
                if self.y_phase == OS / 2 {
                    self.pending_lb = v;
                }
                if self.y_phase == OS - 1 {
                    self.lb_buf.push(self.pending_lb);
                }
                self.y_phase += 1;
                if self.y_phase == OS {
                    self.y_phase = 0;
                }
            }
        } else {
            self.lb_buf.clear();
        }

        self.fir_buf.clear();
        self.fir.push(&self.lb_buf, &mut self.fir_buf);
        if at_end {
            self.fir.finish(&mut self.fir_buf);
        }

        for &v in &self.fir_buf {
            let mut v = v;
            if let Some(sh) = &mut self.sh {
                v = sh.tick(v as f64) as f32;
            }
            if self.have_supply {
                let (add, gm) =
                    supply_disturbance(&self.p.supply, self.g0 + (OS * self.out_k + OS / 2) as i64);
                v = v * (1.0 + gm) + add;
            }
            out.luma.push(v + self.dc_error);
            self.out_k += 1;
        }
    }

    fn finish(mut self, out: &mut SegmentOut) {
        if !self.finished {
            self.finished = true;
            self.prod_buf.clear();
            self.disc.finish(&mut self.prod_buf);
            self.process_prod(out, true);
        }
    }
}

/// Process one playback segment. `rf` is the head-switched RF (after head
/// amplifiers); `g0` its global RF index. The output covers the same span.
pub fn process_segment(rf: &[f32], p: &DeckParams, g0: i64) -> SegmentOut {
    let n = rf.len();
    assert!(n > 0);
    let nb = n / OS;
    let mut out = SegmentOut {
        luma: Vec::with_capacity(nb),
        cunder: Vec::with_capacity(nb),
        dropout: Vec::with_capacity(nb),
    };
    let mut stream = DeckStream::new(p, g0, n);
    let head = n.min(DECK_STARTUP_RF);
    stream.push(&rf[..head], &mut out);
    for piece in rf[head..].chunks(DECK_PIECE_RF) {
        stream.push(piece, &mut out);
    }
    stream.finish(&mut out);
    out
}

/// Resumable centred FIR filter matching [`dsp::fir_centred`] bit-for-bit.
struct FirStream {
    h: Vec<f32>,
    half: usize,
    buf: Vec<f32>,
    base: usize,
    out_i: usize,
}

impl FirStream {
    fn new(h: Vec<f32>) -> Self {
        let half = h.len() / 2;
        let cap = h.len() + DECK_PIECE_RF / OS + 8;
        Self {
            h,
            half,
            buf: Vec::with_capacity(cap),
            base: 0,
            out_i: 0,
        }
    }

    fn push(&mut self, chunk: &[f32], out: &mut Vec<f32>) {
        if chunk.is_empty() {
            return;
        }
        self.buf.extend_from_slice(chunk);
        if self.h.is_empty() {
            out.resize(out.len() + chunk.len(), 0.0);
            self.out_i += chunk.len();
            self.buf.clear();
            self.base += chunk.len();
            return;
        }
        let m = self.h.len();
        let total_seen = self.base + self.buf.len();
        // Output `i` reads up to `i + m - 1 - half`; once below `total_seen`, it never clamps right.
        let ready_end = total_seen.saturating_sub(m - self.half - 1);
        let left_end = self.half.min(ready_end);
        while self.out_i < left_end {
            let i = self.out_i;
            let mut acc = 0.0f32;
            for (k, &c) in self.h.iter().enumerate() {
                let j = (i + k).saturating_sub(self.half);
                acc += c * self.buf[j - self.base];
            }
            out.push(acc);
            self.out_i += 1;
        }
        while self.out_i < ready_end {
            let count = FIR_OUTPUT_BLOCK.min(ready_end - self.out_i);
            let mut sums = [0.0f32; FIR_OUTPUT_BLOCK];
            let start = self.out_i - self.half - self.base;
            for (k, &c) in self.h.iter().enumerate() {
                let row = &self.buf[start + k..start + k + count];
                for j in 0..count {
                    sums[j] += c * row[j];
                }
            }
            out.extend_from_slice(&sums[..count]);
            self.out_i += count;
        }
        let keep_from = self.out_i.saturating_sub(self.half);
        let drop = keep_from.saturating_sub(self.base);
        if drop > 0 {
            self.buf.drain(..drop);
            self.base += drop;
        }
    }

    fn finish(&mut self, out: &mut Vec<f32>) {
        if self.h.is_empty() {
            return;
        }
        let nb = self.base + self.buf.len();
        if nb == 0 {
            return;
        }
        for i in self.out_i..nb {
            let mut acc = 0.0f32;
            for (k, &c) in self.h.iter().enumerate() {
                let j = (i + k).saturating_sub(self.half).min(nb - 1);
                acc += c * self.buf[j - self.base];
            }
            out.push(acc);
        }
        self.out_i = nb;
    }
}

/// Resumable hard limiter and delay-line FM discriminator.
///
/// Mirrors [`discriminate`] sample-for-sample across arbitrary input pieces
/// without materialising a full-field `4 * n` fine grid.
struct DiscStream {
    th: f32,
    delay: f64,
    total_samples: usize,
    state: i8,
    prev_x: f32,
    edges: Vec<f64>,
    edge_i: usize,
    edge_j: usize,
    sign: f32,
    t_prev: f64,
    acc: f64,
    cur: usize,
    fine_ring: [f32; 8],
    out_k: usize,
}

impl DiscStream {
    const SUB: usize = 4;
    const W: f64 = 1.0 / Self::SUB as f64;
    const TRI: [f32; 7] = [1.0, 2.0, 3.0, 4.0, 3.0, 2.0, 1.0];

    fn new(th: f32, delay: f64) -> Self {
        Self {
            th,
            delay,
            total_samples: 0,
            state: 1,
            prev_x: 0.0,
            edges: Vec::new(),
            edge_i: 0,
            edge_j: 0,
            sign: 1.0, // steady-state assumption before the first edge
            t_prev: 0.0,
            acc: 0.0,
            cur: 0,
            fine_ring: [0.0; 8],
            out_k: 0,
        }
    }

    fn next_edge(&self) -> Option<(f64, bool)> {
        let a = self.edges.get(self.edge_i).copied();
        let b = self.edges.get(self.edge_j).map(|e| e + self.delay);
        match (a, b) {
            (Some(a), Some(b)) => Some(if a <= b { (a, true) } else { (b, false) }),
            (Some(a), None) => Some((a, true)),
            (None, Some(b)) => Some((b, false)),
            (None, None) => None,
        }
    }

    #[inline]
    fn push_fine(&mut self, val: f32, out: &mut Vec<f32>) {
        let c = self.cur;
        self.fine_ring[c & 7] = val;
        self.cur = c + 1;
        // Output `k` uses fine cells `4k - 1 ..= 4k + 5`; emit it as soon as `4k + 5` is ready.
        if c >= 5 && (c - 5) & 3 == 0 {
            let k = (c - 5) >> 2;
            let centre = k * Self::SUB + Self::SUB / 2;
            let mut v = 0.0f32;
            for (m, &coeff) in Self::TRI.iter().enumerate() {
                let idx = (centre + m) as i64 - 3;
                if idx >= 0 {
                    v += coeff * self.fine_ring[(idx as usize) & 7];
                }
            }
            out.push(v / 16.0);
            self.out_k = k + 1;
        }
    }

    fn push(&mut self, chunk: &[f32], out: &mut Vec<f32>) {
        if chunk.is_empty() {
            return;
        }
        let mut idx = 0usize;
        if self.total_samples == 0 {
            self.state = if chunk[0] >= 0.0 { 1 } else { -1 };
            self.prev_x = chunk[0];
            self.total_samples = 1;
            idx = 1;
        }
        let th = self.th;
        let mut i = self.total_samples;
        for &b in &chunk[idx..] {
            let a = self.prev_x;
            if self.state < 0 && b > th {
                self.edges.push((i - 1) as f64 + ((th - a) / (b - a)) as f64);
                self.state = 1;
            } else if self.state > 0 && b < -th {
                self.edges.push((i - 1) as f64 + ((-th - a) / (b - a)) as f64);
                self.state = -1;
            }
            self.prev_x = b;
            i += 1;
        }
        self.total_samples = i;

        // Every future direct edge is >= `N - 1` (delayed >= `N - 1 + delay`). Integrate up to
        // edges strictly below that horizon, then only over whole cells ending at or before it.
        let horizon = (self.total_samples - 1) as f64 + self.delay.min(0.0);
        loop {
            let Some((te, first)) = self.next_edge() else {
                break;
            };
            if te >= horizon {
                break;
            }
            let mut t = self.t_prev;
            while t < te {
                let end = ((self.cur + 1) as f64 * Self::W).min(te);
                self.acc += self.sign as f64 * (end - t);
                t = end;
                if t >= (self.cur + 1) as f64 * Self::W {
                    let val = (self.acc / Self::W) as f32;
                    self.acc = 0.0;
                    self.push_fine(val, out);
                }
            }
            self.t_prev = te;
            self.sign = -self.sign;
            if first {
                self.edge_i += 1;
            } else {
                self.edge_j += 1;
            }
        }
        while (self.cur + 1) as f64 * Self::W <= horizon {
            let end = (self.cur + 1) as f64 * Self::W;
            self.acc += self.sign as f64 * (end - self.t_prev);
            self.t_prev = end;
            let val = (self.acc / Self::W) as f32;
            self.acc = 0.0;
            self.push_fine(val, out);
        }

        let consumed = self.edge_i.min(self.edge_j);
        if consumed > 0 {
            self.edges.drain(..consumed);
            self.edge_i -= consumed;
            self.edge_j -= consumed;
        }
    }

    fn finish(&mut self, out: &mut Vec<f32>) {
        let n = self.total_samples;
        if n == 0 {
            return;
        }
        let end_t = n as f64;
        let nb = n * Self::SUB;
        loop {
            let (te, first) = self.next_edge().unwrap_or((end_t, true));
            let te = te.min(end_t);
            let mut t = self.t_prev;
            while t < te {
                let end = ((self.cur + 1) as f64 * Self::W).min(te);
                self.acc += self.sign as f64 * (end - t);
                t = end;
                if t >= (self.cur + 1) as f64 * Self::W {
                    if self.cur < nb {
                        let val = (self.acc / Self::W) as f32;
                        self.push_fine(val, out);
                    } else {
                        self.cur += 1;
                    }
                    self.acc = 0.0;
                }
            }
            self.t_prev = te;
            if te >= end_t {
                break;
            }
            self.sign = -self.sign;
            if first {
                self.edge_i += 1;
            } else {
                self.edge_j += 1;
            }
        }
        while self.out_k < n {
            let k = self.out_k;
            let centre = k * Self::SUB + Self::SUB / 2;
            let mut v = 0.0f32;
            for (m, &coeff) in Self::TRI.iter().enumerate() {
                let idx = (centre + m) as i64 - 3;
                if idx >= 0 && (idx as usize) < nb {
                    v += coeff * self.fine_ring[(idx as usize) & 7];
                }
            }
            out.push(v / 16.0);
            self.out_k = k + 1;
        }
    }
}

/// Hard limiter followed by a delay-line product detector, evaluated exactly
/// in continuous time.
///
/// The limiter is a Schmitt comparator (hysteresis `th`) whose switching times
/// are interpolated to sub-sample accuracy; the product of the limited wave
/// with itself delayed by `delay` samples is piecewise constant, so it is
/// integrated over each sample interval (area sampling). This avoids the
/// edge-timing quantisation (about 6.7 ns at 43 MS/s, ~0.16 rad of phase noise
/// at 3.9 MHz) that clipping the sampled waveform would add, and has no aliased
/// limiter harmonics. For an ideal square wave the output is `1 - 4 f tau`.
pub fn discriminate(x: &[f32], th: f32, delay: f64) -> Vec<f32> {
    assert!(!x.is_empty());
    let mut out = Vec::with_capacity(x.len());
    let mut disc = DiscStream::new(th, delay);
    disc.push(x, &mut out);
    disc.finish(&mut out);
    out
}

/// Idle value of the product detector (hard-limited) for carrier frequency `f`.
fn r_of_freq(f: f64) -> f32 {
    let tau = DEMOD_DELAY_NOMINAL / FS_RF;
    (1.0 - 4.0 * f * tau) as f32
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::standard::TvStandard;
    use std::f64::consts::PI;

    fn process_segment_reference(rf: &[f32], p: &DeckParams, g0: i64) -> SegmentOut {
        let n = rf.len();
        let mut x: Vec<f32> = rf.to_vec();

        // ---- playback equaliser and level ----
        for &(f, q, g) in &p.eq_peaks {
            let mut b = Biquad::peaking(f, q, g, FS_RF);
            b.process(&mut x);
        }
        let gain = p.fm_gain as f32;
        for v in x.iter_mut() {
            *v *= gain;
        }

        // ---- slow RF level control (feed-forward AGC on the mean rectified level) ----
        let reference = if p.agc_enabled {
            p.agc_target as f32
        } else {
            1.0
        };
        if p.agc_enabled {
            let alpha = (1.0 - (-1.0 / (p.agc_ms * 1e-3 * FS_RF)).exp()) as f32;
            let k = n.min(AGC_INIT_RF);
            let mut avg = x[..k].iter().map(|v| v.abs()).sum::<f32>() / k as f32;
            let (gmin, gmax) = (0.25f32, p.agc_max_gain as f32);
            for v in x.iter_mut() {
                let a = v.abs();
                let g =
                    (reference / (avg * std::f32::consts::FRAC_PI_2).max(1e-6)).clamp(gmin, gmax);
                avg += alpha * (a - avg);
                *v *= g;
            }
        }

        // ---- dropout compensator: envelope detect, hysteresis, 1 H recirculation ----
        let mut doc = vec![0u8; n];
        if p.doc_enabled {
            let decay = (-1.0 / (0.35e-6 * FS_RF)).exp() as f32;
            let mut env = reference;
            let mut active = false;
            let mut hold = 0usize;
            let (trip, rel) = (
                p.doc_trip as f32 * reference,
                p.doc_release as f32 * reference,
            );
            let rec = p.doc_recirculation as f32;
            for i in 0..n {
                env = (x[i].abs()).max(env * decay);
                if !active && env < trip {
                    active = true;
                } else if active && env > rel {
                    if hold == 0 {
                        hold = rf_samples(6);
                    }
                    hold -= 1;
                    if hold == 0 {
                        active = false;
                    }
                } else if active {
                    hold = 0;
                }
                if active && i >= RF_PER_LINE {
                    x[i] = x[i - RF_PER_LINE] * rec;
                    doc[i] = 1;
                }
            }
        }

        // ---- colour-under branch: tapped before the limiter ----
        let nb = n / OS;
        let mut cu_rf = x.clone();
        butter_lp(4, p.chroma_lpf_hz, FS_RF).process(&mut cu_rf);
        let mut cu: Vec<f32> = (0..nb)
            .map(|k| {
                let g = &cu_rf[OS * k..OS * (k + 1)];
                let mut sum = g[0];
                for v in &g[1..] {
                    sum += *v;
                }
                sum / OS as f32
            })
            .collect();
        let mut clpf = butter_lp(4, p.chroma_lpf_hz, FS_BB);
        clpf.process(&mut cu);
        // chroma is killed while the compensator is active
        let mut dropout = vec![0u8; nb];
        for k in 0..nb {
            if doc[OS * k..OS * (k + 1)].iter().any(|&d| d != 0) {
                dropout[k] = 1;
            }
        }
        // spread the kill a little (filter memory)
        for k in 0..nb {
            if dropout[k] == 1 {
                cu[k] = 0.0;
            }
        }

        // ---- limiter + delay-line discriminator ----
        let mut fmx = x.clone();
        butter_hp(4, p.fm_hpf_hz, FS_RF).process(&mut fmx);
        let mut prod = discriminate(&fmx, p.limiter_threshold as f32, p.demod_delay);
        let mut lpf = butter_lp(p.demod_lpf_order, p.demod_lpf_hz, FS_RF);
        // the discriminator idles at the sync-tip... settle on the 0 IRE level
        lpf.settle(r_of_freq(F_SYNC_TIP - SYNC_IRE as f64 * HZ_PER_IRE) as f64);
        lpf.process(&mut prod);

        // product -> frequency -> IRE using the *nominal* design delay
        let tau = DEMOD_DELAY_NOMINAL / FS_RF;
        let mut y = vec![0.0f32; n];
        for i in 0..n {
            let f = (1.0 - prod[i] as f64) / (4.0 * tau);
            y[i] = ((f - F_SYNC_TIP) / HZ_PER_IRE + SYNC_IRE as f64) as f32;
        }

        // ---- de-emphasis (inverse of the record emphasis, with component drift), sub-deemphasis ----
        let t = PRE_EMPH_TAU * p.deemph_tau_scale;
        let ratio = PRE_EMPH_RATIO * p.deemph_ratio_scale;
        let mut de = Biquad::first_order_pz(ratio / t, 1.0 / t, FS_RF);
        de.settle(y[0] as f64);
        de.process(&mut y);
        // LP and EP: undo the sub-preemphasis, which was applied before the main emphasis
        if let Some(se) = &p.sub_deemphasis {
            let mut sub = SubDeemphasis::new(se, FS_RF);
            sub.settle(y[0] as f64);
            sub.process(&mut y);
        }

        // ---- coupling capacitor (high-pass), luma low-pass, aperture corrector ----
        let mut cpl = Biquad::one_pole_hp(p.coupling_hpf_hz, FS_RF);
        let mid =
            y[..y.len().min(RF_PER_LINE)].iter().sum::<f32>() / y.len().min(RF_PER_LINE) as f32;
        cpl.settle(mid as f64);
        // DC is restored by the TV clamp, so the high-pass only needs to shape tilt:
        for v in y.iter_mut() {
            *v = (cpl.tick(*v as f64) as f32) + mid;
        }
        // decimate to baseband, then the video low-pass (zero-phase FIR) and the
        // aperture corrector
        let mut lb: Vec<f32> = (0..nb).map(|k| y[OS * k + OS / 2]).collect();
        let fir = kaiser_lowpass(p.luma_lpf_hz, p.luma_lpf_transition_hz, 55.0, FS_BB);
        lb = fir_centred(&lb, &fir);
        if p.sharp_db != 0.0 {
            let mut sh = Biquad::peaking(p.sharp_freq, p.sharp_q, p.sharp_db, FS_BB);
            sh.settle(mid as f64);
            sh.process(&mut lb);
        }

        // ---- supply disturbances and detuning ----
        let have_supply = p.supply.hum_ire != 0.0 || p.supply.smps_ire != 0.0;
        let mut luma = Vec::with_capacity(nb);
        for k in 0..nb {
            let mut v = lb[k];
            if have_supply {
                let (add, gm) = supply_disturbance(&p.supply, g0 + (OS * k + OS / 2) as i64);
                v = v * (1.0 + gm) + add;
            }
            luma.push(v + p.dc_error_ire as f32);
        }
        SegmentOut {
            luma,
            cunder: cu,
            dropout,
        }
    }

    /// These tests pin the figures of the 525-line format; the other engine runs the same
    /// file against its own constants, so they return early there.
    const NTSC: bool = matches!(STANDARD, TvStandard::Ntsc);

    fn synthetic_rf(n: usize) -> Vec<f32> {
        let mut state = 0x1234_5678u32;
        let mut phase = 0.0f64;
        (0..n)
            .map(|i| {
                state = state.wrapping_mul(1_664_525).wrapping_add(1_013_904_223);
                let noise = ((state >> 8) as f32 / 16_777_216.0 - 0.5) * 0.025;
                let f = 3.9e6 + 80.0e3 * (2.0 * PI * i as f64 / (FS_RF * 0.002)).sin();
                phase += 2.0 * PI * f / FS_RF;
                let dip = [
                    (200, 450),
                    (900, 1_150),
                    (RF_PER_LINE + 100, RF_PER_LINE + 500),
                    (2 * RF_PER_LINE + 500, 2 * RF_PER_LINE + 3_500),
                    (20_000, 23_000),
                ];
                let amplitude = if dip.iter().any(|&(a, b)| i >= a && i < b) {
                    0.05
                } else {
                    1.0
                };
                (amplitude * phase.sin() as f32 + noise) * 0.5
            })
            .collect()
    }

    fn assert_segment_bits(a: &SegmentOut, b: &SegmentOut) {
        assert_eq!(a.luma.len(), b.luma.len());
        assert_eq!(a.cunder.len(), b.cunder.len());
        for (x, y) in a.luma.iter().zip(&b.luma) {
            assert_eq!(x.to_bits(), y.to_bits());
        }
        for (x, y) in a.cunder.iter().zip(&b.cunder) {
            assert_eq!(x.to_bits(), y.to_bits());
        }
        assert_eq!(a.dropout, b.dropout);
    }

    #[test]
    fn fused_process_matches_reference_exactly() {
        let variants = vec![
            DeckParams::default(),
            DeckParams {
                agc_enabled: false,
                ..Default::default()
            },
            DeckParams {
                doc_enabled: false,
                ..Default::default()
            },
            DeckParams {
                sub_deemphasis: Some(
                    super::super::emphasis::SubEmphasisParams::for_speed(
                        super::super::format::TapeSpeed::Lp,
                    )
                    .unwrap_or_default(),
                ),
                ..Default::default()
            },
            DeckParams {
                supply: SupplyParams {
                    hum_ire: 2.0,
                    mains_hz: 60.0,
                    ripple_peakiness: 0.7,
                    smps_hz: 180_000.0,
                    smps_ire: 0.8,
                    phase: 0.3,
                },
                ..Default::default()
            },
            DeckParams {
                eq_peaks: vec![(4.8e6, 0.8, 3.0), (6.1e6, 1.1, -2.0), (8.0e6, 1.4, 4.0)],
                ..Default::default()
            },
            DeckParams {
                eq_peaks: vec![],
                ..Default::default()
            },
        ];
        for n in [
            3_000,
            4_000,
            4_001,
            RF_PER_LINE * 3,
            DECK_PIECE_RF - 3,
            DECK_PIECE_RF,
            DECK_PIECE_RF + 1,
            DECK_PIECE_RF + 3,
            2 * DECK_PIECE_RF + 4000,
            60_000,
            727_545,
        ] {
            let rf = synthetic_rf(n);
            for p in &variants {
                assert_segment_bits(
                    &process_segment(&rf, p, 1234),
                    &process_segment_reference(&rf, p, 1234),
                );
            }
        }
    }

    fn discriminate_reference(x: &[f32], th: f32, delay: f64) -> Vec<f32> {
        let n = x.len();
        let mut edges: Vec<f64> = Vec::with_capacity(n / 4);
        let mut state: i8 = if x[0] >= 0.0 { 1 } else { -1 };
        for i in 1..n {
            let (a, b) = (x[i - 1], x[i]);
            if state < 0 && b > th {
                edges.push((i - 1) as f64 + ((th - a) / (b - a)) as f64);
                state = 1;
            } else if state > 0 && b < -th {
                edges.push((i - 1) as f64 + ((-th - a) / (b - a)) as f64);
                state = -1;
            }
        }
        const SUB: usize = 4;
        let nb = n * SUB;
        let mut fine = vec![0.0f32; nb];
        let (mut i, mut j) = (0usize, 0usize);
        let mut sign = 1.0f32;
        let mut t_prev = 0.0f64;
        let mut acc = 0.0f64;
        let mut cur = 0usize;
        let w = 1.0 / SUB as f64;
        let next_edge = |i: usize, j: usize| -> Option<(f64, bool)> {
            let a = edges.get(i).copied();
            let b = edges.get(j).map(|e| e + delay);
            match (a, b) {
                (Some(a), Some(b)) => Some(if a <= b { (a, true) } else { (b, false) }),
                (Some(a), None) => Some((a, true)),
                (None, Some(b)) => Some((b, false)),
                (None, None) => None,
            }
        };
        loop {
            let (te, first) = next_edge(i, j).unwrap_or((n as f64, true));
            let te = te.min(n as f64);
            let mut t = t_prev;
            while t < te {
                let end = ((cur + 1) as f64 * w).min(te);
                acc += sign as f64 * (end - t);
                t = end;
                if t >= (cur + 1) as f64 * w {
                    if cur < nb {
                        fine[cur] = (acc / w) as f32;
                    }
                    acc = 0.0;
                    cur += 1;
                }
            }
            t_prev = te;
            if te >= n as f64 {
                break;
            }
            sign = -sign;
            if first {
                i += 1
            } else {
                j += 1
            }
        }
        let tri = [1.0f32, 2.0, 3.0, 4.0, 3.0, 2.0, 1.0];
        let mut out = vec![0.0f32; n];
        for (k, o) in out.iter_mut().enumerate() {
            let centre = k * SUB + SUB / 2;
            let mut v = 0.0;
            for (m, &c) in tri.iter().enumerate() {
                let idx = (centre + m) as i64 - 3;
                if idx >= 0 && (idx as usize) < nb {
                    v += c * fine[idx as usize];
                }
            }
            *o = v / 16.0;
        }
        out
    }

    #[test]
    fn stream_discriminator_matches_reference_exactly() {
        let mut x = synthetic_rf(40_000);
        butter_hp(4, 1.8e6, FS_RF).process(&mut x);
        for &(th, delay) in &[(0.03f32, 3.0f64), (0.0, 2.5), (0.05, 4.7)] {
            for len in [1usize, 2, 7, 100, 40_000] {
                let slice = &x[..len];
                let want = discriminate_reference(slice, th, delay);
                for chunk in [1usize, 5, 4096, len] {
                    let mut stream = DiscStream::new(th, delay);
                    let mut got = Vec::with_capacity(len);
                    for part in slice.chunks(chunk) {
                        stream.push(part, &mut got);
                    }
                    stream.finish(&mut got);
                    assert_eq!(got.len(), want.len(), "th={th} delay={delay} len={len} chunk={chunk}");
                    for (idx, (a, b)) in got.iter().zip(&want).enumerate() {
                        assert_eq!(
                            a.to_bits(),
                            b.to_bits(),
                            "th={th} delay={delay} len={len} chunk={chunk} idx={idx}"
                        );
                    }
                }
            }
        }
    }

    #[test]
    #[ignore]
    fn measure_fused_process_segment() {
        use rayon::prelude::*;
        use std::hint::black_box;
        use std::time::Instant;

        let rf = synthetic_rf(727_545);
        let p = DeckParams::default();
        let threads = rayon::current_num_threads();

        let before = Instant::now();
        let reference = black_box(process_segment_reference(&rf, &p, 0));
        let before_alone = before.elapsed();

        let after = Instant::now();
        let fused = black_box(process_segment(&rf, &p, 0));
        let after_alone = after.elapsed();

        assert_segment_bits(&fused, &reference);

        let before_par = Instant::now();
        (0..threads).into_par_iter().for_each(|_| {
            black_box(process_segment_reference(&rf, &p, 0));
        });
        let before_all_cores = before_par.elapsed();

        let after_par = Instant::now();
        (0..threads).into_par_iter().for_each(|_| {
            black_box(process_segment(&rf, &p, 0));
        });
        let after_all_cores = after_par.elapsed();

        eprintln!(
            "alone: reference {before_alone:?}, stream {after_alone:?}; \
             {threads} threads: reference {before_all_cores:?}, stream {after_all_cores:?}"
        );
    }

    /// A pure FM carrier at a known frequency must demodulate to the matching
    /// IRE level through limiter + delay-line discriminator.
    #[test]
    fn discriminator_maps_frequency_to_ire() {
        if !NTSC {
            return;
        }
        for ire in [-40.0, 0.0, 50.0, 100.0] {
            let f = F_SYNC_TIP + (ire + 40.0) * HZ_PER_IRE;
            let n = 20_000;
            let rf: Vec<f32> = (0..n)
                .map(|i| (0.5 * (2.0 * PI * f * i as f64 / FS_RF).sin()) as f32)
                .collect();
            let p = DeckParams {
                eq_peaks: vec![],
                sharp_db: 0.0,
                doc_enabled: false,
                fm_gain: 2.0,
                ..Default::default()
            };
            let out = process_segment(&rf, &p, 0);
            let mid = &out.luma[n / OS / 2..n / OS / 2 + 200];
            let m = mid.iter().sum::<f32>() / mid.len() as f32;
            assert!((m as f64 - ire).abs() < 2.5, "ire {ire}: got {m}");
        }
    }

    /// Exact discriminator: for an ideal square wave the DC output is 1 - 4 f tau
    /// at every frequency, with no timing quantisation spurs.
    #[test]
    fn discriminator_is_exact_and_spur_free() {
        for f in [3.4e6, 3.9e6, 4.4e6] {
            let n = 40_000;
            let x: Vec<f32> = (0..n)
                .map(|i| (2.0 * PI * f * i as f64 / FS_RF).sin() as f32)
                .collect();
            let d = discriminate(&x, 0.03, 3.0);
            let mid = &d[2000..n - 2000];
            let mean = mid.iter().sum::<f32>() / mid.len() as f32;
            let want = 1.0 - 4.0 * f * 3.0 / FS_RF;
            // LPF the product so only the in-band content remains, then check ripple
            let mut lp = butter_lp(8, 2.5e6, FS_RF); // -70 dB at 2 x carrier
            let mut y = d.clone();
            lp.process(&mut y);
            let r = &y[4000..n - 2000];
            let rip = r.iter().cloned().fold(f32::MIN, f32::max)
                - r.iter().cloned().fold(f32::MAX, f32::min);
            assert!((mean as f64 - want).abs() < 2e-3, "f={f}: {mean} vs {want}");
            assert!(rip < 4e-3, "f={f}: ripple {rip}"); // < 2 IRE p-p, dominated by one -61 dB alias spur
        }
    }

    /// With a stable carrier lost for a while, the compensator must switch in
    /// the previous line (so the output stays near the true level).
    #[test]
    fn dropout_compensator_repairs_a_dropout() {
        if !NTSC {
            return;
        }
        let f = 3.9e6;
        let n = 6 * RF_PER_LINE;
        let mut rf: Vec<f32> = (0..n)
            .map(|i| (0.5 * (2.0 * PI * f * i as f64 / FS_RF).sin()) as f32)
            .collect();
        let (a, b) = (3 * RF_PER_LINE + 500, 3 * RF_PER_LINE + 1700);
        for v in &mut rf[a..b] {
            *v *= 0.01;
        }
        let base = DeckParams {
            eq_peaks: vec![],
            sharp_db: 0.0,
            fm_gain: 2.0,
            ..Default::default()
        };
        let with = process_segment(
            &rf,
            &DeckParams {
                doc_enabled: true,
                ..base.clone()
            },
            0,
        );
        let without = process_segment(
            &rf,
            &DeckParams {
                doc_enabled: false,
                ..base
            },
            0,
        );
        let dev = |o: &SegmentOut| {
            let r = &o.luma[(a + 200) / OS..(b - 200) / OS];
            r.iter().map(|v| (v - 30.0).abs()).sum::<f32>() / r.len() as f32
        };
        // 3.9 MHz carrier = 30 IRE
        assert!(
            dev(&with) < 3.0,
            "with DOC the dropout region stays near 30 IRE: {}",
            dev(&with)
        );
        assert!(
            dev(&without) > 3.0 * dev(&with),
            "doc {} vs none {}",
            dev(&with),
            dev(&without)
        );
        assert!(with.dropout[(a + 50) / OS] == 1);
    }
}
