//! Acceptance test for the tube constants in seconds (docs/design/CAMERA_SCAN.md, "Tube constants
//! in seconds"): the charge constant `K = C V_T` from a physical beam temperature and a fitted
//! target capacitance, judged by the data sheets' lag measurement run on the scanned target.
//!
//! The measurement follows the handbook: a uniform light on for 5 s then off (decay), or off for
//! 10 s then on (build-up), at the stated signal, beam and bias currents. The signal is read at
//! the middle of the picture, on the middle row of field 0 and the row below it (field 1), so a
//! reading comes about every field; each reading is placed at its row's read time after the
//! change. The light switches at a frame boundary. Decay is the signal above dark and bias as a
//! percentage of the signal before the change, interpolated linearly in its logarithm; build-up
//! is the percentage of the final signal, interpolated linearly.
//!
//! Sources: Mullard 1987 (Plumbicon camera tubes, Figs. 7, 12 and 14, XQ1520 and XQ1410, read
//! from the curves), Philips Data Handbook 5b 1975 and T10 1985 (XQ1020, XQ1410 and XQ1520
//! tables). See docs/CAMERA_RESEARCH.md, "One constant reproduces the handbook curves".

use vhsphys_core::tube::{TubeChannel, TubeKind, BEAM_RATIO_DEFAULT, TARGET_VOLTAGE_V};

const R: usize = 0;
const G: usize = 1;
const B: usize = 2;

/// Seconds of light before a decay, and of darkness before a build-up (the handbook's).
const LIGHT_BEFORE_DECAY_S: f64 = 5.0;
const DARK_BEFORE_BUILD_S: f64 = 10.0;
/// Readings are kept to this long after the change.
const KEEP_S: f64 = 0.25;

/// Handbook lag condition: low key, signal and the largest signal the beam can stabilise, nA.
const LOW_KEY: (f64, f64) = (20.0, 300.0);
/// XQ1410 green figure condition (Fig. 14) and the XQ1020 high-key condition.
const G_FIGURE: (f64, f64) = (40.0, 600.0);
const HIGH_KEY: (f64, f64) = (300.0, 600.0);

/// XQ1520 red at low key, 50 Hz: decay at 60, 80, 120 and 200 ms for 0, 1, 3 and 8 nA of bias
/// light (Mullard 1987, Figs. 7 and 12). The 8 nA curve ends before 200 ms.
const DECAY_TIMES_S: [f64; 4] = [0.06, 0.08, 0.12, 0.2];
const R_DECAY_BY_BIAS: [(f64, [f64; 4]); 4] = [
    (0.0, [13.0, 9.0, 5.3, 3.3]),
    (1.0, [10.0, 7.3, 3.2, 1.5]),
    (3.0, [8.3, 5.5, 2.4, 0.6]),
    (8.0, [3.0, 1.9, 0.25, f64::NAN]),
];
/// Readings off the curves: each point within this factor, and the whole family within this
/// RMS of the natural logarithm of the ratio. A single charge constant fits the family to about
/// 0.2 in the logarithm (research, from the closed-form law).
const CURVE_POINT_FACTOR: f64 = 1.7;
const CURVE_RMS_LOG: f64 = 0.27;

/// XQ1520 red build-up at low key without bias at 40, 60 and 80 ms (Mullard 1987, Fig. 7), with
/// the allowed difference in percentage points. The first field depends on the switching phase
/// and is not judged.
const R_BUILD: [(f64, f64, f64); 3] = [(0.04, 52.0, 10.0), (0.06, 85.0, 7.0), (0.08, 96.0, 4.0)];

/// Table figures at low key without bias, XQ1020 and XQ1520 averaged where they differ: decay
/// at 60 and 200 ms and build-up at 60 ms, `[R, G, B]`.
const TABLE_DECAY_60: [f64; 3] = [12.5, 9.5, 15.0];
const TABLE_DECAY_200: [f64; 3] = [3.75, 3.25, 5.25];
const TABLE_BUILD_60: [f64; 3] = [85.0, 95.0, 70.0];
/// Table figures are rounded and the blue layer adds trap lag the capacitive law leaves out:
/// factors on the decay, percentage points on the build-up.
const TABLE_DECAY_60_FACTOR: f64 = 1.35;
const TABLE_DECAY_200_FACTOR: f64 = 1.4;
const TABLE_BUILD_POINTS: f64 = 10.0;

/// XQ1410 table at 60 ms with its own light bias `[R, G, B]`: a prediction from the XQ1020's fit
/// (same target and gun).
const XQ1410_DECAY_60: [f64; 3] = [8.0, 7.0, 11.0];
const XQ1410_FACTOR: f64 = 1.4;

/// XQ1410 green at 40/600 nA, 60 ms (Mullard 1987, Fig. 14): 8 % without bias, 1 to 1.7 % at
/// 10 to 11 nA of bias. The band at 10 nA allows for reading a steep curve.
const G_FIGURE_DECAY_60: f64 = 8.0;
const G_FIGURE_BIAS_NA: f64 = 10.0;
const G_FIGURE_BIASED_BAND: (f64, f64) = (0.5, 2.5);
/// The bias cuts the lag by at least this factor (the figure: about 6).
const G_FIGURE_BIAS_CUT: f64 = 3.0;

/// XQ1020 high key, 300/600 nA (Philips 5b 1975): 1.2 % at 60 ms and 0.4 % at 200 ms. The
/// capacitive law gives about half the 200 ms figure (research: 0.23 to 0.35 %); the rest is
/// trap lag, a later stage, so 200 ms is held to a wider factor.
const HIGH_KEY_DECAY: [(f64, f64, f64); 2] = [(0.06, 1.2, 1.4), (0.2, 0.4, 2.2)];

/// The beam temperature of a triode gun with a crossover is 1900 to 2350 K, about 1200 K for a
/// diode gun (US 4,388,556): `V_T = k T_b / e` between these bounds, V.
const V_T_RANGE: (f64, f64) = (0.10, 0.20);
/// Plumbicon target capacitance, 30 mm tubes: 1000 to 2000 pF (Philips design rule), 1.1 to
/// 2.2 nF from the fits in the research. F.
const CAPACITANCE_RANGE: (f64, f64) = (1.0e-9, 2.2e-9);
/// Lag runs on time: the 525-line engine (60 Hz) reads the same curve as the 625-line (50 Hz).
const FIELD_RATE_AGREEMENT: f64 = 0.12;

/// The handbook procedure on one engine; `$($eng)::+` is the engine's module path.
macro_rules! procedure {
    ($name:ident, $($eng:ident)::+) => {
        mod $name {
            use $($eng)::+::camera::{read_time_s, UniformColumn};
            use $($eng)::+::consts::{ACTIVE_H, ACTIVE_W, FRAME_RATE};
            use vhsphys_core::tube::TubeChannel;

            /// `(seconds after the change, signal nA)` of the middle rows, from `before` nA of
            /// photocurrent to `after` at the frame boundary `on_s` seconds in (rounded up to a
            /// frame), and the signal of the last full reading before the change.
            fn step(ch: &TubeChannel, on_s: f64, before: f64, after: f64) -> (f64, Vec<(f64, f64)>) {
                let r0 = ACTIVE_H / 2;
                let x = ACTIVE_W / 2;
                let n_change = (on_s * FRAME_RATE).ceil() as i64;
                let t_change = n_change as f64 / FRAME_RATE;
                let n_end = n_change + (super::KEEP_S * FRAME_RATE).ceil() as i64 + 1;
                let mut col = UniformColumn::new(ch);
                let photo = move |t: f64| if t < t_change { before } else { after };
                let (mut level, mut out) = (f64::NAN, Vec::new());
                for n in 0..n_end {
                    let s = col.read(&photo);
                    if n == n_change - 1 {
                        level = s[r0];
                    }
                    for r in [r0, r0 + 1] {
                        let t = read_time_s(n, r, x) - t_change;
                        if t > 0.0 {
                            out.push((t, s[r]));
                        }
                    }
                }
                out.sort_by(|a, b| a.0.total_cmp(&b.0));
                (level, out)
            }

            /// Decay in percent at `times` after `signal` nA has been on for 5 s.
            pub fn decay(ch: &TubeChannel, signal: f64, times: &[f64]) -> Vec<f64> {
                let base = ch.dark_na + ch.bias_na;
                let (lit, r) = step(ch, super::LIGHT_BEFORE_DECAY_S, signal, 0.0);
                let pct: Vec<(f64, f64)> = r.iter().map(|(t, s)| (*t, (100.0 * (s - base) / (lit - base)).max(1e-9))).collect();
                times.iter().map(|&t| super::interpolate(&pct, t, true)).collect()
            }

            /// Build-up in percent of the final signal at `times` after `signal` nA comes on
            /// after 10 s of darkness.
            pub fn build(ch: &TubeChannel, signal: f64, times: &[f64]) -> Vec<f64> {
                let base = ch.dark_na + ch.bias_na;
                let (_, r) = step(ch, super::DARK_BEFORE_BUILD_S, 0.0, signal);
                let pct: Vec<(f64, f64)> = r.iter().map(|(t, s)| (*t, 100.0 * (s - base) / signal)).collect();
                times.iter().map(|&t| super::interpolate(&pct, t, false)).collect()
            }
        }
    };
}
procedure!(at50, vhsphys_core::pal);
procedure!(at60, vhsphys_core);

fn interpolate(readings: &[(f64, f64)], t: f64, log: bool) -> f64 {
    let i = readings.iter().position(|(ti, _)| *ti >= t).expect("a reading after t");
    assert!(i > 0, "no reading before {t} s");
    let ((t0, a), (t1, b)) = (readings[i - 1], readings[i]);
    let f = (t - t0) / (t1 - t0);
    if log { (a.ln() * (1.0 - f) + b.ln() * f).exp() } else { a * (1.0 - f) + b * f }
}

/// Channel `k` of `kind` at the measurement's beam, with its own bias or `bias` nA.
fn channel(kind: TubeKind, k: usize, (_, beam): (f64, f64), bias: Option<f64>) -> TubeChannel {
    let ch = kind.channels(BEAM_RATIO_DEFAULT).expect("a tube")[k];
    TubeChannel { beam_na: beam, bias_na: bias.unwrap_or(ch.bias_na), ..ch }
}

fn within_factor(got: f64, want: f64, factor: f64) -> bool {
    got <= want * factor && got >= want / factor
}

#[test]
fn the_charge_constant_is_a_physical_capacitance_and_beam_temperature() {
    for kind in [TubeKind::Plumbicon, TubeKind::PlumbiconBias, TubeKind::PlumbiconAct] {
        let c = kind.channels(BEAM_RATIO_DEFAULT).unwrap();
        for k in 0..3 {
            let ch = &c[k];
            assert!(ch.v_t >= V_T_RANGE.0 && ch.v_t <= V_T_RANGE.1, "{kind:?} ch{k}: V_T {}", ch.v_t);
            assert!(
                ch.capacitance_f >= CAPACITANCE_RANGE.0 && ch.capacitance_f <= CAPACITANCE_RANGE.1,
                "{kind:?} ch{k}: C {}",
                ch.capacitance_f
            );
            let k_nas = ch.capacitance_f * ch.v_t * 1e9;
            assert!((ch.k_nas - k_nas).abs() < 1e-12 * k_nas, "{kind:?} ch{k}: K {} vs C V_T {}", ch.k_nas, k_nas);
            let plate = ch.capacitance_f * TARGET_VOLTAGE_V * 1e9;
            assert!((ch.plate_nas - plate).abs() < 1e-12 * plate, "{kind:?} ch{k}: plate {} vs C V {}", ch.plate_nas, plate);
            // one target per colour and one kind of gun in all three types
            let plain = TubeKind::Plumbicon.channels(BEAM_RATIO_DEFAULT).unwrap()[k];
            assert_eq!(ch.capacitance_f, plain.capacitance_f, "{kind:?} ch{k}");
            assert_eq!(ch.v_t, plain.v_t, "{kind:?} ch{k}");
        }
    }
}

#[test]
fn the_red_tube_decays_as_the_handbook_curves_against_bias() {
    let (mut sum, mut n) = (0.0, 0.0);
    let mut report = String::new();
    for (bias, want) in R_DECAY_BY_BIAS {
        let ch = channel(TubeKind::PlumbiconAct, R, LOW_KEY, Some(bias));
        let got = at50::decay(&ch, LOW_KEY.0, &DECAY_TIMES_S);
        report += &format!("\n{bias} nA: {got:.2?} vs {want:?}");
        for (g, w) in got.iter().zip(want) {
            if w.is_finite() {
                let l = (g / w).ln();
                sum += l * l;
                n += 1.0;
                assert!(l.abs() <= CURVE_POINT_FACTOR.ln(), "{bias} nA bias: {g:.2} % vs {w} %{report}");
            }
        }
    }
    let rms = (sum / n).sqrt();
    assert!(rms <= CURVE_RMS_LOG, "rms log {rms:.3}{report}");
}

#[test]
fn the_red_tube_builds_up_as_the_handbook_curve() {
    let ch = channel(TubeKind::PlumbiconAct, R, LOW_KEY, Some(0.0));
    let times: Vec<f64> = R_BUILD.iter().map(|p| p.0).collect();
    let got = at50::build(&ch, LOW_KEY.0, &times);
    for ((t, want, points), g) in R_BUILD.iter().zip(&got) {
        assert!((g - want).abs() <= *points, "{:.0} ms: {g:.1} % vs {want} % (all: {got:.1?})", t * 1e3);
    }
}

#[test]
fn each_channel_meets_the_table_at_low_key() {
    for kind in [TubeKind::Plumbicon, TubeKind::PlumbiconAct] {
        for k in [R, G, B] {
            let ch = channel(kind, k, LOW_KEY, Some(0.0));
            let d = at50::decay(&ch, LOW_KEY.0, &[0.06, 0.2]);
            let b = at50::build(&ch, LOW_KEY.0, &[0.06])[0];
            let msg = format!("{kind:?} ch{k}: decay {d:.2?} build {b:.1}");
            assert!(within_factor(d[0], TABLE_DECAY_60[k], TABLE_DECAY_60_FACTOR), "{msg} vs {}", TABLE_DECAY_60[k]);
            assert!(within_factor(d[1], TABLE_DECAY_200[k], TABLE_DECAY_200_FACTOR), "{msg} vs {}", TABLE_DECAY_200[k]);
            assert!((b - TABLE_BUILD_60[k]).abs() <= TABLE_BUILD_POINTS, "{msg} vs {}", TABLE_BUILD_60[k]);
        }
    }
}

/// Not fitted: the XQ1410 is the XQ1020 with light bias. The bias must cut the decay and speed
/// the build-up, and by about the table's amount at 60 ms.
#[test]
fn light_bias_cuts_the_lag_as_the_xq1410_shows() {
    for k in [R, G, B] {
        let plain = channel(TubeKind::Plumbicon, k, LOW_KEY, None);
        let biased = channel(TubeKind::PlumbiconBias, k, LOW_KEY, None);
        assert!(biased.bias_na > 0.0 && plain.bias_na == 0.0);
        let (dp, db) = (at50::decay(&plain, LOW_KEY.0, &[0.06, 0.2]), at50::decay(&biased, LOW_KEY.0, &[0.06, 0.2]));
        let (bp, bb) = (at50::build(&plain, LOW_KEY.0, &[0.06])[0], at50::build(&biased, LOW_KEY.0, &[0.06])[0]);
        let msg = format!("ch{k}: decay {dp:.2?} -> {db:.2?}, build {bp:.1} -> {bb:.1}");
        assert!(db[0] < dp[0] && db[1] < dp[1] && bb > bp, "{msg}");
        assert!(within_factor(db[0], XQ1410_DECAY_60[k], XQ1410_FACTOR), "{msg} vs {}", XQ1410_DECAY_60[k]);
    }
}

#[test]
fn the_green_tube_follows_the_xq1410_figure_with_and_without_bias() {
    let plain = at50::decay(&channel(TubeKind::Plumbicon, G, G_FIGURE, Some(0.0)), G_FIGURE.0, &[0.06])[0];
    let biased = at50::decay(&channel(TubeKind::Plumbicon, G, G_FIGURE, Some(G_FIGURE_BIAS_NA)), G_FIGURE.0, &[0.06])[0];
    assert!(within_factor(plain, G_FIGURE_DECAY_60, TABLE_DECAY_60_FACTOR), "no bias: {plain:.2} % vs {G_FIGURE_DECAY_60} %");
    assert!(biased >= G_FIGURE_BIASED_BAND.0 && biased <= G_FIGURE_BIASED_BAND.1, "{G_FIGURE_BIAS_NA} nA bias: {biased:.2} %");
    assert!(plain / biased >= G_FIGURE_BIAS_CUT, "bias cut {plain:.2} -> {biased:.2}");
}

/// Not fitted: the law predicts that lag in percent falls with the signal level.
#[test]
fn lag_at_high_key_is_the_data_sheets() {
    let ch = channel(TubeKind::Plumbicon, G, HIGH_KEY, Some(0.0));
    let times: Vec<f64> = HIGH_KEY_DECAY.iter().map(|p| p.0).collect();
    let got = at50::decay(&ch, HIGH_KEY.0, &times);
    for ((t, want, factor), g) in HIGH_KEY_DECAY.iter().zip(&got) {
        assert!(within_factor(*g, *want, *factor), "{:.0} ms: {g:.2} % vs {want} %", t * 1e3);
    }
}

#[test]
fn the_same_constants_give_the_same_lag_in_time_at_60_hz() {
    let ch = channel(TubeKind::PlumbiconAct, R, LOW_KEY, Some(0.0));
    let a = at50::decay(&ch, LOW_KEY.0, &DECAY_TIMES_S);
    let b = at60::decay(&ch, LOW_KEY.0, &DECAY_TIMES_S);
    for ((t, x), y) in DECAY_TIMES_S.iter().zip(&a).zip(&b) {
        assert!((y / x - 1.0).abs() <= FIELD_RATE_AGREEMENT, "{:.0} ms: 50 Hz {x:.2} %, 60 Hz {y:.2} %", t * 1e3);
    }
    let a = at50::build(&ch, LOW_KEY.0, &[0.06])[0];
    let b = at60::build(&ch, LOW_KEY.0, &[0.06])[0];
    assert!((b - a).abs() <= TABLE_BUILD_POINTS, "build-up at 60 ms: 50 Hz {a:.1} %, 60 Hz {b:.1} %");
}
