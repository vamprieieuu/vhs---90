//! The held-charge step (docs/design/CAMERA_RETENTION.md, "One held piece" and "The held-charge
//! step"): an element whose layer falls below its collection knee is integrated in midpoint
//! sub-steps of at most half a volt of layer swing. Judged against the same target integrated in
//! fixed sub-steps of a quarter of a millisecond, which the design note shows to be within 1.2e-6 of
//! peak white of the converged solution, and by the number of sub-steps taken.
//!
//! Every reading of every row, and the held charge, trap occupancy, burn and stored charge of every
//! sub-row at the end of every frame, must stay within each case's bound of that reference. Each
//! bound is the error the rule was measured to make plus a quarter, and each lies below the error of
//! the first rule (fixed 0.5 ms sub-steps with the collection taken at their start) in the same case:
//! the step must come closer to the converged target in every case. The cases are peak
//! white and twice peak white on lead oxide (with and without bias light), a Saticon driven past its
//! knee into sticking, and a vidicon burning under the lamp of US 3,546,520.

use vhsphys_core::camera::{ElementState, UniformColumn, HELD_STEP_V, RETENTION_STEP_S, SUB_ROWS};
use vhsphys_core::consts::{ACTIVE_H, FRAME_RATE};
use vhsphys_core::tube::{TubeChannel, TubeKind, BEAM_RATIO_DEFAULT, MAX_TRAPS};

const R: usize = 0;
const G: usize = 1;
const B: usize = 2;

/// The reference: fixed midpoint sub-steps of 0.25 ms in every piece.
const REFERENCE_STEP_S: f64 = 0.25e-3;
/// Traps and burn do not depend on the element's charge and are exact for a constant light at any
/// step: share of a trap's capacity, and burn absolute.
const TRAP_BOUND: f64 = 1e-8;
const BURN_BOUND: f64 = 1e-12;

/// Light for `on` frames (times peak white), then dark for `off`.
const PEAK_WHITE: f64 = 1.0;
const TWICE_PEAK: f64 = 2.0;
const PBO_ON_FRAMES: usize = 20;
const PBO_DARK_FRAMES: usize = 10;
/// Saticon sticking: twenty times peak white (US 4,330,733).
const STICK_LIGHT: f64 = 20.0;
const STICK_ON_FRAMES: usize = 10;
const STICK_DARK_FRAMES: usize = 10;
/// Vidicon burn: the lamp of US 3,546,520 (3500 times peak white).
const LAMP_LIGHT: f64 = 3500.0;
const LAMP_FRAMES: usize = 60;
const LAMP_DARK_FRAMES: usize = 10;
/// The lamp must burn measurably within the run (about 0.038), or the burn bound proves nothing.
const LAMP_BURN_MIN: f64 = 1e-2;

/// Worst errors against the reference: row reading (share of peak white), held charge (share of
/// its capacity), stored charge (share of the plate).
#[derive(Clone, Copy, Debug)]
struct Errors {
    row: f64,
    held: f64,
    charge: f64,
}

/// One case: the tube, its light, the bound for the rule, and the first rule's error.
struct Case {
    name: &'static str,
    kind: TubeKind,
    k: usize,
    light: f64,
    on: usize,
    off: usize,
    bound: Errors,
    first_rule: Errors,
}

const fn e(row: f64, held: f64, charge: f64) -> Errors {
    Errors { row, held, charge }
}

/// Lead oxide. Bounds: the rule's measured error plus a quarter (design note, "The held-charge
/// step"). First rule: its measured error in the same case.
const PBO_CASES: [Case; 6] = [
    Case {
        name: "plumbicon R, peak white",
        kind: TubeKind::Plumbicon,
        k: R,
        light: PEAK_WHITE,
        on: PBO_ON_FRAMES,
        off: PBO_DARK_FRAMES,
        bound: e(2.3e-5, 1.8e-4, 1.6e-6),
        first_rule: e(4.15e-5, 3.93e-4, 3.48e-6),
    },
    Case {
        name: "plumbicon R, twice peak white",
        kind: TubeKind::Plumbicon,
        k: R,
        light: TWICE_PEAK,
        on: PBO_ON_FRAMES,
        off: PBO_DARK_FRAMES,
        bound: e(3.6e-5, 5.9e-5, 2.0e-6),
        first_rule: e(1.87e-3, 1.97e-3, 1.05e-4),
    },
    Case {
        name: "plumbicon G, peak white",
        kind: TubeKind::Plumbicon,
        k: G,
        light: PEAK_WHITE,
        on: PBO_ON_FRAMES,
        off: PBO_DARK_FRAMES,
        bound: e(1.5e-5, 2.4e-4, 2.1e-6),
        first_rule: e(1.31e-4, 2.24e-3, 1.96e-5),
    },
    Case {
        name: "plumbicon G, twice peak white",
        kind: TubeKind::Plumbicon,
        k: G,
        light: TWICE_PEAK,
        on: PBO_ON_FRAMES,
        off: PBO_DARK_FRAMES,
        bound: e(1.3e-5, 1.1e-4, 2.2e-6),
        first_rule: e(8.21e-4, 8.27e-3, 1.08e-4),
    },
    Case {
        name: "plumbicon B, peak white",
        kind: TubeKind::Plumbicon,
        k: B,
        light: PEAK_WHITE,
        on: PBO_ON_FRAMES,
        off: PBO_DARK_FRAMES,
        bound: e(2.2e-6, 1.6e-4, 1.1e-7),
        first_rule: e(3.58e-6, 3.61e-4, 1.79e-7),
    },
    Case {
        name: "plumbicon B, twice peak white",
        kind: TubeKind::Plumbicon,
        k: B,
        light: TWICE_PEAK,
        on: PBO_ON_FRAMES,
        off: PBO_DARK_FRAMES,
        bound: e(4.7e-6, 1.1e-4, 2.3e-7),
        first_rule: e(1.35e-4, 2.20e-3, 6.29e-6),
    },
];

/// Lead oxide with bias light, the case where the step matters most.
const BIAS_CASES: [Case; 2] = [
    Case {
        name: "plumbicon-bias R, peak white",
        kind: TubeKind::PlumbiconBias,
        k: R,
        light: PEAK_WHITE,
        on: PBO_ON_FRAMES,
        off: PBO_DARK_FRAMES,
        bound: e(3.6e-5, 2.0e-4, 7.4e-7),
        first_rule: e(4.45e-5, 4.14e-4, 1.68e-6),
    },
    Case {
        name: "plumbicon-bias R, twice peak white",
        kind: TubeKind::PlumbiconBias,
        k: R,
        light: TWICE_PEAK,
        on: PBO_ON_FRAMES,
        off: PBO_DARK_FRAMES,
        bound: e(1.4e-4, 5.2e-5, 5.5e-6),
        first_rule: e(7.35e-3, 1.87e-3, 2.57e-4),
    },
];

const SATICON_CASE: Case = Case {
    name: "saticon, sticking",
    kind: TubeKind::Saticon,
    k: G,
    light: STICK_LIGHT,
    on: STICK_ON_FRAMES,
    off: STICK_DARK_FRAMES,
    bound: e(2.2e-5, 3.3e-5, 1.2e-5),
    first_rule: e(2.06e-4, 2.12e-4, 7.41e-5),
};

/// The vidicon has no held charge: both rules take one step per piece, and the burn and traps they
/// integrate are exact for the piece's light, so the bound is the reference's own resolution.
const VIDICON_CASE: Case = Case {
    name: "vidicon, lamp",
    kind: TubeKind::Vidicon,
    k: G,
    light: LAMP_LIGHT,
    on: LAMP_FRAMES,
    off: LAMP_DARK_FRAMES,
    bound: e(1e-9, 0.0, 1e-10),
    first_rule: e(1e-9, 0.0, 1e-10),
};

fn channel(kind: TubeKind, k: usize) -> TubeChannel {
    kind.channels(BEAM_RATIO_DEFAULT).expect("a tube")[k]
}

/// Rows of every frame and the state of every sub-row at the end of every frame.
struct Run {
    rows: Vec<Vec<f64>>,
    states: Vec<Vec<ElementState>>,
}

/// `light` times peak white over the whole target for the first `on` frames, then dark for `off`.
fn run(ch: &TubeChannel, light: f64, on: usize, off: usize) -> Run {
    let mut col = UniformColumn::new(ch);
    let lit = light * ch.peak_signal_na;
    let t_off = on as f64 / FRAME_RATE;
    let mut out = Run {
        rows: Vec::new(),
        states: Vec::new(),
    };
    for _ in 0..on + off {
        out.rows
            .push(col.read_rows(&|_, t| if t < t_off { lit } else { 0.0 }));
        out.states
            .push((0..SUB_ROWS * ACTIVE_H).map(|k| col.element(k)).collect());
    }
    out
}

/// Compare the rule with the reference in `case` and assert every bound.
fn within_bounds(case: &Case) {
    let ch = channel(case.kind, case.k);
    let reference = TubeChannel {
        retention_step_s: REFERENCE_STEP_S,
        ..ch
    };
    let got = run(&ch, case.light, case.on, case.off);
    let want = run(&reference, case.light, case.on, case.off);
    let mut err = e(0.0, 0.0, 0.0);
    let (mut trap, mut burn) = (0.0f64, 0.0f64);
    for (a, b) in got.rows.iter().zip(&want.rows) {
        for (x, y) in a.iter().zip(b) {
            err.row = err.row.max((x - y).abs() / ch.peak_signal_na);
        }
    }
    for (a, b) in got.states.iter().zip(&want.states) {
        for (x, y) in a.iter().zip(b) {
            if !ch.collection.is_full() {
                let d = (x.held_nas - y.held_nas).abs() / ch.collection.capacity_nas;
                err.held = err.held.max(d);
            }
            for p in 0..MAX_TRAPS {
                if ch.traps[p].capture > 0.0 {
                    let d = (x.traps_nas[p] - y.traps_nas[p]).abs() / ch.traps[p].capacity_nas;
                    trap = trap.max(d);
                }
            }
            burn = burn.max((x.burn - y.burn).abs());
            err.charge = err
                .charge
                .max((x.charge_nas - y.charge_nas).abs() / ch.plate_nas);
        }
    }
    let name = case.name;
    for (what, got, bound, first) in [
        ("row", err.row, case.bound.row, case.first_rule.row),
        (
            "held charge",
            err.held,
            case.bound.held,
            case.first_rule.held,
        ),
        (
            "stored charge",
            err.charge,
            case.bound.charge,
            case.first_rule.charge,
        ),
    ] {
        assert!(
            got <= bound,
            "{name}: {what} {got:.3e} against the reference (bound {bound:.1e})"
        );
        assert!(
            got <= first,
            "{name}: {what} {got:.3e} is above the first rule's {first:.2e}"
        );
    }
    assert!(trap <= TRAP_BOUND, "{name}: trap {trap:.2e} of capacity");
    assert!(burn <= BURN_BOUND, "{name}: burn {burn:.2e}");
}

#[test]
fn lead_oxide_highlights_are_integrated_within_the_bound_of_the_converged_target() {
    for case in &PBO_CASES {
        within_bounds(case);
    }
}

#[test]
fn lead_oxide_with_bias_light_is_integrated_within_the_bound_of_the_converged_target() {
    for case in &BIAS_CASES {
        within_bounds(case);
    }
}

#[test]
fn a_sticking_saticon_is_integrated_within_the_bound_of_the_converged_target() {
    within_bounds(&SATICON_CASE);
}

#[test]
fn a_burning_vidicon_is_integrated_within_the_bound_of_the_converged_target() {
    let c = &VIDICON_CASE;
    let r = run(&channel(c.kind, c.k), c.light, c.on, 0);
    let burnt = r.states.last().expect("frames")[SUB_ROWS * ACTIVE_H / 2].burn;
    assert!(burnt > LAMP_BURN_MIN, "burn after the lamp {burnt:.2e}");
    within_bounds(c);
}

/// Step count: steady light for `STEP_SETTLE_FRAMES`, then counted over `STEP_COUNT_FRAMES`.
const STEP_SETTLE_FRAMES: usize = 10;
const STEP_COUNT_FRAMES: usize = 20;
/// Low key (a tenth of peak white: under half a volt of swing in any piece), peak white, and an
/// overload far past the plate.
const LOW_KEY_LIGHT: f64 = 0.1;
const OVERLOAD_LIGHT: f64 = 100.0;

/// Sub-steps per sub-row per frame over `STEP_COUNT_FRAMES` of steady `light` times peak white.
fn steps_per_frame(ch: &TubeChannel, light: f64) -> f64 {
    let mut col = UniformColumn::new(ch);
    let p = light * ch.peak_signal_na;
    for _ in 0..STEP_SETTLE_FRAMES {
        col.read(&|_| p);
    }
    let before = col.held_steps();
    for _ in 0..STEP_COUNT_FRAMES {
        col.read(&|_| p);
    }
    (col.held_steps() - before) as f64 / (STEP_COUNT_FRAMES * SUB_ROWS * ACTIVE_H) as f64
}

#[test]
fn the_held_charge_takes_one_step_per_half_volt_of_swing() {
    // a target without retention state keeps the merged rule and takes no sub-steps
    let nv = channel(TubeKind::Newvicon, G);
    let mut col = UniformColumn::new(&nv);
    col.read(&|_| nv.peak_signal_na);
    assert_eq!(col.held_steps(), 0);

    let frame_s = 1.0 / FRAME_RATE;
    for k in [R, G, B] {
        let ch = channel(TubeKind::Plumbicon, k);
        // at low key every piece swings under half a volt: one step per piece
        let pieces = steps_per_frame(&ch, LOW_KEY_LIGHT);
        // at peak white: at least one step per HELD_STEP_V of swing, at most one more per piece
        let swing_steps = (PEAK_WHITE * ch.peak_signal_na + ch.dark_na) * frame_s
            / (ch.capacitance_f * 1e9 * HELD_STEP_V);
        let peak = steps_per_frame(&ch, PEAK_WHITE);
        assert!(
            peak >= swing_steps,
            "ch{k}: {peak:.2} steps a frame for {swing_steps:.2} of swing"
        );
        assert!(
            peak <= swing_steps + pieces,
            "ch{k}: {peak:.2} steps a frame, {pieces:.2} pieces"
        );
        // in overload never more than the 0.5 ms grid
        let grid = frame_s / RETENTION_STEP_S;
        let over = steps_per_frame(&ch, OVERLOAD_LIGHT);
        assert!(
            over <= grid + pieces,
            "ch{k}: {over:.2} steps a frame in overload"
        );
    }
}
