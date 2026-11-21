//! The single-tube stripe colour camera's optics and decoder (docs/design/CAMERA_SCAN.md, "Stripe
//! colour camera"): cyan and yellow stripes of opposite slant behind a birefringent optical low-pass,
//! and the frequency-separation decoder of the Panasonic WV-3110 (trap, band-pass, 1H separation,
//! envelope detection, 2.8 MHz luminance and 0.5 MHz colour). The target is ideal here: its signal is
//! the light it receives.

use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W, FS_BB};
use vhsphys_core::stripe::{
    optical_lowpass, stripe_transmittance, target_light, StripeDecoder, COLOUR_BAND_HZ, LUMA_BAND_HZ, STRIPE_CARRIER_HZ,
};

/// Panasonic's geometry: effective width, stripe pitch and active line, giving the carrier.
const WIDTH_MM: f64 = 7.49;
const PITCH_UM: f64 = 40.0;
const ACTIVE_LINE_US: f64 = 52.3;
const CARRIER_TOLERANCE_HZ: f64 = 0.01e6;
/// Rows at the top and bottom (a field's first line has no line before it) and samples at the line
/// ends left out of the checks.
const EDGE_ROWS: usize = 6;
const MARGIN: usize = 48;
/// Flat colours and how close they must decode.
const COLOURS: [[f64; 3]; 8] = [
    [1.0, 1.0, 1.0],
    [0.3, 0.3, 0.3],
    [1.0, 0.0, 0.0],
    [0.0, 1.0, 0.0],
    [0.0, 0.0, 1.0],
    [0.0, 1.0, 1.0],
    [1.0, 1.0, 0.0],
    [0.8, 0.2, 0.6],
];
const FLAT_TOLERANCE: f64 = 0.02;
/// A colour step and a luminance step across the line: the 10-90 % widths allowed, seconds. The
/// luminance edge is wider than the 2.8 MHz low-pass alone (0.12 us): the trap also takes out the
/// luminance around the carrier, and the stepped carrier's sidebands ripple on the edge (the ideal
/// decoder, with exact Gaussian filters, gives 0.28 us).
const STEP_AT: usize = ACTIVE_W / 2;
const COLOUR_RISE_S: (f64, f64) = (0.4e-6, 1.3e-6);
const LUMA_RISE_S: (f64, f64) = (0.05e-6, 0.35e-6);
/// A colour change between two lines of a field: the next line keeps some of the old colour.
const CHANGE_ROW: usize = ACTIVE_H / 2;
const MIX_MIN: f64 = 0.25;
const SETTLED_TOLERANCE: f64 = 0.05;
/// Fine grey detail near the carrier: frequency, mean and amplitude; false colour without the optical
/// low-pass must be at least this, and the plate must cut it by at least this factor.
const DETAIL_HZ: f64 = 3.4e6;
const DETAIL_MEAN: f64 = 0.5;
const DETAIL_AMPLITUDE: f64 = 0.3;
const FALSE_COLOUR_MIN: f64 = 0.03;
const OLPF_CUT: f64 = 4.0;
/// A white highlight whose target cannot follow the carrier's peaks (beam limit): the ceiling on the
/// target's signal, and how much greener than red and blue it must decode.
const HIGHLIGHT_CEILING: f64 = 2.0;
const GREEN_EXCESS: f64 = 0.15;
/// A carrier weakened by the read (spot and amplifier) is restored by the alignment gain.
const READ_RESPONSE: f64 = 0.6;
const LINEARITY_TOLERANCE: f64 = 0.01;

fn frame(light: impl Fn(usize, usize) -> [f64; 3]) -> Vec<Vec<f32>> {
    (0..ACTIVE_H).map(|r| (0..ACTIVE_W).map(|x| target_light(r, x, light(r, x)) as f32).collect()).collect()
}

/// Rows of one colour channel of the scene light behind the optical low-pass.
fn behind_plate(light: impl Fn(usize, usize) -> [f64; 3]) -> Vec<Vec<f32>> {
    (0..ACTIVE_H)
        .map(|r| {
            let per: Vec<Vec<f64>> = (0..3).map(|c| optical_lowpass(&(0..ACTIVE_W).map(|x| light(r, x)[c]).collect::<Vec<_>>())).collect();
            (0..ACTIVE_W).map(|x| target_light(r, x, [per[0][x], per[1][x], per[2][x]]) as f32).collect()
        })
        .collect()
}

fn inner(decoded: &[[Vec<f32>; 3]], mut f: impl FnMut(usize, usize, [f64; 3])) {
    for r in EDGE_ROWS..ACTIVE_H - EDGE_ROWS {
        for x in MARGIN..ACTIVE_W - MARGIN {
            f(r, x, [decoded[r][0][x] as f64, decoded[r][1][x] as f64, decoded[r][2][x] as f64]);
        }
    }
}

#[test]
fn the_carrier_is_the_scan_speed_over_the_stripe_pitch() {
    let f = WIDTH_MM * 1e-3 / (PITCH_UM * 1e-6 * ACTIVE_LINE_US * 1e-6);
    assert!((f - STRIPE_CARRIER_HZ).abs() < CARRIER_TOLERANCE_HZ, "{f}");
    assert_eq!(LUMA_BAND_HZ, 2.8e6);
    assert_eq!(COLOUR_BAND_HZ, 0.5e6);
}

/// Green passes everywhere; red and blue are modulated at the carrier, their phases turning +90 and
/// -90 degrees from one line of a field to the next.
#[test]
fn the_stripes_modulate_red_and_blue_with_opposite_slant() {
    let per_cycle = FS_BB / STRIPE_CARRIER_HZ;
    let phase = |r: usize, c: usize| {
        let (mut re, mut im) = (0.0, 0.0);
        for x in 0..ACTIVE_W {
            let t = stripe_transmittance(r, x);
            assert!(t[1] == 1.0 && (0.0..=1.0).contains(&t[0]) && (0.0..=1.0).contains(&t[2]));
            let a = 2.0 * std::f64::consts::PI * x as f64 / per_cycle;
            re += (t[c] - 0.5) * a.cos();
            im -= (t[c] - 0.5) * a.sin();
        }
        im.atan2(re)
    };
    let wrap = |d: f64| (d + 3.0 * std::f64::consts::PI).rem_euclid(2.0 * std::f64::consts::PI) - std::f64::consts::PI;
    for r in [10, 101, 240] {
        let dr = wrap(phase(r + 2, 0) - phase(r, 0));
        let db = wrap(phase(r + 2, 2) - phase(r, 2));
        assert!((dr - std::f64::consts::FRAC_PI_2).abs() < 0.02, "red turns {dr}");
        assert!((db + std::f64::consts::FRAC_PI_2).abs() < 0.02, "blue turns {db}");
    }
    let mean: f64 = (0..ACTIVE_W).map(|x| stripe_transmittance(7, x)[0]).sum::<f64>() / ACTIVE_W as f64;
    assert!((mean - 0.5).abs() < 0.01);
}

#[test]
fn flat_colours_decode_to_themselves() {
    let dec = StripeDecoder::new(1.0);
    for c in COLOURS {
        let out = dec.decode(&frame(|_, _| c));
        inner(&out, |r, x, got| {
            for k in 0..3 {
                assert!((got[k] - c[k]).abs() < FLAT_TOLERANCE, "{c:?} row {r} x {x}: {got:?}");
            }
        });
    }
}

/// The decoder is linear in light (the envelope of a positive carrier).
#[test]
fn half_the_light_decodes_to_half() {
    let dec = StripeDecoder::new(1.0);
    let c = COLOURS[7];
    let full = dec.decode(&frame(|_, _| c));
    let half = dec.decode(&frame(|_, _| [0.5 * c[0], 0.5 * c[1], 0.5 * c[2]]));
    let (r, x) = (ACTIVE_H / 2, ACTIVE_W / 2);
    for k in 0..3 {
        let (a, b) = (full[r][k][x] as f64, half[r][k][x] as f64);
        assert!((b - 0.5 * a).abs() < LINEARITY_TOLERANCE, "ch{k}: {a} and {b}");
    }
}

/// A read that weakens the carrier is restored by the alignment gain.
#[test]
fn the_alignment_gain_restores_a_weakened_carrier() {
    let c = COLOURS[7];
    let rows: Vec<Vec<f32>> = (0..ACTIVE_H)
        .map(|r| {
            (0..ACTIVE_W)
                .map(|x| {
                    let t = stripe_transmittance(r, x);
                    let dc = c[1] + 0.5 * c[0] + 0.5 * c[2];
                    let ac = (t[0] - 0.5) * c[0] + (t[2] - 0.5) * c[2];
                    (dc + READ_RESPONSE * ac) as f32
                })
                .collect()
        })
        .collect();
    let out = StripeDecoder::new(1.0 / READ_RESPONSE).decode(&rows);
    inner(&out, |r, x, got| {
        for k in 0..3 {
            assert!((got[k] - c[k]).abs() < FLAT_TOLERANCE, "row {r} x {x}: {got:?}");
        }
    });
}

/// 10-90 % width of `row` across a rising or falling step near `at`, seconds.
fn rise(row: &[f32], at: usize, lo: f64, hi: f64) -> f64 {
    let span = hi - lo;
    let level = |x: usize| (row[x] as f64 - lo) / span;
    let window = at - 40..at + 40;
    let first = window.clone().find(|&x| level(x) >= 0.1).expect("10 %");
    let last = window.clone().find(|&x| level(x) >= 0.9).expect("90 %");
    (last - first) as f64 / FS_BB
}

#[test]
fn colour_is_half_a_megahertz_wide_and_luminance_three() {
    let dec = StripeDecoder::new(1.0);
    let out = dec.decode(&frame(|_, x| if x < STEP_AT { [0.0, 0.5, 0.0] } else { [0.8, 0.5, 0.0] }));
    let w = rise(&out[ACTIVE_H / 2][0], STEP_AT, 0.0, 0.8);
    assert!(w >= COLOUR_RISE_S.0 && w <= COLOUR_RISE_S.1, "red edge {w:e} s");
    let out = dec.decode(&frame(|_, x| if x < STEP_AT { [0.2; 3] } else { [0.8; 3] }));
    let w = rise(&out[ACTIVE_H / 2][1], STEP_AT, 0.2, 0.8);
    assert!(w >= LUMA_RISE_S.0 && w <= LUMA_RISE_S.1, "luminance edge {w:e} s");
}

/// Colour is separated with the line before: where the colour changes between lines of a field, the
/// next line keeps some of the old colour, and the one after is clean.
#[test]
fn a_colour_change_between_lines_lingers_on_the_next() {
    let dec = StripeDecoder::new(1.0);
    let red = [1.0, 0.0, 0.0];
    let blue = [0.0, 0.0, 1.0];
    let out = dec.decode(&frame(|r, _| if r < CHANGE_ROW { red } else { blue }));
    let x = ACTIVE_W / 2;
    let at = |r: usize, k: usize| out[r][k][x] as f64;
    // the first line of each field below the change is separated against red on the line before it:
    // red stays and blue is short, by an amount that depends on the two stripe phases
    for r in [CHANGE_ROW, CHANGE_ROW + 1] {
        assert!(at(r, 0) > MIX_MIN && at(r, 2) < 1.0 - MIX_MIN, "row {r}: R {:.3} B {:.3}", at(r, 0), at(r, 2));
    }
    for r in [CHANGE_ROW + 4, CHANGE_ROW + 5] {
        assert!(at(r, 0).abs() < SETTLED_TOLERANCE && (at(r, 2) - 1.0).abs() < SETTLED_TOLERANCE, "row {r}");
    }
    for r in [CHANGE_ROW - 4, CHANGE_ROW - 3] {
        assert!((at(r, 0) - 1.0).abs() < SETTLED_TOLERANCE && at(r, 2).abs() < SETTLED_TOLERANCE, "row {r}");
    }
}

fn false_colour(rows: &[Vec<f32>]) -> f64 {
    let out = StripeDecoder::new(1.0).decode(rows);
    let (mut sum, mut n) = (0.0, 0.0);
    inner(&out, |_, _, v| {
        sum += (v[0] - v[1]).powi(2) + (v[2] - v[1]).powi(2);
        n += 2.0;
    });
    (sum / n).sqrt()
}

/// Grey detail near the carrier beats with the stripes into false colour; the birefringent plate
/// removes it.
#[test]
fn fine_detail_makes_false_colour_that_the_optical_lowpass_removes() {
    let grey = |_: usize, x: usize| {
        let v = DETAIL_MEAN + DETAIL_AMPLITUDE * (2.0 * std::f64::consts::PI * DETAIL_HZ * x as f64 / FS_BB).sin();
        [v; 3]
    };
    let bare = false_colour(&frame(grey));
    let plate = false_colour(&behind_plate(grey));
    assert!(bare > FALSE_COLOUR_MIN, "false colour without the plate {bare:.4}");
    assert!(bare / plate >= OLPF_CUT, "the plate cuts false colour from {bare:.4} to {plate:.4}");
    // a flat field passes the plate unchanged
    let flat = optical_lowpass(&vec![0.4; ACTIVE_W]);
    assert!(flat.iter().all(|v| (v - 0.4).abs() < 1e-12));
}

/// When the target cannot follow the carrier's peaks, red and blue lose their carrier and the
/// highlight turns green.
#[test]
fn a_clipped_highlight_turns_green() {
    let rows: Vec<Vec<f32>> = frame(|_, _| [1.0, 1.0, 1.0]).into_iter().map(|row| row.into_iter().map(|v| v.min(HIGHLIGHT_CEILING as f32)).collect()).collect();
    let out = StripeDecoder::new(1.0).decode(&rows);
    let (mut sum, mut n) = ([0.0; 3], 0.0);
    inner(&out, |_, _, v| {
        for k in 0..3 {
            sum[k] += v[k];
        }
        n += 1.0;
    });
    let (red, green, blue) = (sum[0] / n, sum[1] / n, sum[2] / n);
    assert!(green - red > GREEN_EXCESS && green - blue > GREEN_EXCESS, "R {red:.3} G {green:.3} B {blue:.3}");
}

/// The 625-line engine decodes too: its carrier is not a whole number of samples per cycle.
#[test]
fn the_625_line_engine_decodes_flat_colours() {
    use vhsphys_core::pal::consts as pc;
    use vhsphys_core::pal::stripe as ps;
    let dec = ps::StripeDecoder::new(1.0);
    for c in COLOURS {
        let rows: Vec<Vec<f32>> = (0..pc::ACTIVE_H).map(|r| (0..pc::ACTIVE_W).map(|x| ps::target_light(r, x, c) as f32).collect()).collect();
        let out = dec.decode(&rows);
        for r in EDGE_ROWS..pc::ACTIVE_H - EDGE_ROWS {
            for x in MARGIN..pc::ACTIVE_W - MARGIN {
                for k in 0..3 {
                    let got = out[r][k][x] as f64;
                    assert!((got - c[k]).abs() < FLAT_TOLERANCE * 1.5, "{c:?} row {r} x {x} ch{k}: {got}");
                }
            }
        }
    }
}
