//! The camera's electronics on the line signal: head-amplifier noise and band, clamp, gain, knee,
//! gamma, aperture correction with coring, clip (docs/design/CAMERA_SCAN.md, "Camera electronics").

use vhsphys_core::camera::{CameraParams, CameraTube, TargetSignal};
use vhsphys_core::camera_processing::{
    gamma_correct, gamma_inverse, horizontal_gain, CameraVideo, HeadAmplifier, Processing, VideoProcessor, BLACK_CLIP,
    WHITE_CLIP,
};
use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W, FS_BB};
use vhsphys_core::tube::{oetf, TubeChannel, TubeKind, BEAM_RATIO_DEFAULT};
use vhsphys_core::video::Frame;

/// Elementary charge, C.
const Q_E: f64 = 1.602176634e-19;
/// Boltzmann constant, J/K.
const K_B: f64 = 1.380649e-23;
/// nA^2 per A^2.
const NA2_PER_A2: f64 = 1e18;
/// A dark target's current, nA: dark and bias current of the order of the tubes'.
const DARK_LEVEL_NA: f64 = 3.0;
/// Green's peak-white current, nA.
const GREEN_PEAK_NA: f64 = 300.0;
/// Upper edge of the band the noise figures are quoted in, Hz.
const NOISE_BAND_HZ: f64 = 5e6;
/// Where the spectrum's shape is compared: amplitude rising with frequency puts most of the power
/// in the upper half of the band.
const MID_BAND_HZ: f64 = 2.5e6;
/// Bands, Hz, in which the measured density is checked against the formula.
const CHECK_BANDS_HZ: [(f64, f64); 5] = [(0.5e6, 1.5e6), (1.5e6, 2.5e6), (2.5e6, 3.5e6), (3.5e6, 4.5e6), (4.5e6, 5.0e6)];
/// Every fourth row is enough for the spectra.
const ROW_STEP: usize = 4;
/// Samples left out at each line end where a measurement must not see the ends.
const LINE_MARGIN: usize = 24;
/// Amplitude of the test sine in the head-amplifier band test, nA, and the level it rides on.
const SINE_NA: f64 = 100.0;
const SINE_LEVEL_NA: f64 = 150.0;
/// Frequencies of the band test, Hz.
const BAND_TEST_HZ: [f64; 4] = [1e6, 3e6, 5e6, 6e6];
/// A highlight as narrow as the beam spot (sigma 40 ns, in samples) and its peak, nA.
const PULSE_SIGMA_SAMPLES: f64 = 0.57;
const PULSE_PEAK_NA: f64 = 235.0;
/// Samples from the pulse over which the amplifier's response must have died away, and how far it
/// may stray from the baseline there as a share of the peak (the noise is about 1 nA).
const PULSE_QUIET_FROM: usize = 3;
const PULSE_QUIET_TO: usize = 12;
const PULSE_RING_LIMIT: f64 = 0.025;
/// Mid grey of the detail tests in the gamma-corrected signal, and the detail's amplitude.
const DETAIL_GREY: f64 = 0.5;
const DETAIL_AMPLITUDE: f64 = 0.2;
/// Amplitude of a detail smaller than the studio coring threshold (its `d_h` peaks at twice it).
const FINE_AMPLITUDE: f64 = 0.002;
/// Frequencies of the horizontal corrector test, Hz (the peak `1 / (2T)` is added per profile).
const DETAIL_TEST_HZ: [f64; 4] = [0.5e6, 1e6, 2e6, 4.5e6];
/// Highlight test: background and bar in linear light, bar position in samples.
const HIGHLIGHT_BACKGROUND: f64 = 0.2;
const HIGHLIGHT_BAR: f64 = 4.0;
const BAR_START: usize = 300;
const BAR_END: usize = 340;
/// Samples after the bar in which its dark edge lies.
const EDGE_SPAN: usize = 10;
/// Rows left out at the top and bottom where a whole-frame check must not see the raster edge.
const EDGE_ROWS: usize = 8;
/// Frames run before a camera measurement so the target has settled.
const SETTLE_FRAMES: usize = 3;

fn plumbicon() -> [TubeChannel; 3] {
    TubeKind::Plumbicon.channels(BEAM_RATIO_DEFAULT).unwrap()
}

fn black_of(ch: &[TubeChannel; 3]) -> [f64; 3] {
    std::array::from_fn(|k| ch[k].dark_na + ch[k].bias_na)
}

/// A frame of signal whose sample `(row, x)` of channel `k` is `f(k, row, x)` nA.
fn signal(f: impl Fn(usize, usize, usize) -> f64) -> TargetSignal {
    TargetSignal {
        rows: (0..ACTIVE_H).map(|r| std::array::from_fn(|k| (0..ACTIVE_W).map(|x| f(k, r, x) as f32).collect())).collect(),
    }
}

fn flat_signal(level: [f64; 3]) -> TargetSignal {
    signal(|k, _, _| level[k])
}

/// The current that reads `v` after clamp, gain and gamma.
fn current_for(ch: &[TubeChannel; 3], black: &[f64; 3], k: usize, v: f64) -> f64 {
    black[k] + ch[k].peak_signal_na * gamma_inverse(v)
}

/// The current that reads linear light `l` after clamp and gain.
fn current_for_linear(ch: &[TubeChannel; 3], black: &[f64; 3], k: usize, l: f64) -> f64 {
    black[k] + ch[k].peak_signal_na * l
}

/// The head amplifier's magnitude: an analog second-order Butterworth low-pass.
fn amp_response(band_hz: f64, f: f64) -> f64 {
    1.0 / (1.0 + (f / band_hz).powi(4)).sqrt()
}

/// One-sided noise density of the head amplifier's formula at the output, nA^2/Hz.
fn expected_density(p: Processing, current_na: f64, f: f64) -> f64 {
    let s = p.spec().head_amp;
    let shot = 2.0 * Q_E * current_na * 1e-9;
    let feedback = 4.0 * K_B * s.temp_k / s.r_f_ohm;
    let fet = (2.0 * std::f64::consts::PI * f * s.input_c_f).powi(2) * 4.0 * K_B * s.temp_k * s.r_n_ohm;
    (shot + feedback + fet) * NA2_PER_A2 * amp_response(s.band_hz, f).powi(2)
}

/// Noise power per DFT bin of the rows, averaged over rows: index `k` holds bin `k`'s power, nA^2
/// (the one-sided bins sum to the variance).
fn bin_powers(sig: &TargetSignal, ch: usize, level: f64) -> Vec<f64> {
    let n = ACTIVE_W;
    let kmax = n / 2;
    let mut acc = vec![0.0; kmax];
    let mut rows = 0.0;
    for r in (0..ACTIVE_H).step_by(ROW_STEP) {
        let x: Vec<f64> = sig.rows[r][ch].iter().map(|&v| v as f64 - level).collect();
        for k in 1..kmax {
            let w = 2.0 * std::f64::consts::PI * k as f64 / n as f64;
            let (mut re, mut im) = (0.0, 0.0);
            for (i, v) in x.iter().enumerate() {
                let a = w * i as f64;
                re += v * a.cos();
                im -= v * a.sin();
            }
            acc[k] += 2.0 * (re * re + im * im) / (n * n) as f64;
        }
        rows += 1.0;
    }
    acc.iter().map(|a| a / rows).collect()
}

fn bin_hz(k: usize) -> f64 {
    k as f64 * FS_BB / ACTIVE_W as f64
}

fn band_sum(bins: &[f64], lo: f64, hi: f64) -> f64 {
    bins.iter().enumerate().filter(|(k, _)| *k > 0 && bin_hz(*k) >= lo && bin_hz(*k) < hi).map(|(_, p)| p).sum()
}

fn expected_band(p: Processing, current_na: f64, lo: f64, hi: f64) -> f64 {
    let df = FS_BB / ACTIVE_W as f64;
    (1..ACTIVE_W / 2).map(bin_hz).filter(|f| *f >= lo && *f < hi).map(|f| expected_density(p, current_na, f) * df).sum()
}

fn amplified(p: Processing, level: [f64; 3], seed: u64, frame: i64) -> TargetSignal {
    let mut sig = flat_signal(level);
    HeadAmplifier::new(&p.spec().head_amp, seed).amplify(&mut sig, frame);
    sig
}

fn noise_rms(p: Processing, current_na: f64) -> f64 {
    let sig = amplified(p, [current_na; 3], 1, 0);
    band_sum(&bin_powers(&sig, 1, current_na), 0.0, NOISE_BAND_HZ).sqrt()
}

#[test]
fn the_profiles_use_the_published_amplifier_figures() {
    for p in [Processing::Studio, Processing::Consumer] {
        let h = p.spec().head_amp;
        assert!((15e-12..=30e-12).contains(&h.input_c_f), "{p:?} C_in {}", h.input_c_f);
        assert!((50.0..=100.0).contains(&h.r_n_ohm), "{p:?} R_n {}", h.r_n_ohm);
        assert!((300e3..=1e6).contains(&h.r_f_ohm), "{p:?} R_f {}", h.r_f_ohm);
        assert!((5e6..=6e6).contains(&h.band_hz), "{p:?} band {}", h.band_hz);
        assert!(p.spec().aperture.coring > 0.0);
    }
    let studio = Processing::Studio.spec();
    assert_eq!(studio.knee_range, Some(2.3));
    assert!((studio.aperture.delay_s - 150e-9).abs() < 1e-12);
    assert_eq!(studio.aperture.vertical, 0.5);
    let consumer = Processing::Consumer.spec();
    assert_eq!(consumer.knee_range, None);
    assert!((consumer.aperture.delay_s - 200e-9).abs() < 1e-12);
    assert_eq!(consumer.aperture.vertical, 0.0);
    assert_eq!(Processing::default(), Processing::Studio);
}

/// The measured density follows `S(f) |H(f)|^2` band by band, for both profiles and every channel.
#[test]
fn head_amplifier_noise_follows_its_formula() {
    for p in [Processing::Studio, Processing::Consumer] {
        let sig = amplified(p, [DARK_LEVEL_NA; 3], 7, 3);
        for ch in 0..3 {
            let bins = bin_powers(&sig, ch, DARK_LEVEL_NA);
            let got = band_sum(&bins, 0.0, NOISE_BAND_HZ);
            let want = expected_band(p, DARK_LEVEL_NA, 0.0, NOISE_BAND_HZ);
            assert!((got / want - 1.0).abs() < 0.08, "{p:?} ch{ch}: {got:.3} nA^2 against {want:.3}");
            if ch == 1 {
                for (lo, hi) in CHECK_BANDS_HZ {
                    let (g, w) = (band_sum(&bins, lo, hi), expected_band(p, DARK_LEVEL_NA, lo, hi));
                    assert!((g / w - 1.0).abs() < 0.15, "{p:?} {:.1}-{:.1} MHz: {g:.4} against {w:.4}", lo / 1e6, hi / 1e6);
                }
            }
        }
    }
}

/// "About 1 nA, as low as 0.5 nA" for a good preamplifier, with most of the power in the upper
/// half of a 5 MHz band (the triangular spectrum).
#[test]
fn the_studio_noise_at_black_is_triangular_and_about_a_nanoampere() {
    let sig = amplified(Processing::Studio, [DARK_LEVEL_NA; 3], 1, 0);
    let bins = bin_powers(&sig, 1, DARK_LEVEL_NA);
    let total = band_sum(&bins, 0.0, NOISE_BAND_HZ);
    let rms = total.sqrt();
    assert!((0.5..=1.6).contains(&rms), "rms {rms:.3} nA");
    let upper = band_sum(&bins, MID_BAND_HZ, NOISE_BAND_HZ) / total;
    assert!(upper >= 0.75, "share above 2.5 MHz {upper:.3}");
    let (lo, hi) = (CHECK_BANDS_HZ[0], (4e6, 5e6));
    let density = |b: (f64, f64)| band_sum(&bins, b.0, b.1) / (b.1 - b.0);
    let ratio = density(hi) / density(lo);
    assert!(ratio >= 5.0, "density at 4-5 MHz over 0.5-1.5 MHz {ratio:.2}");
}

/// The signal current brings its own shot noise, `2 q I`.
#[test]
fn the_signal_carries_its_own_shot_noise() {
    let p = Processing::Studio;
    let bright = band_sum(&bin_powers(&amplified(p, [GREEN_PEAK_NA; 3], 2, 0), 1, GREEN_PEAK_NA), 0.0, NOISE_BAND_HZ);
    let dark = band_sum(&bin_powers(&amplified(p, [DARK_LEVEL_NA; 3], 2, 0), 1, DARK_LEVEL_NA), 0.0, NOISE_BAND_HZ);
    let want = expected_band(p, GREEN_PEAK_NA, 0.0, NOISE_BAND_HZ) - expected_band(p, DARK_LEVEL_NA, 0.0, NOISE_BAND_HZ);
    let got = bright - dark;
    assert!((got / want - 1.0).abs() < 0.25, "shot noise {got:.3} nA^2 against {want:.3}");
}

/// Green at peak white: the studio camera near 49 dB, the consumer camera near the "> 45 dB" of
/// a home camera of the period.
#[test]
fn signal_to_noise_at_peak_white() {
    let sn = |p| 20.0 * (GREEN_PEAK_NA / noise_rms(p, GREEN_PEAK_NA)).log10();
    let (studio, consumer) = (sn(Processing::Studio), sn(Processing::Consumer));
    assert!(studio >= 48.0, "studio {studio:.1} dB");
    assert!((44.0..=48.0).contains(&consumer), "consumer {consumer:.1} dB");
    assert!(studio - consumer >= 3.0, "studio {studio:.1} dB, consumer {consumer:.1} dB");
}

/// Amplitude of the component at `f` of `row[LINE_MARGIN..W - LINE_MARGIN]` about `level`.
fn amplitude(row: &[f32], level: f64, f: f64) -> f64 {
    let w = 2.0 * std::f64::consts::PI * f / FS_BB;
    let (mut re, mut im, mut n) = (0.0, 0.0, 0.0);
    for (i, &v) in row.iter().enumerate().take(ACTIVE_W - LINE_MARGIN).skip(LINE_MARGIN) {
        let a = w * i as f64;
        re += (v as f64 - level) * a.cos();
        im += (v as f64 - level) * a.sin();
        n += 1.0;
    }
    2.0 * (re * re + im * im).sqrt() / n
}

#[test]
fn the_head_amplifier_band_is_the_butterworth() {
    let p = Processing::Studio;
    let band = p.spec().head_amp.band_hz;
    for f in BAND_TEST_HZ {
        let mut sig = signal(|_, _, x| SINE_LEVEL_NA + SINE_NA * (2.0 * std::f64::consts::PI * f * x as f64 / FS_BB).sin());
        HeadAmplifier::new(&p.spec().head_amp, 3).amplify(&mut sig, 0);
        let rows: Vec<usize> = (EDGE_ROWS..ACTIVE_H - EDGE_ROWS).step_by(ROW_STEP).collect();
        let got = rows.iter().map(|&r| amplitude(&sig.rows[r][1], SINE_LEVEL_NA, f)).sum::<f64>() / rows.len() as f64 / SINE_NA;
        let want = amp_response(band, f);
        assert!((got - want).abs() < 0.02, "{:.0} MHz: {got:.3} against {want:.3}", f / 1e6);
    }
}

/// A highlight narrower than the band does not ring: past its own spread the amplifier's response
/// stays near the baseline, as the analog amplifier's does (its impulse response falls by e^-1.7
/// per sample at this rate), and the pulse stays where it was.
#[test]
fn a_highlight_does_not_ring_through_the_head_amplifier() {
    let p = Processing::Studio;
    let centre_x = ACTIVE_W / 2;
    let pulse = |x: usize| PULSE_PEAK_NA * (-0.5 * ((x as f64 - centre_x as f64) / PULSE_SIGMA_SAMPLES).powi(2)).exp();
    let mut sig = signal(|_, _, x| DARK_LEVEL_NA + pulse(x));
    HeadAmplifier::new(&p.spec().head_amp, 4).amplify(&mut sig, 0);
    for r in (EDGE_ROWS..ACTIVE_H - EDGE_ROWS).step_by(ROW_STEP * 4) {
        let row = &sig.rows[r][1];
        let peak = row[centre_x] as f64;
        assert!(peak > 0.8 * PULSE_PEAK_NA, "row {r}: the pulse reads {peak:.1}");
        assert!(peak >= (row[centre_x + 1] as f64).max(row[centre_x - 1] as f64), "row {r}: the pulse moved");
        for d in PULSE_QUIET_FROM..PULSE_QUIET_TO {
            for x in [centre_x + d, centre_x - d] {
                let off = (row[x] as f64 - DARK_LEVEL_NA).abs();
                assert!(off < PULSE_RING_LIMIT * PULSE_PEAK_NA, "row {r}, {d} samples from the pulse: {off:.1} nA");
            }
        }
    }
}

fn correlation(a: &[f32], b: &[f32], level: f64) -> f64 {
    let (mut ab, mut aa, mut bb) = (0.0, 0.0, 0.0);
    for (x, y) in a.iter().zip(b) {
        let (x, y) = (*x as f64 - level, *y as f64 - level);
        ab += x * y;
        aa += x * x;
        bb += y * y;
    }
    ab / (aa * bb).sqrt()
}

#[test]
fn the_noise_is_fixed_by_seed_frame_row_and_channel_and_independent_between_them() {
    let p = Processing::Studio;
    let a = amplified(p, [DARK_LEVEL_NA; 3], 5, 10);
    assert!(a == amplified(p, [DARK_LEVEL_NA; 3], 5, 10), "the same seed and frame give the same noise");
    assert!(a != amplified(p, [DARK_LEVEL_NA; 3], 5, 11), "the next frame has new noise");
    assert!(a != amplified(p, [DARK_LEVEL_NA; 3], 6, 10), "another seed has other noise");
    let concat = |s: &TargetSignal, row_offset: usize, ch: usize| -> Vec<f32> {
        (0..ACTIVE_H - 1).flat_map(|r| s.rows[r + row_offset][ch].clone()).collect()
    };
    let rows_now = concat(&a, 0, 1);
    let rows_next = concat(&a, 1, 1);
    let red = concat(&a, 0, 0);
    let next_frame = amplified(p, [DARK_LEVEL_NA; 3], 5, 11);
    let green_next_frame = concat(&next_frame, 0, 1);
    for (what, other) in [("next row", &rows_next), ("red channel", &red), ("next frame", &green_next_frame)] {
        let c = correlation(&rows_now, other, DARK_LEVEL_NA);
        assert!(c.abs() < 0.05, "correlation with the {what}: {c:.3}");
    }
}

fn processor(p: Processing) -> ([TubeChannel; 3], [f64; 3], VideoProcessor) {
    let ch = plumbicon();
    let black = black_of(&ch);
    let proc = VideoProcessor::new(p, &ch, black);
    (ch, black, proc)
}

/// The value of channel `k` in the middle of the picture.
fn centre(v: &CameraVideo, k: usize) -> f64 {
    v.rows[ACTIVE_H / 2][k][ACTIVE_W / 2] as f64
}

fn flat_linear(ch: &[TubeChannel; 3], black: &[f64; 3], l: f64) -> TargetSignal {
    signal(|k, _, _| current_for_linear(ch, black, k, l))
}

#[test]
fn the_clamp_and_gain_put_black_at_zero_and_peak_white_at_one() {
    for p in [Processing::Studio, Processing::Consumer] {
        let (ch, black, proc) = processor(p);
        for (l, want) in [(0.0, 0.0), (1.0, 1.0)] {
            let v = proc.process(&flat_linear(&ch, &black, l));
            for r in EDGE_ROWS..ACTIVE_H - EDGE_ROWS {
                for k in 0..3 {
                    let worst = v.rows[r][k].iter().map(|&s| (s as f64 - want).abs()).fold(0.0, f64::max);
                    assert!(worst < 1e-4, "{p:?} linear {l} row {r} ch{k}: off by {worst}");
                }
            }
        }
    }
}

#[test]
fn the_gamma_corrector_is_the_045_law_carried_past_its_ends() {
    for l in [0.0, 0.01, 0.018, 0.1, 0.5, 1.0] {
        assert!((gamma_correct(l) - oetf(l)).abs() < 1e-12, "{l}");
    }
    assert!((gamma_correct(-0.004) + 0.018).abs() < 1e-12);
    assert!((gamma_correct(1.5) - (1.099 * 1.5f64.powf(0.45) - 0.099)).abs() < 1e-12);
    for v in [-0.02, 0.05, 0.3, 0.9, 1.05] {
        assert!((gamma_correct(gamma_inverse(v)) - v).abs() < 1e-9, "{v}");
    }
}

/// Linear to 100 % of rated light, 230 % reaching the white clip.
#[test]
fn the_studio_knee_takes_230_percent_to_the_white_clip() {
    let (ch, black, proc) = processor(Processing::Studio);
    let read = |l: f64| centre(&proc.process(&flat_linear(&ch, &black, l)), 1);
    assert!((read(0.5) - gamma_correct(0.5)).abs() < 1e-4);
    assert!((read(1.0) - 1.0).abs() < 1e-4);
    let mid = read(1.6);
    assert!(mid > 1.0 + 1e-3 && mid < WHITE_CLIP - 1e-3, "160 %: {mid}");
    assert!((read(2.3) - WHITE_CLIP).abs() < 1e-3, "230 %: {}", read(2.3));
    assert!((read(4.0) - WHITE_CLIP).abs() < 1e-6);
    let mut last = 0.0;
    for i in 0..=40 {
        let v = read(i as f64 * 0.075);
        assert!(v >= last - 1e-6, "not monotonic at {}", i as f64 * 0.075);
        last = v;
    }
}

#[test]
fn the_consumer_camera_clips_without_a_knee() {
    let (ch, black, proc) = processor(Processing::Consumer);
    let read = |l: f64| centre(&proc.process(&flat_linear(&ch, &black, l)), 1);
    assert!((read(1.05) - gamma_correct(1.05)).abs() < 1e-4);
    assert!(gamma_correct(1.05) < WHITE_CLIP);
    assert!((read(1.2) - WHITE_CLIP).abs() < 1e-6);
    assert!((read(1.6) - WHITE_CLIP).abs() < 1e-6);
}

/// Every row carries `DETAIL_GREY + a sin(2 pi f t)` in the gamma-corrected signal of the channels
/// in `lit`, the others carry `DETAIL_GREY`.
fn sine_rows(ch: &[TubeChannel; 3], black: &[f64; 3], f: f64, a: f64, lit: [bool; 3]) -> TargetSignal {
    signal(|k, _, x| {
        let s = if lit[k] { a * (2.0 * std::f64::consts::PI * f * x as f64 / FS_BB).sin() } else { 0.0 };
        current_for(ch, black, k, DETAIL_GREY + s)
    })
}

fn response(v: &CameraVideo, k: usize, f: f64, a: f64) -> f64 {
    let rows: Vec<usize> = (EDGE_ROWS..ACTIVE_H - EDGE_ROWS).step_by(ROW_STEP * 4).collect();
    rows.iter().map(|&r| amplitude(&v.rows[r][k], DETAIL_GREY, f)).sum::<f64>() / rows.len() as f64 / a
}

#[test]
fn the_horizontal_gain_restores_the_spot_at_the_corrector_peak() {
    let ch = plumbicon();
    for p in [Processing::Studio, Processing::Consumer] {
        let s = p.spec();
        let t = s.aperture.delay_s;
        let fp = 1.0 / (2.0 * t);
        let spot = (-2.0 * std::f64::consts::PI.powi(2) * ch[1].sigma_h_s.powi(2) * fp * fp).exp();
        let want = (1.0 / (spot * amp_response(s.head_amp.band_hz, fp)) - 1.0) / 2.0;
        let k = horizontal_gain(t, ch[1].sigma_h_s, s.head_amp.band_hz);
        assert!((k - want).abs() < 1e-9, "{p:?}: {k} against {want}");
        let (_, _, proc) = processor(p);
        assert!((proc.horizontal_gain() - want).abs() < 1e-9);
    }
}

#[test]
fn the_horizontal_corrector_lifts_detail_as_its_delay_line_sets() {
    for p in [Processing::Studio, Processing::Consumer] {
        let (ch, black, proc) = processor(p);
        let t = p.spec().aperture.delay_s;
        let k = proc.horizontal_gain();
        let mut freqs = DETAIL_TEST_HZ.to_vec();
        freqs.push(1.0 / (2.0 * t));
        for f in freqs {
            let v = proc.process(&sine_rows(&ch, &black, f, DETAIL_AMPLITUDE, [true; 3]));
            let want = 1.0 + k * (1.0 - (2.0 * std::f64::consts::PI * f * t).cos());
            for c in 0..3 {
                let got = response(&v, c, f, DETAIL_AMPLITUDE);
                assert!((got / want - 1.0).abs() < 0.02, "{p:?} ch{c} {:.2} MHz: {got:.3} against {want:.3}", f / 1e6);
            }
        }
    }
}

#[test]
fn detail_below_the_coring_threshold_is_left_alone() {
    let p = Processing::Studio;
    let (ch, black, proc) = processor(p);
    assert!(2.0 * FINE_AMPLITUDE < p.spec().aperture.coring);
    let f = 1.0 / (2.0 * p.spec().aperture.delay_s);
    let v = proc.process(&sine_rows(&ch, &black, f, FINE_AMPLITUDE, [true; 3]));
    let got = response(&v, 1, f, FINE_AMPLITUDE);
    assert!((got - 1.0).abs() < 0.01, "{got:.4}");
}

/// The corrector's 1H delays reach the lines of the same field: a pattern that alternates every
/// field line (every other frame row) is lifted, one that alternates every frame row is not seen.
#[test]
fn the_vertical_corrector_sees_the_field_lines() {
    for p in [Processing::Studio, Processing::Consumer] {
        let (ch, black, proc) = processor(p);
        let kv = p.spec().aperture.vertical;
        for (period_rows, want) in [(4usize, 1.0 + 2.0 * kv), (2usize, 1.0)] {
            let sign = |r: usize| if (r % period_rows) < period_rows / 2 { 1.0 } else { -1.0 };
            let v = proc.process(&signal(|k, r, _| current_for(&ch, &black, k, DETAIL_GREY + DETAIL_AMPLITUDE * sign(r))));
            for r in (ACTIVE_H / 2 - 4)..(ACTIVE_H / 2 + 4) {
                let got = (v.rows[r][1][ACTIVE_W / 2] as f64 - DETAIL_GREY) / (DETAIL_AMPLITUDE * sign(r));
                assert!((got / want - 1.0).abs() < 0.02, "{p:?} period {period_rows} row {r}: {got:.3} against {want:.3}");
            }
        }
    }
}

#[test]
fn the_detail_comes_from_green_and_goes_to_all_three() {
    let p = Processing::Studio;
    let (ch, black, proc) = processor(p);
    let t = p.spec().aperture.delay_s;
    let f = 1.0 / (2.0 * t);
    let k = proc.horizontal_gain();
    let green_only = proc.process(&sine_rows(&ch, &black, f, DETAIL_AMPLITUDE, [false, true, false]));
    for c in [0, 2] {
        let got = response(&green_only, c, f, DETAIL_AMPLITUDE);
        let want = 2.0 * k;
        assert!((got / want - 1.0).abs() < 0.04, "ch{c} from green detail: {got:.3} against {want:.3}");
    }
    let red_only = proc.process(&sine_rows(&ch, &black, f, DETAIL_AMPLITUDE, [true, false, false]));
    let got = response(&red_only, 0, f, DETAIL_AMPLITUDE);
    assert!((got - 1.0).abs() < 0.01, "red detail is not lifted: {got:.3}");
    let r = ACTIVE_H / 2;
    let green_spread = red_only.rows[r][1].iter().map(|&v| (v as f64 - DETAIL_GREY).abs()).fold(0.0, f64::max);
    assert!(green_spread < 1e-4, "green picks up red detail: {green_spread}");
}

fn bar(ch: &[TubeChannel; 3], black: &[f64; 3], background: f64) -> TargetSignal {
    signal(|k, _, x| {
        let l = if (BAR_START..BAR_END).contains(&x) { HIGHLIGHT_BAR } else { background };
        current_for_linear(ch, black, k, l)
    })
}

/// The corrector's overshoot on a highlight is clipped at white, its undershoot stays: a dark edge
/// beside the highlight.
#[test]
fn a_highlight_keeps_its_dark_edge_and_loses_its_bright_one() {
    let (ch, black, proc) = processor(Processing::Studio);
    let v = proc.process(&bar(&ch, &black, HIGHLIGHT_BACKGROUND));
    let row = &v.rows[ACTIVE_H / 2][1];
    let mid = row[(BAR_START + BAR_END) / 2] as f64;
    assert!((mid - WHITE_CLIP).abs() < 1e-6, "bar {mid}");
    assert!(row.iter().all(|&s| (s as f64) <= WHITE_CLIP + 1e-6));
    let edge = row[BAR_END..BAR_END + EDGE_SPAN].iter().map(|&s| s as f64).fold(f64::MAX, f64::min);
    let ground = gamma_correct(HIGHLIGHT_BACKGROUND);
    assert!(edge < ground - 0.03, "dark edge {edge:.3} against the background {ground:.3}");
    let far = row[BAR_END + 4 * EDGE_SPAN] as f64;
    assert!((far - ground).abs() < 1e-4, "away from the bar {far:.4} against {ground:.4}");
}

#[test]
fn the_dark_edge_stops_at_the_black_clip() {
    let (ch, black, proc) = processor(Processing::Studio);
    let v = proc.process(&bar(&ch, &black, 0.0));
    let row = &v.rows[ACTIVE_H / 2][1];
    let edge = row[BAR_END..BAR_END + EDGE_SPAN].iter().map(|&s| s as f64).fold(f64::MAX, f64::min);
    assert!((edge - BLACK_CLIP).abs() < 1e-6, "dark edge {edge}");
}

fn flat_frame(v: u8) -> Frame {
    let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
    f.rgb.fill(v);
    f
}

fn inner_stats(v: &CameraVideo, k: usize) -> (f64, f64) {
    let s: Vec<f64> = (EDGE_ROWS..ACTIVE_H - EDGE_ROWS).flat_map(|r| v.rows[r][k].iter().map(|&x| x as f64)).collect();
    let m = s.iter().sum::<f64>() / s.len() as f64;
    let var = s.iter().map(|x| (x - m) * (x - m)).sum::<f64>() / s.len() as f64;
    (m, var.sqrt())
}

/// With the lens capped the camera reads black on average, under the head amplifier's noise
/// lifted by the gamma corrector's slope.
#[test]
fn a_capped_camera_reads_black_through_the_noise() {
    let params = CameraParams { tube: TubeKind::Plumbicon, ..CameraParams::default() };
    let mut cam = CameraTube::new(&params).unwrap();
    let dark = flat_frame(0);
    let mut v = cam.video(&dark, &dark);
    for _ in 0..SETTLE_FRAMES {
        v = cam.video(&dark, &dark);
    }
    for k in 0..3 {
        let (m, sd) = inner_stats(&v, k);
        assert!(m.abs() < 0.004, "ch{k}: mean {m:.4}");
        assert!(sd > 0.004 && sd < 0.04, "ch{k}: noise {sd:.4}");
    }
}

#[test]
fn the_camera_noise_is_new_every_frame_and_fixed_by_the_seed() {
    let params = CameraParams { tube: TubeKind::Plumbicon, ..CameraParams::default() };
    let grey = flat_frame(128);
    let run = |seed: u64| -> Vec<CameraVideo> {
        let mut cam = CameraTube::seeded(&params, seed).unwrap();
        (0..SETTLE_FRAMES + 2).map(|_| cam.video(&grey, &grey)).collect()
    };
    let a = run(1);
    let b = run(1);
    let c = run(2);
    assert!(a.iter().zip(&b).all(|(x, y)| x.rows == y.rows), "the same seed gives the same pictures");
    let n = SETTLE_FRAMES;
    assert!(a[n].rows != c[n].rows, "another seed gives other noise");
    let r = ACTIVE_H / 2;
    let diff: f64 = a[n].rows[r][1].iter().zip(&a[n + 1].rows[r][1]).map(|(x, y)| ((x - y) as f64).abs()).sum::<f64>()
        / ACTIVE_W as f64;
    assert!(diff > 0.001, "the noise of a still scene changes from frame to frame: {diff:.5}");
}

#[test]
fn process_frame_writes_the_video() {
    let params = CameraParams { tube: TubeKind::Plumbicon, iris_stops: 1.0, ..CameraParams::default() };
    let input = vhsphys_core::video::test_pattern();
    let mut as_video = CameraTube::new(&params).unwrap();
    let mut as_frame = CameraTube::new(&params).unwrap();
    for _ in 0..2 {
        let v = as_video.video(&input, &input);
        let f = as_frame.process_frame(&input);
        for r in 0..ACTIVE_H {
            for x in 0..ACTIVE_W {
                for k in 0..3 {
                    let want = (255.0f32 * v.rows[r][k][x]).round().clamp(0.0, 255.0) as u8;
                    assert_eq!(f.rgb[(r * ACTIVE_W + x) * 3 + k], want, "row {r} x {x} ch{k}");
                }
            }
        }
    }
}

#[test]
fn the_profile_is_chosen_in_the_config() {
    let p: CameraParams = toml::from_str("tube = \"plumbicon\"\nprocessing = \"consumer\"").unwrap();
    assert_eq!(p.processing, Processing::Consumer);
    let q: CameraParams = toml::from_str("tube = \"plumbicon\"").unwrap();
    assert_eq!(q.processing, Processing::Studio);
    let cam = CameraTube::new(&q).unwrap();
    let ch = cam.channels();
    let b = cam.black();
    for k in 0..3 {
        assert!(b[k] > 0.0 && b[k] < 0.05 * ch[k].peak_signal_na, "ch{k} black {}", b[k]);
    }
}

/// The 625-line engine generates the same noise density on its own sample grid.
#[test]
fn the_625_line_head_amplifier_follows_the_same_formula() {
    use vhsphys_core::pal::camera::TargetSignal as PalSignal;
    use vhsphys_core::pal::camera_processing::{HeadAmplifier as PalAmp, Processing as PalProcessing};
    use vhsphys_core::pal::consts as pc;
    let p = PalProcessing::Studio;
    let mut sig = PalSignal {
        rows: (0..pc::ACTIVE_H).map(|_| std::array::from_fn(|_| vec![DARK_LEVEL_NA as f32; pc::ACTIVE_W])).collect(),
    };
    PalAmp::new(&p.spec().head_amp, 1).amplify(&mut sig, 0);
    let h = p.spec().head_amp;
    let n = pc::ACTIVE_W;
    let df = pc::FS_BB / n as f64;
    let (mut got, mut want, mut rows) = (0.0, 0.0, 0.0);
    for r in (0..pc::ACTIVE_H).step_by(ROW_STEP) {
        let x: Vec<f64> = sig.rows[r][1].iter().map(|&v| v as f64 - DARK_LEVEL_NA).collect();
        for k in 1..n / 2 {
            let f = k as f64 * df;
            if f >= NOISE_BAND_HZ {
                break;
            }
            let w = 2.0 * std::f64::consts::PI * k as f64 / n as f64;
            let (mut re, mut im) = (0.0, 0.0);
            for (i, v) in x.iter().enumerate() {
                re += v * (w * i as f64).cos();
                im -= v * (w * i as f64).sin();
            }
            got += 2.0 * (re * re + im * im) / (n * n) as f64;
            if rows == 0.0 {
                let resp = amp_response(h.band_hz, f);
                let fet = (2.0 * std::f64::consts::PI * f * h.input_c_f).powi(2) * 4.0 * K_B * h.temp_k * h.r_n_ohm;
                let s = 2.0 * Q_E * DARK_LEVEL_NA * 1e-9 + 4.0 * K_B * h.temp_k / h.r_f_ohm + fet;
                want += s * NA2_PER_A2 * resp * resp * df;
            }
        }
        rows += 1.0;
    }
    got /= rows;
    assert!((got / want - 1.0).abs() < 0.08, "625 lines: {got:.3} nA^2 against {want:.3}");
}
