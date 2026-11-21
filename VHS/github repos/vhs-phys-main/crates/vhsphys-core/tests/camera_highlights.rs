//! Highlights (docs/design/CAMERA_SCAN.md, "Highlights"): the ACT flyback spread over a span of lines,
//! dynamic beam control, beam bending from element potential, and the colour of a trail.
//!
//! Tails are counted on the middle row of a target column, one reading per frame, from the first frame
//! whose middle row integrated no light; a frame that reads above peak white is a frame of clipped
//! white tail. A highlight of `L` times peak white is the light `L` through the tube's transfer
//! (`photo_current`). Sources: docs/CAMERA_RESEARCH.md ("Beam capacity of twice peak white turns
//! highlights into comet tails"): Mullard 1987 (ACT 32 times, dynamic beam control 16 times over a spot
//! of 10 % of the picture height; XQ1520 cutting levels; XQ3440 and XQ1520 low-key lag), US 4,143,305
//! (the flyback spans many lines, pre-discharges a few lines ahead and misses the top of the picture),
//! US 4,229,767 (the Ampex beam law), Philips 1963 (beam bending, the 10 V swing), RCA 1977 and
//! Neuhauser 1978 (lead oxide trails red, the Saticon's residue is white and gone in about 2 s).
//! A camera without a tube is covered by the golden test.

use vhsphys_core::camera::{
    act_flyback_nas, read_time_s, sample_pitch_lines, CameraParams, CameraTube, TargetSignal,
    UniformColumn,
};
use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W, FRAME_RATE};
use vhsphys_core::tube::{
    photo_current, TubeChannel, TubeKind, ACT_BEAM_NA, ACT_FLYBACK_S, ACT_LEVEL_V, ACT_SPAN_PH,
    BEAM_RATIO_DEFAULT, BEND_RADIUS_MAX_LINES, DBC, TARGET_ASPECT, TARGET_VOLTAGE_V,
    V_BEND_ONSET_V, V_T_DIODE_GUN_V, V_T_TRIODE_GUN_V,
};

const R: usize = 0;
const G: usize = 1;
const B: usize = 2;

/// The data's highlight: a spot of a tenth of the picture height, centred.
const SPOT_ROWS: usize = ACTIVE_H / 10;
/// ACT handles 5 stops (32 times peak white), dynamic beam control 4 (16 times) (Mullard 1987).
const ACT_RATED: f64 = 32.0;
const DBC_RATED: f64 = 16.0;
/// A stop beyond the dynamic beam control's rating.
const DBC_BEYOND: f64 = 32.0;
/// Frames a highlight is held, and dark frames read after it.
const ON_FRAMES: usize = 30;
const DARK_FRAMES: usize = 40;
/// The first dark frame after a highlight a gun has taken holds only lag: below this share of peak
/// white (as `camera_tube.rs` judges the ACT gun).
const LAG_ONLY: f64 = 0.25;
/// A plain gun at these overloads stores the overload up to the plate and drains it at its beam over
/// at least this many frames (green, the shortest: a 51.75 nA s plate at 20 nA s a frame).
const PLAIN_TAIL_MIN: usize = 2;

/// A steady light the dynamic beam control reads linearly (Mullard 1987: 16 times), and how closely.
const DBC_LINEAR_LIGHT: f64 = 16.0;
const LINEAR_TOLERANCE: f64 = 0.01;
/// Beyond its rating the beam stops at its largest: the reading of a beam-limited target is the beam.
const DBC_MAX_TOLERANCE: f64 = 0.03;
/// A plain gun's reading stops at its beam, twice peak white (Mullard 1987), with this margin.
const PLAIN_LIMIT_MARGIN: f64 = 1.02;
const STEADY_FRAMES: usize = 20;

/// The handbook's low-key condition (20 nA signal, 300 nA beam, no bias, 50 Hz), seconds of light
/// before the decay, the readings kept, and the decay points judged.
const LOW_KEY: (f64, f64) = (20.0, 300.0);
const LIGHT_BEFORE_DECAY_S: f64 = 5.0;
const KEEP_S: f64 = 0.25;
const DECAY_TIMES_S: [f64; 2] = [0.06, 0.2];
/// Below the cutting level the ACT gun leaves the plain gun's readings, and the diode gun's target keeps
/// `K` (the XQ3440 and XQ1520 print the same low-key lag, Mullard 1987): their lag agrees to rounding.
const SAME_LAG: f64 = 0.01;

/// Overload and bending strength for the bending checks: at the 45 V plate, `bending = 1` reaches
/// 3.5 frame-line pitches.
const BENDING: f64 = 1.0;
const BENT_LIGHT: f64 = 16.0;
const MILD_LIGHT: f64 = 4.0;
/// A vertical bar of columns, full height, and the rows its line signal is averaged over.
const BAR_X: (usize, usize) = (360, 400);
const MID_ROWS: (usize, usize) = (200, 280);
/// The third sample outside the bar is 2.5 samples from its edge: past the horizontal spot (sigma
/// about 0.57 samples), inside the bending radius at the plate (2.55 of 3.5 line pitches). The sixth is
/// 5.1 line pitches from the edge, past the radius.
const BEYOND_SPOT: usize = 3;
const BEYOND_RADIUS: usize = 6;
/// A bloom is at least this share of peak white where the plain gun reads dark; dark is within this
/// share of the dark current.
const BLOOM_MIN: f64 = 0.25;
const DARK_MAX: f64 = 0.02;
/// Frames the bar is read before it is measured (the plate fills within a field at these lights).
const BAR_FRAMES: usize = 5;
/// A horizontal band of rows read on one column, and the frames whose readings are averaged.
const BAND_ROWS: (usize, usize) = (216, 264);
const BAND_SETTLE: usize = 10;
const BAND_FRAMES: usize = 20;
/// Rows outside the band summed for the bloom, on each side.
const BLOOM_ROWS: usize = 6;
/// The bloom grows with the overload: summed over the rows outside, by at least this factor from 4 to
/// 16 times peak white.
const BLOOM_GROWTH: f64 = 1.3;

/// ACT and the top of the picture: a bar at the top and one in the middle, the rows read, and the frames.
const TOP_BAR: ((usize, usize), (usize, usize)) = ((0, SPOT_ROWS), (200, 240));
const MID_BAR: ((usize, usize), (usize, usize)) = (
    (ACTIVE_H / 2 - SPOT_ROWS / 2, ACTIVE_H / 2 + SPOT_ROWS / 2),
    (500, 540),
);
const TOP_FRAMES: usize = 4;

/// Trail colour: an overexposed PbO highlight (4 times, beyond the 2 times beam), then darkness.
const TRAIL_LIGHT: f64 = 4.0;
const TRAIL_ON: usize = 40;
const TRAIL_DARK: usize = 40;
/// Red's clipped tail outlasts green's by at least this many frames.
const RED_OVER_GREEN_FRAMES: usize = 2;
/// Readings are compared as shares of each channel's peak, to this rounding, until red is below the floor.
const SHARE_ROUNDING: f64 = 0.01;
const TRAIL_FLOOR: f64 = 0.02;
/// Saticon at a beam of 3.5 times the highlight (RCA 1977: 3 to 4 times), a highlight of 3 times peak
/// white held 2 s; its residue is white and gone (below this share of peak white) 2 s after.
const SATICON_BEAM_RATIO: f64 = 3.5;
const SATICON_LIGHT: f64 = 3.0;
const SATICON_ON_S: f64 = 2.0;
const SATICON_GONE_S: f64 = 2.0;
const SATICON_GONE: f64 = 0.005;
const WHITE_SPREAD: f64 = 0.002;

fn chans(kind: TubeKind) -> [TubeChannel; 3] {
    kind.channels(BEAM_RATIO_DEFAULT).expect("a tube")
}

fn spot(r: usize) -> bool {
    r >= ACTIVE_H / 2 - SPOT_ROWS / 2 && r < ACTIVE_H / 2 + SPOT_ROWS / 2
}

/// Middle-row readings, nA, from the first frame lit by no light, after `light` times peak white on the
/// spot for `on` frames.
fn after_spot(ch: &TubeChannel, light: f64, on: usize, dark: usize) -> Vec<f64> {
    let mut col = UniformColumn::new(ch);
    let p = photo_current(ch, light);
    let t_off = on as f64 / FRAME_RATE;
    let photo = move |r: usize, t: f64| if t < t_off && spot(r) { p } else { 0.0 };
    (0..on + 1 + dark)
        .map(|_| col.read_rows(&photo)[ACTIVE_H / 2])
        .skip(on + 1)
        .collect()
}

fn tail_frames(readings: &[f64], ch: &TubeChannel) -> usize {
    readings
        .iter()
        .take_while(|&&s| s > ch.peak_signal_na)
        .count()
}

// ------------------------------------------------------------------ operating points

#[test]
fn the_highlight_guns_are_the_data_sheets() {
    let act = chans(TubeKind::PlumbiconAct);
    let dbc = chans(TubeKind::PlumbiconDbc);
    let plain = chans(TubeKind::Plumbicon);
    for k in [R, G, B] {
        let a = &act[k];
        assert!(a.act && a.dbc.is_none());
        assert_eq!(a.act_level_v, ACT_LEVEL_V[k]);
        let q_act = a.capacitance_f * ACT_LEVEL_V[k] * 1e9;
        assert!(
            (a.act_level_nas() - q_act).abs() <= 1e-12 * q_act,
            "ch{k}: {} vs {q_act}",
            a.act_level_nas()
        );
        assert!(!plain[k].act && plain[k].dbc.is_none());

        // the diode gun's cooler beam on a target of more capacitance: the same K, a larger plate
        let d = &dbc[k];
        assert!(!d.act && d.dbc == Some(DBC), "ch{k}: {d:?}");
        assert_eq!(d.v_t, V_T_DIODE_GUN_V);
        let c = a.capacitance_f * V_T_TRIODE_GUN_V / V_T_DIODE_GUN_V;
        assert!(
            (d.capacitance_f - c).abs() <= 1e-12 * c,
            "ch{k}: C {} vs {c}",
            d.capacitance_f
        );
        assert!(
            (d.k_nas - a.k_nas).abs() <= 1e-9 * a.k_nas,
            "ch{k}: K {} vs {}",
            d.k_nas,
            a.k_nas
        );
        let plate = d.capacitance_f * TARGET_VOLTAGE_V * 1e9;
        assert!(
            (d.plate_nas - plate).abs() <= 1e-12 * plate,
            "ch{k}: plate {} vs {plate}",
            d.plate_nas
        );
        assert_eq!((d.peak_signal_na, d.beam_na), (a.peak_signal_na, a.beam_na));

        // bending is off unless the camera sets it
        assert_eq!((a.bending, d.bending, plain[k].bending), (0.0, 0.0, 0.0));
    }
    assert_eq!(DBC.headroom, 0.5);
    assert_eq!(DBC.max_ratio, 17.0);

    let p: CameraParams = toml::from_str("tube = \"plumbicon-dbc\"\nblooming = 0.5").unwrap();
    assert_eq!(p.tube, TubeKind::PlumbiconDbc);
    let cam = CameraTube::new(&p).unwrap();
    assert!(cam
        .channels()
        .iter()
        .all(|c| c.bending == 0.5 && c.dbc == Some(DBC)));
    let off = CameraTube::new(&CameraParams {
        tube: TubeKind::Plumbicon,
        ..CameraParams::default()
    })
    .unwrap();
    assert!(off.channels().iter().all(|c| c.bending == 0.0));
}

/// The flyback offers each element of its span the beam over the span's share of the target. Four
/// flybacks before the read (a half span of 8 lines at 525) clear a full element to the cutting level in
/// every channel; one does not (US 4,143,305: only a few lines ahead, so the top is left).
#[test]
fn the_flyback_clears_a_full_element_in_a_few_lines_but_not_in_one() {
    let b = act_flyback_nas();
    let want = ACT_BEAM_NA * ACT_FLYBACK_S / ACT_SPAN_PH;
    assert!((b - want).abs() <= 1e-12 * want, "{b} vs {want}");
    let ahead = (ACT_SPAN_PH * ACTIVE_H as f64 / 4.0).floor();
    for (k, ch) in chans(TubeKind::PlumbiconAct).iter().enumerate() {
        let overload = ch.plate_nas - ch.act_level_nas();
        assert!(
            ahead * b >= overload,
            "ch{k}: {ahead} flybacks of {b} nA s against {overload}"
        );
        assert!(
            b < overload,
            "ch{k}: one flyback of {b} nA s clears {overload}"
        );
    }
    let pitch = TARGET_ASPECT * ACTIVE_H as f64 / ACTIVE_W as f64;
    assert!(
        (sample_pitch_lines() - pitch).abs() <= 1e-12,
        "{} vs {pitch}",
        sample_pitch_lines()
    );
}

#[test]
fn the_bending_radius_grows_from_the_onset_to_its_bound() {
    let ch = TubeChannel {
        bending: BENDING,
        ..chans(TubeKind::Plumbicon)[G]
    };
    assert_eq!(ch.bend_radius_lines(V_BEND_ONSET_V), 0.0);
    assert_eq!(ch.bend_radius_lines(0.5 * V_BEND_ONSET_V), 0.0);
    assert!((ch.bend_radius_lines(TARGET_VOLTAGE_V) - 3.5).abs() < 1e-12);
    let strong = TubeChannel { bending: 2.0, ..ch };
    assert_eq!(
        strong.bend_radius_lines(TARGET_VOLTAGE_V),
        BEND_RADIUS_MAX_LINES
    );
    let none = chans(TubeKind::Plumbicon)[G];
    assert_eq!(none.bend_radius_lines(TARGET_VOLTAGE_V), 0.0);
    let q = 30.0;
    assert!((ch.potential_v(q) - q / (ch.capacitance_f * 1e9)).abs() < 1e-12);
}

// ------------------------------------------------------------------ ACT

#[test]
fn the_act_gun_takes_32_times_peak_white_over_a_tenth_of_the_picture_height() {
    let (act, plain) = (chans(TubeKind::PlumbiconAct), chans(TubeKind::Plumbicon));
    for k in [R, G, B] {
        let s = after_spot(&act[k], ACT_RATED, ON_FRAMES, DARK_FRAMES);
        assert_eq!(tail_frames(&s, &act[k]), 0, "ACT ch{k}: {:.0?}", &s[..6]);
        assert!(
            s[0] < LAG_ONLY * act[k].peak_signal_na,
            "ACT ch{k}: first dark frame {:.1} nA",
            s[0]
        );
        let p = after_spot(&plain[k], ACT_RATED, ON_FRAMES, DARK_FRAMES);
        assert!(
            tail_frames(&p, &plain[k]) >= PLAIN_TAIL_MIN,
            "plain ch{k}: {:.0?}",
            &p[..6]
        );
    }
}

/// Mean of channel `ch` over `rows` at column `x`, nA.
fn at_column(s: &TargetSignal, ch: usize, rows: (usize, usize), x: usize) -> f64 {
    (rows.0..rows.1)
        .map(|r| s.rows[r][ch][x] as f64)
        .sum::<f64>()
        / (rows.1 - rows.0) as f64
}

/// The signal after `frames` frames of `light` times peak white wherever `lit(r, x)`, and the channels.
fn read_lit(
    kind: TubeKind,
    blooming: f64,
    light: f64,
    lit: &(dyn Fn(usize, usize) -> bool + Sync),
    frames: usize,
) -> (TargetSignal, [TubeChannel; 3]) {
    let mut cam = CameraTube::new(&CameraParams {
        tube: kind,
        blooming,
        ..CameraParams::default()
    })
    .unwrap();
    let chs = *cam.channels();
    let p: [f64; 3] = std::array::from_fn(|k| photo_current(&chs[k], light));
    let mut s = cam.read_currents(&|ch, r, x, _| if lit(r, x) { p[ch] } else { 0.0 });
    for _ in 1..frames {
        s = cam.read_currents(&|ch, r, x, _| if lit(r, x) { p[ch] } else { 0.0 });
    }
    (s, chs)
}

fn inside(((r0, r1), (x0, x1)): ((usize, usize), (usize, usize)), r: usize, x: usize) -> bool {
    r >= r0 && r < r1 && x >= x0 && x < x1
}

/// ACT pre-discharges a highlight before the read beam reaches it, so its elements stay below the
/// bending onset and the highlight does not bloom; at the top of the picture the first lines see one
/// flyback and bloom as on a plain gun (US 4,143,305). Green, 32 times peak white, bending on.
#[test]
fn the_act_flyback_cuts_blooming_except_at_the_top_of_the_picture() {
    let lit = |r: usize, x: usize| inside(TOP_BAR, r, x) || inside(MID_BAR, r, x);
    let (bent, chs) = read_lit(TubeKind::PlumbiconAct, BENDING, ACT_RATED, &lit, TOP_FRAMES);
    let (straight, _) = read_lit(TubeKind::PlumbiconAct, 0.0, ACT_RATED, &lit, TOP_FRAMES);
    let (plain, plain_chs) = read_lit(TubeKind::Plumbicon, BENDING, ACT_RATED, &lit, TOP_FRAMES);
    let peak = chs[G].peak_signal_na;

    // the first line of each field, just outside the top bar's edge
    let top_rows = (0, 2);
    let top_x = TOP_BAR.1 .1;
    let top = at_column(&bent, G, top_rows, top_x) - at_column(&straight, G, top_rows, top_x);
    assert!(
        top >= BLOOM_MIN * peak,
        "ACT, top of the picture: bloom {top:.1} nA"
    );

    // the middle of the middle bar, just outside its edge and further out
    let mid_rows = (ACTIVE_H / 2 - 4, ACTIVE_H / 2 + 4);
    for d in [0, BEYOND_SPOT - 1] {
        let x = MID_BAR.1 .1 + d;
        let mid = at_column(&bent, G, mid_rows, x) - at_column(&straight, G, mid_rows, x);
        assert!(
            mid.abs() <= DARK_MAX * peak,
            "ACT, middle, {d} samples out: bloom {mid:.2} nA"
        );
    }
    let x = MID_BAR.1 .1 + BEYOND_SPOT - 1;
    let dark = plain_chs[G].dark_na + plain_chs[G].bias_na;
    let bloom = at_column(&plain, G, mid_rows, x) - dark;
    assert!(
        bloom >= BLOOM_MIN * peak,
        "plain gun, middle: bloom {bloom:.1} nA"
    );
}

// ------------------------------------------------------------------ dynamic beam control

#[test]
fn dynamic_beam_control_takes_16_times_and_fails_a_stop_beyond() {
    let (dbc, plain) = (chans(TubeKind::PlumbiconDbc), chans(TubeKind::Plumbicon));
    for k in [R, G, B] {
        let s = after_spot(&dbc[k], DBC_RATED, ON_FRAMES, DARK_FRAMES);
        assert_eq!(
            tail_frames(&s, &dbc[k]),
            0,
            "DBC ch{k} at {DBC_RATED}: {:.0?}",
            &s[..6]
        );
        assert!(
            s[0] < LAG_ONLY * dbc[k].peak_signal_na,
            "DBC ch{k}: first dark frame {:.1} nA",
            s[0]
        );
        let p = after_spot(&plain[k], DBC_RATED, ON_FRAMES, DARK_FRAMES);
        assert!(
            tail_frames(&p, &plain[k]) >= PLAIN_TAIL_MIN,
            "plain ch{k}: {:.0?}",
            &p[..6]
        );
    }
    // red and blue store more than one pass of the largest beam takes; green's plate fits in one
    for k in [R, B] {
        let s = after_spot(&dbc[k], DBC_BEYOND, ON_FRAMES, DARK_FRAMES);
        assert!(
            tail_frames(&s, &dbc[k]) >= 1,
            "DBC ch{k} at {DBC_BEYOND}: {:.0?}",
            &s[..6]
        );
    }
}

/// Mean reading of the middle rows of a uniformly lit column after `STEADY_FRAMES` frames, nA.
fn steady(ch: &TubeChannel, photo_na: f64) -> f64 {
    let mut col = UniformColumn::new(ch);
    let mut s = col.read(&|_| photo_na);
    for _ in 1..STEADY_FRAMES {
        s = col.read(&|_| photo_na);
    }
    let rows = ACTIVE_H / 2 - 8..ACTIVE_H / 2 + 8;
    let n = rows.len() as f64;
    rows.map(|r| s[r]).sum::<f64>() / n
}

/// The Ampex law keeps the beam above the video, so a steady light reads its own current to the
/// rating; a plain gun stops at its beam; beyond the rating the beam stops at its largest.
#[test]
fn the_raised_beam_reads_linearly_to_its_rating_and_stops_at_its_largest() {
    let (dbc, plain) = (chans(TubeKind::PlumbiconDbc), chans(TubeKind::Plumbicon));
    for k in [R, G, B] {
        let i = photo_current(&dbc[k], DBC_LINEAR_LIGHT);
        let want = i + dbc[k].dark_na + dbc[k].bias_na;
        let got = steady(&dbc[k], i);
        assert!(
            (got - want).abs() <= LINEAR_TOLERANCE * want,
            "DBC ch{k}: {got:.1} nA vs {want:.1}"
        );
        let flat = steady(&plain[k], i);
        assert!(
            flat <= PLAIN_LIMIT_MARGIN * plain[k].beam_na,
            "plain ch{k}: {flat:.1} nA, beam {}",
            plain[k].beam_na
        );
    }
    let red = &dbc[R];
    let largest = DBC.max_ratio * red.peak_signal_na;
    let got = steady(red, photo_current(red, DBC_BEYOND));
    assert!(
        (got - largest).abs() <= DBC_MAX_TOLERANCE * largest,
        "DBC red beyond: {got:.1} nA vs {largest}"
    );
}

// ------------------------------------------------------------------ low-key lag

fn interpolate_log(readings: &[(f64, f64)], t: f64) -> f64 {
    let i = readings
        .iter()
        .position(|(ti, _)| *ti >= t)
        .expect("a reading after t");
    assert!(i > 0, "no reading before {t} s");
    let ((t0, a), (t1, b)) = (readings[i - 1], readings[i]);
    let f = (t - t0) / (t1 - t0);
    (a.ln() * (1.0 - f) + b.ln() * f).exp()
}

/// The handbook decay at 50 Hz: the signal above dark and bias as a percentage of the signal before the
/// light goes out, on the middle row and the row below at their read times, at `DECAY_TIMES_S`.
fn low_key_decay(ch: &TubeChannel) -> Vec<f64> {
    use vhsphys_core::pal::camera::{read_time_s, UniformColumn};
    use vhsphys_core::pal::consts::{ACTIVE_H, ACTIVE_W, FRAME_RATE};
    let (r0, x) = (ACTIVE_H / 2, ACTIVE_W / 2);
    let n_off = (LIGHT_BEFORE_DECAY_S * FRAME_RATE).ceil() as i64;
    let t_off = n_off as f64 / FRAME_RATE;
    let n_end = n_off + (KEEP_S * FRAME_RATE).ceil() as i64 + 1;
    let mut col = UniformColumn::new(ch);
    let photo = move |t: f64| if t < t_off { LOW_KEY.0 } else { 0.0 };
    let base = ch.dark_na + ch.bias_na;
    let (mut lit, mut out) = (f64::NAN, Vec::new());
    for n in 0..n_end {
        let s = col.read(&photo);
        if n == n_off - 1 {
            lit = s[r0];
        }
        for r in [r0, r0 + 1] {
            let t = read_time_s(n, r, x) - t_off;
            if t > 0.0 {
                out.push((t, s[r]));
            }
        }
    }
    out.sort_by(|a, b| a.0.total_cmp(&b.0));
    let pct: Vec<(f64, f64)> = out
        .iter()
        .map(|(t, s)| (*t, (100.0 * (s - base) / (lit - base)).max(1e-9)))
        .collect();
    DECAY_TIMES_S
        .iter()
        .map(|&t| interpolate_log(&pct, t))
        .collect()
}

/// Neither highlight gun touches ordinary low-key lag: the ACT flyback leaves elements below its cutting
/// level alone and the raised beam stays at its base, and the diode gun's target keeps `K`.
#[test]
fn low_key_lag_is_the_same_whatever_the_gun() {
    let at = |kind: TubeKind, k: usize| {
        let ch = chans(kind)[k];
        low_key_decay(&TubeChannel {
            beam_na: LOW_KEY.1,
            bias_na: 0.0,
            ..ch
        })
    };
    for k in [R, G, B] {
        let plain = at(TubeKind::Plumbicon, k);
        for kind in [TubeKind::PlumbiconAct, TubeKind::PlumbiconDbc] {
            let got = at(kind, k);
            for (i, (g, p)) in got.iter().zip(&plain).enumerate() {
                assert!(
                    (g / p - 1.0).abs() <= SAME_LAG,
                    "{kind:?} ch{k} at {} ms: {g:.3} % vs plain {p:.3} %",
                    DECAY_TIMES_S[i] * 1e3
                );
            }
        }
    }
}

// ------------------------------------------------------------------ beam bending

/// Excess of `a` over `b`, channel `ch`, averaged over `MID_ROWS` at column `x`, nA.
fn excess(a: &TargetSignal, b: &TargetSignal, ch: usize, x: usize) -> f64 {
    at_column(a, ch, MID_ROWS, x) - at_column(b, ch, MID_ROWS, x)
}

/// An overloaded element far above the onset draws the beam aimed beside it, so the bar's line signal
/// widens past the beam spot (where a straight beam reads dark) out to the radius at the plate.
#[test]
fn beam_bending_spreads_a_highlight_beyond_the_spot_out_to_its_radius() {
    let lit = |_: usize, x: usize| x >= BAR_X.0 && x < BAR_X.1;
    let (bent, chs) = read_lit(TubeKind::Plumbicon, BENDING, BENT_LIGHT, &lit, BAR_FRAMES);
    let (straight, _) = read_lit(TubeKind::Plumbicon, 0.0, BENT_LIGHT, &lit, BAR_FRAMES);
    let ch = &chs[G];
    let (peak, dark) = (ch.peak_signal_na, ch.dark_na + ch.bias_na);
    let edge = BAR_X.1 - 1;
    for x in [edge + BEYOND_SPOT, BAR_X.0 - BEYOND_SPOT] {
        let plain = at_column(&straight, G, MID_ROWS, x) - dark;
        assert!(
            plain.abs() <= DARK_MAX * peak,
            "straight beam at {x}: {plain:.2} nA above dark"
        );
        let bloom = excess(&bent, &straight, G, x);
        assert!(
            bloom >= BLOOM_MIN * peak,
            "bent beam at {x}: {bloom:.1} nA more"
        );
    }
    for x in [edge + BEYOND_RADIUS, BAR_X.0 - BEYOND_RADIUS] {
        let far = excess(&bent, &straight, G, x);
        assert!(
            far.abs() <= DARK_MAX * peak,
            "bent beam at {x}, past the radius: {far:.2} nA more"
        );
    }
}

/// Readings of one column with a band of rows lit, averaged over frames `BAND_SETTLE..BAND_FRAMES`.
fn band_readings(ch: &TubeChannel, light: f64) -> Vec<f64> {
    let mut col = UniformColumn::new(ch);
    let p = photo_current(ch, light);
    let photo = move |r: usize, _: f64| {
        if r >= BAND_ROWS.0 && r < BAND_ROWS.1 {
            p
        } else {
            0.0
        }
    };
    let mut sum = vec![0.0; ACTIVE_H];
    for n in 0..BAND_FRAMES {
        let s = col.read_rows(&photo);
        if n >= BAND_SETTLE {
            for (a, v) in sum.iter_mut().zip(&s) {
                *a += v;
            }
        }
    }
    sum.iter()
        .map(|a| a / (BAND_FRAMES - BAND_SETTLE) as f64)
        .collect()
}

/// The rows just outside the band, nearest first, on both sides.
fn outside_rows(d: usize) -> [usize; 2] {
    [BAND_ROWS.1 + d, BAND_ROWS.0 - 1 - d]
}

/// The brighter the highlight, the further its elements stay above the onset and the more beam they
/// draw: across lines, the bloom outside a lit band grows with the overload and reaches past the
/// vertical spot (whose wings end 1.95 lines out).
#[test]
fn beam_bending_grows_with_the_overload() {
    let g = chans(TubeKind::Plumbicon)[G];
    let bent = TubeChannel {
        bending: BENDING,
        ..g
    };
    let (peak, dark) = (g.peak_signal_na, g.dark_na + g.bias_na);
    let (mild_b, mild_s) = (
        band_readings(&bent, MILD_LIGHT),
        band_readings(&g, MILD_LIGHT),
    );
    let (hard_b, hard_s) = (
        band_readings(&bent, BENT_LIGHT),
        band_readings(&g, BENT_LIGHT),
    );
    let total = |b: &[f64], s: &[f64]| {
        (0..BLOOM_ROWS)
            .flat_map(outside_rows)
            .map(|r| b[r] - s[r])
            .sum::<f64>()
    };
    let (mild, hard) = (total(&mild_b, &mild_s), total(&hard_b, &hard_s));
    assert!(mild > 0.0, "no bloom at {MILD_LIGHT} times: {mild:.1} nA");
    assert!(
        hard >= BLOOM_GROWTH * mild,
        "bloom {mild:.1} nA at {MILD_LIGHT} times, {hard:.1} nA at {BENT_LIGHT}"
    );
    for r in outside_rows(BEYOND_SPOT - 1) {
        assert!(
            (hard_s[r] - dark).abs() <= DARK_MAX * peak,
            "straight beam, row {r}: {:.2} nA",
            hard_s[r]
        );
        assert!(
            hard_b[r] - hard_s[r] >= BLOOM_MIN * peak,
            "bent beam, row {r}: {:.1} nA more",
            hard_b[r] - hard_s[r]
        );
    }
}

// ------------------------------------------------------------------ trail colour

/// Middle-row readings as shares of each channel's peak above dark and bias, per dark frame, after
/// `light` times peak white on the whole target for `on` frames.
fn trail(chs: &[TubeChannel; 3], light: f64, on: usize, dark: usize) -> Vec<[f64; 3]> {
    let t_off = on as f64 / FRAME_RATE;
    let per: Vec<Vec<f64>> = chs
        .iter()
        .map(|ch| {
            let mut col = UniformColumn::new(ch);
            let p = photo_current(ch, light);
            let base = ch.dark_na + ch.bias_na;
            (0..on + 1 + dark)
                .map(|_| {
                    (col.read(&|t| if t < t_off { p } else { 0.0 })[ACTIVE_H / 2] - base)
                        / ch.peak_signal_na
                })
                .skip(on + 1)
                .collect()
        })
        .collect();
    (0..dark)
        .map(|n| [per[R][n], per[G][n], per[B][n]])
        .collect()
}

/// Seconds after the light went out (at the start of frame `on`) at which dark frame `n` reads the
/// middle row.
fn after_off_s(on: usize, n: usize) -> f64 {
    read_time_s((on + 1 + n) as i64, ACTIVE_H / 2, ACTIVE_W / 2) - on as f64 / FRAME_RATE
}

/// Lead oxide: the red tube stores more overload per unit of beam than green, so the trail turns red
/// (RCA 1977, "red-trail"): red reads at least green's share all the way down, still reads clipped white
/// when green has fallen below half, and its clipped tail outlasts green's.
#[test]
fn a_lead_oxide_trail_is_red() {
    let chs = chans(TubeKind::Plumbicon);
    let t = trail(&chs, TRAIL_LIGHT, TRAIL_ON, TRAIL_DARK);
    for (n, f) in t.iter().take_while(|f| f[R] >= TRAIL_FLOOR).enumerate() {
        assert!(
            f[R] >= f[G] - SHARE_ROUNDING,
            "dark frame {n}: R {:.3} G {:.3}",
            f[R],
            f[G]
        );
    }
    let n = t
        .iter()
        .position(|f| f[G] < 0.5)
        .expect("green falls below half");
    assert!(
        t[n][R] >= 1.0,
        "dark frame {n}: green {:.3}, red {:.3} of peak",
        t[n][G],
        t[n][R]
    );
    let clipped = |k: usize| t.iter().take_while(|f| f[k] > 1.0).count();
    assert!(
        clipped(R) >= clipped(G) + RED_OVER_GREEN_FRAMES,
        "clipped tail: R {} frames, G {}",
        clipped(R),
        clipped(G)
    );
}

/// The Saticon camera's three tubes are alike, so its residue is white, and at a beam of 3.5 times the
/// highlight it is gone 2 s after the highlight (RCA 1977, Neuhauser 1978).
#[test]
fn a_saticon_residue_is_white_and_gone_in_two_seconds() {
    let chs = TubeKind::Saticon
        .channels(SATICON_BEAM_RATIO)
        .expect("a tube");
    let on = (SATICON_ON_S * FRAME_RATE).ceil() as usize;
    let dark = (SATICON_GONE_S * FRAME_RATE).ceil() as usize + 1;
    let t = trail(&chs, SATICON_LIGHT, on, dark);
    for (n, f) in t.iter().enumerate() {
        let spread =
            f.iter().cloned().fold(f64::MIN, f64::max) - f.iter().cloned().fold(f64::MAX, f64::min);
        assert!(spread <= WHITE_SPREAD, "dark frame {n}: {f:.4?}");
    }
    let gone = (0..dark)
        .find(|&n| after_off_s(on, n) >= SATICON_GONE_S)
        .expect("a reading 2 s after");
    let f = t[gone];
    assert!(
        f.iter().all(|v| *v < SATICON_GONE),
        "{:.3} s after: {f:.4?}",
        after_off_s(on, gone)
    );
}
