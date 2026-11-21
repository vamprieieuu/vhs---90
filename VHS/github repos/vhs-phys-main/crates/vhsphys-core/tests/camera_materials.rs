//! The target materials and their traps (docs/design/CAMERA_SCAN.md, "Target materials and
//! traps"): Sb2S3 vidicon, Newvicon, Saticon and Chalnicon beside lead oxide, each with its transfer
//! exponent, dark current against temperature, layer capacitance and trap populations, judged by the
//! data sheets' lag measurements run on the scanned target (`UniformColumn`).
//!
//! Lag is the signal above dark and bias as a percentage of the signal current, read on the middle
//! row of field 0 and the row below it at their read times after the light goes out at a frame
//! boundary, interpolated in its logarithm. Sources: docs/CAMERA_RESEARCH.md (Mullard 1987, Hitachi
//! H8397A, Philips T12, Matsushita, RCA 1977 and the RCA handbook's exposure families) and
//! docs/design/CAMERA_RETENTION.md (the BC4908 family, RCA 1977, Neuhauser 1978).

use vhsphys_core::camera::CameraParams;
use vhsphys_core::camera_processing::{gamma_correct, Processing, VideoProcessor};
use vhsphys_core::tube::{dark_at, photo_current, TubeChannel, TubeKind, BEAM_RATIO_DEFAULT, MAX_TRAPS};

const R: usize = 0;
const G: usize = 1;
const B: usize = 2;

/// Seconds of steady light before a decay (the handbook's), and how long readings are kept after.
const LIGHT_BEFORE_DECAY_S: f64 = 5.0;
const KEEP_S: f64 = 0.25;

/// The new kinds and the reference temperature of `channels`.
const NEW_KINDS: [TubeKind; 4] = [TubeKind::Vidicon, TubeKind::Newvicon, TubeKind::Saticon, TubeKind::Chalnicon];
const REFERENCE_C: f64 = 25.0;
/// Peak signal of the new kinds, nA.
const NEW_PEAK_NA: f64 = 200.0;

/// Dark current curves read from the data, `(deg C, nA)`, and how close the law must pass.
const SATICON_DARK: [(f64, f64); 2] = [(20.0, 0.15), (60.0, 2.7)];
const CHALNICON_DARK: [(f64, f64); 3] = [(0.0, 0.1), (30.0, 1.0), (60.0, 10.0)];
const NEWVICON_DARK: [(f64, f64); 2] = [(25.0, 5.0), (32.5, 10.0)];
const VIDICON_DARK: [(f64, f64); 2] = [(25.0, 20.0), (32.5, 40.0)];
const DARK_TOLERANCE: f64 = 0.15;

/// Transfer exponents: the data's ranges.
const GAMMA_RANGES: [(TubeKind, f64, f64); 5] = [
    (TubeKind::Plumbicon, 0.90, 1.0),
    (TubeKind::Vidicon, 0.55, 0.85),
    (TubeKind::Newvicon, 0.95, 1.05),
    (TubeKind::Saticon, 0.95, 1.05),
    (TubeKind::Chalnicon, 0.90, 0.95),
];

/// Layer capacitance checks, F: the Saticon's measured layer on a 2/3-inch target, the Newvicon
/// against PbO green, PbO blue against the mean of red and green.
const SATICON_C_RANGE: (f64, f64) = (0.58e-9, 1.16e-9);
const NEWVICON_OVER_PBO_GREEN: f64 = 2.0;
const PBO_BLUE_C_SPREAD: f64 = 0.15;
/// Trap release times allowed, s.
const RELEASE_RANGE_S: (f64, f64) = (0.005, 20.0);

/// XQ1410B blue at 20/300 nA, 50 Hz: decay at 200 ms against bias light (Mullard 1987, Fig. 12/14).
const BLUE_FLOOR_KEY: (f64, f64) = (20.0, 300.0);
const BLUE_FLOOR: [(f64, f64); 8] =
    [(0.0, 5.2), (1.0, 3.6), (2.0, 2.9), (3.0, 2.6), (4.0, 2.4), (5.0, 2.2), (8.0, 2.1), (12.0, 2.0)];
const BLUE_FLOOR_FACTOR: f64 = 1.5;
/// From 5 to 12 nA of bias the blue floor hardly moves; red with 8 nA of bias falls below this, %.
const BLUE_FLOOR_FLAT: f64 = 1.35;
const RED_BIASED_200_MAX: f64 = 1.3;
const RED_BIAS_NA: f64 = 8.0;

/// RCA handbook exposure families: 30 nA signal, 5 nA bias, 100 nA beam, 60 Hz, a dark target lit
/// for a number of fields; lag at 50 and 200 ms after the light goes out (Figs. 11-61/11-62).
const RCA_SIGNAL_NA: f64 = 30.0;
const RCA_BIAS_NA: f64 = 5.0;
const RCA_BEAM_NA: f64 = 100.0;
const RCA_DARK_FIELDS: i64 = 64;
/// The XQ1427B of the PbO family is a 2/3-inch tube (scanned 6.6 x 8.8 mm) where our blue channel is
/// a 30 mm one (12.8 x 17.1 mm). The layer is the same, so its capacitance and its trap capacity, both
/// counted over the whole target, scale with the scanned area; capture and release do not.
const PBO_FAMILY_AREA_RATIO: f64 = (6.6 * 8.8) / (12.8 * 17.1);
const EXPOSURE_FIELDS: [i64; 5] = [2, 4, 16, 64, 256];
const PBO_BLUE_EXPOSURE: [(f64, f64); 5] = [(1.9, 0.3), (2.4, 0.45), (3.4, 1.25), (4.5, 1.95), (4.7, 2.25)];
const EXPOSURE_FACTOR: f64 = 1.7;
/// The Saticon BC4391 family is judged by its shape only: each reading as a share of the 256-field
/// one, within `EXPOSURE_FACTOR`. Its levels cannot be met together with the BC4908 family below
/// (the Saticon is fitted to the BC4908, which reads 4.4 to 5.6 % at 50 ms here against 1.5 to
/// 2.55 %); the BC4391's size and gun are not known.
const SATICON_EXPOSURE: [(f64, f64); 5] = [(1.5, 0.2), (1.8, 0.25), (2.3, 0.45), (2.45, 0.5), (2.55, 0.5)];
/// Shape of the families: the PbO blue 200 ms tail grows at least this much from 2 to 256 fields
/// and has saturated by 64; the Saticon's grows between these bounds (data 2.5), and its 50 ms
/// reading grows less than its 200 ms tail (data 1.7 against 2.5).
const PBO_TAIL_GROWTH_MIN: f64 = 4.0;
const PBO_TAIL_SATURATED: f64 = 1.35;
const SATICON_TAIL_GROWTH: (f64, f64) = (1.6, 4.0);

/// BC4908, the H8397A's RCA equivalent (RCA 1977, Neuhauser 1978 Fig. 12): lag at the third field
/// (50 ms, 60 Hz) at 30 / 50 / 200 nA of signal with 0 / 5 / 10 nA of bias light, beam 800 nA. The
/// faceplate temperature is not given; the H8397A's 30 deg C is used.
const BC4908_BEAM_NA: f64 = 800.0;
const SATICON_DATA_C: f64 = 30.0;
const BC4908_SIGNALS: [f64; 3] = [30.0, 50.0, 200.0];
const BC4908: [(f64, [f64; 3]); 3] = [(0.0, [12.4, 8.7, 3.6]), (5.0, [6.1, 4.3, 2.4]), (10.0, [2.9, 2.2, 1.85])];
const BC4908_FACTOR: f64 = 1.3;

/// Newvicon: S4075 about 10 % at 1/20 s (0.2 uA, 60 Hz); T12 2/3-inch 8 to 13 % at 60 ms (50 Hz).
const NEWVICON_50MS: f64 = 10.0;
const NEWVICON_60MS_BAND: (f64, f64) = (8.0, 13.0);
/// Chalnicon (18 mm): 10 % at 50 ms.
const CHALNICON_50MS: f64 = 10.0;
const CAPACITIVE_FACTOR: f64 = 1.25;
/// Warming the Newvicon raises its dark current, which acts as bias light: lag falls.
const WARM_C: f64 = 45.0;
const WARM_LAG_CUT: f64 = 1.2;

/// Home-camera vidicon: 20 % at 50 ms (200 nA, 20 nA dark, 60 Hz); XQ1240 8 % at 200 ms; RCA's
/// vidicon with 20 nA of bias: 13 to 9.2 % at 50 ms, 3.3 to 1.2 % at 200 ms.
const VIDICON_50MS: f64 = 20.0;
const VIDICON_50MS_FACTOR: f64 = 1.3;
const VIDICON_200MS_BAND: (f64, f64) = (5.0, 12.0);
const VIDICON_BIAS_NA: f64 = 20.0;
const VIDICON_BIAS_CUT_50: f64 = 1.25;
const VIDICON_BIAS_CUT_200: f64 = 1.8;

/// Still flat grey through a camera with a new kind: frames to settle, inner rows, and the
/// tolerance on the mean of the gamma-corrected video.
const STILL_LIGHT: f64 = 0.2;
const STILL_FRAMES: usize = 60;
const EDGE_ROWS: usize = 8;
const STILL_TOLERANCE: f64 = 0.004;

/// The readings of one engine.
macro_rules! engine {
    ($name:ident, $($eng:ident)::+) => {
        mod $name {
            use $($eng)::+::camera::{read_time_s, UniformColumn};
            use $($eng)::+::consts::{ACTIVE_H, ACTIVE_W, FRAME_RATE};
            use vhsphys_core::tube::TubeChannel;

            pub fn frame_rate() -> f64 {
                FRAME_RATE
            }

            /// `(seconds after frame boundary n_off, signal nA)` of the middle rows of a target lit
            /// by `light(t)`.
            pub fn after(ch: &TubeChannel, light: &dyn Fn(f64) -> f64, n_off: i64) -> Vec<(f64, f64)> {
                let (r0, x) = (ACTIVE_H / 2, ACTIVE_W / 2);
                let t_off = n_off as f64 / FRAME_RATE;
                let n_end = n_off + (super::KEEP_S * FRAME_RATE).ceil() as i64 + 1;
                let mut col = UniformColumn::new(ch);
                let mut out = Vec::new();
                for n in 0..n_end {
                    let s = col.read(light);
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
        }
    };
}
engine!(at50, vhsphys_core::pal);
engine!(at60, vhsphys_core);

fn interpolate_log(readings: &[(f64, f64)], t: f64) -> f64 {
    let i = readings.iter().position(|(ti, _)| *ti >= t).expect("a reading after t");
    assert!(i > 0, "no reading before {t} s");
    let ((t0, a), (t1, b)) = (readings[i - 1], readings[i]);
    let f = (t - t0) / (t1 - t0);
    (a.max(1e-9).ln() * (1.0 - f) + b.max(1e-9).ln() * f).exp()
}

/// Lag in percent of `signal` at `times` from `readings` of a channel.
fn lag(ch: &TubeChannel, signal: f64, readings: &[(f64, f64)], times: &[f64]) -> Vec<f64> {
    let base = ch.dark_na + ch.bias_na;
    let pct: Vec<(f64, f64)> = readings.iter().map(|(t, s)| (*t, 100.0 * (s - base) / signal)).collect();
    times.iter().map(|&t| interpolate_log(&pct, t)).collect()
}

/// Decay after `signal` nA of steady light, at 60 Hz or 50 Hz.
fn decay(ch: &TubeChannel, signal: f64, times: &[f64], sixty: bool) -> Vec<f64> {
    let fr = if sixty { at60::frame_rate() } else { at50::frame_rate() };
    let n_off = (LIGHT_BEFORE_DECAY_S * fr).ceil() as i64;
    let t_off = n_off as f64 / fr;
    let light = move |t: f64| if t < t_off { signal } else { 0.0 };
    let r = if sixty { at60::after(ch, &light, n_off) } else { at50::after(ch, &light, n_off) };
    lag(ch, signal, &r, times)
}

/// RCA exposure procedure at 60 Hz: dark for 64 fields, lit for `fields`, then lag at `times`.
fn exposure(ch: &TubeChannel, fields: i64, times: &[f64]) -> Vec<f64> {
    let fr = at60::frame_rate();
    let n_on = RCA_DARK_FIELDS / 2;
    let n_off = n_on + fields / 2;
    let (t_on, t_off) = (n_on as f64 / fr, n_off as f64 / fr);
    let light = move |t: f64| if t >= t_on && t < t_off { RCA_SIGNAL_NA } else { 0.0 };
    lag(ch, RCA_SIGNAL_NA, &at60::after(ch, &light, n_off), times)
}

fn channel(kind: TubeKind, k: usize, beam_ratio: f64, temperature_c: f64) -> TubeChannel {
    kind.channels_at(beam_ratio, temperature_c).expect("a tube")[k]
}

fn rca(kind: TubeKind, k: usize) -> TubeChannel {
    TubeChannel { beam_na: RCA_BEAM_NA, bias_na: RCA_BIAS_NA, ..channel(kind, k, BEAM_RATIO_DEFAULT, REFERENCE_C) }
}

/// The same layer on a target `area` times the size: capacitance, plate limit and trap capacities
/// scale with it.
fn resized(ch: TubeChannel, area: f64) -> TubeChannel {
    let mut traps = ch.traps;
    for t in traps.iter_mut() {
        t.capacity_nas *= area;
    }
    TubeChannel {
        capacitance_f: ch.capacitance_f * area,
        k_nas: ch.k_nas * area,
        plate_nas: ch.plate_nas * area,
        traps,
        ..ch
    }
}

fn within_factor(got: f64, want: f64, factor: f64) -> bool {
    got <= want * factor && got >= want / factor
}

#[test]
fn the_new_kinds_are_named_in_the_config() {
    let p: CameraParams = toml::from_str("tube = \"saticon\"\ntemperature_c = 40.0").unwrap();
    assert_eq!(p.tube, TubeKind::Saticon);
    assert_eq!(p.temperature_c, 40.0);
    assert_eq!(CameraParams::default().temperature_c, REFERENCE_C);
    for (name, kind) in [("vidicon", TubeKind::Vidicon), ("newvicon", TubeKind::Newvicon), ("chalnicon", TubeKind::Chalnicon)] {
        let p: CameraParams = toml::from_str(&format!("tube = \"{name}\"")).unwrap();
        assert_eq!(p.tube, kind);
    }
}

#[test]
fn each_material_has_its_transfer_exponent_and_operating_point() {
    for (kind, lo, hi) in GAMMA_RANGES {
        for ch in kind.channels(BEAM_RATIO_DEFAULT).unwrap() {
            assert!(ch.gamma >= lo && ch.gamma <= hi, "{kind:?} gamma {}", ch.gamma);
            let want = ch.peak_signal_na * 0.25f64.powf(ch.gamma);
            assert!((photo_current(&ch, 0.25) - want).abs() < 1e-9 * want);
            assert_eq!(ch.linearise, kind == TubeKind::Vidicon, "{kind:?}");
        }
    }
    for kind in NEW_KINDS {
        let c = kind.channels(BEAM_RATIO_DEFAULT).unwrap();
        for k in 0..3 {
            assert_eq!(c[k].peak_signal_na, NEW_PEAK_NA);
            assert_eq!(c[k], c[0], "{kind:?}: three identical tubes");
            assert!((c[k].plate_nas - c[k].capacitance_f * c[k].target_voltage_v * 1e9).abs() < 1e-9);
        }
        assert_eq!(kind.channels(BEAM_RATIO_DEFAULT), kind.channels_at(BEAM_RATIO_DEFAULT, REFERENCE_C));
    }
    let pbo = TubeKind::Plumbicon.channels(BEAM_RATIO_DEFAULT).unwrap();
    assert!(pbo.iter().all(|c| c.target_voltage_v == 45.0));
    assert_eq!(TubeKind::Saticon.channels(BEAM_RATIO_DEFAULT).unwrap()[0].target_voltage_v, 50.0);
}

#[test]
fn dark_current_follows_the_data_against_temperature() {
    for (kind, curve) in [
        (TubeKind::Saticon, &SATICON_DARK[..]),
        (TubeKind::Chalnicon, &CHALNICON_DARK[..]),
        (TubeKind::Newvicon, &NEWVICON_DARK[..]),
        (TubeKind::Vidicon, &VIDICON_DARK[..]),
    ] {
        for &(t, want) in curve {
            let got = dark_at(kind, t).unwrap();
            assert!((got / want - 1.0).abs() < DARK_TOLERANCE, "{kind:?} at {t} C: {got:.3} nA, data {want}");
            assert_eq!(channel(kind, G, BEAM_RATIO_DEFAULT, t).dark_na, got);
        }
    }
    assert_eq!(dark_at(TubeKind::Plumbicon, 10.0), Some(1.0));
    assert_eq!(dark_at(TubeKind::Plumbicon, 50.0), Some(1.0));
    assert_eq!(dark_at(TubeKind::Off, 25.0), None);
}

#[test]
fn layer_capacitances_agree_with_the_data() {
    let sat = channel(TubeKind::Saticon, G, BEAM_RATIO_DEFAULT, REFERENCE_C).capacitance_f;
    assert!(sat >= SATICON_C_RANGE.0 && sat <= SATICON_C_RANGE.1, "Saticon C {sat:e}");
    let pbo = TubeKind::Plumbicon.channels(BEAM_RATIO_DEFAULT).unwrap();
    let new = channel(TubeKind::Newvicon, G, BEAM_RATIO_DEFAULT, REFERENCE_C).capacitance_f;
    assert!(new > NEWVICON_OVER_PBO_GREEN * pbo[G].capacitance_f, "Newvicon C {new:e}");
    let mean_rg = 0.5 * (pbo[R].capacitance_f + pbo[G].capacitance_f);
    assert!((pbo[B].capacitance_f / mean_rg - 1.0).abs() < PBO_BLUE_C_SPREAD, "PbO blue C {:e} against {mean_rg:e}", pbo[B].capacitance_f);
    for kind in NEW_KINDS {
        assert_eq!(channel(kind, G, BEAM_RATIO_DEFAULT, REFERENCE_C).v_t, vhsphys_core::tube::V_T_TRIODE_GUN_V);
    }
}

#[test]
fn traps_sit_where_the_data_put_them() {
    let has = |ch: &TubeChannel| ch.traps.iter().any(|t| t.capture > 0.0);
    let pbo = TubeKind::Plumbicon.channels(BEAM_RATIO_DEFAULT).unwrap();
    assert!(!has(&pbo[R]) && !has(&pbo[G]), "PbO red and green have negligible traps");
    assert!(has(&pbo[B]), "PbO blue has traps");
    for kind in [TubeKind::Saticon, TubeKind::Vidicon] {
        assert!(has(&channel(kind, G, BEAM_RATIO_DEFAULT, REFERENCE_C)), "{kind:?} has traps");
    }
    for kind in [TubeKind::Newvicon, TubeKind::Chalnicon] {
        assert!(!has(&channel(kind, G, BEAM_RATIO_DEFAULT, REFERENCE_C)), "{kind:?} lag is capacitive");
    }
    assert_eq!(pbo[B].traps.len(), MAX_TRAPS);
    for kind in [TubeKind::Plumbicon, TubeKind::Saticon, TubeKind::Vidicon] {
        for ch in kind.channels(BEAM_RATIO_DEFAULT).unwrap() {
            for t in ch.traps.iter().filter(|t| t.capture > 0.0) {
                assert!(t.capture < 1.0 && t.capacity_nas > 0.0, "{kind:?} {t:?}");
                assert!(t.release_s >= RELEASE_RANGE_S.0 && t.release_s <= RELEASE_RANGE_S.1, "{kind:?} {t:?}");
            }
        }
    }
}

/// The blue tube's tail does not go below about 2 % at 200 ms however much bias light it gets;
/// red with bias does.
#[test]
fn the_blue_plumbicon_keeps_a_floor_that_bias_light_does_not_remove() {
    let mut at = Vec::new();
    for (bias, want) in BLUE_FLOOR {
        let ch = TubeChannel { beam_na: BLUE_FLOOR_KEY.1, bias_na: bias, ..channel(TubeKind::Plumbicon, B, BEAM_RATIO_DEFAULT, REFERENCE_C) };
        let got = decay(&ch, BLUE_FLOOR_KEY.0, &[0.2], false)[0];
        assert!(within_factor(got, want, BLUE_FLOOR_FACTOR), "blue, {bias} nA bias: {got:.2} % at 200 ms, data {want}");
        at.push(got);
    }
    let (five, twelve) = (at[5], at[7]);
    assert!(five / twelve < BLUE_FLOOR_FLAT, "5 nA {five:.2} %, 12 nA {twelve:.2} %");
    let red = TubeChannel { beam_na: BLUE_FLOOR_KEY.1, bias_na: RED_BIAS_NA, ..channel(TubeKind::Plumbicon, R, BEAM_RATIO_DEFAULT, REFERENCE_C) };
    let r = decay(&red, BLUE_FLOOR_KEY.0, &[0.2], false)[0];
    assert!(r < RED_BIASED_200_MAX, "red with {RED_BIAS_NA} nA bias: {r:.2} %");
}

/// The longer a target is lit, the more its traps hold: the RCA exposure families. PbO blue is
/// judged on its levels, the Saticon on its shape. Scanned model, 50 / 200 ms after 2 / 4 / 16 /
/// 64 / 256 fields: Saticon 4.42, 0.15 / 4.75, 0.25 / 5.45, 0.47 / 5.56, 0.50 / 5.56, 0.50 %, as
/// shares of the last 0.79, 0.30 / 0.85, 0.50 / 0.98, 0.94 / 1, 1 / 1, 1 against data 0.59, 0.4 /
/// 0.71, 0.5 / 0.90, 0.9 / 0.96, 1 / 1, 1; the 200 ms tail grows 3.3x, the 50 ms reading 1.26x.
#[test]
fn trap_tails_grow_with_exposure_as_the_rca_families_show() {
    let pbo = resized(rca(TubeKind::Plumbicon, B), PBO_FAMILY_AREA_RATIO);
    let mut pbo_tails = Vec::new();
    for (fields, (w50, w200)) in EXPOSURE_FIELDS.iter().zip(PBO_BLUE_EXPOSURE) {
        let got = exposure(&pbo, *fields, &[0.05, 0.2]);
        assert!(within_factor(got[0], w50, EXPOSURE_FACTOR), "PbO blue, {fields} fields: {:.2} % at 50 ms, data {w50}", got[0]);
        assert!(within_factor(got[1], w200, EXPOSURE_FACTOR), "PbO blue, {fields} fields: {:.2} % at 200 ms, data {w200}", got[1]);
        pbo_tails.push(got[1]);
    }
    let sat = rca(TubeKind::Saticon, G);
    let got: Vec<Vec<f64>> = EXPOSURE_FIELDS.iter().map(|&f| exposure(&sat, f, &[0.05, 0.2])).collect();
    let (last, want_last) = (&got[4], SATICON_EXPOSURE[4]);
    for ((fields, g), (w50, w200)) in EXPOSURE_FIELDS.iter().zip(&got).zip(SATICON_EXPOSURE) {
        let (s50, s200) = (g[0] / last[0], g[1] / last[1]);
        let (d50, d200) = (w50 / want_last.0, w200 / want_last.1);
        assert!(within_factor(s50, d50, EXPOSURE_FACTOR), "Saticon, {fields} fields: 50 ms at {s50:.2} of 256 fields, data {d50:.2}");
        assert!(within_factor(s200, d200, EXPOSURE_FACTOR), "Saticon, {fields} fields: 200 ms at {s200:.2} of 256 fields, data {d200:.2}");
    }
    let (pbo_growth, pbo_late) = (pbo_tails[4] / pbo_tails[0], pbo_tails[4] / pbo_tails[3]);
    let (sat_growth, sat_growth_50) = (last[1] / got[0][1], last[0] / got[0][0]);
    assert!(pbo_growth >= PBO_TAIL_GROWTH_MIN, "PbO blue 200 ms tail grows {pbo_growth:.2}x from 2 to 256 fields");
    assert!(pbo_late <= PBO_TAIL_SATURATED, "PbO blue tail still grows {pbo_late:.2}x after 64 fields");
    assert!(sat_growth >= SATICON_TAIL_GROWTH.0 && sat_growth <= SATICON_TAIL_GROWTH.1, "Saticon tail grows {sat_growth:.2}x");
    assert!(sat_growth_50 < sat_growth, "Saticon 50 ms grows {sat_growth_50:.2}x, 200 ms {sat_growth:.2}x");
    assert!(pbo_growth > sat_growth);
}

/// The BC4908 family: at a fixed beam the Saticon's lag falls with the signal, as a beam-limited
/// capacitor's does, and bias light cuts it down to a tail it does not remove at 200 nA (a trap that
/// stays proportional to the light). Scanned model, 50 ms at 30 / 50 / 200 nA: 12.08 / 8.00 / 3.06 %
/// unbiased, 5.48 / 4.07 / 2.25 % with 5 nA, 2.96 / 2.54 / 1.93 % with 10 nA; the worst point is 1.18
/// times from its datum.
#[test]
fn the_saticon_lag_falls_with_signal_and_bias_light_as_the_bc4908_shows() {
    let mut above = [f64::MAX; 3];
    for (bias, want) in BC4908 {
        let ch = TubeChannel { beam_na: BC4908_BEAM_NA, bias_na: bias, ..channel(TubeKind::Saticon, G, BEAM_RATIO_DEFAULT, SATICON_DATA_C) };
        let got: Vec<f64> = BC4908_SIGNALS.iter().map(|&s| decay(&ch, s, &[0.05], true)[0]).collect();
        for (i, (g, w)) in got.iter().zip(want).enumerate() {
            assert!(
                within_factor(*g, w, BC4908_FACTOR),
                "Saticon, {} nA with {bias} nA bias: {g:.2} % at 50 ms, data {w}",
                BC4908_SIGNALS[i]
            );
            assert!(*g < above[i], "{} nA: {g:.2} % with {bias} nA bias, {:.2} % with less", BC4908_SIGNALS[i], above[i]);
        }
        assert!(got[0] > got[1] && got[1] > got[2], "{bias} nA bias, 30 / 50 / 200 nA: {got:.2?} %");
        above.copy_from_slice(&got);
    }
}

#[test]
fn newvicon_and_chalnicon_lag_is_their_large_capacitance() {
    let nv = channel(TubeKind::Newvicon, G, BEAM_RATIO_DEFAULT, REFERENCE_C);
    let at50ms = decay(&nv, NEW_PEAK_NA, &[0.05], true)[0];
    assert!(within_factor(at50ms, NEWVICON_50MS, CAPACITIVE_FACTOR), "Newvicon {at50ms:.2} % at 50 ms");
    let at60ms = decay(&nv, NEW_PEAK_NA, &[0.06], false)[0];
    assert!(at60ms >= NEWVICON_60MS_BAND.0 && at60ms <= NEWVICON_60MS_BAND.1, "Newvicon {at60ms:.2} % at 60 ms, 50 Hz");
    let ch = channel(TubeKind::Chalnicon, G, BEAM_RATIO_DEFAULT, REFERENCE_C);
    let c = decay(&ch, NEW_PEAK_NA, &[0.05], true)[0];
    assert!(within_factor(c, CHALNICON_50MS, CAPACITIVE_FACTOR), "Chalnicon {c:.2} % at 50 ms");
}

/// A warm Newvicon has more dark current, which biases its target like light: less lag.
#[test]
fn a_warm_newvicon_lags_less() {
    let cold = decay(&channel(TubeKind::Newvicon, G, BEAM_RATIO_DEFAULT, REFERENCE_C), NEW_PEAK_NA, &[0.05], true)[0];
    let warm = decay(&channel(TubeKind::Newvicon, G, BEAM_RATIO_DEFAULT, WARM_C), NEW_PEAK_NA, &[0.05], true)[0];
    assert!(cold / warm >= WARM_LAG_CUT, "25 C {cold:.2} %, {WARM_C} C {warm:.2} %");
}

/// The home-camera vidicon: heavy lag, a long trap tail, and bias light cutting the tail more than
/// the first fields.
#[test]
fn the_vidicon_lags_as_home_cameras_did() {
    let ch = channel(TubeKind::Vidicon, G, BEAM_RATIO_DEFAULT, REFERENCE_C);
    let plain = decay(&ch, NEW_PEAK_NA, &[0.05, 0.2], true);
    assert!(within_factor(plain[0], VIDICON_50MS, VIDICON_50MS_FACTOR), "vidicon {:.2} % at 50 ms", plain[0]);
    assert!(plain[1] >= VIDICON_200MS_BAND.0 && plain[1] <= VIDICON_200MS_BAND.1, "vidicon {:.2} % at 200 ms", plain[1]);
    let biased = decay(&TubeChannel { bias_na: VIDICON_BIAS_NA, ..ch }, NEW_PEAK_NA, &[0.05, 0.2], true);
    let (cut50, cut200) = (plain[0] / biased[0], plain[1] / biased[1]);
    assert!(cut50 >= VIDICON_BIAS_CUT_50, "bias cuts 50 ms by {cut50:.2}x");
    assert!(cut200 >= VIDICON_BIAS_CUT_200, "bias cuts 200 ms by {cut200:.2}x");
    assert!(cut200 > cut50);
}

/// The vidicon camera's gamma corrector is set for its 0.65 tube: the overall law is the 0.45 one.
/// A Plumbicon's 0.95 is left in.
#[test]
fn the_vidicon_camera_corrects_its_tubes_transfer() {
    use vhsphys_core::camera::TargetSignal;
    use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W};
    for (kind, l) in [(TubeKind::Vidicon, 0.2), (TubeKind::Plumbicon, 0.2)] {
        let ch = kind.channels(BEAM_RATIO_DEFAULT).unwrap();
        let black: [f64; 3] = std::array::from_fn(|k| ch[k].dark_na + ch[k].bias_na);
        let sig = TargetSignal {
            rows: (0..ACTIVE_H)
                .map(|_| std::array::from_fn(|k| vec![(black[k] + photo_current(&ch[k], l)) as f32; ACTIVE_W]))
                .collect(),
        };
        let v = VideoProcessor::new(Processing::Studio, &ch, black).process(&sig);
        let got = v.rows[ACTIVE_H / 2][G][ACTIVE_W / 2] as f64;
        let want = if kind == TubeKind::Vidicon { gamma_correct(l) } else { gamma_correct(l.powf(ch[G].gamma)) };
        assert!((got - want).abs() < 1e-4, "{kind:?}: {got:.4} against {want:.4}");
    }
}

/// A camera with each new kind reads a still flat field at its level once the target and its traps
/// have settled: the traps give back all they hold.
#[test]
fn a_still_field_reads_its_level_through_every_material() {
    use vhsphys_core::camera::CameraTube;
    use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W};
    use vhsphys_core::tube::{oetf, oetf_inverse};
    use vhsphys_core::video::Frame;
    let v = (255.0 * oetf(STILL_LIGHT)).round() as u8;
    let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
    f.rgb.fill(v);
    let light = oetf_inverse(v as f64 / 255.0);
    for kind in [TubeKind::Saticon, TubeKind::Vidicon, TubeKind::Newvicon] {
        let mut cam = CameraTube::new(&CameraParams { tube: kind, ..CameraParams::default() }).unwrap();
        let mut out = cam.video(&f, &f);
        for _ in 0..STILL_FRAMES {
            out = cam.video(&f, &f);
        }
        let ch = cam.channels()[G];
        let want = if ch.linearise { gamma_correct(light) } else { gamma_correct(light.powf(ch.gamma)) };
        let rows = EDGE_ROWS..ACTIVE_H - EDGE_ROWS;
        let n = (rows.len() * ACTIVE_W) as f64;
        let got = rows.map(|r| out.rows[r][G].iter().map(|&s| s as f64).sum::<f64>()).sum::<f64>() / n;
        assert!((got - want).abs() < STILL_TOLERANCE, "{kind:?}: {got:.4} against {want:.4}");
    }
}
