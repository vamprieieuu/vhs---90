//! The camera pickup tube: the data-sheet operating points, comet tails from the beam limit,
//! the ACT gun, and the stage in the chain (off = untouched; a static scene is not restyled).
//! The data-sheet lag is in `camera_lag.rs`.

mod common;
use vhsphys_core::camera::UniformColumn;
use vhsphys_core::tube::*;
use vhsphys_core::{config, consts::*, pipeline::Session, video::Frame};

/// Still-scene checks: the flat greys, how many frames they are held first, the edge rows left
/// out, and the tolerances in 8-bit levels (flat: the rounding of the output; blocks: the tube's
/// gamma, about 5 levels in the shadows and mid greys, plus the spot on the finest stripes).
const STILL_GREYS: [u8; 3] = [32, 128, 200];
const SETTLE_FRAMES: usize = 20;
const EDGE_ROWS: usize = 8;
const FLAT_TOLERANCE: f64 = 1.0;
const TONE_BLOCK: usize = 16;
const TONE_TOLERANCE: f64 = 5.0;
/// Frames read after a highlight in the tail checks.
const DARK_FRAMES: usize = 200;

fn chans(kind: TubeKind) -> [TubeChannel; 3] {
    kind.channels(BEAM_RATIO_DEFAULT).expect("a tube")
}

#[test]
fn off_has_no_channels() {
    assert!(TubeKind::Off.channels(BEAM_RATIO_DEFAULT).is_none());
}

#[test]
fn operating_points_are_the_data_sheets() {
    for kind in [TubeKind::Plumbicon, TubeKind::PlumbiconBias, TubeKind::PlumbiconAct] {
        let c = chans(kind);
        for k in 0..3 {
            assert_eq!(c[k].peak_signal_na, PEAK_SIGNAL_NA[k]);
            assert!((c[k].beam_na - 2.0 * PEAK_SIGNAL_NA[k]).abs() < 1e-9);
            assert_eq!(c[k].act, kind == TubeKind::PlumbiconAct);
            let bias = if kind == TubeKind::PlumbiconBias { LIGHT_BIAS_XQ1410_NA[k] } else { 0.0 };
            assert_eq!(c[k].bias_na, bias);
            assert_eq!(c[k].dark_na, DARK_NA);
        }
    }
}

/// Signal of the middle row of a uniformly lit target, one reading per frame: `light` (1.0 =
/// peak white) for `on` frames, then darkness. The readings start with the first frame whose
/// middle row integrated no light (the frame of the change still holds some).
fn after_highlight(ch: &TubeChannel, light: f64, on: usize, dark: usize) -> Vec<f64> {
    let mut col = UniformColumn::new(ch);
    let p = photo_current(ch, light);
    let t_off = on as f64 / FRAME_RATE;
    let photo = move |t: f64| if t < t_off { p } else { 0.0 };
    (0..on + 1 + dark).map(|_| col.read(&photo)[ACTIVE_H / 2]).skip(on + 1).collect()
}

/// Frames after the highlight has gone that still read above peak white (clipped white).
fn tail_frames(ch: &TubeChannel, light: f64, on: usize) -> usize {
    after_highlight(ch, light, on, DARK_FRAMES).iter().take_while(|&&s| s > ch.peak_signal_na).count()
}

#[test]
fn a_highlight_beyond_the_beam_leaves_a_comet_tail() {
    let g = chans(TubeKind::Plumbicon)[1];
    let s = after_highlight(&g, 4.0, 20, 5);
    assert!(s[0] > 0.95 * g.beam_na, "first dark frame reads {} (beam {})", s[0], g.beam_na);
    assert!(tail_frames(&g, 4.0, 20) >= 2);
    // within the beam's reach there is only lag
    assert_eq!(tail_frames(&g, 1.0, 20), 0);
    assert!(after_highlight(&g, 1.0, 20, 1)[0] < 0.5 * g.peak_signal_na);
}

#[test]
fn the_tail_grows_with_the_overload_and_shrinks_with_the_beam() {
    let g = chans(TubeKind::Plumbicon)[1];
    let t: Vec<usize> = [2.0, 4.0, 8.0].iter().map(|&l| tail_frames(&g, l, 30)).collect();
    assert!(t[0] <= t[1] && t[1] <= t[2] && t[0] < t[2], "{t:?}");
    let strong = TubeKind::Plumbicon.channels(3.0).unwrap()[1];
    assert!(tail_frames(&strong, 4.0, 30) < tail_frames(&g, 4.0, 30));
    // the stored charge stops at the signal-plate voltage, so a far brighter highlight leaves
    // no longer a tail than one that already filled the element
    let (bright, blinding) = (tail_frames(&g, 8.0, 300), tail_frames(&g, 100.0, 300));
    assert!(blinding.abs_diff(bright) <= 1, "8x: {bright} frames, 100x: {blinding} frames");
}

#[test]
fn the_act_gun_leaves_no_tail() {
    let g = chans(TubeKind::PlumbiconAct)[1];
    assert_eq!(tail_frames(&g, 8.0, 30), 0);
    assert!(after_highlight(&g, 8.0, 30, 1)[0] < 0.25 * g.peak_signal_na);
}

/// The red and blue tubes carry half the peak current with half the beam on a target of about
/// the same capacitance, so they store more overload per unit of beam: a white highlight's tail
/// outlasts on red and blue (it ends magenta).
#[test]
fn red_and_blue_tails_outlast_green() {
    let c = chans(TubeKind::Plumbicon);
    let (r, g, b) = (tail_frames(&c[0], 4.0, 40), tail_frames(&c[1], 4.0, 40), tail_frames(&c[2], 4.0, 40));
    assert!(r > g && b > g, "R {r} G {g} B {b}");
}

// ------------------------------------------------------------------ in the chain

fn square_at(cx: usize) -> Frame {
    let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
    for y in ACTIVE_H / 2 - 16..ACTIVE_H / 2 + 16 {
        for x in cx - 16..cx + 16 {
            let i = (y * ACTIVE_W + x) * 3;
            f.rgb[i..i + 3].fill(255);
        }
    }
    f
}

/// A white square held for 8 frames, then moved right; the luma at the old position in the
/// first frame lit only by the new picture and two frames later. Each line integrates the light
/// of the frame period before it is read, so frame 8 still holds the square's own light from
/// picture 7: the trail is measured from frame 9.
fn moved_square(camera: &str) -> [f64; 2] {
    let p = config::build(Some("pristine"), camera, 3).unwrap();
    let mut input: Vec<Frame> = (0..8).map(|_| square_at(200)).collect();
    input.extend((0..4).map(|_| square_at(500)));
    let out = Session::run(p, &input);
    [9, 11].map(|k| {
        let f = &out[k];
        let mut acc = 0.0;
        for y in ACTIVE_H / 2 - 8..ACTIVE_H / 2 + 8 {
            for x in 192..208 {
                let i = (y * f.w + x) * 3;
                acc += 0.299 * f.rgb[i] as f64 + 0.587 * f.rgb[i + 1] as f64 + 0.114 * f.rgb[i + 2] as f64;
            }
        }
        acc / 256.0
    })
}

/// Overexposed by two stops, the square's charge outlives it at clipped white; the ACT gun
/// removes that, leaving only the lag, which is faint two frames later.
#[test]
fn an_overexposed_highlight_leaves_a_trail_on_tape_and_the_act_gun_removes_it() {
    let off = moved_square("");
    let tube = moved_square("[camera]\ntube = \"plumbicon\"\niris_stops = 2.0\n");
    let act = moved_square("[camera]\ntube = \"plumbicon-act\"\niris_stops = 2.0\n");
    for k in 0..2 {
        assert!(tube[k] > off[k] + 40.0, "frame {k}: trail {} vs no tube {}", tube[k], off[k]);
    }
    assert!(act[0] < tube[0] - 40.0, "ACT {} vs plain {}", act[0], tube[0]);
    assert!(act[1] < off[1] + 15.0, "ACT {} vs no tube {}", act[1], off[1]);
}

/// A still flat grey comes out of the camera at the level the tube's gamma of 0.95 gives it.
/// The rows at the raster edge are left out: the beam's wings there find no neighbouring line.
/// Measured at the camera's output: the tape chain's noise decorrelates under any change of its
/// input, so a whole-chain comparison cannot isolate the camera.
#[test]
fn a_still_flat_grey_keeps_its_level_apart_from_the_tube_gamma() {
    let params = vhsphys_core::camera::CameraParams { tube: TubeKind::Plumbicon, ..Default::default() };
    for grey in STILL_GREYS {
        let mut cam = vhsphys_core::camera::CameraTube::new(&params).unwrap();
        let mut input = Frame::new(ACTIVE_W, ACTIVE_H);
        input.rgb.fill(grey);
        let mut out = cam.process_frame(&input);
        for _ in 0..SETTLE_FRAMES {
            out = cam.process_frame(&input);
        }
        let want = 255.0 * oetf(oetf_inverse(grey as f64 / 255.0).powf(GAMMA));
        let inner = &out.rgb[EDGE_ROWS * ACTIVE_W * 3..(ACTIVE_H - EDGE_ROWS) * ACTIVE_W * 3];
        let got = inner.iter().map(|&v| v as f64).sum::<f64>() / inner.len() as f64;
        assert!((got - want).abs() < FLAT_TOLERANCE, "grey {grey}: {got} vs {want}");
    }
}

/// A still scene at normal exposure keeps its tones: averaged over blocks the camera's output is
/// its input, apart from the tube's gamma and the spot, which softens the finest stripes in
/// linear light before the gamma corrector and so lifts them a little. The tones are read through
/// the processing amplifier without the head amplifier's noise, which is checked on its own in
/// `camera_electronics.rs`: noise clipped one-sided at black would lift the dark blocks by about a
/// level and hide what this check is about.
#[test]
fn a_still_scene_at_normal_exposure_is_not_restyled() {
    use vhsphys_core::camera_processing::VideoProcessor;
    let params = vhsphys_core::camera::CameraParams { tube: TubeKind::Plumbicon, ..Default::default() };
    let mut cam = vhsphys_core::camera::CameraTube::new(&params).unwrap();
    let input = vhsphys_core::video::test_pattern();
    let mut sig = cam.read(&input, &input);
    for _ in 0..SETTLE_FRAMES {
        sig = cam.read(&input, &input);
    }
    let video = VideoProcessor::new(params.processing, cam.channels(), cam.black()).process(&sig);
    let mut out = Frame::new(input.w, input.h);
    for (r, row) in video.rows.iter().enumerate() {
        for x in 0..input.w {
            for k in 0..3 {
                out.rgb[(r * input.w + x) * 3 + k] = (255.0f32 * row[k][x]).round().clamp(0.0, 255.0) as u8;
            }
        }
    }
    let (mut err, mut n) = (0.0, 0.0);
    for by in (0..=input.h - TONE_BLOCK).step_by(TONE_BLOCK) {
        for bx in (0..=input.w - TONE_BLOCK).step_by(TONE_BLOCK) {
            for c in 0..3 {
                let (mut a, mut b) = (0.0, 0.0);
                for y in by..by + TONE_BLOCK {
                    for x in bx..bx + TONE_BLOCK {
                        let i = (y * input.w + x) * 3 + c;
                        a += input.rgb[i] as f64;
                        b += out.rgb[i] as f64;
                    }
                }
                err += (a - b).abs() / (TONE_BLOCK * TONE_BLOCK) as f64;
                n += 1.0;
            }
        }
    }
    let d = err / n;
    assert!(d < TONE_TOLERANCE, "mean abs difference of the block means {d}");
}
