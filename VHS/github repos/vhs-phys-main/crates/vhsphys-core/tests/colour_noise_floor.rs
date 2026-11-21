//! The colour noise the arithmetic makes. With the head noise, the media noise and the transport
//! errors switched off, a flat grey should come out flat. What colour noise is left is made by the
//! sampling of the tape signal: where a pulse lands between two samples, how a magnetisation
//! reversal is placed between two samples of the record current, and how the colour-under is
//! taken down to the baseband grid. A real tape has none of it, so it must be small at every rate,
//! fall as the grid gets finer, and be the same at the top of the picture as at the bottom (the
//! position of a pulse along the track must not matter).
//!
//! Measured before the sampling was made exact (blue rms, levels): 8x 21.7, 12x 7.6 (5.1 at the top,
//! 11.4 at the bottom), 16x 4.5; on a clean tape at 12x 19.2.

use vhsphys_core::frame::Frame;

/// Frames in a clip; the frame looked at.
const FRAMES: usize = 6;
const LOOK: usize = 3;

/// The deck with its noise sources off.
const QUIET: &str = "[head]\ncnr_db = 400.0\nmedia_am_noise = 0.0\n[mech]\nslow_us = 0.0\ndrum_ns = 0.0\ntension_ns = 0.0\nstick_ns = 0.0\nline_ns = 0.0\nhead_step_us = 0.0\nswitch_us = 0.0\n";

/// Blue is where the colour noise shows most (it carries the most of U).
const BLUE: usize = 2;

/// RMS of one channel about its mean over rows `y0..y1`, clear of the picture's edges.
fn rms(f: &Frame, y0: usize, y1: usize, k: usize) -> f64 {
    let v: Vec<f64> = (y0..y1).flat_map(|y| (f.w / 12..f.w * 11 / 12).map(move |x| (x, y))).map(|(x, y)| f.rgb[(y * f.w + x) * 3 + k] as f64).collect();
    let m = v.iter().sum::<f64>() / v.len() as f64;
    (v.iter().map(|a| (a - m).powi(2)).sum::<f64>() / v.len() as f64).sqrt()
}

macro_rules! grey_run {
    ($m:ident) => {
        fn $m(preset: &str, text: &str) -> Frame {
            use vhsphys_core::$m::consts::{ACTIVE_H, ACTIVE_W};
            use vhsphys_core::$m::{config, pipeline::Session};
            let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
            f.rgb.fill(110);
            let input: Vec<Frame> = (0..FRAMES).map(|_| f.clone()).collect();
            Session::run(config::build(Some(preset), text, 1).unwrap(), &input).swap_remove(LOOK)
        }
    };
}

grey_run!(ntsc_rf8);
grey_run!(ntsc);
grey_run!(ntsc_rf16);
grey_run!(pal);

#[test]
fn a_quiet_deck_makes_little_colour_noise_of_its_own() {
    // Measured with the sampling exact (blue rms, levels): 8x 8.5, 12x 2.5, 16x 1.8.
    for (name, f, bound) in [("8x", ntsc_rf8("pristine", QUIET), 10.5), ("12x", ntsc("pristine", QUIET), 3.2), ("16x", ntsc_rf16("pristine", QUIET), 2.4)] {
        let b = rms(&f, f.h / 12, f.h * 5 / 6, BLUE);
        assert!(b < bound, "{name}: blue rms {b:.2} levels on a quiet deck, the bound is {bound}");
    }
}

#[test]
fn the_colour_noise_of_the_arithmetic_does_not_depend_on_where_the_pulse_is_on_the_track() {
    // The top of the picture is read early in a track, the bottom late. Measured before the pulse
    // position was taken exactly: 5.1 at the top and 11.4 at the bottom at 12x.
    for (name, f) in [("ntsc", ntsc("pristine", QUIET)), ("pal", pal("pristine", &format!("standard = \"pal\"\n{QUIET}")))] {
        let (top, bottom) = (rms(&f, f.h / 12, f.h / 4, BLUE), rms(&f, f.h * 2 / 3, f.h * 5 / 6, BLUE));
        assert!((bottom / top - 1.0).abs() < 0.15, "{name}: blue rms {top:.2} at the top, {bottom:.2} at the bottom");
    }
}

#[test]
fn a_clean_tape_shows_less_colour_noise_than_the_arithmetic_used_to_add() {
    // The same picture with the noise sources on: the tape's own noise is what is left. Measured
    // at 12x: 19.2 before, 16.6 with the sampling exact.
    let f = ntsc("pristine", "");
    let b = rms(&f, f.h / 12, f.h * 5 / 6, BLUE);
    assert!(b < 18.0, "blue rms {b:.2} levels on a clean tape");
}
