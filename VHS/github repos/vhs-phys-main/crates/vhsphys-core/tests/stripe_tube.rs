//! The stripe colour camera joined to the scanned tube (docs/design/CAMERA_STRIPE.md, "Joined to the
//! tube"): the scene's light passes the optical low-pass and the stripes onto one target, the scan
//! engine reads it, the head amplifier adds its noise to the one line signal, the decoder takes colour
//! out of the carrier, and the processing amplifier clamps, balances and corrects R, G and B.
//!
//! Sources: docs/CAMERA_RESEARCH.md, "Single-tube camcorders trade colour bandwidth for a striped
//! carrier" (Panasonic WV-3110: 3.58 MHz carrier, 2.8 MHz luminance, 0.5 MHz colour, green highlights
//! when the beam cannot discharge the target) and "Traps add an exposure-dependent tail, strongest in
//! blue PbO and in Sb2S3"; Philips 1963 (lag is longer at lower current). The camera off and the
//! three-tube camera are covered by the golden test and the other camera tests.

use vhsphys_core::camera::{
    CameraParams, CameraTube, Layout, UniformColumn, ALIGN_FRAMES, STRIPE_SLOT,
};
use vhsphys_core::camera_processing::{gamma_inverse, Processing};
use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W, FS_BB};
use vhsphys_core::stripe::{STRIPE_CARRIER_HZ, STRIPE_WHITE_LIGHT};
use vhsphys_core::tube::{oetf, oetf_inverse, photo_current, TubeKind, BEAM_RATIO_DEFAULT};
use vhsphys_core::video::{test_pattern, Frame};

use std::f64::consts::PI;

const R: usize = 0;
const G: usize = 1;
const B: usize = 2;

/// A linear target for the colour checks (the 1/2-inch Saticon of JVC's GR-C1) and the WV-3110's own
/// Sb2S3 vidicon for lag (20 % at 50 ms).
const COLOUR_TUBE: TubeKind = TubeKind::Saticon;
const LAG_TUBE: TubeKind = TubeKind::Vidicon;

/// Frames a still scene is read before it is measured: enough for a Saticon target to come within a
/// few tenths of a percent of its settled reading from dark.
const SETTLE_FRAMES: usize = 12;

/// The read's line signal: frames of a red field before the spectrum is taken, rows and line-end
/// samples left out, how far from the carrier the floor is taken, and the bounds. A red field puts a
/// carrier of the light's own size on every row (`T_R = (1 + cos) / 2`); the horizontal spot weakens
/// it by `exp(-2 pi^2 sigma_h^2 f_c^2)`, and the vertical overlap of the slanted stripes of the other
/// field's lines weakens it further, by no more than a quarter.
const CARRIER_FRAMES: usize = 3;
const SPECTRUM_ROWS: (usize, usize) = (200, 280);
const LINE_MARGIN: usize = 64;
const OFF_CARRIER_HZ: f64 = 1.0e6;
const CARRIER_SHARE: (f64, f64) = (0.75, 1.02);
const OFF_CARRIER_MAX: f64 = 0.03;

/// Alignment on a white card: the white levels of a linear target are close to one, and the decoded
/// noise of a capped lens has a positive envelope in red and blue.
const WHITE_LEVEL_TOLERANCE: f64 = 0.03;
const FLOOR_MAX: f64 = 0.1;

/// A still white and a mid grey (linear light) and how close the gamma-corrected channels must agree.
/// Darker greys lean green (the noise's envelope is clamped off a weak carrier) and are not judged.
const WHITE_TOLERANCE: f64 = 0.015;
const MID_GREY: f64 = 0.5;
const GREY_SPREAD: f64 = 0.025;
const GREY_LEVEL_TOLERANCE: f64 = 0.03;

/// Flat greys on which the stripe camera and the three-tube camera with the same target material must
/// give the same luminance (gamma-corrected `0.299 R + 0.587 G + 0.114 B`).
const LUMA_GREYS: [f64; 2] = [0.18, 0.5];
const LUMA_TOLERANCE: f64 = 0.01;

/// Colour bars (`test_pattern`, 75 %): rows averaged, the share of each bar's width averaged about its
/// centre, and the bounds on hue (degrees, in the linear colour-difference plane), saturation (ratio of
/// the colour-difference magnitudes) and linear luminance.
const BAR_ROWS: (usize, usize) = (40, 280);
const BAR_INNER: (f64, f64) = (0.3, 0.7);
const BAR_FRAMES: usize = 10;
const HUE_TOLERANCE_DEG: f64 = 5.0;
const SATURATION_RATIO: (f64, f64) = (0.9, 1.05);
const BAR_LUMA_TOLERANCE: f64 = 0.015;

/// A colour step and a luminance step across the line, measured on the decoded linear colour: the
/// 10-90 % widths allowed (the decoder's own bounds, `tests/stripe_camera.rs`) and how much wider colour
/// must be than luminance (0.5 MHz against 2.8 MHz).
const STEP_AT: usize = ACTIVE_W / 2;
const STEP_ROWS: (usize, usize) = (200, 280);
const STEP_FRAMES: usize = 6;
const STEP_LEVEL_OFFSET: usize = 60;
const COLOUR_RISE_S: (f64, f64) = (0.4e-6, 1.3e-6);
const LUMA_RISE_S: (f64, f64) = (0.05e-6, 0.35e-6);
const COLOUR_OVER_LUMA_MIN: f64 = 2.5;

/// Lag: a grey held, then darkness. The first dark frame still holds light from the lit picture in
/// most rows; the next two hold only what the target kept.
const LAG_LIGHT: f64 = 0.5;
const LAG_LIT_FRAMES: usize = 12;
const LAG_DARK_FRAMES: usize = 3;
const LAG_JUDGED: [usize; 2] = [1, 2];
/// One target: red and blue keep the same share of their lit reading, to this.
const RED_BLUE_ALIKE: f64 = 0.02;
/// Three PbO tubes: blue's traps keep at least this multiple of red's share.
const PBO_BLUE_OVER_RED: f64 = 1.3;
/// The carrier's residue fades before the luminance's: green keeps at least this much more share.
const GREEN_TRAIL_EXCESS: f64 = 0.1;

/// Highlights: two stops over white put every element past the beam (the white's stripe troughs reach
/// twice peak current, the beam), one stop puts the stripe peaks past it.
const BEYOND_BEAM_STOPS: f64 = 2.0;
const PEAKS_BEYOND_STOPS: f64 = 1.0;
const HIGHLIGHT_FRAMES: usize = 6;
/// With no carrier left, red and blue read at most this (linear) and green at least this, and green
/// stands this far above them in the gamma-corrected picture.
const NO_CARRIER_MAX: f64 = 0.1;
const BEYOND_GREEN_MIN: f64 = 3.0;
const GREEN_OVER_RB_VIDEO: f64 = 0.5;
/// One stop over: without the beam limit red and blue would read 2; the carrier stops near 1.
const CLIPPED_CARRIER_MAX: f64 = 1.3;
const PEAKS_GREEN_MIN: f64 = 2.2;

fn stripe(kind: TubeKind, iris_stops: f64) -> CameraTube {
    CameraTube::new(&CameraParams {
        tube: kind,
        layout: Layout::Stripe,
        processing: Processing::Consumer,
        iris_stops,
        ..CameraParams::default()
    })
    .unwrap()
}

fn three_tube(kind: TubeKind) -> CameraTube {
    CameraTube::new(&CameraParams {
        tube: kind,
        processing: Processing::Consumer,
        ..CameraParams::default()
    })
    .unwrap()
}

/// A picture of linear light `light(r, x)` through the input's gamma.
fn picture(light: impl Fn(usize, usize) -> [f64; 3]) -> Frame {
    let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
    for r in 0..ACTIVE_H {
        for x in 0..ACTIVE_W {
            let l = light(r, x);
            for k in 0..3 {
                f.rgb[(r * ACTIVE_W + x) * 3 + k] = (oetf(l[k]) * 255.0).round() as u8;
            }
        }
    }
    f
}

fn flat(l: f64) -> Frame {
    picture(|_, _| [l; 3])
}

/// Mean of each channel over the middle half of the rows and of the line.
fn centre(rows: &[[Vec<f32>; 3]]) -> [f64; 3] {
    let (h, w) = (rows.len(), rows[0][G].len());
    let mut sum = [0.0; 3];
    let mut n = 0.0;
    for row in &rows[h / 4..3 * h / 4] {
        for x in w / 4..3 * w / 4 {
            for k in 0..3 {
                sum[k] += row[k][x] as f64;
            }
            n += 1.0;
        }
    }
    sum.map(|s| s / n)
}

fn luma(v: [f64; 3]) -> f64 {
    0.299 * v[R] + 0.587 * v[G] + 0.114 * v[B]
}

/// The horizontal spot's response at the carrier.
fn spot_at_carrier(sigma_h_s: f64) -> f64 {
    (-2.0 * PI * PI * sigma_h_s * sigma_h_s * STRIPE_CARRIER_HZ * STRIPE_CARRIER_HZ).exp()
}

/// The head amplifier's band at the carrier (analog second-order Butterworth).
fn band_at_carrier(processing: Processing) -> f64 {
    let band = processing.spec().head_amp.band_hz;
    1.0 / (1.0 + (STRIPE_CARRIER_HZ / band).powi(4)).sqrt()
}

#[test]
fn the_layout_is_named_in_the_config() {
    assert_eq!(CameraParams::default().layout, Layout::ThreeTube);
    let p: CameraParams = toml::from_str("tube = \"saticon\"\nlayout = \"stripe\"").unwrap();
    assert_eq!((p.tube, p.layout), (TubeKind::Saticon, Layout::Stripe));
    let q: CameraParams = toml::from_str("tube = \"saticon\"\nlayout = \"three-tube\"").unwrap();
    assert_eq!(q.layout, Layout::ThreeTube);
    let mut cam = three_tube(TubeKind::Saticon);
    assert_eq!(cam.layout(), Layout::ThreeTube);
    assert!(cam.stripe_alignment().is_none());
    let f = flat(MID_GREY);
    assert!(cam.stripe_colour(&f, &f).is_none());
}

/// One target, the kind's green tube, reads the stripes: its line signal carries the 3.58 MHz carrier
/// (Panasonic WV-3110), weakened by the read and nowhere else in the band.
#[test]
fn one_target_carries_the_stripe_carrier() {
    for kind in [COLOUR_TUBE, TubeKind::Plumbicon] {
        let green = kind.channels_at(BEAM_RATIO_DEFAULT, 25.0).unwrap()[G];
        let cam = stripe(kind, 0.0);
        assert_eq!(cam.layout(), Layout::Stripe);
        assert!(cam.channels().iter().all(|c| *c == green), "{kind:?}");
        let black = cam.black();
        assert!(black.iter().all(|&b| b == black[STRIPE_SLOT]), "{kind:?}");
    }

    let mut cam = stripe(COLOUR_TUBE, 0.0);
    let red = picture(|_, _| [1.0, 0.0, 0.0]);
    let mut sig = cam.read(&red, &red);
    for _ in 1..CARRIER_FRAMES {
        sig = cam.read(&red, &red);
    }
    assert!(sig.rows.iter().all(|r| r[R].is_empty() && r[B].is_empty()));
    let span = LINE_MARGIN..ACTIVE_W - LINE_MARGIN;
    let share_at = |hz: f64| {
        let rows = SPECTRUM_ROWS.0..SPECTRUM_ROWS.1;
        let n_rows = rows.len() as f64;
        rows.map(|r| {
            let line = &sig.rows[r][STRIPE_SLOT];
            let n = span.len() as f64;
            let mean = span.clone().map(|x| line[x] as f64).sum::<f64>() / n;
            let (mut re, mut im) = (0.0, 0.0);
            for x in span.clone() {
                let a = 2.0 * PI * hz * x as f64 / FS_BB;
                re += (line[x] as f64 - mean) * a.cos();
                im += (line[x] as f64 - mean) * a.sin();
            }
            2.0 * re.hypot(im) / n / mean
        })
        .sum::<f64>()
            / n_rows
    };
    let spot = spot_at_carrier(cam.channels()[STRIPE_SLOT].sigma_h_s);
    let at = share_at(STRIPE_CARRIER_HZ);
    assert!(
        at >= CARRIER_SHARE.0 * spot && at <= CARRIER_SHARE.1 * spot,
        "carrier {at:.4} of the light, spot {spot:.4}"
    );
    for hz in [
        STRIPE_CARRIER_HZ - OFF_CARRIER_HZ,
        STRIPE_CARRIER_HZ + OFF_CARRIER_HZ,
    ] {
        let off = share_at(hz);
        assert!(off <= OFF_CARRIER_MAX, "{hz} Hz: {off:.4}");
    }
}

/// The alignment restores the carrier the read weakened: its gain is at least the reciprocal of the
/// spot and amplifier at the carrier, and at most a quarter more for the vertical overlap; white then
/// decodes to one in every colour, and the capped lens leaves the envelope of the noise in red and blue.
#[test]
fn the_alignment_restores_the_carrier_and_balances_white() {
    assert!(ALIGN_FRAMES >= 1);
    let cam = stripe(COLOUR_TUBE, 0.0);
    let a = cam.stripe_alignment().unwrap();
    let read = spot_at_carrier(cam.channels()[STRIPE_SLOT].sigma_h_s)
        * band_at_carrier(Processing::Consumer);
    assert!(
        a.carrier_gain >= 1.0 / read && a.carrier_gain <= 1.0 / (CARRIER_SHARE.0 * read),
        "carrier gain {:.4}, read {read:.4}",
        a.carrier_gain
    );
    for k in 0..3 {
        assert!(
            (a.white[k] - 1.0).abs() < WHITE_LEVEL_TOLERANCE,
            "white {:?}",
            a.white
        );
    }
    for k in [R, B] {
        assert!(
            a.floor[k] > 0.0 && a.floor[k] < FLOOR_MAX,
            "floor {:?}",
            a.floor
        );
    }
    assert_eq!(STRIPE_WHITE_LIGHT, 2.0);
}

/// A still white reads white and a mid grey reads grey.
#[test]
fn a_still_grey_scene_reads_grey() {
    for (level, frames, spread, tolerance) in [
        (1.0, ALIGN_FRAMES, WHITE_TOLERANCE, WHITE_TOLERANCE),
        (MID_GREY, SETTLE_FRAMES, GREY_SPREAD, GREY_LEVEL_TOLERANCE),
    ] {
        let mut cam = stripe(COLOUR_TUBE, 0.0);
        let f = flat(level);
        let mut v = [0.0; 3];
        for _ in 0..frames {
            v = centre(&cam.video(&f, &f).rows);
        }
        let hi = v.iter().cloned().fold(f64::MIN, f64::max);
        let lo = v.iter().cloned().fold(f64::MAX, f64::min);
        assert!(hi - lo <= spread, "{level}: {v:.4?}");
        for c in v {
            assert!((c - oetf(level)).abs() <= tolerance, "{level}: {v:.4?}");
        }
    }
}

/// The stripe camera and three tubes of the same material give a flat field the same luminance.
#[test]
fn the_stripe_and_three_tube_cameras_agree_on_luminance() {
    for level in LUMA_GREYS {
        let f = flat(level);
        let mut one = stripe(COLOUR_TUBE, 0.0);
        let mut three = three_tube(COLOUR_TUBE);
        let (mut a, mut b) = ([0.0; 3], [0.0; 3]);
        for _ in 0..SETTLE_FRAMES {
            a = centre(&one.video(&f, &f).rows);
            b = centre(&three.video(&f, &f).rows);
        }
        assert!(
            (luma(a) - luma(b)).abs() <= LUMA_TOLERANCE,
            "{level}: stripe {a:.4?}, three tubes {b:.4?}"
        );
    }
}

/// Hue and saturation in the linear colour-difference plane.
fn hue_saturation(v: [f64; 3]) -> (f64, f64) {
    let y = luma(v);
    let (u, w) = (v[B] - y, v[R] - y);
    (w.atan2(u).to_degrees(), u.hypot(w))
}

/// The colour bars come out with their hue and saturation.
#[test]
fn colour_bars_keep_their_hue_and_saturation() {
    let mut cam = stripe(COLOUR_TUBE, 0.0);
    let f = test_pattern();
    let mut rows = Vec::new();
    for _ in 0..BAR_FRAMES {
        rows = cam.video(&f, &f).rows;
    }
    // bar 0 is grey; the six after it carry colour
    for bar in 1..7 {
        let x0 = ((bar as f64 + BAR_INNER.0) * ACTIVE_W as f64 / 7.0) as usize;
        let x1 = ((bar as f64 + BAR_INNER.1) * ACTIVE_W as f64 / 7.0) as usize;
        let mut sum = [0.0; 3];
        let mut n = 0.0;
        for row in &rows[BAR_ROWS.0..BAR_ROWS.1] {
            for x in x0..x1 {
                for k in 0..3 {
                    sum[k] += row[k][x] as f64;
                }
                n += 1.0;
            }
        }
        let out = sum.map(|s| gamma_inverse(s / n));
        let p = (BAR_ROWS.0 * ACTIVE_W + (x0 + x1) / 2) * 3;
        let input: [f64; 3] = std::array::from_fn(|k| oetf_inverse(f.rgb[p + k] as f64 / 255.0));
        let (hue_in, sat_in) = hue_saturation(input);
        let (hue_out, sat_out) = hue_saturation(out);
        let d_hue = (hue_out - hue_in + 540.0).rem_euclid(360.0) - 180.0;
        let ratio = sat_out / sat_in;
        assert!(
            d_hue.abs() <= HUE_TOLERANCE_DEG,
            "bar {bar}: {input:.3?} -> {out:.3?}, hue {d_hue:.1}"
        );
        assert!(
            ratio >= SATURATION_RATIO.0 && ratio <= SATURATION_RATIO.1,
            "bar {bar}: {input:.3?} -> {out:.3?}, saturation x{ratio:.3}"
        );
        assert!(
            (luma(out) - luma(input)).abs() <= BAR_LUMA_TOLERANCE,
            "bar {bar}: {input:.3?} -> {out:.3?}"
        );
    }
}

/// 10-90 % width of a step at `STEP_AT` in channel `k`, averaged over `STEP_ROWS`, seconds.
fn step_width(rows: &[[Vec<f32>; 3]], k: usize) -> f64 {
    let n = (STEP_ROWS.1 - STEP_ROWS.0) as f64;
    let line: Vec<f64> = (0..ACTIVE_W)
        .map(|x| {
            rows[STEP_ROWS.0..STEP_ROWS.1]
                .iter()
                .map(|r| r[k][x] as f64)
                .sum::<f64>()
                / n
        })
        .collect();
    let (lo, hi) = (
        line[STEP_AT - STEP_LEVEL_OFFSET],
        line[STEP_AT + STEP_LEVEL_OFFSET],
    );
    let level = |x: usize| (line[x] - lo) / (hi - lo);
    let window = STEP_AT - 40..STEP_AT + 40;
    let first = window.clone().find(|&x| level(x) >= 0.1).expect("10 %");
    let last = window.clone().find(|&x| level(x) >= 0.9).expect("90 %");
    (last - first) as f64 / FS_BB
}

/// Colour is about half a megahertz wide and luminance about three, through the whole read.
#[test]
fn colour_is_half_a_megahertz_wide_and_luminance_three() {
    let measure = |light: &dyn Fn(usize, usize) -> [f64; 3], k: usize| {
        let mut cam = stripe(COLOUR_TUBE, 0.0);
        let f = picture(light);
        let mut rows = Vec::new();
        for _ in 0..STEP_FRAMES {
            rows = cam.stripe_colour(&f, &f).unwrap();
        }
        step_width(&rows, k)
    };
    let colour = measure(
        &|_, x| {
            if x < STEP_AT {
                [0.0, 0.5, 0.0]
            } else {
                [0.8, 0.5, 0.0]
            }
        },
        R,
    );
    let grey = measure(&|_, x| if x < STEP_AT { [0.2; 3] } else { [0.8; 3] }, G);
    assert!(
        colour >= COLOUR_RISE_S.0 && colour <= COLOUR_RISE_S.1,
        "colour edge {colour:e} s"
    );
    assert!(
        grey >= LUMA_RISE_S.0 && grey <= LUMA_RISE_S.1,
        "luminance edge {grey:e} s"
    );
    assert!(
        colour >= COLOUR_OVER_LUMA_MIN * grey,
        "colour {colour:e} s, luminance {grey:e} s"
    );
}

/// Each colour's share of its lit reading on the dark frames after a grey, through the stripe camera.
fn stripe_trail(kind: TubeKind) -> Vec<[f64; 3]> {
    let mut cam = stripe(kind, 0.0);
    let (lit_f, dark_f) = (flat(LAG_LIGHT), flat(0.0));
    let mut lit = [0.0; 3];
    for _ in 0..LAG_LIT_FRAMES {
        lit = centre(&cam.stripe_colour(&lit_f, &lit_f).unwrap());
    }
    (0..LAG_DARK_FRAMES)
        .map(|_| {
            let d = centre(&cam.stripe_colour(&dark_f, &dark_f).unwrap());
            std::array::from_fn(|k| d[k] / lit[k])
        })
        .collect()
}

/// One target lags alike in red and blue: both ride the same carrier off the same charge. Three PbO
/// tubes do not: blue's traps keep its trail longest.
#[test]
fn one_target_trails_red_and_blue_alike() {
    let trail = stripe_trail(LAG_TUBE);
    for i in LAG_JUDGED {
        assert!(
            (trail[i][R] - trail[i][B]).abs() <= RED_BLUE_ALIKE,
            "dark frame {i}: {:.4?}",
            trail[i]
        );
    }

    let pbo = TubeKind::Plumbicon.channels(BEAM_RATIO_DEFAULT).unwrap();
    let shares: Vec<Vec<f64>> = pbo
        .iter()
        .map(|ch| {
            let mut col = UniformColumn::new(ch);
            let p = photo_current(ch, LAG_LIGHT);
            let mid = ACTIVE_H / 2;
            let mut lit = 0.0;
            for _ in 0..LAG_LIT_FRAMES {
                lit = col.read(&|_| p)[mid] - ch.dark_na;
            }
            (0..LAG_DARK_FRAMES)
                .map(|_| (col.read(&|_| 0.0)[mid] - ch.dark_na) / lit)
                .collect()
        })
        .collect();
    for i in LAG_JUDGED {
        assert!(
            shares[B][i] >= PBO_BLUE_OVER_RED * shares[R][i],
            "PbO dark frame {i}: R {:.4} B {:.4}",
            shares[R][i],
            shares[B][i]
        );
    }
}

/// The carrier's residue fades before the light's: lag is longer at lower current (Philips 1963), and
/// the stripes' troughs carry less current than their peaks, so what the target keeps is less
/// modulated than the light was, and a trail through the stripes is greener than the picture it
/// follows.
#[test]
fn a_trail_loses_its_colour_before_its_light() {
    let trail = stripe_trail(LAG_TUBE);
    for i in LAG_JUDGED {
        let t = trail[i];
        assert!(
            t[G] - t[R].max(t[B]) >= GREEN_TRAIL_EXCESS,
            "dark frame {i}: {t:.4?}"
        );
    }
}

/// A highlight the beam cannot discharge loses its carrier and turns green (Panasonic WV-3110).
#[test]
fn a_highlight_beyond_the_beam_turns_green() {
    let white = flat(1.0);

    let mut cam = stripe(COLOUR_TUBE, BEYOND_BEAM_STOPS);
    let mut v = [0.0; 3];
    for _ in 0..HIGHLIGHT_FRAMES {
        v = centre(&cam.video(&white, &white).rows);
    }
    assert!(
        v[G] - v[R] >= GREEN_OVER_RB_VIDEO && v[G] - v[B] >= GREEN_OVER_RB_VIDEO,
        "video {v:.4?}"
    );
    let d = centre(&cam.stripe_colour(&white, &white).unwrap());
    assert!(
        d[R] <= NO_CARRIER_MAX && d[B] <= NO_CARRIER_MAX && d[G] >= BEYOND_GREEN_MIN,
        "decoded {d:.4?}"
    );

    let mut cam = stripe(COLOUR_TUBE, PEAKS_BEYOND_STOPS);
    let mut d = [0.0; 3];
    for _ in 0..HIGHLIGHT_FRAMES {
        d = centre(&cam.stripe_colour(&white, &white).unwrap());
    }
    assert!(
        d[R] <= CLIPPED_CARRIER_MAX && d[B] <= CLIPPED_CARRIER_MAX && d[G] >= PEAKS_GREEN_MIN,
        "decoded {d:.4?}"
    );
}

/// The 625-line engine runs the stripe camera too: its carrier is not a whole number of samples per
/// cycle, and the alignment still balances white and grey.
#[test]
fn the_625_line_engine_runs_the_stripe_camera() {
    use vhsphys_core::pal::camera as pal_camera;
    use vhsphys_core::pal::camera_processing::Processing as PalProcessing;
    use vhsphys_core::pal::consts as pc;

    let mut cam = pal_camera::CameraTube::new(&pal_camera::CameraParams {
        tube: COLOUR_TUBE,
        layout: pal_camera::Layout::Stripe,
        processing: PalProcessing::Consumer,
        ..pal_camera::CameraParams::default()
    })
    .unwrap();
    for (level, spread, tolerance) in [
        (1.0, WHITE_TOLERANCE, WHITE_TOLERANCE),
        (MID_GREY, GREY_SPREAD, GREY_LEVEL_TOLERANCE),
    ] {
        let mut f = Frame::new(pc::ACTIVE_W, pc::ACTIVE_H);
        f.rgb.fill((oetf(level) * 255.0).round() as u8);
        let mut v = [0.0; 3];
        for _ in 0..SETTLE_FRAMES {
            v = centre(&cam.video(&f, &f).rows);
        }
        let hi = v.iter().cloned().fold(f64::MIN, f64::max);
        let lo = v.iter().cloned().fold(f64::MAX, f64::min);
        assert!(hi - lo <= spread, "{level}: {v:.4?}");
        for c in v {
            assert!((c - oetf(level)).abs() <= tolerance, "{level}: {v:.4?}");
        }
    }
}
