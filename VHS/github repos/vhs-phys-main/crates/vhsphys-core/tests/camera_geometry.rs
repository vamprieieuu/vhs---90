//! Geometry (docs/design/CAMERA_SCAN.md, "Geometry"): deflection that moves the beam's landing and so its
//! read time, gain and spot in time; the registration of three tubes; black and white shading of the
//! target; the lens's veil and lead oxide's halation on the light; corner focus; and the processing
//! amplifier's correctors.
//!
//! Beam effects are judged on the target's line signal (`read_currents`, no noise), optics on the
//! photocurrents a picture drives (`photo_currents`), correctors on the processor fed a known signal,
//! and the whole camera once. Sources: docs/CAMERA_RESEARCH.md ("Interlaced readout with an oversized
//! spot": Philips T10 4.2, signal current and scan speed; Philips T10 and T12 deflection and
//! registration limits; black and white shading; corner limiting resolution. "Beam capacity of twice
//! peak white": the black-square flare data of the Hitachi H8397A and Philips T10, the anti-halation
//! disc. "Head-amplifier noise and processing": US 4032974, US 4001502). A camera with geometry off is
//! the engine before it, checked here bit for bit; a camera without a tube is the golden test's.

use vhsphys_core::camera::{picture_position_ph, CameraParams, CameraTube, TargetSignal};
use vhsphys_core::camera_processing::{
    gamma_correct, gamma_inverse, CameraVideo, Processing, VideoProcessor,
};
use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W, FS_BB};
use vhsphys_core::geometry::{
    black_shading_factor, displacement_ph, halation_radius_ph, scan_gain, white_shading_factor,
    Deflection, GeometrySpec, Halation, ANTI_HALATION_DISC_MM, BLACK_SQUARE_PH, GLASS_INDEX,
    PH_PER_NS, RHO_CORNER_PH, X_EDGE_PH,
};
use vhsphys_core::tube::{dark_at, oetf, TubeChannel, TubeKind, BEAM_RATIO_DEFAULT, TARGET_ASPECT};
use vhsphys_core::video::Frame;

const R: usize = 0;
const G: usize = 1;
const B: usize = 2;

/// Philips deflection units: 0.5 % of the picture height inside a circle of diameter H, 1 % outside;
/// vidicon coil units 2 %. The profiles sit at no less than half of their bound.
const STUDIO_INSIDE: f64 = 0.005;
const STUDIO_OUTSIDE: f64 = 0.01;
const CONSUMER_BOUND: f64 = 0.02;
const NEAR_BOUND: f64 = 0.5;
/// Matched triplets: 25-40, 40-80 and 80-120 ns in zones A, B and C (Philips T10).
const ZONE_NS: [(f64, f64); 3] = [(25.0, 40.0), (40.0, 80.0), (80.0, 120.0)];
/// Black shading reaches 12.5 % at the corners (Philips T10).
const BLACK_CORNER: f64 = 0.125;
/// White shading: "a few per cent" in PbO red and green, blue flat; up to 15 % on the Saticon.
const PBO_WHITE_FALL: (f64, f64) = (0.02, 0.05);
const SATICON_WHITE_FALL: (f64, f64) = (0.10, 0.15);
/// Corner limiting resolution below the centre's: 25-35 % for Sb2S3 and the Newvicon (XQ1274 650 to
/// 450 TVL), about 20 % for the Chalnicon; the studio tubes' is not published.
const SB2S3_FOCUS_LOSS: (f64, f64) = (0.25, 0.35);
const CHALNICON_FOCUS_LOSS: f64 = 0.20;
const CHALNICON_FOCUS_TOLERANCE: f64 = 0.02;
const STUDIO_FOCUS_LOSS: (f64, f64) = (0.15, 0.35);
/// The 30 mm tube's scanned height and its faceplate, mm, from the contract.
const TARGET_HEIGHT_MM: f64 = 12.8;
const FACEPLATE_MM: f64 = 1.5;

/// The light of the beam tests, nA, and the frames run before a reading.
const LIT_NA: f64 = 100.0;
const LIT_FRAMES: usize = 4;
/// Search window and the samples beyond it that give a line's or an edge's surroundings.
const WINDOW: usize = 6;
/// A one-column line's place is read to this, in samples; a one-row line's to this, in lines.
const PLACE_TOLERANCE: f64 = 0.1;
const ROW_TOLERANCE: f64 = 0.3;
/// Linearity of the speed tests: the scan runs from 1.2 (or 1.3) times nominal speed at the left end
/// to 0.8 (or 0.7) at the right.
const SPEED_LINEARITY: f64 = -0.2;
const SPOT_LINEARITY: f64 = -0.3;
/// Signal current follows scan speed to this.
const SPEED_TOLERANCE: f64 = 0.005;
const DARK_SPEED_TOLERANCE: f64 = 0.01;
/// The spot read at speed matches the moving Gaussian to this share of its peak, and the same column
/// holds the same charge wherever it is read, to this.
const SHAPE_TOLERANCE: f64 = 0.02;
const SAME_CHARGE: f64 = 0.01;
/// Columns kept clear of the line ends.
const END_MARGIN: usize = 16;
/// Registration edges are placed to this, in samples (0.05 sample is 3.5 ns).
const EDGE_TOLERANCE: f64 = 0.07;
/// Frames for a shaded dark target to settle (the vidicon's 84 ms time constant, six times over).
const DARK_FRAMES: usize = 15;
const DARK_TOLERANCE: f64 = 0.01;
/// The correctors on a noise-free signal: exact to this in the video.
const CORRECTED: f64 = 1e-3;
/// A shaded camera without correctors departs from flat by at least this in the video (the Saticon's
/// consumer camera: 0.034 at the top-right check point).
const SHADED_MIN: f64 = 0.02;
/// The black-square data (H8397A Table 1, Philips T10): Saticon 1 % centre and 0.5 % corner, vidicon 5 %
/// centre and 1 % corner, with the tolerances of a reading on a chart.
const SATICON_SQUARE: (f64, f64) = (0.010, 0.005);
const SATICON_SQUARE_TOLERANCE: f64 = 0.0012;
const VIDICON_SQUARE: (f64, f64) = (0.05, 0.01);
const VIDICON_SQUARE_TOLERANCE: f64 = 0.005;
/// A shift-invariant veil through the vidicon's transfer reads a corner square at 0.51 of the centre
/// square at best (the contract's open point): below this, and above the data's corner.
const VIDICON_CORNER_RATIO: f64 = 0.6;
/// The flare corrector blacks the chart's centre square to this, and leaves the corner square below black
/// by between these, in linear light.
const FLARE_BLACK: f64 = 0.0015;
const CORNER_BELOW: (f64, f64) = (-0.009, -0.002);
const FLARE_COEFFICIENT_TOLERANCE: f64 = 0.12;
/// Halation: a spot of this radius, PH, at 16 times peak white; annuli inside and on the ring, PH; the
/// ring's light against its derivation; red against green as their reflectances.
const SPOT_RADIUS_PH: f64 = 0.02;
const SPOT_IRIS_STOPS: f64 = 4.0;
const INSIDE_RING: (f64, f64) = (0.12, 0.16);
const ON_RING: (f64, f64) = (0.23, 0.27);
const RING_TOLERANCE: f64 = 0.10;
const RED_OVER_GREEN_TOLERANCE: f64 = 0.05;
const NO_LIGHT: f64 = 1e-9;
/// Corner focus: the spot's second moment against the moving Gaussian's, and the least growth at the
/// corner column.
const FOCUS_TOLERANCE: f64 = 0.02;
const CORNER_GROWTH_MIN: f64 = 1.3;
/// The whole camera on a grey field: blocks of samples and rows at the corners and the centre, frames
/// run, and how flat the corrected camera reads in linear light.
const GREY_LIGHT: f64 = 0.5;
const GREY_FRAMES: usize = 8;
const BLOCK: usize = 16;
const BLOCK_INSET: usize = 24;
const FLAT: f64 = 0.01;
const LEFT_EDGE_SHADING_MIN: f64 = 0.03;

// ------------------------------------------------------------------ helpers

fn chans(kind: TubeKind) -> [TubeChannel; 3] {
    kind.channels(BEAM_RATIO_DEFAULT).expect("a tube")
}

fn params(kind: TubeKind, processing: Processing) -> CameraParams {
    CameraParams {
        tube: kind,
        processing,
        ..CameraParams::default()
    }
}

fn camera(kind: TubeKind, processing: Processing, g: &GeometrySpec) -> CameraTube {
    CameraTube::seeded_geometry(&params(kind, processing), 0, g).expect("a tube")
}

/// The contract's displacement, written out: `(dx, dy)` PH of tube `ch` at nominal `(x, y)`.
fn disp(g: &GeometrySpec, ch: usize, x: f64, y: f64) -> (f64, f64) {
    let rho2 = x * x + y * y;
    let (k, p) = (g.deflection.linearity, g.deflection.pincushion);
    let (s, q) = (g.registration[ch].size, g.registration[ch].pincushion);
    let dx = k * (x * x - X_EDGE_PH * X_EDGE_PH) / (2.0 * X_EDGE_PH) + (p + q) * x * rho2 + s * x;
    let dy = (p + q) * y * rho2 + s * y;
    (dx, dy)
}

/// Samples per PH along the line.
fn samples_per_ph() -> f64 {
    ACTIVE_W as f64 / TARGET_ASPECT
}

/// The nominal sample at which tube `ch`'s beam on row `r` lands on line position `x_target` (PH).
fn landing_sample(g: &GeometrySpec, ch: usize, r: usize, x_target: f64) -> f64 {
    let y = picture_position_ph(r, 0.0).1;
    let mut s = (x_target / TARGET_ASPECT + 0.5) * ACTIVE_W as f64 - 0.5;
    for _ in 0..50 {
        let x = picture_position_ph(r, s).0;
        let f = x + disp(g, ch, x, y).0 - x_target;
        let h = 1e-3;
        let x2 = picture_position_ph(r, s + h).0;
        let f2 = x2 + disp(g, ch, x2, y).0 - x_target;
        s -= f * h / (f2 - f);
    }
    s
}

/// Line position, PH, of the boundary before target column `j`.
fn boundary_ph(j: usize) -> f64 {
    (j as f64 / ACTIVE_W as f64 - 0.5) * TARGET_ASPECT
}

/// Column whose centre is nearest `x` PH, and row nearest `y` PH.
fn column_at(x: f64) -> usize {
    ((x / TARGET_ASPECT + 0.5) * ACTIVE_W as f64 - 0.5).round() as usize
}

fn row_at(y: f64) -> usize {
    ((y + 0.5) * ACTIVE_H as f64 - 0.5).round() as usize
}

/// Read `frames` frames of a steady light `photo(r, x)` on every channel; the last reading.
fn read_steady(
    cam: &mut CameraTube,
    photo: &(dyn Fn(usize, usize, usize) -> f64 + Sync),
    frames: usize,
) -> TargetSignal {
    let mut s = cam.read_currents(&|ch, r, x, _| photo(ch, r, x));
    for _ in 1..frames {
        s = cam.read_currents(&|ch, r, x, _| photo(ch, r, x));
    }
    s
}

fn row_of(s: &TargetSignal, r: usize, ch: usize) -> Vec<f64> {
    s.rows[r][ch].iter().map(|v| *v as f64).collect()
}

/// Centroid of `v - base` over `s0 - WINDOW..=s0 + WINDOW`, `base` from the samples just beyond.
fn peak_centroid(v: &[f64], s0: usize) -> f64 {
    let base = (v[s0 - WINDOW - 2] + v[s0 + WINDOW + 2]) / 2.0;
    let (mut num, mut den) = (0.0, 0.0);
    for s in s0 - WINDOW..=s0 + WINDOW {
        let w = (v[s] - base).max(0.0);
        num += w * s as f64;
        den += w;
    }
    num / den
}

/// Centroid of the step's slope around `s0`: where an edge, as the spot reads it, lies.
fn edge_centroid(v: &[f64], s0: usize) -> f64 {
    let (mut num, mut den) = (0.0, 0.0);
    for s in s0 - WINDOW..s0 + WINDOW {
        let d = v[s + 1] - v[s];
        num += d * (s as f64 + 0.5);
        den += d;
    }
    num / den
}

fn flat_frame(v: u8) -> Frame {
    let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
    f.rgb.fill(v);
    f
}

/// A white picture with a black square of `BLACK_SQUARE_PH` centred at `(cx, cy)` PH.
fn chart(cx: f64, cy: f64) -> Frame {
    let mut f = flat_frame(255);
    for r in 0..ACTIVE_H {
        for s in 0..ACTIVE_W {
            let (x, y) = picture_position_ph(r, s as f64);
            if (x - cx).abs() < BLACK_SQUARE_PH / 2.0 && (y - cy).abs() < BLACK_SQUARE_PH / 2.0 {
                let p = (r * ACTIVE_W + s) * 3;
                f.rgb[p..p + 3].fill(0);
            }
        }
    }
    f
}

/// The corner square's centre: flush with the bottom-right corner.
fn corner_square() -> (f64, f64) {
    (
        X_EDGE_PH - BLACK_SQUARE_PH / 2.0,
        0.5 - BLACK_SQUARE_PH / 2.0,
    )
}

/// The signal a target reading `photo` (nA, as `photo_currents` indexes it) plus `dark` gives.
fn signal_from(photo: &[f64], dark: [f64; 3]) -> TargetSignal {
    TargetSignal {
        rows: (0..ACTIVE_H)
            .map(|r| {
                std::array::from_fn(|k| {
                    (0..ACTIVE_W)
                        .map(|s| (photo[(r * ACTIVE_W + s) * 3 + k] + dark[k]) as f32)
                        .collect()
                })
            })
            .collect(),
    }
}

fn synthetic(f: impl Fn(usize, usize, usize) -> f64) -> TargetSignal {
    TargetSignal {
        rows: (0..ACTIVE_H)
            .map(|r| std::array::from_fn(|k| (0..ACTIVE_W).map(|s| f(k, r, s) as f32).collect()))
            .collect(),
    }
}

fn video_at(v: &CameraVideo, ch: usize, r: usize, s: usize) -> f64 {
    v.rows[r][ch][s] as f64
}

/// Sample points of the corrector checks: the centre, the four corners and the middles of the edges.
fn check_points() -> Vec<(usize, usize)> {
    let (r0, r1, rm) = (BLOCK_INSET, ACTIVE_H - 1 - BLOCK_INSET, ACTIVE_H / 2);
    let (s0, s1, sm) = (BLOCK_INSET, ACTIVE_W - 1 - BLOCK_INSET, ACTIVE_W / 2);
    vec![
        (rm, sm),
        (r0, s0),
        (r0, s1),
        (r1, s0),
        (r1, s1),
        (rm, s0),
        (rm, s1),
        (r0, sm),
        (r1, sm),
    ]
}

/// Linear light of a block's mean video, `BLOCK` rows and samples from `(r, s)`.
fn block_linear(v: &CameraVideo, ch: usize, r: usize, s: usize) -> f64 {
    let mut sum = 0.0;
    for row in &v.rows[r..r + BLOCK] {
        for x in s..s + BLOCK {
            sum += row[ch][x] as f64;
        }
    }
    gamma_inverse(sum / (BLOCK * BLOCK) as f64)
}

// ------------------------------------------------------------------ the profiles

/// The profiles carry the published bounds: the deflection inside Philips's limits and near them, the
/// registration inside the matched triplets' bands by zone, black shading at 12.5 % in a corner, white
/// shading, halation and corner focus as the tubes' data give them, and the studio camera's correctors.
#[test]
fn the_profiles_carry_the_published_figures() {
    let kinds = [
        TubeKind::Plumbicon,
        TubeKind::PlumbiconBias,
        TubeKind::PlumbiconAct,
        TubeKind::PlumbiconDbc,
        TubeKind::Vidicon,
        TubeKind::Newvicon,
        TubeKind::Saticon,
        TubeKind::Chalnicon,
    ];
    let n = 60;
    let grid: Vec<(f64, f64)> = (0..=n)
        .flat_map(|i| {
            (0..=n).map(move |j| {
                (
                    (2.0 * i as f64 / n as f64 - 1.0) * X_EDGE_PH,
                    (j as f64 / n as f64 - 0.5),
                )
            })
        })
        .collect();
    for p in [Processing::Studio, Processing::Consumer] {
        let g = p.geometry(TubeKind::Plumbicon);
        // the implementation's field is the contract's
        for &(x, y) in grid.iter().step_by(97) {
            for ch in [R, G, B] {
                let (a, b) = (displacement_ph(&g, ch, x, y), disp(&g, ch, x, y));
                assert!(
                    (a.0 - b.0).abs() <= 1e-12 && (a.1 - b.1).abs() <= 1e-12,
                    "{p:?} ch{ch} at ({x:.3}, {y:.3}): {a:?} against {b:?}"
                );
            }
        }
        let (mut inside, mut outside) = (0.0f64, 0.0f64);
        let mut zones = [0.0f64; 3];
        for &(x, y) in &grid {
            let (dx, dy) = displacement_ph(&g, G, x, y);
            let d = dx.hypot(dy);
            let rho = x.hypot(y);
            if rho <= 0.5 {
                inside = inside.max(d);
            } else {
                outside = outside.max(d);
            }
            let zone = if rho <= 0.5 {
                0
            } else if rho <= 2.0 / 3.0 {
                1
            } else {
                2
            };
            for ch in [R, B] {
                let (ax, ay) = displacement_ph(&g, ch, x, y);
                let ns = (ax - dx).hypot(ay - dy) / PH_PER_NS;
                zones[zone] = zones[zone].max(ns);
            }
        }
        let (bound_in, bound_out) = match p {
            Processing::Studio => (STUDIO_INSIDE, STUDIO_OUTSIDE),
            Processing::Consumer => (CONSUMER_BOUND, CONSUMER_BOUND),
        };
        assert!(
            inside <= bound_in && outside <= bound_out,
            "{p:?}: {inside:.4} / {outside:.4} PH"
        );
        assert!(
            inside.max(outside) >= NEAR_BOUND * bound_out,
            "{p:?}: largest displacement {:.4} PH",
            inside.max(outside)
        );
        for z in 0..3 {
            assert!(
                zones[z] <= ZONE_NS[z].1,
                "{p:?} zone {z}: {:.1} ns",
                zones[z]
            );
            if p == Processing::Studio {
                assert!(
                    zones[z] >= ZONE_NS[z].0,
                    "studio zone {z}: {:.1} ns",
                    zones[z]
                );
            }
        }
        assert_eq!(g.registration[G].size, 0.0);
        assert_eq!(g.registration[G].pincushion, 0.0);

        let b_max = grid
            .iter()
            .map(|&(x, y)| black_shading_factor(&g.black_shading, x, y))
            .fold(f64::MIN, f64::max);
        assert!(
            (b_max - BLACK_CORNER).abs() <= 1e-3,
            "{p:?}: black shading {b_max:.4}"
        );
        let mean = (0..ACTIVE_H)
            .flat_map(|r| (0..ACTIVE_W).map(move |s| picture_position_ph(r, s as f64)))
            .map(|(x, y)| black_shading_factor(&g.black_shading, x, y))
            .sum::<f64>()
            / (ACTIVE_W * ACTIVE_H) as f64;
        assert!(mean.abs() <= 1e-3, "{p:?}: black shading mean {mean:.5}");

        for kind in kinds {
            let g = p.geometry(kind);
            let chs = chans(kind);
            for ch in [R, G, B] {
                let want = dark_at(kind, 25.0).unwrap() + chs[ch].bias_na;
                assert!(
                    (g.black_reference_na[ch] - want).abs() <= 1e-12,
                    "{kind:?} ch{ch}: reference {} nA against {want}",
                    g.black_reference_na[ch]
                );
                assert!(g.veil[ch].fraction > 0.0, "{kind:?}: no veil");
            }
            let loss = |ch: usize| 1.0 - 1.0 / (1.0 + g.corner_focus[ch]);
            let pbo = matches!(
                kind,
                TubeKind::Plumbicon
                    | TubeKind::PlumbiconBias
                    | TubeKind::PlumbiconAct
                    | TubeKind::PlumbiconDbc
            );
            for ch in [R, G, B] {
                let l = loss(ch);
                match kind {
                    TubeKind::Vidicon | TubeKind::Newvicon => assert!(
                        l >= SB2S3_FOCUS_LOSS.0 && l <= SB2S3_FOCUS_LOSS.1,
                        "{kind:?}: corner loss {l:.3}"
                    ),
                    TubeKind::Chalnicon => assert!(
                        (l - CHALNICON_FOCUS_LOSS).abs() <= CHALNICON_FOCUS_TOLERANCE,
                        "Chalnicon: corner loss {l:.3}"
                    ),
                    _ => assert!(
                        l >= STUDIO_FOCUS_LOSS.0 && l <= STUDIO_FOCUS_LOSS.1,
                        "{kind:?}: corner loss {l:.3}"
                    ),
                }
            }
            if pbo {
                for ch in [R, G] {
                    let f = g.white_fall[ch];
                    assert!(
                        f >= PBO_WHITE_FALL.0 && f <= PBO_WHITE_FALL.1,
                        "{kind:?} ch{ch}: {f}"
                    );
                }
                assert_eq!(g.white_fall[B], 0.0);
                let h = g.halation;
                assert!(h[R].reflectance > h[G].reflectance && h[G].reflectance > h[B].reflectance);
                assert!(h[B].reflectance > 0.0);
                for ch in [R, G, B] {
                    assert_eq!(h[ch].disc_mm, ANTI_HALATION_DISC_MM);
                    assert_eq!(h[ch].target_height_mm, TARGET_HEIGHT_MM);
                }
            } else {
                assert!(g.halation.iter().all(|h| h.reflectance == 0.0), "{kind:?}");
                let want = if kind == TubeKind::Saticon {
                    SATICON_WHITE_FALL
                } else {
                    (0.0, 0.0)
                };
                for ch in [R, G, B] {
                    let f = g.white_fall[ch];
                    assert!(
                        f >= want.0 && f <= want.1,
                        "{kind:?} ch{ch}: white fall {f}"
                    );
                }
            }
        }
    }
    let c = Processing::Studio.spec().corrections;
    assert!(c.flare && c.black_shading && c.white_shading);
    let c = Processing::Consumer.spec().corrections;
    assert!(!c.flare && !c.black_shading && !c.white_shading);
    assert_eq!(
        Processing::Studio.geometry(TubeKind::Off),
        GeometrySpec::NONE
    );
    assert!(!CameraParams::default().geometry);
    let p: CameraParams = toml::from_str("tube = \"plumbicon\"\ngeometry = true").unwrap();
    assert!(p.geometry);
}

/// With geometry off the camera is the engine without it, bit for bit; with it on, the profile's spec
/// for the tube is in use.
#[test]
fn geometry_off_is_the_engine_without_it() {
    let p = params(TubeKind::Plumbicon, Processing::Studio);
    let mut plain = CameraTube::new(&p).unwrap();
    let mut none = CameraTube::seeded_geometry(&p, 0, &GeometrySpec::NONE).unwrap();
    assert_eq!(plain.geometry(), &GeometrySpec::NONE);
    let mut f = flat_frame(40);
    for r in ACTIVE_H / 3..2 * ACTIVE_H / 3 {
        for s in ACTIVE_W / 3..2 * ACTIVE_W / 3 {
            let px = (r * ACTIVE_W + s) * 3;
            f.rgb[px..px + 3].copy_from_slice(&[250, 180, 90]);
        }
    }
    for _ in 0..2 {
        assert!(plain.read(&f, &f) == none.read(&f, &f));
    }
    let on = CameraTube::new(&CameraParams {
        geometry: true,
        ..p.clone()
    })
    .unwrap();
    assert_eq!(
        on.geometry(),
        &Processing::Studio.geometry(TubeKind::Plumbicon)
    );
}

// ------------------------------------------------------------------ deflection

/// The beam reads the target where the deflection puts it: a one-column line appears at the sample where
/// the displaced beam crosses it, in every tube and at the top, middle and bottom of the picture, and a
/// one-row line on the row whose displaced pass is centred on it.
#[test]
fn the_beam_lands_where_the_deflection_puts_it() {
    let c = Processing::Consumer.geometry(TubeKind::Newvicon);
    let g = GeometrySpec {
        deflection: c.deflection,
        registration: c.registration,
        ..GeometrySpec::NONE
    };
    let lines: Vec<usize> = (1..8).map(|i| i * ACTIVE_W / 8).collect();
    let mut cam = camera(TubeKind::Newvicon, Processing::Studio, &g);
    let lit = lines.clone();
    let s = read_steady(
        &mut cam,
        &|_, _, x| if lit.contains(&x) { LIT_NA } else { 0.0 },
        LIT_FRAMES,
    );
    for r in [24, ACTIVE_H / 2, ACTIVE_H - 25] {
        for ch in [R, G, B] {
            let v = row_of(&s, r, ch);
            for &j in &lines {
                let want = landing_sample(&g, ch, r, picture_position_ph(r, j as f64).0);
                let got = peak_centroid(&v, want.round() as usize);
                assert!(
                    (got - want).abs() <= PLACE_TOLERANCE,
                    "row {r} ch{ch} column {j}: read at {got:.3}, landing {want:.3}"
                );
            }
        }
    }

    let rows = [40, ACTIVE_H - 41];
    let mut cam = camera(TubeKind::Newvicon, Processing::Studio, &g);
    let s = read_steady(
        &mut cam,
        &|_, r, _| if rows.contains(&r) { LIT_NA } else { 0.0 },
        LIT_FRAMES,
    );
    for &r0 in &rows {
        for col in [ACTIVE_W / 2, ACTIVE_W - 40] {
            for ch in [R, G, B] {
                let (x, y) = picture_position_ph(r0, col as f64);
                let want = r0 as f64 - disp(&g, ch, x, y).1 * ACTIVE_H as f64;
                let c0 = want.round() as usize;
                let base = (s.rows[c0 - 16][ch][col] + s.rows[c0 + 16][ch][col]) as f64 / 2.0;
                let (mut num, mut den) = (0.0, 0.0);
                for r in c0 - 8..=c0 + 8 {
                    let w = (s.rows[r][ch][col] as f64 - base).max(0.0);
                    num += w * r as f64;
                    den += w;
                }
                let got = num / den;
                assert!(
                    (got - want).abs() <= ROW_TOLERANCE,
                    "row line {r0}, sample {col}, ch{ch}: read at row {got:.2}, pass at {want:.2}"
                );
            }
        }
    }
}

/// Signal current is proportional to scan speed (Philips T10 4.2): a uniformly lit target, and a dark
/// one, read in proportion to the beam's local speed along the line.
#[test]
fn signal_current_follows_scan_speed() {
    let g = GeometrySpec {
        deflection: Deflection {
            linearity: SPEED_LINEARITY,
            pincushion: 0.0,
        },
        ..GeometrySpec::NONE
    };
    let speed = |r: usize, s: usize| {
        let (x, _) = picture_position_ph(r, s as f64);
        1.0 + SPEED_LINEARITY * x / X_EDGE_PH
    };
    for light in [LIT_NA, 0.0] {
        let mut cam = camera(TubeKind::Newvicon, Processing::Studio, &g);
        let dark = cam.channels()[G].dark_na + cam.channels()[G].bias_na;
        let s = read_steady(&mut cam, &|_, _, _| light, LIT_FRAMES + 2);
        let tol = if light > 0.0 {
            SPEED_TOLERANCE
        } else {
            DARK_SPEED_TOLERANCE
        };
        for r in ACTIVE_H / 2 - 4..ACTIVE_H / 2 + 4 {
            for x in (END_MARGIN..ACTIVE_W - END_MARGIN).step_by(8) {
                let got = s.rows[r][G][x] as f64 / (light + dark);
                let want = speed(r, x);
                assert!(
                    (got / want - 1.0).abs() <= tol,
                    "light {light} nA, row {r}, sample {x}: {got:.4} against speed {want:.4}"
                );
            }
        }
        // the scan gain the correctors use is the same speed here
        let (x, y) = picture_position_ph(ACTIVE_H / 2, (ACTIVE_W / 8) as f64);
        let j = scan_gain(&g, G, x, y);
        assert!(
            (j - (1.0 + SPEED_LINEARITY * x / X_EDGE_PH)).abs() <= 1e-9,
            "J {j}"
        );
    }
}

/// The spot is the beam's width moving at scan speed: a one-column line reads as the Gaussian of the spot
/// sampled where the beam is, times its speed, so it is shorter in time where the scan is fast; the
/// column's charge is the same wherever it is read.
#[test]
fn the_spot_is_read_at_scan_speed() {
    let g = GeometrySpec {
        deflection: Deflection {
            linearity: SPOT_LINEARITY,
            pincushion: 0.0,
        },
        ..GeometrySpec::NONE
    };
    let (fast, slow) = (ACTIVE_W / 8, 7 * ACTIVE_W / 8);
    let mut cam = camera(TubeKind::Newvicon, Processing::Studio, &g);
    let ch = cam.channels()[G];
    let dark = ch.dark_na + ch.bias_na;
    let sigma = ch.sigma_h_s * FS_BB;
    let s = read_steady(
        &mut cam,
        &|_, _, x| if x == fast || x == slow { LIT_NA } else { 0.0 },
        LIT_FRAMES,
    );
    let r = ACTIVE_H / 2;
    let v = row_of(&s, r, G);
    let column_of = |x: f64| {
        let (xp, y) = picture_position_ph(r, x);
        x + disp(&g, G, xp, y).0 * samples_per_ph()
    };
    let speed = |x: f64| (column_of(x + 1e-3) - column_of(x - 1e-3)) / 2e-3;
    let mut amplitudes = Vec::new();
    let mut widths = Vec::new();
    for j in [fast, slow] {
        let centre = landing_sample(&g, G, r, picture_position_ph(r, j as f64).0).round() as usize;
        let samples: Vec<usize> = (centre - WINDOW..=centre + WINDOW).collect();
        let shape: Vec<f64> = samples
            .iter()
            .map(|&x| {
                let col = column_of(x as f64);
                let g_at = |c: f64| (-0.5 * ((col - c) / sigma).powi(2)).exp();
                let lo = (col - 4.0 * sigma).ceil() as i64;
                let hi = (col + 4.0 * sigma).floor() as i64;
                let norm: f64 = (lo..=hi).map(|c| g_at(c as f64)).sum();
                speed(x as f64) * g_at(j as f64) / norm
            })
            .collect();
        let measured: Vec<f64> = samples
            .iter()
            .map(|&x| v[x] - dark * speed(x as f64))
            .collect();
        let a = measured.iter().zip(&shape).map(|(m, e)| m * e).sum::<f64>()
            / shape.iter().map(|e| e * e).sum::<f64>();
        let peak = measured.iter().cloned().fold(f64::MIN, f64::max);
        for (i, (m, e)) in measured.iter().zip(&shape).enumerate() {
            assert!(
                (m - a * e).abs() <= SHAPE_TOLERANCE * peak,
                "column {j}, sample {}: {m:.3} nA against {:.3}",
                samples[i],
                a * e
            );
        }
        let total: f64 = measured.iter().sum();
        let mean: f64 = samples
            .iter()
            .zip(&measured)
            .map(|(x, m)| *x as f64 * m)
            .sum::<f64>()
            / total;
        let var: f64 = samples
            .iter()
            .zip(&measured)
            .map(|(x, m)| (*x as f64 - mean).powi(2) * m)
            .sum::<f64>()
            / total;
        amplitudes.push(a);
        widths.push(var.sqrt());
    }
    assert!(
        (amplitudes[0] / amplitudes[1] - 1.0).abs() <= SAME_CHARGE,
        "the same column reads {:.2} and {:.2} nA of charge",
        amplitudes[0],
        amplitudes[1]
    );
    assert!(
        widths[0] < widths[1],
        "spot in time: {:.3} samples where fast, {:.3} where slow",
        widths[0],
        widths[1]
    );
}

// ------------------------------------------------------------------ registration

/// The three tubes misregister as the profile's residuals say, and inside the matched triplets' bands:
/// vertical edges in zones A, B and C are read by red and blue this far from green.
#[test]
fn three_tubes_misregister_by_zone() {
    let s = Processing::Studio.geometry(TubeKind::Newvicon);
    let g = GeometrySpec {
        deflection: s.deflection,
        registration: s.registration,
        ..GeometrySpec::NONE
    };
    let j = |x: f64| column_at(x);
    let (a0, a1, b0, b1) = (j(-0.4), j(0.0), j(0.4), j(0.6));
    let lit = move |x: usize| (x >= a0 && x < a1) || (x >= b0 && x < b1);
    let mut cam = camera(TubeKind::Newvicon, Processing::Studio, &g);
    let sig = read_steady(
        &mut cam,
        &|_, _, x| if lit(x) { LIT_NA } else { 0.0 },
        LIT_FRAMES,
    );
    // (row, boundary column, zone, rising)
    let mid = ACTIVE_H / 2;
    let corner = row_at(0.45);
    let edges = [
        (mid, a0, 0, true),
        (mid, b0, 0, true),
        (mid, b1, 1, false),
        (corner, b1, 2, false),
    ];
    for (r, col, zone, rising) in edges {
        let x_b = boundary_ph(col);
        let read = |ch: usize| {
            let v = row_of(&sig, r, ch);
            let v: Vec<f64> = if rising {
                v
            } else {
                v.iter().map(|a| -a).collect()
            };
            let want = landing_sample(&g, ch, r, x_b);
            let got = edge_centroid(&v, want.round() as usize);
            (got, want)
        };
        let (g_got, g_want) = read(G);
        assert!(
            (g_got - g_want).abs() <= EDGE_TOLERANCE,
            "green edge at column {col}, row {r}: {g_got:.3} against {g_want:.3}"
        );
        for ch in [R, B] {
            let (got, want) = read(ch);
            let (d_got, d_want) = (got - g_got, want - g_want);
            assert!(
                (d_got - d_want).abs() <= EDGE_TOLERANCE,
                "ch{ch} against green at column {col}, row {r}: {d_got:.3} samples, {d_want:.3} expected"
            );
            let ns = d_got.abs() / FS_BB * 1e9;
            assert!(ns <= ZONE_NS[zone].1, "ch{ch} zone {zone}: {ns:.1} ns");
        }
    }
}

// ------------------------------------------------------------------ shading

/// Black shading is the target's dark current, uneven as the profile says, and settles with it; the
/// studio corrector, set on a capped camera at 25 deg C, removes it there and leaves the shading of the
/// current the target has gained since when the camera is warm. The consumer camera leaves it.
#[test]
fn black_shading_follows_the_dark_current_and_its_corrector_is_set_at_25_degrees() {
    let studio = Processing::Studio.geometry(TubeKind::Vidicon);
    let g = GeometrySpec {
        black_shading: studio.black_shading,
        black_reference_na: studio.black_reference_na,
        ..GeometrySpec::NONE
    };
    let mut cam = camera(TubeKind::Vidicon, Processing::Studio, &g);
    let dark = cam.channels()[G].dark_na;
    let s = read_steady(&mut cam, &|_, _, _| 0.0, DARK_FRAMES);
    for (r, x) in check_points() {
        let mut got = 0.0;
        let mut want = 0.0;
        for rr in r..r + 4 {
            for xx in x..x + 4 {
                let (px, py) = picture_position_ph(rr, xx as f64);
                got += s.rows[rr][G][xx] as f64;
                want += dark * (1.0 + black_shading_factor(&g.black_shading, px, py));
            }
        }
        assert!(
            (got - want).abs() <= DARK_TOLERANCE * dark * 16.0,
            "row {r}, sample {x}: {:.3} nA against {:.3}",
            got / 16.0,
            want / 16.0
        );
    }

    let shaded = |chs: &[TubeChannel; 3]| {
        let chs = *chs;
        let gs = g;
        synthetic(move |k, r, s| {
            let (x, y) = picture_position_ph(r, s as f64);
            (chs[k].dark_na + chs[k].bias_na)
                * (1.0 + black_shading_factor(&gs.black_shading, x, y))
        })
    };
    let cold = chans(TubeKind::Vidicon);
    let black = |chs: &[TubeChannel; 3]| -> [f64; 3] {
        std::array::from_fn(|k| chs[k].dark_na + chs[k].bias_na)
    };
    let linearised = |chs: &[TubeChannel; 3], k: usize, l: f64| {
        let e = if chs[k].linearise {
            1.0 / chs[k].gamma
        } else {
            1.0
        };
        if l == 0.0 {
            0.0
        } else {
            l.abs().powf(e).copysign(l)
        }
    };
    let corrected = VideoProcessor::new(Processing::Studio, &cold, black(&cold))
        .with_geometry(&g, [0.0; 3])
        .process(&shaded(&cold));
    let plain = VideoProcessor::new(Processing::Consumer, &cold, black(&cold))
        .with_geometry(&g, [0.0; 3])
        .process(&shaded(&cold));
    let warm = TubeKind::Vidicon
        .channels_at(BEAM_RATIO_DEFAULT, 40.0)
        .unwrap();
    let warm_out = VideoProcessor::new(Processing::Studio, &warm, black(&warm))
        .with_geometry(&g, [0.0; 3])
        .process(&shaded(&warm));
    for (r, x) in check_points() {
        let (px, py) = picture_position_ph(r, x as f64);
        let b = black_shading_factor(&g.black_shading, px, py);
        for k in [R, G, B] {
            let v = video_at(&corrected, k, r, x);
            assert!(
                v.abs() <= CORRECTED,
                "corrected at 25 deg C, row {r} sample {x}: {v:.5}"
            );
            let want = gamma_correct(linearised(
                &cold,
                k,
                cold[k].dark_na * b / cold[k].peak_signal_na,
            ));
            let v = video_at(&plain, k, r, x);
            assert!(
                (v - want).abs() <= CORRECTED,
                "consumer, row {r} sample {x}: {v:.5} against {want:.5}"
            );
            let residual = b * (warm[k].dark_na - g.black_reference_na[k]) / warm[k].peak_signal_na;
            let want = gamma_correct(linearised(&warm, k, residual));
            let v = video_at(&warm_out, k, r, x);
            assert!(
                (v - want).abs() <= CORRECTED,
                "corrected at 40 deg C, row {r} sample {x}: {v:.5} against {want:.5}"
            );
        }
    }
}

/// White shading is the target's sensitivity, centre-high by the tube's fall (lead-oxide blue flat), on
/// the light it converts. The studio correctors, set on a flat white field, read a grey field flat
/// together with the scan gain of the deflection; the consumer camera shows the shading.
#[test]
fn white_shading_and_its_correction() {
    for kind in [TubeKind::Saticon, TubeKind::Plumbicon] {
        let full = Processing::Studio.geometry(kind);
        let g = GeometrySpec {
            white_fall: full.white_fall,
            ..GeometrySpec::NONE
        };
        let cam = camera(kind, Processing::Studio, &g);
        let chs = *cam.channels();
        let p = cam.photo_currents(&flat_frame(255));
        for (r, s) in check_points() {
            let (x, y) = picture_position_ph(r, s as f64);
            for k in [R, G, B] {
                let want = chs[k].peak_signal_na
                    * white_shading_factor(g.white_fall[k], x, y).powf(chs[k].gamma);
                let got = p[(r * ACTIVE_W + s) * 3 + k];
                assert!(
                    (got / want - 1.0).abs() <= 1e-6,
                    "{kind:?} ch{k} row {r} sample {s}: {got:.3} nA against {want:.3}"
                );
            }
        }
    }

    let kind = TubeKind::Saticon;
    let chs = chans(kind);
    for processing in [Processing::Studio, Processing::Consumer] {
        let g = processing.geometry(kind);
        let black: [f64; 3] = std::array::from_fn(|k| chs[k].dark_na + chs[k].bias_na);
        let sig = synthetic(|k, r, s| {
            let (x, y) = picture_position_ph(r, s as f64);
            let j = scan_gain(&g, k, x, y);
            let w = white_shading_factor(g.white_fall[k], x, y);
            let b = black_shading_factor(&g.black_shading, x, y);
            j * (black[k] * (1.0 + b) + chs[k].peak_signal_na * (w * GREY_LIGHT).powf(chs[k].gamma))
        });
        let out = VideoProcessor::new(processing, &chs, black)
            .with_geometry(&g, [0.0; 3])
            .process(&sig);
        let flat = gamma_correct(GREY_LIGHT.powf(chs[G].gamma));
        let worst = check_points()
            .iter()
            .flat_map(|&(r, s)| [R, G, B].map(|k| (video_at(&out, k, r, s) - flat).abs()))
            .fold(0.0, f64::max);
        match processing {
            Processing::Studio => assert!(worst <= CORRECTED, "studio: off flat by {worst:.5}"),
            Processing::Consumer => {
                assert!(worst >= SHADED_MIN, "consumer: off flat by {worst:.5}")
            }
        }
    }
}

// ------------------------------------------------------------------ flare

/// The black-square chart read through the veil: the Saticon's 1 % at the centre and 0.5 % at the corner,
/// the vidicon's 5 % at the centre through its 0.65 transfer; its corner reads below the centre but above
/// the data's 1 %, which no shift-invariant veil reaches (the contract's open point).
#[test]
fn a_black_square_reads_the_data_sheets_flare() {
    for (kind, (centre, corner), tol) in [
        (TubeKind::Saticon, SATICON_SQUARE, SATICON_SQUARE_TOLERANCE),
        (TubeKind::Vidicon, VIDICON_SQUARE, VIDICON_SQUARE_TOLERANCE),
    ] {
        let full = Processing::Studio.geometry(kind);
        let g = GeometrySpec {
            veil: full.veil,
            ..GeometrySpec::NONE
        };
        let cam = camera(kind, Processing::Studio, &g);
        let chs = *cam.channels();
        let reading = |(cx, cy): (f64, f64)| {
            let p = cam.photo_currents(&chart(cx, cy));
            let (r, s) = (row_at(cy), column_at(cx));
            let at = (r * ACTIVE_W + s) * 3;
            [R, G, B].map(|k| p[at + k] / chs[k].peak_signal_na)
        };
        let c = reading((0.0, 0.0));
        let k = reading(corner_square());
        for ch in [R, G, B] {
            assert!(
                (c[ch] - centre).abs() <= tol,
                "{kind:?} ch{ch}: centre square {:.4} against {centre}",
                c[ch]
            );
            if kind == TubeKind::Saticon {
                assert!(
                    (k[ch] - corner).abs() <= tol,
                    "Saticon ch{ch}: corner square {:.4} against {corner}",
                    k[ch]
                );
            } else {
                assert!(
                    k[ch] < VIDICON_CORNER_RATIO * c[ch] && k[ch] > corner,
                    "vidicon ch{ch}: corner square {:.4}, centre {:.4}",
                    k[ch],
                    c[ch]
                );
            }
        }
    }
}

/// The flare corrector, set on the chart, blacks its centre square by taking the average picture level
/// off the clamp (US 4001502); being one level for the whole picture it leaves the corner square, which
/// gathers less flare, below black. The consumer camera leaves the flare.
#[test]
fn the_flare_corrector_blacks_the_centre_square() {
    let kind = TubeKind::Saticon;
    let full = Processing::Studio.geometry(kind);
    let g = GeometrySpec {
        veil: full.veil,
        ..GeometrySpec::NONE
    };
    let studio = camera(kind, Processing::Studio, &g);
    let consumer = camera(kind, Processing::Consumer, &g);
    let black = studio.black();
    let chart_mean = 1.0 - BLACK_SQUARE_PH * BLACK_SQUARE_PH / TARGET_ASPECT;
    let c = studio.processor().flare_coefficient();
    for ch in [R, G, B] {
        let want = SATICON_SQUARE.0 / chart_mean;
        assert!(
            (c[ch] / want - 1.0).abs() <= FLARE_COEFFICIENT_TOLERANCE,
            "ch{ch}: flare coefficient {:.4} against {want:.4}",
            c[ch]
        );
    }
    assert_eq!(consumer.processor().flare_coefficient(), [0.0; 3]);

    let read = |cam: &CameraTube, square: (f64, f64)| {
        let sig = signal_from(&cam.photo_currents(&chart(square.0, square.1)), black);
        let out = cam.processor().process(&sig);
        let (r, s) = (row_at(square.1), column_at(square.0));
        [R, G, B].map(|k| gamma_inverse(video_at(&out, k, r, s)))
    };
    let centre = read(&studio, (0.0, 0.0));
    let corner = read(&studio, corner_square());
    let plain = read(&consumer, (0.0, 0.0));
    for ch in [R, G, B] {
        assert!(
            centre[ch].abs() <= FLARE_BLACK,
            "studio ch{ch}: centre square {:.5}",
            centre[ch]
        );
        assert!(
            corner[ch] >= CORNER_BELOW.0 && corner[ch] <= CORNER_BELOW.1,
            "studio ch{ch}: corner square {:.5}",
            corner[ch]
        );
        assert!(
            (plain[ch] - SATICON_SQUARE.0).abs() <= SATICON_SQUARE_TOLERANCE,
            "consumer ch{ch}: centre square {:.5}",
            plain[ch]
        );
    }
}

/// The contract's ring kernel, written out, per PH^2.
fn ring_kernel(h: &Halation, d: f64) -> f64 {
    let t = (h.faceplate_mm + h.disc_mm) / h.target_height_mm;
    let tan_c = (1.0 / GLASS_INDEX).asin().tan();
    if d < 2.0 * t * tan_c {
        return 0.0;
    }
    (1.0 + tan_c * tan_c)
        / (std::f64::consts::PI * (2.0 * t).powi(2))
        / (1.0 + (d / (2.0 * t)).powi(2)).powi(2)
}

/// Lead oxide reflects light into the faceplate, which returns it as a ring (red most, as the layer
/// reflects red most). Without the disc the ring's inner edge is at 0.21 PH and its light is the
/// derivation's; the anti-halation disc moves it beyond the picture.
#[test]
fn lead_oxide_halation_is_a_ring_the_disc_takes_away() {
    let full = Processing::Studio.geometry(TubeKind::Plumbicon);
    let with_disc = GeometrySpec {
        halation: full.halation,
        ..GeometrySpec::NONE
    };
    let mut bare = with_disc;
    for h in bare.halation.iter_mut() {
        h.disc_mm = 0.0;
    }
    let radius = 2.0 * FACEPLATE_MM * (1.0 / GLASS_INDEX).asin().tan() / TARGET_HEIGHT_MM;
    assert!((halation_radius_ph(&bare.halation[R]) - radius).abs() <= 1e-9);
    assert!(halation_radius_ph(&with_disc.halation[R]) > RHO_CORNER_PH);

    let mut spot = flat_frame(0);
    let mut spot_px = Vec::new();
    for r in 0..ACTIVE_H {
        for s in 0..ACTIVE_W {
            let (x, y) = picture_position_ph(r, s as f64);
            if x.hypot(y) <= SPOT_RADIUS_PH {
                let p = (r * ACTIVE_W + s) * 3;
                spot.rgb[p..p + 3].fill(255);
                spot_px.push((x, y));
            }
        }
    }
    let pixel_area = (TARGET_ASPECT / ACTIVE_W as f64) * (1.0 / ACTIVE_H as f64);
    let light = SPOT_IRIS_STOPS.exp2();
    let run = |g: &GeometrySpec| {
        let cam = CameraTube::seeded_geometry(
            &CameraParams {
                tube: TubeKind::Plumbicon,
                iris_stops: SPOT_IRIS_STOPS,
                ..CameraParams::default()
            },
            0,
            g,
        )
        .unwrap();
        let chs = *cam.channels();
        let p = cam.photo_currents(&spot);
        (p, chs)
    };
    let (p_bare, chs) = run(&bare);
    let (p_disc, _) = run(&with_disc);
    let lin = |p: &[f64], r: usize, s: usize, k: usize| {
        (p[(r * ACTIVE_W + s) * 3 + k] / chs[k].peak_signal_na).powf(1.0 / chs[k].gamma)
    };
    let mut ring = [0.0; 3];
    let mut ring_want = [0.0; 3];
    let mut n_ring = 0.0;
    for r in 0..ACTIVE_H {
        for s in 0..ACTIVE_W {
            let (x, y) = picture_position_ph(r, s as f64);
            let d = x.hypot(y);
            if d > INSIDE_RING.0 && d < INSIDE_RING.1 {
                for k in [R, G, B] {
                    assert!(
                        lin(&p_bare, r, s, k) <= NO_LIGHT,
                        "inside the ring at {d:.3} PH"
                    );
                }
            }
            if d > SPOT_RADIUS_PH + 0.01 {
                for k in [R, G, B] {
                    assert!(
                        lin(&p_disc, r, s, k) <= NO_LIGHT,
                        "with the disc at {d:.3} PH"
                    );
                }
            }
            if d > ON_RING.0 && d < ON_RING.1 {
                n_ring += 1.0;
                for k in [R, G, B] {
                    ring[k] += lin(&p_bare, r, s, k);
                    let h = &bare.halation[k];
                    let share = h.reflectance * (1.0 - 1.0 / (GLASS_INDEX * GLASS_INDEX));
                    ring_want[k] += share
                        * light
                        * pixel_area
                        * spot_px
                            .iter()
                            .map(|(qx, qy)| ring_kernel(h, (x - qx).hypot(y - qy)))
                            .sum::<f64>();
                }
            }
        }
    }
    for k in [R, G, B] {
        assert!(
            (ring[k] / ring_want[k] - 1.0).abs() <= RING_TOLERANCE,
            "ch{k}: ring light {:.3e} against {:.3e}",
            ring[k] / n_ring,
            ring_want[k] / n_ring
        );
    }
    let red_over_green = ring[R] / ring[G];
    let want = bare.halation[R].reflectance / bare.halation[G].reflectance;
    assert!(
        (red_over_green / want - 1.0).abs() <= RED_OVER_GREEN_TOLERANCE,
        "red over green {red_over_green:.3} against {want:.3}"
    );
}

// ------------------------------------------------------------------ corner focus

/// The spot grows toward the corners as the tube's corner focus says: a one-column line near a corner is
/// read with the wider Gaussian.
#[test]
fn corner_focus_softens_the_corners() {
    let full = Processing::Studio.geometry(TubeKind::Newvicon);
    let g = GeometrySpec {
        corner_focus: full.corner_focus,
        ..GeometrySpec::NONE
    };
    let (centre_col, corner_col) = (ACTIVE_W / 2, ACTIVE_W - 24);
    let mut cam = camera(TubeKind::Newvicon, Processing::Studio, &g);
    let ch = cam.channels()[G];
    let dark = ch.dark_na + ch.bias_na;
    let s = read_steady(
        &mut cam,
        &|_, _, x| {
            if x == centre_col || x == corner_col {
                LIT_NA
            } else {
                0.0
            }
        },
        LIT_FRAMES,
    );
    let mut ratio = Vec::new();
    for (r, j) in [(ACTIVE_H / 2, centre_col), (10, corner_col)] {
        let v = row_of(&s, r, G);
        let (x, y) = picture_position_ph(r, j as f64);
        let rho = x.hypot(y) / RHO_CORNER_PH;
        let sigma = ch.sigma_h_s * FS_BB * (1.0 + g.corner_focus[G] * rho * rho);
        let moments = |w: &dyn Fn(usize) -> f64| {
            let xs: Vec<usize> = (j - WINDOW..=j + WINDOW).collect();
            let total: f64 = xs.iter().map(|x| w(*x)).sum();
            let mean = xs.iter().map(|x| *x as f64 * w(*x)).sum::<f64>() / total;
            xs.iter()
                .map(|x| (*x as f64 - mean).powi(2) * w(*x))
                .sum::<f64>()
                / total
        };
        let got = moments(&|x| v[x] - dark).sqrt();
        let want = moments(&|x| {
            let g_at = |c: f64| (-0.5 * ((x as f64 - c) / sigma).powi(2)).exp();
            let lo = (x as f64 - 4.0 * sigma).ceil() as i64;
            let hi = (x as f64 + 4.0 * sigma).floor() as i64;
            g_at(j as f64) / (lo..=hi).map(|c| g_at(c as f64)).sum::<f64>()
        })
        .sqrt();
        assert!(
            (got / want - 1.0).abs() <= FOCUS_TOLERANCE,
            "row {r}, column {j}: spot {got:.3} samples against {want:.3}"
        );
        ratio.push(got);
    }
    assert!(
        ratio[1] / ratio[0] >= CORNER_GROWTH_MIN,
        "corner spot {:.3} against centre {:.3}",
        ratio[1],
        ratio[0]
    );
}

// ------------------------------------------------------------------ the whole camera

/// The studio camera with its geometry reads a grey field flat across the picture: its correctors take
/// off the scan gain, the shading and the flare's average. The consumer camera shows its deflection's
/// faster left edge and its tubes' white shading.
#[test]
fn a_studio_camera_reads_its_field_flat_and_a_consumer_camera_does_not() {
    let level = (oetf(GREY_LIGHT) * 255.0).round() as u8;
    let frame = flat_frame(level);
    let (r0, r1) = (BLOCK_INSET, ACTIVE_H - BLOCK_INSET - BLOCK);
    let (s0, s1) = (BLOCK_INSET, ACTIVE_W - BLOCK_INSET - BLOCK);
    let (rm, sm) = (ACTIVE_H / 2 - BLOCK / 2, ACTIVE_W / 2 - BLOCK / 2);
    for processing in [Processing::Studio, Processing::Consumer] {
        let mut cam = CameraTube::new(&CameraParams {
            geometry: true,
            ..params(TubeKind::Plumbicon, processing)
        })
        .unwrap();
        let mut v = cam.video(&frame, &frame);
        for _ in 1..GREY_FRAMES {
            v = cam.video(&frame, &frame);
        }
        for k in [R, G, B] {
            let centre = block_linear(&v, k, rm, sm);
            let corners = [(r0, s0), (r0, s1), (r1, s0), (r1, s1)]
                .map(|(r, s)| block_linear(&v, k, r, s) - centre);
            match processing {
                Processing::Studio => {
                    for d in corners {
                        assert!(
                            d.abs() <= FLAT,
                            "studio ch{k}: a corner off the centre by {d:.4}"
                        );
                    }
                }
                Processing::Consumer => {
                    for d in [corners[0], corners[2]] {
                        assert!(
                            d >= LEFT_EDGE_SHADING_MIN,
                            "consumer ch{k}: a left corner above the centre by {d:.4}"
                        );
                    }
                }
            }
        }
    }
}
