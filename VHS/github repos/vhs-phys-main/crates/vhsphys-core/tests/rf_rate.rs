//! The tape signal sampled at 8, 12 or 16 x fsc. Each rate is its own engine (`ntsc_rf8`,
//! `ntsc` at the reference 12 x fsc, `ntsc_rf16`, and the same for 625 lines). A rate changes
//! how finely the signal between the record head and the demodulator is worked out and nothing
//! else: the deck is aligned to the same level, the luma noise is the same in the video band, the
//! picture is the same within what the coarser grid costs (mainly colour noise of the grid's own),
//! and the finer grid is the nearer to the continuous signal. The 12 x fsc output itself is frozen in `bit_identity.rs`.

use vhsphys_core::frame::Frame;
use vhsphys_core::rf_rate::RfRate;

/// Frames in a clip: two to settle, the frame that is looked at, and the look-ahead after it.
const FRAMES: usize = 6;
/// The frame looked at.
const LOOK: usize = 3;

/// The deck with its noise sources off: what is left between two rates is the sampling.
const QUIET: &str = "[head]\ncnr_db = 400.0\nmedia_am_noise = 0.0\n[mech]\nslow_us = 0.0\ndrum_ns = 0.0\ntension_ns = 0.0\nstick_ns = 0.0\nline_ns = 0.0\nhead_step_us = 0.0\nswitch_us = 0.0\n";

fn luma(f: &Frame, x: usize, y: usize) -> f64 {
    let i = (y * f.w + x) * 3;
    0.299 * f.rgb[i] as f64 + 0.587 * f.rgb[i + 1] as f64 + 0.114 * f.rgb[i + 2] as f64
}

/// RMS deviation of the luma from its mean, clear of the head switch and the sync edge.
fn luma_noise(f: &Frame) -> f64 {
    let (mut s, mut s2, mut n) = (0.0, 0.0, 0.0);
    for y in f.h / 12..f.h * 5 / 6 {
        for x in f.w / 12..f.w * 11 / 12 {
            let v = luma(f, x, y);
            s += v;
            s2 += v * v;
            n += 1.0;
        }
    }
    let m = s / n;
    (s2 / n - m * m).max(0.0).sqrt()
}

/// Mean absolute difference of two frames, in levels, clear of the head switch.
fn mean_abs_error(a: &Frame, b: &Frame) -> f64 {
    let (y0, y1) = (a.h / 12, a.h * 5 / 6);
    let (lo, hi) = (y0 * a.w * 3, y1 * a.w * 3);
    a.rgb[lo..hi].iter().zip(b.rgb[lo..hi].iter()).map(|(x, y)| (*x as f64 - *y as f64).abs()).sum::<f64>() / (hi - lo) as f64
}

/// Mean colour in the middle of each of the seven bars, rows in the top half of the picture.
fn bar_colours(f: &Frame) -> Vec<[f64; 3]> {
    (0..7)
        .map(|b| {
            let (x0, x1) = ((b as f64 + 0.3) * f.w as f64 / 7.0, (b as f64 + 0.7) * f.w as f64 / 7.0);
            let (mut acc, mut n) = ([0.0f64; 3], 0.0);
            for y in f.h / 12..f.h / 2 {
                for x in x0 as usize..x1 as usize {
                    for k in 0..3 {
                        acc[k] += f.rgb[(y * f.w + x) * 3 + k] as f64;
                    }
                    n += 1.0;
                }
            }
            acc.map(|a| a / n)
        })
        .collect()
}

fn largest_bar_difference(a: &Frame, b: &Frame) -> f64 {
    bar_colours(a).iter().zip(bar_colours(b).iter()).flat_map(|(x, y)| (0..3).map(move |k| (x[k] - y[k]).abs())).fold(0.0, f64::max)
}

/// What every engine is asked, by rate and raster.
macro_rules! engine_runs {
    ($m:ident) => {
        pub mod $m {
            use super::*;
            use vhsphys_core::$m::consts::{ACTIVE_H, ACTIVE_W};
            use vhsphys_core::$m::pipeline::{Chain, Params, Session};
            use vhsphys_core::$m::{config, consts, video};

            pub const RATE: RfRate = consts::RF_RATE;
            pub const RF_PER_LINE: usize = consts::RF_PER_LINE;
            pub const FS_RF: f64 = consts::FS_RF;

            pub fn params(preset: &str, text: &str, seed: u64) -> Result<Params, String> {
                config::build(Some(preset), text, seed)
            }

            pub fn bars() -> Frame {
                video::test_pattern()
            }

            pub fn flat(level: u8) -> Frame {
                let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
                f.rgb.fill(level);
                f
            }

            pub fn run(preset: &str, text: &str, seed: u64, frame: &Frame) -> Vec<Frame> {
                let input: Vec<Frame> = (0..FRAMES).map(|_| frame.clone()).collect();
                Session::run(params(preset, text, seed).unwrap(), &input)
            }

            /// The carrier level at the limiter, field 4 (1.0 = the deck's alignment level times its AGC target).
            pub fn carrier_level(preset: &str, seed: u64) -> f64 {
                let mut s = Session::new(params(preset, "", seed).unwrap());
                for _ in 0..FRAMES {
                    s.push(&bars());
                }
                s.finish();
                s.debug_field(4).carrier_level as f64
            }

            /// The same clip asked for one frame at a time and eight at a time.
            pub fn batches_agree(preset: &str, seed: u64) -> bool {
                let input: Vec<Frame> = (0..FRAMES + 1).map(|i| if i % 2 == 0 { bars() } else { flat(90) }).collect();
                let run = |batch: usize| {
                    let mut s = Session::new(params(preset, "", seed).unwrap());
                    let mut out = Vec::new();
                    for f in &input {
                        s.push(f);
                        while s.ready() > 0 {
                            out.extend(s.process(batch));
                        }
                    }
                    s.finish();
                    while s.ready() > 0 {
                        out.extend(s.process(batch));
                    }
                    out
                };
                let (a, b) = (run(1), run(8));
                a.len() == b.len() && a.iter().zip(b.iter()).all(|(x, y)| x.rgb == y.rgb)
            }

            pub fn dub_frames(preset: &str, seed: u64) -> usize {
                let input: Vec<Frame> = (0..FRAMES).map(|_| bars()).collect();
                Chain::run(config::generation_params(Some(preset), "", seed, 2).unwrap(), &input).len()
            }
        }
    };
}

engine_runs!(ntsc_rf8);
engine_runs!(ntsc);
engine_runs!(ntsc_rf16);
engine_runs!(pal_rf8);
engine_runs!(pal);
engine_runs!(pal_rf16);

#[test]
fn the_rates_and_their_grids() {
    assert_eq!(RfRate::ALL, [RfRate::X8, RfRate::X12, RfRate::X16]);
    assert_eq!(RfRate::default(), RfRate::X12);
    assert_eq!(RfRate::REFERENCE, RfRate::X12);
    assert_eq!(RfRate::ALL.map(|r| r.name()), ["8x", "12x", "16x"]);
    assert_eq!(RfRate::ALL.map(|r| r.os()), [2, 3, 4]);
    for r in RfRate::ALL {
        assert_eq!(RfRate::from_name(r.name()), Some(r));
    }
    assert_eq!(RfRate::from_name("9x"), None);
    assert_eq!([ntsc_rf8::RATE, ntsc::RATE, ntsc_rf16::RATE], RfRate::ALL);
    assert_eq!([pal_rf8::RATE, pal::RATE, pal_rf16::RATE], RfRate::ALL);
    assert_eq!([ntsc_rf8::RF_PER_LINE, ntsc::RF_PER_LINE, ntsc_rf16::RF_PER_LINE], [1820, 2730, 3640]);
    assert_eq!([pal_rf8::RF_PER_LINE, pal::RF_PER_LINE, pal_rf16::RF_PER_LINE], [1820, 2730, 3640]);
    assert!((ntsc_rf8::FS_RF / ntsc::FS_RF - 2.0 / 3.0).abs() < 1e-12);
    assert!((ntsc_rf16::FS_RF / ntsc::FS_RF - 4.0 / 3.0).abs() < 1e-12);
}

#[test]
fn a_config_names_the_rate_and_an_engine_refuses_another() {
    // an engine takes a config that names its own rate, or none
    assert_eq!(ntsc_rf8::params("worn", "rf_rate = \"8x\"\n", 1).unwrap().rf_rate, RfRate::X8);
    assert_eq!(ntsc_rf8::params("worn", "", 1).unwrap().rf_rate, RfRate::X8);
    assert_eq!(ntsc::params("worn", "", 1).unwrap().rf_rate, RfRate::X12);
    assert_eq!(pal_rf16::params("worn", "standard = \"pal\"\nrf_rate = \"16x\"\n", 1).unwrap().rf_rate, RfRate::X16);
    // and refuses one that names another, with a message that says which it plays
    let e = ntsc::params("worn", "rf_rate = \"8x\"\n", 1).unwrap_err();
    assert!(e.contains("12x") && e.contains("8x"), "{e}");
    assert!(ntsc_rf16::params("worn", "rf_rate = \"12x\"\n", 1).is_err());
    assert!(ntsc::params("worn", "rf_rate = \"9x\"\n", 1).is_err());
}

#[test]
fn every_rate_plays_the_colour_bars() {
    // the same bound the reference rate is held to in `pipeline.rs`
    for (name, out, want) in [
        ("ntsc 8x", ntsc_rf8::run("pristine", "", 1, &ntsc_rf8::bars()), ntsc_rf8::bars()),
        ("ntsc 16x", ntsc_rf16::run("pristine", "", 1, &ntsc_rf16::bars()), ntsc_rf16::bars()),
        ("pal 8x", pal_rf8::run("pristine", "standard = \"pal\"\n", 1, &pal_rf8::bars()), pal_rf8::bars()),
        ("pal 16x", pal_rf16::run("pristine", "standard = \"pal\"\n", 1, &pal_rf16::bars()), pal_rf16::bars()),
    ] {
        assert_eq!(out.len(), FRAMES, "{name}");
        let d = largest_bar_difference(&out[LOOK], &want);
        assert!(d < 28.0, "{name}: a bar is {d:.1} levels off the source");
    }
}

#[test]
fn on_a_quiet_deck_the_rates_agree_and_the_finer_is_the_nearer() {
    // With the noise sources off, what differs between two rates is how finely the signal was
    // worked out. 16 x fsc is the nearest to the continuous signal, so it is the reference here:
    // 12 x fsc must be nearer to it than 8 x fsc is.
    let ntsc_out = [ntsc_rf8::run("pristine", QUIET, 1, &ntsc::bars()), ntsc::run("pristine", QUIET, 1, &ntsc::bars()), ntsc_rf16::run("pristine", QUIET, 1, &ntsc::bars())];
    let pal_text = format!("standard = \"pal\"\n{QUIET}");
    let pal_out = [pal_rf8::run("pristine", &pal_text, 1, &pal::bars()), pal::run("pristine", &pal_text, 1, &pal::bars()), pal_rf16::run("pristine", &pal_text, 1, &pal::bars())];
    for (raster, out) in [("ntsc", &ntsc_out), ("pal", &pal_out)] {
        let (x8, x12, x16) = (&out[0][LOOK], &out[1][LOOK], &out[2][LOOK]);
        let (e8, e12) = (mean_abs_error(x8, x16), mean_abs_error(x12, x16));
        assert!(e12 <= e8, "{raster}: 12x is {e12:.3} levels from 16x, 8x {e8:.3}");
        // Measured: 8x is 3.7 levels from 16x at 525 lines (2.4 at 625), 12x 1.7 (1.2). What is left
        // is mostly colour noise the sampling grid makes (see the next test).
        assert!(e8 < 5.0, "{raster}: 8x is {e8:.3} levels from 16x");
        assert!(e12 < 2.5, "{raster}: 12x is {e12:.3} levels from 16x");
        // Measured: the largest bar difference from 12x is 1.2 levels at 8x (1.3 at 625 lines).
        for (name, f) in [("8x", x8), ("16x", x16)] {
            let d = largest_bar_difference(f, x12);
            assert!(d < 3.0, "{raster} {name}: a bar is {d:.2} levels off 12x");
        }
    }
}

/// RMS of each colour channel about its mean over the middle of a flat picture.
fn channel_noise(f: &Frame) -> [f64; 3] {
    let mut out = [0.0; 3];
    for (k, o) in out.iter_mut().enumerate() {
        let v: Vec<f64> = (f.h / 12..f.h * 5 / 6).flat_map(|y| (f.w / 12..f.w * 11 / 12).map(move |x| (x, y))).map(|(x, y)| f.rgb[(y * f.w + x) * 3 + k] as f64).collect();
        let m = v.iter().sum::<f64>() / v.len() as f64;
        *o = (v.iter().map(|a| (a - m).powi(2)).sum::<f64>() / v.len() as f64).sqrt();
    }
    out
}

#[test]
fn the_sampling_grid_makes_less_colour_noise_the_finer_it_is() {
    // What the rate trades: with the noise sources off, a flat grey still shows some colour noise
    // that the sampling grid makes, and it falls as the grid gets finer. Measured at 525 lines,
    // R/G/B rms: 8x 4.8/3.1/8.5, 12x 1.9/1.7/2.5, 16x 1.5/1.4/1.8 levels.
    let f = ntsc::flat(110);
    let n = [channel_noise(&ntsc_rf8::run("pristine", QUIET, 1, &f)[LOOK]), channel_noise(&ntsc::run("pristine", QUIET, 1, &f)[LOOK]), channel_noise(&ntsc_rf16::run("pristine", QUIET, 1, &f)[LOOK])];
    for k in 0..3 {
        assert!(n[0][k] > n[1][k] && n[1][k] > n[2][k], "channel {k}: 8x {:.2}, 12x {:.2}, 16x {:.2}", n[0][k], n[1][k], n[2][k]);
    }
}

#[test]
fn the_noise_in_the_picture_does_not_depend_on_the_rate() {
    // The head's noise is set by its carrier-to-noise ratio in the video band, so a finer grid
    // spreads the same noise power over a wider band and the picture sees the same.
    let grey = |raster: &str| if raster == "ntsc" { ntsc::flat(110) } else { pal::flat(110) };
    for raster in ["ntsc", "pal"] {
        let text = if raster == "ntsc" { "" } else { "standard = \"pal\"\n" };
        let f = grey(raster);
        let n = |out: Vec<Frame>| luma_noise(&out[LOOK]);
        let (n8, n12, n16) = if raster == "ntsc" {
            (n(ntsc_rf8::run("good", text, 4, &f)), n(ntsc::run("good", text, 4, &f)), n(ntsc_rf16::run("good", text, 4, &f)))
        } else {
            (n(pal_rf8::run("good", text, 4, &f)), n(pal::run("good", text, 4, &f)), n(pal_rf16::run("good", text, 4, &f)))
        };
        // Measured: equal at 525 lines (2.74 at 8x and 12x, 2.86 at 16x); at 625 lines 8x is 24 % above
        // 12x (3.36 against 2.72), a cost of the coarser grid there.
        for (name, v) in [("8x", n8), ("16x", n16)] {
            assert!((v / n12 - 1.0).abs() < 0.3, "{raster} {name}: luma noise {v:.3} against {n12:.3} at 12x");
        }
    }
}

#[test]
fn the_deck_is_aligned_to_the_same_level_at_every_rate() {
    let ntsc_levels = [ntsc_rf8::carrier_level("pristine", 1), ntsc::carrier_level("pristine", 1), ntsc_rf16::carrier_level("pristine", 1)];
    let pal_levels = [pal_rf8::carrier_level("pristine", 1), pal::carrier_level("pristine", 1), pal_rf16::carrier_level("pristine", 1)];
    for (raster, l) in [("ntsc", ntsc_levels), ("pal", pal_levels)] {
        for (name, v) in [("8x", l[0]), ("16x", l[2])] {
            // Measured: 0.628 at 8x against 0.593 at 12x at 525 lines, 0.638 against 0.587 at 625. The
            // playback equaliser's peaks are designed on the sample grid, and on the coarser grid their
            // shape around the carrier is a little different; the deck's AGC takes the difference up.
            assert!((v / l[1] - 1.0).abs() < 0.10, "{raster} {name}: carrier level {v:.3} at the limiter, {:.3} at 12x", l[1]);
        }
    }
}

#[test]
fn every_rate_gives_the_same_picture_however_the_work_is_cut_up() {
    assert!(ntsc_rf8::batches_agree("worn", 5), "ntsc 8x");
    assert!(ntsc_rf16::batches_agree("worn", 5), "ntsc 16x");
    assert!(pal_rf8::batches_agree("worn", 5), "pal 8x");
}

#[test]
fn a_dub_plays_at_every_rate() {
    assert_eq!(ntsc_rf8::dub_frames("worn", 6), FRAMES);
    assert_eq!(ntsc_rf16::dub_frames("worn", 6), FRAMES);
    assert_eq!(pal_rf8::dub_frames("worn", 6), FRAMES);
}
