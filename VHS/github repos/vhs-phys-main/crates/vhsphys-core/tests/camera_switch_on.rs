// CONTRACT
//! Switching the camera on (docs/design/CAMERA_RETENTION.md, "Beam off and idle time"): the lens cap
//! before the shot and the beam held off at its start, as two camera settings.
//!
//! `CameraParams::idle_s` is lens-capped time with the beam on before the first frame, so a cold
//! camera can warm up before it shoots. `CameraParams::beam_off_s` holds the beam off for the first
//! seconds of the shot with the lens open: the targets store the scene unread, and when the beam
//! comes on it reads what built up (and a Saticon burns, a lead-oxide burn is erased). Both are
//! rounded to whole frames; `0` (the default) is none, and a negative or non-finite value counts as
//! `0`. Each is exactly the hand-driven sequence on `CameraTube`: `idle(idle_s)` right after seeding,
//! then `set_beam_on(false)` until frame `round(beam_off_s * FRAME_RATE)` of the shot has been read,
//! then `set_beam_on(true)`. A hand call of `set_beam_on` cancels a pending switch-on.

use vhsphys_core::camera::{CameraParams, CameraTube};
use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W, FRAME_RATE};
use vhsphys_core::tube::TubeKind;
use vhsphys_core::video::{Baseband, Frame};

const SEED: u64 = 11;

/// The test card: a white block on black, so a stored picture and a lifted black both show.
fn scene() -> Frame {
    let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
    for y in ACTIVE_H / 3..2 * ACTIVE_H / 3 {
        for x in ACTIVE_W / 3..2 * ACTIVE_W / 3 {
            let i = (y * ACTIVE_W + x) * 3;
            f.rgb[i..i + 3].fill(255);
        }
    }
    f
}

fn same(a: &Baseband, b: &Baseband) -> bool {
    a.y == b.y && a.cu == b.cu && a.cv == b.cv
}

fn tube(params: &CameraParams) -> CameraTube {
    CameraTube::seeded(params, SEED).expect("a tube")
}

fn mean_luma(b: &Baseband) -> f64 {
    b.y.iter().map(|&v| v as f64).sum::<f64>() / b.y.len() as f64
}

#[test]
fn the_switch_on_settings_default_to_none_and_read_from_a_config() {
    let p = CameraParams::default();
    assert_eq!((p.idle_s, p.beam_off_s), (0.0, 0.0));
    let p: CameraParams = toml::from_str("tube = \"vidicon\"\nidle_s = 600.0\nbeam_off_s = 0.5").unwrap();
    assert_eq!((p.idle_s, p.beam_off_s), (600.0, 0.5));
}

#[test]
fn the_lens_cap_is_idle_time_before_the_first_frame() {
    // A cold vidicon: its dark current is still low, so the idle time shows as a warmer black.
    let base = CameraParams { tube: TubeKind::Vidicon, cold_start_c: Some(-10.0), ..Default::default() };
    let mut by_setting = tube(&CameraParams { idle_s: 120.0, ..base.clone() });
    let mut by_hand = tube(&base);
    by_hand.idle(120.0);
    let mut uncapped = tube(&base);
    let f = scene();
    let mut differs = false;
    for n in 0..3 {
        let a = by_setting.encode_baseband(&f);
        assert!(same(&a, &by_hand.encode_baseband(&f)), "frame {n}: idle_s is idle() after seeding");
        differs |= !same(&a, &uncapped.encode_baseband(&f));
    }
    assert!(differs, "two minutes under the cap change a cold camera");
}

#[test]
fn the_beam_comes_on_after_its_off_time() {
    let base = CameraParams { tube: TubeKind::Plumbicon, iris_stops: 1.0, ..Default::default() };
    // 2.6 frames round to 3
    let off = 3;
    let mut by_setting = tube(&CameraParams { beam_off_s: 2.6 / FRAME_RATE, ..base.clone() });
    let mut by_hand = tube(&base);
    by_hand.set_beam_on(false);
    let mut always_on = tube(&base);
    let f = scene();
    for n in 0..off + 3 {
        if n == off {
            by_hand.set_beam_on(true);
        }
        let a = by_setting.encode_baseband(&f);
        assert!(same(&a, &by_hand.encode_baseband(&f)), "frame {n}");
        let on = always_on.encode_baseband(&f);
        if n < off {
            assert!(mean_luma(&a) < mean_luma(&on), "frame {n}: with the beam off nothing is read");
        }
    }
}

#[test]
fn the_cap_comes_off_before_the_beam_comes_on() {
    let base = CameraParams { tube: TubeKind::Saticon, ..Default::default() };
    let mut by_setting = tube(&CameraParams { idle_s: 1.0, beam_off_s: 2.0 / FRAME_RATE, ..base.clone() });
    let mut by_hand = tube(&base);
    by_hand.idle(1.0);
    by_hand.set_beam_on(false);
    let f = scene();
    for n in 0..4 {
        if n == 2 {
            by_hand.set_beam_on(true);
        }
        assert!(same(&by_setting.encode_baseband(&f), &by_hand.encode_baseband(&f)), "frame {n}");
    }
}

#[test]
fn a_hand_switch_cancels_the_pending_switch_on() {
    let base = CameraParams { tube: TubeKind::Newvicon, ..Default::default() };
    let mut switched = tube(&CameraParams { beam_off_s: 1.0, ..base.clone() });
    switched.set_beam_on(true);
    let mut plain = tube(&base);
    let f = scene();
    for n in 0..2 {
        assert!(same(&switched.encode_baseband(&f), &plain.encode_baseband(&f)), "frame {n}");
    }
    // and switched off by hand, it stays off past the setting's time
    let mut held = tube(&CameraParams { beam_off_s: 1.0 / FRAME_RATE, ..base.clone() });
    held.set_beam_on(false);
    let mut off = tube(&base);
    off.set_beam_on(false);
    for n in 0..3 {
        assert!(same(&held.encode_baseband(&f), &off.encode_baseband(&f)), "frame {n}");
    }
}

#[test]
fn negative_and_non_finite_times_count_as_none() {
    let base = CameraParams { tube: TubeKind::Chalnicon, ..Default::default() };
    let f = scene();
    for (idle_s, beam_off_s) in [(-5.0, -1.0), (f64::NAN, f64::NAN), (f64::INFINITY, f64::NEG_INFINITY)] {
        let mut odd = tube(&CameraParams { idle_s, beam_off_s, ..base.clone() });
        let mut plain = tube(&base);
        assert!(same(&odd.encode_baseband(&f), &plain.encode_baseband(&f)), "{idle_s} {beam_off_s}");
    }
}
