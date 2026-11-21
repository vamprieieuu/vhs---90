//! Retention (docs/design/CAMERA_RETENTION.md): carriers a lead-oxide or Saticon layer holds where its
//! field falls, the reversible burn of antimony trisulphide, the beam switched off, idle time with the
//! lens capped, and a camera warming up from cold. Everything is judged on the scanned target
//! (`UniformColumn`), from the data sheets' own measurements.
//!
//! Lag is the signal above dark and bias as a percentage of the signal current, read on the middle row
//! of field 0 and the row below it at their read times after the light goes out at a frame boundary,
//! interpolated in its logarithm (as `camera_materials.rs`). A retained image is the middle row of a
//! spot (a tenth of the picture height, centred) less a row a quarter of the picture below it, read in
//! the same frame. Sources: RCA 1977 (AN-5042 and its Fig. 2; Neuhauser 1978, Figs. 11 and 12),
//! Hitachi Hyoron 1976 (Fig. 8), US 4,330,733 (Fig. 6), US 3,546,520, US 4,492,982, Philips 5b 1975,
//! Mullard 1987 and the dark-current laws of docs/CAMERA_RESEARCH.md.

use vhsphys_core::camera::{read_time_s, CameraParams, UniformColumn};
use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W, FRAME_RATE};
use vhsphys_core::tube::{
    dark_at, photo_current, Burn, Collection, TubeChannel, TubeKind, BEAM_RATIO_DEFAULT,
    PBO_COLLECTION, SATICON_COLLECTION, VIDICON_BURN, WARM_UP_S,
};

const R: usize = 0;
const G: usize = 1;
const B: usize = 2;

/// Seconds of steady light before a decay (the handbook's), and how long readings are kept after.
const LIGHT_BEFORE_DECAY_S: f64 = 5.0;
const KEEP_S: f64 = 0.25;

/// Published collection curves, normalised at the operating voltage: `(V, share)`. RCA AN-5042 Fig. 2
/// (PbO, red and blue light, 45 V) and Hitachi Hyoron 1976 Fig. 8 (Te-doped Saticon junction, 50 V).
const PBO_RED_CURVE: [(f64, f64); 4] = [(13.0, 0.60), (20.0, 0.82), (25.0, 0.90), (35.0, 0.96)];
const PBO_BLUE_CURVE: [(f64, f64); 4] = [(10.0, 0.70), (15.0, 0.84), (20.0, 0.92), (30.0, 0.98)];
const SATICON_CURVE: [(f64, f64); 4] = [(10.0, 0.21), (12.5, 0.43), (16.0, 0.64), (25.0, 0.85)];
/// Reading accuracy of the curves plus the law's fit.
const CURVE_TOLERANCE: f64 = 0.06;
/// The Saticon's element reaches 41.7 V at its 200 nA peak white (0.80 nF): its knee must lie below that.
const SATICON_PEAK_LAYER_V: f64 = 41.7;

/// Handbook conditions, 50 Hz: low key (signal, beam) and the XQ1020 high key.
const LOW_KEY: (f64, f64) = (20.0, 300.0);
const G_FIGURE: (f64, f64) = (40.0, 600.0);
const HIGH_KEY: (f64, f64) = (300.0, 600.0);
const LOW_KEY_BIAS: [f64; 3] = [0.0, 3.0, 8.0];
const DECAY_TIMES_S: [f64; 4] = [0.06, 0.08, 0.12, 0.2];
/// Where the field stays high, the held charge must leave the readings as they were.
const UNCHANGED: f64 = 1e-3;
/// XQ1020 high key, 300/600 nA (Philips 5b 1975): 1.2 % at 60 ms and 0.4 % at 200 ms.
const HIGH_KEY_DECAY: [(f64, f64); 2] = [(0.06, 1.2), (0.2, 0.4)];
const HIGH_KEY_FACTOR: f64 = 1.3;

/// High-light sticking (US 4,330,733 Fig. 6): after a 4 uA signal (20 times the 0.2 uA of normal
/// use), the residual 10 fields after cut-off is 8 % (fluoride-doped) to 27 % (undoped) of 0.2 uA.
/// The exposure time is not given. The H8397A's beam practice (3.5 times) and faceplate (30 C).
const STICK_LIGHT: f64 = 20.0;
const STICK_ON_S: f64 = 10.0;
const STICK_FIELDS: f64 = 10.0;
const STICK_BAND: (f64, f64) = (8.0, 27.0);
/// Normal use leaves far less at the same time: BC4390 at most 1 % at 150 ms with 6 nA bias; without
/// bias this bound, and the sticking at least this many times it.
const NORMAL_AT_STICK_MAX: f64 = 2.0;
const STICK_OVER_NORMAL: f64 = 8.0;
const SATICON_BEAM_RATIO: f64 = 3.5;
const SATICON_DATA_C: f64 = 30.0;
const SATICON_PEAK_NA: f64 = 200.0;

/// An image left on a Saticon: normal white on the spot for 10 s with the beam on and with it off,
/// then the lens capped and the beam on; the retained image read 0.5 s later, share of peak white.
const IMAGE_ON_S: f64 = 10.0;
const RETAINED_AFTER_S: f64 = 0.5;
const BEAM_ON_RETAINED_MAX: f64 = 0.005;
const BEAM_OFF_RETAINED_MIN: f64 = 0.01;
const BEAM_OFF_OVER_ON: f64 = 5.0;

/// BC4908, the H8397A's RCA equivalent (Neuhauser 1978, Fig. 12): lag at 50 ms in the dark at 30,
/// 50 and 200 nA, unbiased, beam set for four times a 200 nA highlight. 12.4 / 8.7 / 3.6 %.
const BC4908_BEAM_NA: f64 = 800.0;
const BC4908_SIGNALS: [f64; 3] = [30.0, 50.0, 200.0];
const BC4908_LOW_OVER_HIGH_MIN: f64 = 2.6;

/// Lead-oxide highlight burn (RCA AN-5042): a specular highlight held on a plain Plumbicon, then the
/// lens capped. 20 times peak white for 5 s on the spot; read 3 s after.
const BURN_LIGHT: f64 = 20.0;
const BURN_ON_S: f64 = 5.0;
const BURN_READ_S: f64 = 3.0;
/// "Primarily in the red and green channels, and almost negligible in the blue": red and green leave
/// a visible share of their peak (a few code values after a 4.5 gamma slope), blue at most a third
/// of green's share.
const BURN_VISIBLE: f64 = 0.002;
const BLUE_OVER_GREEN_MAX: f64 = 1.0 / 3.0;
/// "Erased almost completely and almost instantly by exposing the camera to a white scene and
/// cutting the beam current off for several seconds": white (peak) for 3 s with the beam off.
const ERASE_LIGHT: f64 = 1.0;
const ERASE_S: f64 = 3.0;
const ERASED_SHARE_MAX: f64 = 0.1;

/// US 3,546,520: a 60 W frosted lamp imaged at f/1.5 for 2 min. On the model's target it is about
/// 3500 times the home vidicon's peak white (design note: 1.7e4 lx against about 5 lx).
const LAMP_LIGHT: f64 = 3500.0;
const LAMP_ON_S: f64 = 120.0;
/// Sensitivity 0.080 to about 0.070 uA/uW; dark current 10 to nearly 11 nA; normal after 30 min.
const BURNT_SENSITIVITY: f64 = 0.875;
const SENSITIVITY_TOLERANCE: f64 = 0.03;
const BURNT_DARK_RISE: (f64, f64) = (0.05, 0.12);
const RECOVERY_S: f64 = 1800.0;
const RECOVERED_MAX: f64 = 0.01;
/// Readings after the lamp: capped, then a grey light for the sensitivity, each long enough for the
/// vidicon's lag and traps to settle.
const CAPPED_S: f64 = 3.0;
const GREY_LIGHT: f64 = 0.5;
const GREY_S: f64 = 2.0;

/// Warm-up: a Newvicon (dark current doubling every 7.5 C) switched on at 10 C in a camera that runs
/// at 35 C. Dark readings settle within two seconds of a change; the 200 nA lag at 50 ms.
const COLD_C: f64 = 10.0;
const WARM_C: f64 = 35.0;
const SETTLE_S: f64 = 2.0;
const DARK_LAW_TOLERANCE: f64 = 0.03;
const STEADY_LAG_TOLERANCE: f64 = 0.10;
/// A Newvicon from 25 to 45 C cuts its lag at least 1.2 times (`camera_materials.rs`); 10 to 35 C more.
const WARM_LAG_CUT: f64 = 1.2;
const NEW_PEAK_NA: f64 = 200.0;

/// The readings of one engine.
macro_rules! engine {
    ($name:ident, $($eng:ident)::+) => {
        mod $name {
            use $($eng)::+::camera::{read_time_s, UniformColumn};
            use $($eng)::+::consts::{ACTIVE_H, ACTIVE_W, FRAME_RATE};
            use vhsphys_core::tube::TubeChannel;

            /// `(seconds after the light goes out, signal nA)` of the middle rows of `col`, lit by
            /// `signal` nA for `on_s` (rounded up to a frame) from its next frame, then dark.
            pub fn after(col: &mut UniformColumn, signal: f64, on_s: f64) -> Vec<(f64, f64)> {
                let (r0, x) = (ACTIVE_H / 2, ACTIVE_W / 2);
                let n0 = col.frame();
                let n_off = n0 + (on_s * FRAME_RATE).ceil() as i64;
                let t_off = n_off as f64 / FRAME_RATE;
                let n_end = n_off + (super::KEEP_S * FRAME_RATE).ceil() as i64 + 1;
                let mut out = Vec::new();
                for n in n0..n_end {
                    let s = col.read(&|t| if t < t_off { signal } else { 0.0 });
                    for r in [r0, r0 + 1] {
                        let t = read_time_s(n, r, x) - t_off;
                        if t > 0.0 {
                            out.push((t, s[r]));
                        }
                    }
                }
                out.sort_by(|a, b| a.0.total_cmp(&b.0));
                out
            }

            /// Decay in percent of `signal` at `times`, from a fresh target.
            pub fn decay(ch: &TubeChannel, signal: f64, times: &[f64]) -> Vec<f64> {
                let mut col = UniformColumn::new(ch);
                let r = after(&mut col, signal, super::LIGHT_BEFORE_DECAY_S);
                super::lag(ch.dark_na + ch.bias_na, signal, &r, times)
            }
        }
    };
}
engine!(at50, vhsphys_core::pal);
engine!(at60, vhsphys_core);

fn interpolate_log(readings: &[(f64, f64)], t: f64) -> f64 {
    let i = readings
        .iter()
        .position(|(ti, _)| *ti >= t)
        .expect("a reading after t");
    assert!(i > 0, "no reading before {t} s");
    let ((t0, a), (t1, b)) = (readings[i - 1], readings[i]);
    let f = (t - t0) / (t1 - t0);
    (a.max(1e-9).ln() * (1.0 - f) + b.max(1e-9).ln() * f).exp()
}

/// Lag in percent of `signal` above `base` nA at `times`.
fn lag(base: f64, signal: f64, readings: &[(f64, f64)], times: &[f64]) -> Vec<f64> {
    let pct: Vec<(f64, f64)> = readings
        .iter()
        .map(|(t, s)| (*t, 100.0 * (s - base) / signal))
        .collect();
    times.iter().map(|&t| interpolate_log(&pct, t)).collect()
}

fn channel(kind: TubeKind, k: usize, beam_ratio: f64, temperature_c: f64) -> TubeChannel {
    kind.channels_at(beam_ratio, temperature_c).expect("a tube")[k]
}

fn within_factor(got: f64, want: f64, factor: f64) -> bool {
    got <= want * factor && got >= want / factor
}

/// The data's spot: a tenth of the picture height, centred.
fn spot(r: usize) -> bool {
    r >= ACTIVE_H / 2 - ACTIVE_H / 20 && r < ACTIVE_H / 2 + ACTIVE_H / 20
}

const MID: usize = ACTIVE_H / 2;
const BACKGROUND: usize = ACTIVE_H / 2 + ACTIVE_H / 4;

/// Read `seconds` of frames (at least one) with `photo(r)` nA on row `r`; the last frame's rows.
fn hold(col: &mut UniformColumn, seconds: f64, photo: &dyn Fn(usize) -> f64) -> Vec<f64> {
    let n = ((seconds * FRAME_RATE).round() as usize).max(1);
    let mut s = Vec::new();
    for _ in 0..n {
        s = col.read_rows(&|r, _| photo(r));
    }
    s
}

/// Seconds from the start of the next frame of `col` to the read of its middle row.
fn mid_row_delay_s(col: &UniformColumn) -> f64 {
    read_time_s(col.frame(), MID, ACTIVE_W / 2) - col.frame() as f64 / FRAME_RATE
}

/// The spot's retained image, share of peak, `after_s` after the light on it went out (dark frames,
/// the beam on), read on the frame whose middle row is read nearest that time.
fn retained_after(col: &mut UniformColumn, ch: &TubeChannel, after_s: f64) -> f64 {
    let frames = ((after_s - mid_row_delay_s(col)) * FRAME_RATE)
        .round()
        .max(0.0) as usize
        + 1;
    let mut s = Vec::new();
    for _ in 0..frames {
        s = col.read_rows(&|_, _| 0.0);
    }
    (s[MID] - s[BACKGROUND]) / ch.peak_signal_na
}

// ------------------------------------------------------------------ constants

#[test]
fn the_retention_constants_follow_the_published_curves() {
    for (law, curve) in [
        (PBO_COLLECTION[R], &PBO_RED_CURVE),
        (PBO_COLLECTION[G], &PBO_RED_CURVE),
        (PBO_COLLECTION[B], &PBO_BLUE_CURVE),
        (SATICON_COLLECTION, &SATICON_CURVE),
    ] {
        for &(v, share) in curve.iter() {
            let got = law.efficiency(v);
            assert!(
                (got - share).abs() <= CURVE_TOLERANCE,
                "{law:?} at {v} V: {got:.3} vs {share}"
            );
        }
        assert!(!law.is_full());
        assert_eq!(law.efficiency(law.knee_v), 1.0);
        assert_eq!(law.efficiency(law.knee_v + 5.0), 1.0);
        assert_eq!(law.efficiency(law.zero_v), 0.0);
        assert!(law.capacity_nas > 0.0 && law.release_s > 0.0, "{law:?}");
    }
    // the PbO knee is its operating voltage; the Saticon's lies below its peak-white swing
    let pbo = TubeKind::Plumbicon.channels(BEAM_RATIO_DEFAULT).unwrap();
    for k in [R, G, B] {
        assert_eq!(PBO_COLLECTION[k].knee_v, pbo[k].target_voltage_v);
    }
    assert!(SATICON_COLLECTION.knee_v < SATICON_PEAK_LAYER_V);
    // blue holds least ("almost negligible in the blue")
    assert!(PBO_COLLECTION[B].capacity_nas < PBO_COLLECTION[R].capacity_nas);
    assert_eq!(PBO_COLLECTION[G], PBO_COLLECTION[R]);

    // which kinds carry what
    for kind in [
        TubeKind::Plumbicon,
        TubeKind::PlumbiconBias,
        TubeKind::PlumbiconAct,
        TubeKind::PlumbiconDbc,
    ] {
        let c = kind.channels(BEAM_RATIO_DEFAULT).unwrap();
        for k in [R, G, B] {
            assert_eq!(c[k].collection, PBO_COLLECTION[k], "{kind:?} ch{k}");
            assert_eq!(c[k].burn, Burn::NONE, "{kind:?} ch{k}");
            assert_eq!(c[k].cold_offset_c, 0.0);
        }
    }
    let sat = channel(TubeKind::Saticon, G, BEAM_RATIO_DEFAULT, 25.0);
    assert_eq!((sat.collection, sat.burn), (SATICON_COLLECTION, Burn::NONE));
    let vid = channel(TubeKind::Vidicon, G, BEAM_RATIO_DEFAULT, 25.0);
    assert_eq!((vid.collection, vid.burn), (Collection::FULL, VIDICON_BURN));
    for kind in [TubeKind::Newvicon, TubeKind::Chalnicon] {
        let c = channel(kind, G, BEAM_RATIO_DEFAULT, 25.0);
        assert_eq!(
            (c.collection, c.burn),
            (Collection::FULL, Burn::NONE),
            "{kind:?}"
        );
    }
    assert!(Collection::FULL.is_full());
    assert_eq!(Collection::FULL.efficiency(0.0), 1.0);
    assert_eq!(WARM_UP_S, 600.0);

    // the layer voltage and the warm-up law
    let ch = pbo[G];
    let q = 10.0;
    let want = ch.target_voltage_v - q / (ch.capacitance_f * 1e9);
    assert!((ch.layer_voltage_v(q) - want).abs() < 1e-12);
    let nv = TubeChannel {
        cold_offset_c: COLD_C - WARM_C,
        ..channel(TubeKind::Newvicon, G, BEAM_RATIO_DEFAULT, WARM_C)
    };
    let cold = dark_at(TubeKind::Newvicon, COLD_C).unwrap();
    assert!((nv.dark_na_at(0.0) / cold - 1.0).abs() < 1e-9);
    assert!((nv.dark_na_at(-1.0) / cold - 1.0).abs() < 1e-9);
    let t_mid = dark_at(
        TubeKind::Newvicon,
        WARM_C + (COLD_C - WARM_C) / std::f64::consts::E,
    );
    assert!((nv.dark_na_at(WARM_UP_S) / t_mid.unwrap() - 1.0).abs() < 1e-9);
    assert_eq!(pbo[G].dark_na_at(1.0), pbo[G].dark_na);

    // the camera's switch-on temperature is a config key, off by default
    assert_eq!(CameraParams::default().cold_start_c, None);
    let p: CameraParams =
        toml::from_str("tube = \"newvicon\"\ntemperature_c = 35.0\ncold_start_c = 10.0").unwrap();
    assert_eq!(p.cold_start_c, Some(COLD_C));
}

// ------------------------------------------------------------------ lead oxide: high key, low key

/// Philips: lead oxide's photoconductive lag is negligible at low key. Where the layer stays above its
/// knee the held charge never forms and the readings are those without it.
#[test]
fn lead_oxide_holds_carriers_only_where_its_field_falls() {
    let pbo = TubeKind::Plumbicon.channels(BEAM_RATIO_DEFAULT).unwrap();
    let mut cases = Vec::new();
    for bias in LOW_KEY_BIAS {
        let ch = TubeChannel {
            beam_na: LOW_KEY.1,
            bias_na: bias,
            ..pbo[R]
        };
        cases.push((ch, LOW_KEY.0));
    }
    cases.push((
        TubeChannel {
            beam_na: LOW_KEY.1,
            ..pbo[G]
        },
        LOW_KEY.0,
    ));
    cases.push((
        TubeChannel {
            beam_na: G_FIGURE.1,
            ..pbo[G]
        },
        G_FIGURE.0,
    ));
    for (ch, signal) in cases {
        let with = at50::decay(&ch, signal, &DECAY_TIMES_S);
        let without = at50::decay(
            &TubeChannel {
                collection: Collection::FULL,
                ..ch
            },
            signal,
            &DECAY_TIMES_S,
        );
        for (a, b) in with.iter().zip(&without) {
            assert!(
                (a / b - 1.0).abs() <= UNCHANGED,
                "{signal} nA, {} nA bias: {with:.4?} vs {without:.4?}",
                ch.bias_na
            );
        }
    }
}

/// At high key the green element swings deep enough for the layer to hold some carriers, which come
/// back over seconds: the XQ1020's 200 ms tail.
#[test]
fn the_lead_oxide_high_key_tail_is_the_xq1020s() {
    let ch = TubeChannel {
        beam_na: HIGH_KEY.1,
        bias_na: 0.0,
        ..channel(TubeKind::Plumbicon, G, BEAM_RATIO_DEFAULT, 25.0)
    };
    let times: Vec<f64> = HIGH_KEY_DECAY.iter().map(|p| p.0).collect();
    let got = at50::decay(&ch, HIGH_KEY.0, &times);
    for ((t, want), g) in HIGH_KEY_DECAY.iter().zip(&got) {
        assert!(
            within_factor(*g, *want, HIGH_KEY_FACTOR),
            "{:.0} ms: {g:.3} % vs {want} % (all {got:.3?})",
            t * 1e3
        );
    }
}

// ------------------------------------------------------------------ Saticon

/// High-light sticking: a highlight beyond what the beam holds collapses the field in the Te-doped
/// layer, which keeps carriers and gives them back over the next fields.
#[test]
fn a_saticon_sticks_after_twenty_times_peak_white() {
    let ch = channel(TubeKind::Saticon, G, SATICON_BEAM_RATIO, SATICON_DATA_C);
    let t = STICK_FIELDS / (2.0 * FRAME_RATE);
    let at = |light: f64| {
        let mut col = UniformColumn::new(&ch);
        let r = at60::after(&mut col, photo_current(&ch, light), STICK_ON_S);
        lag(ch.dark_na + ch.bias_na, SATICON_PEAK_NA, &r, &[t])[0]
    };
    let (stuck, normal) = (at(STICK_LIGHT), at(1.0));
    assert!(
        stuck >= STICK_BAND.0 && stuck <= STICK_BAND.1,
        "{STICK_LIGHT} times: {stuck:.2} % of 0.2 uA at the 10th field"
    );
    assert!(
        normal < NORMAL_AT_STICK_MAX,
        "0.2 uA: {normal:.2} % at the 10th field"
    );
    assert!(
        stuck >= STICK_OVER_NORMAL * normal,
        "{stuck:.2} % against {normal:.3} %"
    );
}

/// RCA: the Saticon resists burn for many minutes with the beam on and the field held across the
/// layer, but an image left on it with the beam off burns in (also US 4,492,982).
#[test]
fn a_saticon_burns_with_the_beam_off_and_not_with_it_on() {
    let ch = channel(TubeKind::Saticon, G, BEAM_RATIO_DEFAULT, 25.0);
    let p = photo_current(&ch, 1.0);
    let retained = |beam_on: bool| {
        let mut col = UniformColumn::new(&ch);
        col.set_beam_on(beam_on);
        hold(&mut col, IMAGE_ON_S, &|r| if spot(r) { p } else { 0.0 });
        col.set_beam_on(true);
        retained_after(&mut col, &ch, RETAINED_AFTER_S)
    };
    let (on, off) = (retained(true), retained(false));
    assert!(on.abs() < BEAM_ON_RETAINED_MAX, "beam on: {on:.4} of peak");
    assert!(off >= BEAM_OFF_RETAINED_MIN, "beam off: {off:.4} of peak");
    assert!(
        off >= BEAM_OFF_OVER_ON * on.abs(),
        "beam off {off:.4}, on {on:.4}"
    );
}

/// The H8397A's RCA equivalent: at a fixed beam the lag in percent falls with the signal, as a
/// beam-limited capacitor's does. Nothing in the stage makes it rise.
#[test]
fn the_saticon_lag_falls_with_signal_as_its_rca_equivalent_shows() {
    let ch = TubeChannel {
        beam_na: BC4908_BEAM_NA,
        bias_na: 0.0,
        ..channel(TubeKind::Saticon, G, BEAM_RATIO_DEFAULT, SATICON_DATA_C)
    };
    let got: Vec<f64> = BC4908_SIGNALS
        .iter()
        .map(|&s| at60::decay(&ch, s, &[0.05])[0])
        .collect();
    assert!(
        got[0] > got[1] && got[1] > got[2],
        "30 / 50 / 200 nA: {got:.2?} %"
    );
    assert!(
        got[0] / got[2] >= BC4908_LOW_OVER_HIGH_MIN,
        "30 over 200 nA: {got:.2?} %"
    );
}

// ------------------------------------------------------------------ lead-oxide highlight burn

/// A plain Plumbicon after `BURN_LIGHT` times peak white on the spot for `BURN_ON_S`, beam on.
fn burnt(ch: &TubeChannel) -> UniformColumn {
    let mut col = UniformColumn::new(ch);
    let p = photo_current(ch, BURN_LIGHT);
    hold(&mut col, BURN_ON_S, &|r| if spot(r) { p } else { 0.0 });
    col
}

#[test]
fn a_lead_oxide_highlight_burns_in_red_and_green_not_blue() {
    let pbo = TubeKind::Plumbicon.channels(BEAM_RATIO_DEFAULT).unwrap();
    let share: Vec<f64> = (0..3)
        .map(|k| retained_after(&mut burnt(&pbo[k]), &pbo[k], BURN_READ_S))
        .collect();
    for k in [R, G] {
        assert!(share[k] >= BURN_VISIBLE, "ch{k}: {share:.4?} of peak");
    }
    assert!(
        share[B] <= BLUE_OVER_GREEN_MAX * share[G],
        "R G B: {share:.4?} of peak"
    );
}

#[test]
fn white_light_with_the_beam_off_erases_a_lead_oxide_burn() {
    let ch = channel(TubeKind::Plumbicon, R, BEAM_RATIO_DEFAULT, 25.0);
    let white = photo_current(&ch, ERASE_LIGHT);
    let mut kept = burnt(&ch);
    hold(&mut kept, ERASE_S, &|_| 0.0);
    let kept = retained_after(&mut kept, &ch, BURN_READ_S);
    let mut erased = burnt(&ch);
    erased.set_beam_on(false);
    hold(&mut erased, ERASE_S, &|_| white);
    erased.set_beam_on(true);
    let erased = retained_after(&mut erased, &ch, BURN_READ_S);
    assert!(kept > 0.0, "the burn: {kept:.4} of peak");
    assert!(
        erased.abs() <= ERASED_SHARE_MAX * kept,
        "erased {erased:.5} against {kept:.5} of peak"
    );
}

// ------------------------------------------------------------------ antimony trisulphide burn

/// Dark ratio (spot over background) after `CAPPED_S` capped, then the sensitivity ratio of the
/// signal above dark after `GREY_S` of grey light.
fn dark_and_sensitivity(col: &mut UniformColumn, ch: &TubeChannel) -> (f64, f64) {
    let d = hold(col, CAPPED_S, &|_| 0.0);
    let grey = photo_current(ch, GREY_LIGHT);
    let s = hold(col, GREY_S, &|_| grey);
    let dark = d[MID] / d[BACKGROUND];
    let sens = (s[MID] - d[MID]) / (s[BACKGROUND] - d[BACKGROUND]);
    (dark, sens)
}

#[test]
fn an_antimony_trisulphide_burn_recovers_within_half_an_hour() {
    let ch = channel(TubeKind::Vidicon, G, BEAM_RATIO_DEFAULT, 25.0);
    let lamp = photo_current(&ch, LAMP_LIGHT);
    let mut col = UniformColumn::new(&ch);
    hold(&mut col, LAMP_ON_S, &|r| if spot(r) { lamp } else { 0.0 });
    let (dark, sens) = dark_and_sensitivity(&mut col, &ch);
    assert!(
        (sens - BURNT_SENSITIVITY).abs() <= SENSITIVITY_TOLERANCE,
        "sensitivity {sens:.4} of the unburnt"
    );
    assert!(
        dark - 1.0 >= BURNT_DARK_RISE.0 && dark - 1.0 <= BURNT_DARK_RISE.1,
        "dark current {dark:.4} of the unburnt"
    );
    col.idle(RECOVERY_S);
    let (dark, sens) = dark_and_sensitivity(&mut col, &ch);
    assert!(
        (1.0 - sens).abs() <= RECOVERED_MAX && (dark - 1.0).abs() <= RECOVERED_MAX,
        "after {RECOVERY_S} s: sensitivity {sens:.4}, dark {dark:.4}"
    );
}

// ------------------------------------------------------------------ warm-up

/// Faceplate temperature of the warm-up law at session time `t`, deg C.
fn warming_c(t: f64) -> f64 {
    WARM_C + (COLD_C - WARM_C) * (-t.max(0.0) / WARM_UP_S).exp()
}

/// Mean dark reading of the middle rows after `SETTLE_S` capped, and the session time of the read.
fn dark_reading(col: &mut UniformColumn) -> (f64, f64) {
    let s = hold(col, SETTLE_S, &|_| 0.0);
    let t = read_time_s(col.frame() - 1, MID, ACTIVE_W / 2);
    let rows = MID - 8..MID + 8;
    let n = rows.len() as f64;
    (rows.map(|r| s[r]).sum::<f64>() / n, t)
}

/// Lag at 50 ms of the 200 nA decay on `col` from its next frame, and the session time it starts.
fn lag_now(col: &mut UniformColumn, ch: &TubeChannel) -> (f64, f64) {
    let t0 = col.frame() as f64 / FRAME_RATE;
    let r = at60::after(col, NEW_PEAK_NA, LIGHT_BEFORE_DECAY_S);
    let base = ch.dark_na_at(t0 + LIGHT_BEFORE_DECAY_S);
    (lag(base, NEW_PEAK_NA, &r, &[0.05])[0], t0)
}

#[test]
fn a_camera_warms_up_from_cold() {
    let warm = channel(TubeKind::Newvicon, G, BEAM_RATIO_DEFAULT, WARM_C);
    let ch = TubeChannel {
        cold_offset_c: COLD_C - WARM_C,
        ..warm
    };

    // the dark current follows the faceplate as it warms
    let mut col = UniformColumn::new(&ch);
    for idle in [0.0, WARM_UP_S, 2.0 * WARM_UP_S] {
        col.idle(idle);
        let (got, t) = dark_reading(&mut col);
        let want = dark_at(TubeKind::Newvicon, warming_c(t)).unwrap();
        assert!(
            (got / want - 1.0).abs() <= DARK_LAW_TOLERANCE,
            "{t:.0} s: {got:.3} nA vs {want:.3} nA at {:.1} C",
            warming_c(t)
        );
    }

    // the lag follows it too: a cold camera lags as a cold tube, a warmed one as a warm tube
    let mut col = UniformColumn::new(&ch);
    let (cold_lag, t_cold) = lag_now(&mut col, &ch);
    col.idle(3.0 * WARM_UP_S);
    let (warm_lag, t_warm) = lag_now(&mut col, &ch);
    for (got, t) in [(cold_lag, t_cold), (warm_lag, t_warm)] {
        let steady_ch = channel(TubeKind::Newvicon, G, BEAM_RATIO_DEFAULT, warming_c(t));
        let want = at60::decay(&steady_ch, NEW_PEAK_NA, &[0.05])[0];
        assert!(
            within_factor(got, want, 1.0 + STEADY_LAG_TOLERANCE),
            "{t:.0} s: {got:.2} % vs {want:.2} % at a steady {:.1} C",
            warming_c(t)
        );
    }
    assert!(
        cold_lag >= WARM_LAG_CUT * warm_lag,
        "cold {cold_lag:.2} %, warm {warm_lag:.2} %"
    );
}
