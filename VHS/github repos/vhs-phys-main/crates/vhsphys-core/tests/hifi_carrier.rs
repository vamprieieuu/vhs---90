//! Acceptance test for the Hi-Fi carrier level per field: the share of the head over its own
//! track, the tape's magnetisation and the spacing loss at the carrier's wavelength, and what
//! the deck does when the carrier falls.

use vhsphys_core::audio::AudioProcessor;
use vhsphys_core::config;
use vhsphys_core::consts::{HEAD_SPEED, TRACK_PITCH_UM};
use vhsphys_core::pipeline::Params;

const RATE: f64 = 48_000.0;
/// Hi-Fi carriers of the 525-line format (left, right), Hz.
const F_LEFT: f64 = 1.3e6;
const F_RIGHT: f64 = 1.7e6;
/// Fields looked at by the statistical tests (10 s).
const FIELDS: i64 = 600;

/// Pristine deck with Hi-Fi on and every source of carrier loss switched off, so a test can
/// add exactly one. `condition` is extra `[condition]` text (may be empty).
fn clean(condition: &str) -> Params {
    let text = format!("[condition]\n{condition}\n[audio]\nhifi = true\n");
    let mut p = config::build(Some("pristine"), &text, 1).unwrap();
    p.tracking.offset_um = 0.0;
    p.tracking.slope_um = 0.0;
    p.tracking.bow_um = 0.0;
    p.tracking.wander_um = 0.0;
    p.tape.defects_per_mm2 = 0.0;
    p.tape.rough_um = 0.0;
    p.tape.edge_top_um = 0.0;
    p.tape.edge_bottom_um = 0.0;
    p.tape.creases_per_m = 0.0;
    p.audio.head_switch_buzz = 0.0;
    p
}

/// Carrier level under which the deck's detector calls the carrier lost: the level of a head
/// `hifi_tracking_threshold_um` (given at the SP pitch) off the centre of its track.
fn threshold(p: &Params) -> f32 {
    (1.0 - p.audio.hifi_tracking_threshold_um / TRACK_PITCH_UM) as f32
}

fn min(v: &[f32]) -> f32 {
    v.iter().cloned().fold(f32::INFINITY, f32::min)
}

fn mean(v: &[f32]) -> f64 {
    v.iter().map(|x| *x as f64).sum::<f64>() / v.len() as f64
}

fn rms(v: &[f32]) -> f64 {
    (v.iter().map(|x| (*x as f64).powi(2)).sum::<f64>() / v.len() as f64).sqrt()
}

#[test]
fn a_centred_head_on_a_clean_tape_reads_the_full_carrier() {
    let p = clean("");
    let a = AudioProcessor::new(&p, RATE, 2);
    for n in 0..120 {
        for c in 0..2 {
            let car = a.hifi_carrier(n, c);
            assert!(car.len() >= 800 && car.len() <= 802, "field {n}: {} samples", car.len());
            assert!(min(&car) > 0.999, "field {n}, channel {c}: {}", min(&car));
        }
        assert!(a.plays_hifi(n), "field {n}");
    }
}

#[test]
fn the_carrier_falls_with_the_share_of_the_head_off_its_track() {
    for speed in ["sp", "lp", "ep"] {
        let mut p = clean(&format!("tape_speed = \"{speed}\""));
        let pitch = p.speed.track_pitch_um();
        p.tracking.offset_um = 0.25 * pitch;
        let a = AudioProcessor::new(&p, RATE, 1);
        let m = mean(&a.hifi_carrier(10, 0));
        assert!((m - 0.75).abs() < 0.01, "{speed}: a head a quarter pitch off reads {m:.3} of the carrier");
        assert!(a.plays_hifi(10), "{speed}: a quarter pitch is inside the threshold");
    }
}

#[test]
fn spacing_at_the_start_of_the_track_costs_the_higher_carrier_more() {
    let mut p = clean("");
    p.tape.edge_top_um = 0.2;
    let a = AudioProcessor::new(&p, RATE, 2);
    let (l, r) = (a.hifi_carrier(10, 0), a.hifi_carrier(10, 1));
    // Wallace: exp(-2 pi d / lambda) with lambda = head speed / carrier, so the log of the
    // loss scales with the carrier frequency whatever the exact spacing
    let ratio = (r[0] as f64).ln() / (l[0] as f64).ln();
    assert!(l[0] < 0.95, "the edge lifts the tape at the start of the track: {}", l[0]);
    assert!((ratio - F_RIGHT / F_LEFT).abs() < 0.01 * F_RIGHT / F_LEFT, "log-loss ratio {ratio:.4}");
    let worst = (-2.0 * std::f64::consts::PI * 0.2e-6 * F_LEFT / HEAD_SPEED).exp();
    assert!(l[0] as f64 >= worst - 1e-3, "no more loss than the full lift gives: {} < {worst:.3}", l[0]);
    assert!(l[l.len() / 2] > 0.99, "the lift is gone by the middle of the track: {}", l[l.len() / 2]);
}

#[test]
fn fm_noise_rises_in_inverse_proportion_to_the_carrier() {
    // the hiss is the same random sequence at every offset, so differences cancel the hum and
    // leave the noise times (1/A - 1)
    let silence = vec![0.0f32; 72_000];
    let run = |share: f64| {
        let mut p = clean("");
        p.tracking.offset_um = share * p.speed.track_pitch_um();
        AudioProcessor::new(&p, RATE, 1).process(&silence, 0)[12_000..].to_vec()
    };
    let (y0, ya, yb) = (run(0.0), run(0.1), run(0.25));
    let da: Vec<f32> = ya.iter().zip(&y0).map(|(a, b)| a - b).collect();
    let db: Vec<f32> = yb.iter().zip(&y0).map(|(a, b)| a - b).collect();
    // (1/0.75 - 1) / (1/0.9 - 1) = 3
    let ratio = rms(&db) / rms(&da);
    assert!((ratio - 3.0).abs() < 0.3, "noise growth ratio {ratio:.2}");
    assert!(rms(&da) > 0.0, "the noise depends on the carrier level");
}

#[test]
fn a_one_field_loss_is_bridged_and_a_lasting_one_falls_back() {
    // a head 14 um off centre with 4 um of random wander per track crosses the threshold on
    // some tracks only
    let mut isolated = None;
    let mut fractions = vec![];
    for seed in 1..=20u64 {
        let mut p = clean("");
        p.seed = seed;
        p.tracking.offset_um = 14.0;
        p.tracking.wander_um = 4.0;
        let th = threshold(&p);
        let a = AudioProcessor::new(&p, RATE, 1);
        let lost: Vec<bool> = (0..FIELDS + 1).map(|n| min(&a.hifi_carrier(n, 0)) < th).collect();
        let plays: Vec<bool> = (0..FIELDS).map(|n| a.plays_hifi(n)).collect();
        fractions.push(plays.iter().filter(|x| **x).count() as f64 / FIELDS as f64);
        for n in 40..FIELDS as usize - 1 {
            // a lost field whose neighbours and the 32 fields before it are all fine
            if lost[n] && !lost[n + 1] && lost[n - 33..n].iter().all(|x| !x) {
                assert!(plays[n], "seed {seed}: one lost field must not switch to the linear track (field {n})");
                isolated.get_or_insert((seed, n));
            }
        }
    }
    assert!(isolated.is_some(), "no isolated lost field in 20 seeds; the test needs one");
    let (lo, hi) = fractions.iter().fold((1.0f64, 0.0f64), |(lo, hi), f| (lo.min(*f), hi.max(*f)));
    assert!(lo < 0.95 && hi > 0.05, "the deck should play Hi-Fi on some fields and not others: {lo:.2}..{hi:.2}");
    let mixed = fractions.iter().filter(|f| **f > 0.05 && **f < 0.95).count();
    assert!(mixed > 0, "no seed both falls back and recovers: {fractions:?}");
}

#[test]
fn presets_keep_or_lose_hifi_as_their_tracking_says() {
    // a bad tape drops in and out of Hi-Fi: its per-track wander carries the head past the limit
    for seed in 1..=8u64 {
        for (preset, lo, hi) in [("worn", 0.99, 1.0), ("bad", 0.01, 0.5), ("ruined", 0.0, 0.1)] {
            let mut p = config::build(Some(preset), "", seed).unwrap();
            p.audio.hifi = true;
            let a = AudioProcessor::new(&p, RATE, 1);
            let f = (0..FIELDS).filter(|n| a.plays_hifi(*n)).count() as f64 / FIELDS as f64;
            assert!(f >= lo && f <= hi, "{preset}, seed {seed}: Hi-Fi on {f:.3} of the fields");
        }
    }
}

#[test]
fn the_625_line_deck_uses_its_own_carriers() {
    use vhsphys_core::pal;
    let text = "[audio]\nhifi = true\n";
    let mut p = pal::config::build(Some("pristine"), text, 1).unwrap();
    p.tracking = Default::default();
    p.tape.defects_per_mm2 = 0.0;
    p.tape.rough_um = 0.0;
    p.tape.edge_bottom_um = 0.0;
    p.tape.creases_per_m = 0.0;
    p.tape.edge_top_um = 0.2;
    let a = pal::audio::AudioProcessor::new(&p, RATE, 2);
    let (l, r) = (a.hifi_carrier(10, 0), a.hifi_carrier(10, 1));
    // 625-line Hi-Fi carriers: 1.4 and 1.8 MHz
    let ratio = (r[0] as f64).ln() / (l[0] as f64).ln();
    assert!((ratio - 1.8 / 1.4).abs() < 0.01 * 1.8 / 1.4, "log-loss ratio {ratio:.4}");
    assert!(a.plays_hifi(10));
    assert!(a.hifi_carrier(10, 0).len() >= 960 && a.hifi_carrier(10, 0).len() <= 961);
}
