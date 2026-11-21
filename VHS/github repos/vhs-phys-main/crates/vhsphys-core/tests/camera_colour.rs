//! Spectral taking and the colour matrix (docs/design/CAMERA_SCAN.md, "Spectral taking and the colour
//! matrix"): the scene's three primaries as spectra, each tube's taking behind the IR-cut filter and its
//! arm of the prism, weighted by its photoconductor's response, and the processing amplifier's linear
//! matrix that restores the primaries.
//!
//! Sources: docs/CAMERA_RESEARCH.md ("Six photoconductors differ in gamma, dark current and lag
//! mechanism": Philips T10 and T12, Hitachi H8397A and Toshiba 1973 spectral figures; "Head-amplifier
//! noise and processing lie mostly above 2.5 MHz": US 4032974's order and the BBC's Plumbicon matrix,
//! Jones 1968). ITU-R BT.709 primaries, white and luminance shares. The CIE 1931 observer is taken
//! through the analytic multi-lobe fit of Wyman, Sloan and Shirley 2013 (JCGT 2(2)), written out here
//! so the chromaticities are checked against an observer the camera does not share.

use vhsphys_core::camera::{CameraParams, CameraTube, TargetSignal};
use vhsphys_core::camera_processing::{gamma_correct, gamma_inverse, Processing, VideoProcessor};
use vhsphys_core::consts::{ACTIVE_H, ACTIVE_W};
use vhsphys_core::spectral::{
    colour_matrix, primary_spectrum, taking, taking_matrix, wavelength_nm, Material, MATERIALS,
    N_LAMBDA,
};
use vhsphys_core::tube::{
    oetf, oetf_inverse, photo_current, TubeChannel, TubeKind, BEAM_RATIO_DEFAULT,
};
use vhsphys_core::video::Frame;

const R: usize = 0;
const G: usize = 1;
const B: usize = 2;

/// BT.709 primaries and D65 white, CIE 1931 (x, y), and the primaries' luminance shares of white.
const BT709_XY: [(f64, f64); 3] = [(0.64, 0.33), (0.30, 0.60), (0.15, 0.06)];
const D65_XY: (f64, f64) = (0.3127, 0.3290);
const BT709_Y: [f64; 3] = [0.2126, 0.7152, 0.0722];
/// The shapes are solved for the primaries to well inside this; the Wyman fit itself is good to about
/// 1 % of the observer, a few thousandths in chromaticity.
const XY_TOLERANCE: f64 = 0.003;
const LUMINANCE_TOLERANCE: f64 = 0.002;

/// Spectral figures are read by eye to about +-10 nm and a few hundredths of response.
const PEAK_NM_TOLERANCE: f64 = 10.0;
const FIG_TOLERANCE: f64 = 0.03;

/// The BBC's optimum matrix for Plumbicon cameras (Jones 1968), and how close the Plumbicon camera's
/// must come: the prism and IR-cut are fitted to it [est], so each term within a tenth.
const BBC_PLUMBICON: [[f64; 3]; 3] = [
    [1.14, -0.18, 0.04],
    [-0.06, 1.23, -0.17],
    [-0.03, 0.02, 1.01],
];
const BBC_TOLERANCE: f64 = 0.10;
/// The readings of a camera without matrix are the inverse of its matrix; the BBC matrix's inverse,
/// against the Plumbicon camera's taking, to this.
const BBC_TAKING_TOLERANCE: f64 = 0.07;
/// The standard layer is dark by 650 nm, so a red arm of it reads green strongly: its red row's green
/// term is at least this negative, where the BBC's is -0.18.
const STANDARD_RED_ROW_GREEN: f64 = -0.5;
/// Every camera needs the matrix: the BBC's largest off-diagonal is 0.18; no material's is below this.
const OFF_DIAGONAL_MIN: f64 = 0.10;
/// Without a matrix each primary loses at least this much on its own channel.
const SATURATION_LOSS_MIN: f64 = 0.03;
/// Materials whose red reading is within this of each other are not ranked.
const RANK_TOLERANCE: f64 = 0.01;
/// Behind a matrix made for lead oxide, a Newvicon's red comes out over-saturated and a Saticon's
/// under-corrected, by at least these (half the effect of the tables' knots).
const NEWVICON_OVER: f64 = 0.03;
const SATICON_UNDER: f64 = 0.05;

/// Sums that must be exact up to rounding.
const EXACT: f64 = 1e-9;
/// The same engine fed the same currents: equal to f32 rounding.
const SAME_READING: f64 = 1e-5;
/// A synthetic, noise-free line signal through the processor.
const PROCESSOR_TOLERANCE: f64 = 2e-3;

/// The colour-bar test of the whole camera: linear light of the bars, frames run, the bars' columns
/// and the rows averaged, and how close the camera's video comes to the expected.
const BAR_LIGHT: f64 = 0.5;
const BAR_FRAMES: usize = 8;
const BAR_ROWS: (usize, usize) = (ACTIVE_H / 4, 3 * ACTIVE_H / 4);
const BAR_MARGIN: usize = 24;
const VIDEO_TOLERANCE: f64 = 0.02;
/// A camera with a matrix restores the ideal camera's colours to this (lead oxide's 0.95 transfer
/// before the matrix leaves a few thousandths in linear light).
const RESTORED_TOLERANCE: f64 = 0.03;

const KINDS: [TubeKind; 5] = [
    TubeKind::Plumbicon,
    TubeKind::Vidicon,
    TubeKind::Newvicon,
    TubeKind::Saticon,
    TubeKind::Chalnicon,
];

// ------------------------------------------------------------------ helpers

/// Asymmetric Gaussian lobe of the Wyman, Sloan and Shirley fit.
fn lobe(l: f64, mu: f64, s1: f64, s2: f64) -> f64 {
    let s = if l < mu { s1 } else { s2 };
    (-0.5 * ((l - mu) / s).powi(2)).exp()
}

/// CIE 1931 2 degree colour matching functions, multi-lobe fit (Wyman, Sloan and Shirley 2013).
fn cie(l: f64) -> [f64; 3] {
    let x = 1.056 * lobe(l, 599.8, 37.9, 31.0) + 0.362 * lobe(l, 442.0, 16.0, 26.7)
        - 0.065 * lobe(l, 501.1, 20.4, 26.2);
    let y = 0.821 * lobe(l, 568.8, 46.9, 40.5) + 0.286 * lobe(l, 530.9, 16.3, 31.1);
    let z = 1.217 * lobe(l, 437.0, 11.8, 36.0) + 0.681 * lobe(l, 459.0, 26.0, 13.8);
    [x, y, z]
}

fn tristimulus(spectrum: &dyn Fn(f64) -> f64) -> [f64; 3] {
    let mut t = [0.0; 3];
    for i in 0..N_LAMBDA {
        let l = wavelength_nm(i);
        let e = spectrum(l);
        let c = cie(l);
        for k in 0..3 {
            t[k] += c[k] * e;
        }
    }
    t
}

fn chromaticity(t: [f64; 3]) -> (f64, f64) {
    let s = t[0] + t[1] + t[2];
    (t[0] / s, t[1] / s)
}

fn materials(kind: TubeKind) -> [Material; 3] {
    kind.materials().expect("a tube")
}

fn mul(a: &[[f64; 3]; 3], b: &[[f64; 3]; 3]) -> [[f64; 3]; 3] {
    std::array::from_fn(|i| std::array::from_fn(|j| (0..3).map(|k| a[i][k] * b[k][j]).sum()))
}

fn inverse(m: &[[f64; 3]; 3]) -> [[f64; 3]; 3] {
    let det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
        - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
        + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    let c = |i: usize, j: usize| {
        let (r0, r1) = ((i + 1) % 3, (i + 2) % 3);
        let (c0, c1) = ((j + 1) % 3, (j + 2) % 3);
        m[r0][c0] * m[r1][c1] - m[r0][c1] * m[r1][c0]
    };
    std::array::from_fn(|i| std::array::from_fn(|j| c(j, i) / det))
}

fn max_off_diagonal(m: &[[f64; 3]; 3]) -> f64 {
    let mut worst: f64 = 0.0;
    for i in 0..3 {
        for j in 0..3 {
            if i != j {
                worst = worst.max(m[i][j].abs());
            }
        }
    }
    worst
}

/// Grid wavelength of a material's largest response.
fn peak_nm(m: Material) -> f64 {
    (0..N_LAMBDA)
        .map(wavelength_nm)
        .max_by(|a, b| m.response(*a).total_cmp(&m.response(*b)))
        .expect("a grid")
}

fn camera(kind: TubeKind, processing: Processing, spectral: bool) -> CameraTube {
    CameraTube::new(&CameraParams {
        tube: kind,
        processing,
        spectral,
        ..CameraParams::default()
    })
    .expect("a tube")
}

fn flat_frame(v: [u8; 3]) -> Frame {
    let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
    for px in f.rgb.chunks_mut(3) {
        px.copy_from_slice(&v);
    }
    f
}

/// The level whose linear light is nearest `l`.
fn level_for(l: f64) -> u8 {
    (oetf(l) * 255.0).round() as u8
}

/// Four vertical bars across the picture: red, green, blue and grey, each of `level` in its primaries.
fn bars(level: u8) -> Frame {
    let mut f = Frame::new(ACTIVE_W, ACTIVE_H);
    for r in 0..ACTIVE_H {
        for x in 0..ACTIVE_W {
            let bar = 4 * x / ACTIVE_W;
            let v = match bar {
                0 => [level, 0, 0],
                1 => [0, level, 0],
                2 => [0, 0, level],
                _ => [level; 3],
            };
            let p = (r * ACTIVE_W + x) * 3;
            f.rgb[p..p + 3].copy_from_slice(&v);
        }
    }
    f
}

/// Columns of bar `i` away from its edges.
fn bar_columns(i: usize) -> (usize, usize) {
    (
        i * ACTIVE_W / 4 + BAR_MARGIN,
        (i + 1) * ACTIVE_W / 4 - BAR_MARGIN,
    )
}

/// The linear light of each primary in bar `i`.
fn bar_light(i: usize, l: f64) -> [f64; 3] {
    match i {
        0 => [l, 0.0, 0.0],
        1 => [0.0, l, 0.0],
        2 => [0.0, 0.0, l],
        _ => [l; 3],
    }
}

fn mean_video(rows: &[[Vec<f32>; 3]], ch: usize, cols: (usize, usize)) -> f64 {
    let mut sum = 0.0;
    let mut n = 0.0;
    for row in &rows[BAR_ROWS.0..BAR_ROWS.1] {
        for x in cols.0..cols.1 {
            sum += row[ch][x] as f64;
            n += 1.0;
        }
    }
    sum / n
}

// ------------------------------------------------------------------ the scene

/// Each primary's spectrum has the CIE chromaticity of its BT.709 primary and its share of white's
/// luminance, and the three add to a white of D65's chromaticity. The spectra are light: never negative.
#[test]
fn the_scene_primaries_are_the_bt709_primaries() {
    assert_eq!(N_LAMBDA, 81);
    assert_eq!(wavelength_nm(0), 380.0);
    assert_eq!(wavelength_nm(N_LAMBDA - 1), 780.0);
    for c in [R, G, B] {
        for i in 0..N_LAMBDA {
            let e = primary_spectrum(c, wavelength_nm(i));
            assert!(e >= 0.0, "primary {c} at {} nm: {e}", wavelength_nm(i));
        }
    }
    let white = tristimulus(&|l| (0..3).map(|c| primary_spectrum(c, l)).sum());
    for c in [R, G, B] {
        let t = tristimulus(&|l| primary_spectrum(c, l));
        let (x, y) = chromaticity(t);
        let (wx, wy) = BT709_XY[c];
        assert!(
            (x - wx).abs() <= XY_TOLERANCE && (y - wy).abs() <= XY_TOLERANCE,
            "primary {c}: ({x:.4}, {y:.4}) against ({wx}, {wy})"
        );
        let share = t[1] / white[1];
        assert!(
            (share - BT709_Y[c]).abs() <= LUMINANCE_TOLERANCE,
            "primary {c}: luminance share {share:.4} against {}",
            BT709_Y[c]
        );
    }
    let (x, y) = chromaticity(white);
    assert!(
        (x - D65_XY.0).abs() <= XY_TOLERANCE && (y - D65_XY.1).abs() <= XY_TOLERANCE,
        "white ({x:.4}, {y:.4})"
    );
}

// ------------------------------------------------------------------ the materials

/// The responses pass through the points read from the data sheets' figures, and every kind has the
/// layers the contract gives it (the red Plumbicon on the red-extended layer).
#[test]
fn the_material_responses_pass_through_the_published_points() {
    let near = |m: Material, nm: f64, want: f64| {
        let got = m.response(nm);
        assert!(
            (got - want).abs() <= FIG_TOLERANCE,
            "{m:?} at {nm} nm: {got:.3} against {want}"
        );
    };
    let peak_in = |m: Material, lo: f64, hi: f64| {
        let p = peak_nm(m);
        assert!(
            p >= lo - PEAK_NM_TOLERANCE && p <= hi + PEAK_NM_TOLERANCE,
            "{m:?} peaks at {p} nm, data {lo}-{hi}"
        );
    };
    for m in MATERIALS {
        let top = (0..N_LAMBDA)
            .map(|i| m.response(wavelength_nm(i)))
            .fold(0.0, f64::max);
        assert!((top - 1.0).abs() <= EXACT, "{m:?} peak {top}");
        for i in 0..N_LAMBDA {
            assert!(m.response(wavelength_nm(i)) >= 0.0);
        }
    }
    // standard PbO peaks near 485-500 nm and is dark by 650 nm (Philips T10, Fig. 1)
    peak_in(Material::Pbo, 485.0, 500.0);
    assert!(Material::Pbo.response(650.0) <= 0.01);
    // the sulphur-doped layer tails to 850-950 nm: it still responds at the grid's end, and above
    // the standard layer everywhere
    assert!(Material::PboExtendedRed.response(780.0) > 0.05);
    for i in 0..N_LAMBDA {
        let l = wavelength_nm(i);
        assert!(Material::PboExtendedRed.response(l) >= Material::Pbo.response(l) - EXACT);
    }
    // Sb2S3 peaks near 550 nm and is at half by 675 nm
    peak_in(Material::Sb2s3, 550.0, 550.0);
    near(Material::Sb2s3, 675.0, 0.5);
    // Newvicon peaks at 730-740 nm (575 mA/W) and keeps 220 mA/W at 400 nm (Philips T12, Fig. 1)
    peak_in(Material::Newvicon, 730.0, 740.0);
    near(Material::Newvicon, 400.0, 220.0 / 575.0);
    // Saticon: 100 % at 430-450 nm, about 40 % at 595 nm, about 3 % at 700 nm (H8397A, Fig. 3)
    near(Material::Saticon, 430.0, 1.0);
    near(Material::Saticon, 450.0, 1.0);
    near(Material::Saticon, 595.0, 0.40);
    near(Material::Saticon, 700.0, 0.03);
    // Chalnicon: flat to rising, 0.30 to 0.47 uA/uW, up to about 710 nm, near zero by 770 nm
    peak_in(Material::Chalnicon, 710.0, 710.0);
    near(Material::Chalnicon, 400.0, 0.30 / 0.47);
    assert!(Material::Chalnicon.response(770.0) <= 0.05);
    let mut last = 0.0;
    for i in 0..N_LAMBDA {
        let l = wavelength_nm(i);
        if (400.0..=710.0).contains(&l) {
            let r = Material::Chalnicon.response(l);
            assert!(r >= last - EXACT, "Chalnicon falls at {l} nm");
            last = r;
        }
    }

    assert_eq!(
        materials(TubeKind::Plumbicon),
        [Material::PboExtendedRed, Material::Pbo, Material::Pbo]
    );
    for kind in [
        TubeKind::PlumbiconBias,
        TubeKind::PlumbiconAct,
        TubeKind::PlumbiconDbc,
    ] {
        assert_eq!(materials(kind), materials(TubeKind::Plumbicon));
    }
    assert_eq!(materials(TubeKind::Vidicon), [Material::Sb2s3; 3]);
    assert_eq!(materials(TubeKind::Newvicon), [Material::Newvicon; 3]);
    assert_eq!(materials(TubeKind::Saticon), [Material::Saticon; 3]);
    assert_eq!(materials(TubeKind::Chalnicon), [Material::Chalnicon; 3]);
    assert!(TubeKind::Off.materials().is_none());

    // each arm of the prism takes its own band: red reads nothing at 400 nm, blue nothing at 700 nm
    for m in MATERIALS {
        assert!(
            taking(R, m, 400.0) <= 1e-6 && taking(B, m, 700.0) <= 1e-6,
            "{m:?}"
        );
    }
}

// ------------------------------------------------------------------ white balance

/// The operating point is set on white: every material's taking rows add to 1, so white gives every
/// tube its peak current and any grey reads as with ideal taking; the matrix's rows add to 1 as well
/// and it inverts the taking.
#[test]
fn white_balance_holds_for_every_material() {
    let grey = level_for(BAR_LIGHT);
    let l_grey = oetf_inverse(grey as f64 / 255.0);
    for kind in KINDS {
        let t = taking_matrix(materials(kind));
        let m = colour_matrix(materials(kind));
        for ch in [R, G, B] {
            let row: f64 = t[ch].iter().sum();
            assert!(
                (row - 1.0).abs() <= EXACT,
                "{kind:?} taking row {ch}: {row}"
            );
            assert!(t[ch].iter().all(|v| *v >= 0.0), "{kind:?}: {t:?}");
            let row: f64 = m[ch].iter().sum();
            assert!(
                (row - 1.0).abs() <= EXACT,
                "{kind:?} matrix row {ch}: {row}"
            );
        }
        let id = mul(&m, &t);
        for i in 0..3 {
            for j in 0..3 {
                let want = if i == j { 1.0 } else { 0.0 };
                assert!((id[i][j] - want).abs() <= EXACT, "{kind:?}: M T = {id:?}");
            }
        }

        let cam = camera(kind, Processing::Studio, true);
        let chs = *cam.channels();
        for (v, l) in [(255u8, 1.0), (grey, l_grey)] {
            let p = cam.photo_currents(&flat_frame([v; 3]));
            let at = (ACTIVE_H / 2 * ACTIVE_W + ACTIVE_W / 2) * 3;
            for ch in [R, G, B] {
                let want = photo_current(&chs[ch], l);
                assert!(
                    (p[at + ch] / want - 1.0).abs() <= EXACT,
                    "{kind:?} ch{ch} at light {l}: {} nA against {want}",
                    p[at + ch]
                );
            }
        }

        // the processor with its matrix reads a grey as each channel's own transfer leaves it
        let black: [f64; 3] = std::array::from_fn(|k| chs[k].dark_na + chs[k].bias_na);
        let proc = VideoProcessor::new(Processing::Studio, &chs, black).with_matrix(m);
        let sig = signal(|k, _, _| black[k] + photo_current(&chs[k], l_grey));
        let out = proc.process(&sig);
        for ch in [R, G, B] {
            // the tube's transfer as the processor leaves it: undone where the camera linearises
            let read = photo_current(&chs[ch], l_grey) / chs[ch].peak_signal_na;
            let lin = if chs[ch].linearise {
                read.powf(1.0 / chs[ch].gamma)
            } else {
                read
            };
            let want = gamma_correct(lin);
            let v = out.rows[ACTIVE_H / 2][ch][ACTIVE_W / 2] as f64;
            assert!(
                (v - want).abs() <= PROCESSOR_TOLERANCE,
                "{kind:?} ch{ch}: grey reads {v:.4} against {want:.4}"
            );
        }
    }
}

// ------------------------------------------------------------------ the matrix

/// The Plumbicon camera's matrix is the BBC's for Plumbicon cameras, term by term. On the standard
/// layer, dark by 650 nm, the red arm would need a red row far from it: hence the red-extended layer.
#[test]
fn the_plumbicon_matrix_is_the_bbc_matrix() {
    let m = colour_matrix(materials(TubeKind::Plumbicon));
    for i in 0..3 {
        for j in 0..3 {
            assert!(
                (m[i][j] - BBC_PLUMBICON[i][j]).abs() <= BBC_TOLERANCE,
                "term ({i}, {j}): {:.3} against the BBC's {}",
                m[i][j],
                BBC_PLUMBICON[i][j]
            );
        }
    }
    let standard = colour_matrix([Material::Pbo; 3]);
    assert!(
        standard[R][G] <= STANDARD_RED_ROW_GREEN,
        "standard red row {:?}",
        standard[R]
    );
}

/// No material's taking is its primaries': every matrix has off-diagonal terms. And each material needs
/// its own: behind the lead-oxide camera's matrix, a Newvicon's red (response rising to 735 nm) comes out
/// over-saturated and a Saticon's (3 % by 700 nm) under-corrected.
#[test]
fn every_material_needs_a_matrix_of_its_own() {
    for kind in KINDS {
        let m = colour_matrix(materials(kind));
        let off = max_off_diagonal(&m);
        assert!(
            off >= OFF_DIAGONAL_MIN,
            "{kind:?}: largest off-diagonal {off:.3}"
        );
    }
    let pbo = colour_matrix(materials(TubeKind::Plumbicon));
    // The lead-oxide matrix applied to another material's taking: column R is the red primary as the
    // three channels read it, and row R's green term is how the red channel reads a saturated green.
    let mt = |kind: TubeKind| mul(&pbo, &taking_matrix(materials(kind)));
    let newvicon = mt(TubeKind::Newvicon);
    assert!(
        newvicon[R][R] >= 1.0 + NEWVICON_OVER && newvicon[R][G] <= -NEWVICON_OVER,
        "Newvicon through the lead-oxide matrix: red primary on red {:.3}, green primary read in red {:.3}",
        newvicon[R][R],
        newvicon[R][G]
    );
    let saticon = mt(TubeKind::Saticon);
    assert!(
        saticon[R][R] <= 1.0 - SATICON_UNDER && saticon[R][G] >= SATICON_UNDER,
        "Saticon through the lead-oxide matrix: red primary on red {:.3}, green primary read in red {:.3}",
        saticon[R][R],
        saticon[R][G]
    );
}

// ------------------------------------------------------------------ without the matrix

/// Positive, overlapping taking curves read a saturated primary partly in the other channels: it loses
/// saturation and turns toward its spectral neighbour (red and green toward yellow, blue toward cyan).
/// A Plumbicon camera's readings are those the BBC matrix undoes. Of the materials, the Newvicon keeps
/// red the most saturated and the Saticon the least.
#[test]
fn without_the_matrix_saturated_colours_pale_toward_their_neighbours() {
    for kind in KINDS {
        let t = taking_matrix(materials(kind));
        for c in [R, G, B] {
            assert!(
                t[c][c] <= 1.0 - SATURATION_LOSS_MIN,
                "{kind:?} primary {c} reads {:.3} on its own channel",
                t[c][c]
            );
        }
        assert!(
            t[G][R] > t[B][R],
            "{kind:?}: red reads more green than blue {t:.3?}"
        );
        assert!(
            t[R][G] > t[B][G],
            "{kind:?}: green reads more red than blue {t:.3?}"
        );
        assert!(
            t[G][B] > t[R][B],
            "{kind:?}: blue reads more green than red {t:.3?}"
        );
    }
    let t = taking_matrix(materials(TubeKind::Plumbicon));
    let bbc = inverse(&BBC_PLUMBICON);
    for i in 0..3 {
        for j in 0..3 {
            assert!(
                (t[i][j] - bbc[i][j]).abs() <= BBC_TAKING_TOLERANCE,
                "Plumbicon taking ({i}, {j}): {:.3} against {:.3} from the BBC matrix",
                t[i][j],
                bbc[i][j]
            );
        }
    }
    let red: Vec<(TubeKind, f64)> = KINDS
        .iter()
        .map(|k| (*k, taking_matrix(materials(*k))[R][R]))
        .collect();
    let newvicon = red
        .iter()
        .find(|(k, _)| *k == TubeKind::Newvicon)
        .unwrap()
        .1;
    let saticon = red.iter().find(|(k, _)| *k == TubeKind::Saticon).unwrap().1;
    for (k, v) in &red {
        assert!(
            newvicon >= v - RANK_TOLERANCE,
            "red on red: Newvicon {newvicon:.3}, {k:?} {v:.3}"
        );
        assert!(
            saticon <= *v,
            "red on red: Saticon {saticon:.3}, {k:?} {v:.3}"
        );
    }
}

// ------------------------------------------------------------------ the camera

/// A frame of signal whose sample `(row, x)` of channel `k` is `f(k, row, x)` nA.
fn signal(f: impl Fn(usize, usize, usize) -> f64) -> TargetSignal {
    TargetSignal {
        rows: (0..ACTIVE_H)
            .map(|r| std::array::from_fn(|k| (0..ACTIVE_W).map(|x| f(k, r, x) as f32).collect()))
            .collect(),
    }
}

/// The spectral camera's targets receive the taking's mixture of the primaries, through each tube's
/// transfer: reading the bars equals the ideal camera fed those currents.
#[test]
fn the_spectral_camera_reads_what_its_targets_receive() {
    let frame = bars(level_for(BAR_LIGHT));
    let mut spectral = camera(TubeKind::Plumbicon, Processing::Studio, true);
    let mut ideal = camera(TubeKind::Plumbicon, Processing::Studio, false);
    let chs = *ideal.channels();
    let t = taking_matrix(materials(TubeKind::Plumbicon));
    assert_eq!(spectral.taking(), t);
    let lin: Vec<f64> = frame
        .rgb
        .iter()
        .map(|v| oetf_inverse(*v as f64 / 255.0))
        .collect();
    let photo = |ch: usize, r: usize, x: usize, _: f64| {
        let p = (r * ACTIVE_W + x) * 3;
        let l: f64 = (0..3).map(|c| t[ch][c] * lin[p + c]).sum();
        photo_current(&chs[ch], l)
    };
    // the second frame is compared: by then each line has integrated the bars only
    spectral.read(&frame, &frame);
    ideal.read_currents(&photo);
    let got = spectral.read(&frame, &frame);
    let want = ideal.read_currents(&photo);
    for (r, (a, b)) in got.rows.iter().zip(&want.rows).enumerate() {
        for ch in [R, G, B] {
            for x in 0..ACTIVE_W {
                let (a, b) = (a[ch][x] as f64, b[ch][x] as f64);
                assert!(
                    (a - b).abs() <= SAME_READING * b.abs().max(1.0),
                    "row {r} ch{ch} x {x}: {a} against {b}"
                );
            }
        }
    }
}

/// The matrix comes after the clamp, the gain and the vidicon's linearising, before the gamma corrector:
/// a vidicon camera reading a saturated red restores it exactly with the studio profile, and the
/// consumer profile, which has none, leaves the taking's mixture.
#[test]
fn the_matrix_acts_on_linear_light_before_the_gamma_corrector() {
    let chs: [TubeChannel; 3] = TubeKind::Vidicon.channels(BEAM_RATIO_DEFAULT).unwrap();
    let black: [f64; 3] = std::array::from_fn(|k| chs[k].dark_na + chs[k].bias_na);
    let t = taking_matrix(materials(TubeKind::Vidicon));
    let m = colour_matrix(materials(TubeKind::Vidicon));
    let red = signal(|k, _, _| black[k] + photo_current(&chs[k], t[k][R] * BAR_LIGHT));
    let at = |v: &vhsphys_core::camera_processing::CameraVideo, ch: usize| {
        v.rows[ACTIVE_H / 2][ch][ACTIVE_W / 2] as f64
    };

    let plain = VideoProcessor::new(Processing::Studio, &chs, black);
    assert_eq!(
        plain.matrix(),
        [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]
    );
    let studio = plain.clone().with_matrix(m);
    assert_eq!(studio.matrix(), m);
    let out = studio.process(&red);
    let want = [gamma_correct(BAR_LIGHT), 0.0, 0.0];
    for ch in [R, G, B] {
        assert!(
            (at(&out, ch) - want[ch]).abs() <= PROCESSOR_TOLERANCE,
            "studio ch{ch}: {:.4} against {:.4}",
            at(&out, ch),
            want[ch]
        );
    }
    let consumer = VideoProcessor::new(Processing::Consumer, &chs, black);
    let out = consumer.process(&red);
    for ch in [R, G, B] {
        let want = gamma_correct(t[ch][R] * BAR_LIGHT);
        assert!(
            (at(&out, ch) - want).abs() <= PROCESSOR_TOLERANCE,
            "consumer ch{ch}: {:.4} against {want:.4}",
            at(&out, ch)
        );
    }

    // the cameras carry the matrix their profile has, only with spectral taking
    assert!(Processing::Studio.spec().matrix && !Processing::Consumer.spec().matrix);
    let identity = [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]];
    assert_eq!(
        camera(TubeKind::Vidicon, Processing::Studio, true).colour_matrix(),
        m
    );
    assert_eq!(
        camera(TubeKind::Vidicon, Processing::Consumer, true).colour_matrix(),
        identity
    );
    assert_eq!(
        camera(TubeKind::Vidicon, Processing::Studio, false).colour_matrix(),
        identity
    );
    assert_eq!(
        camera(TubeKind::Vidicon, Processing::Studio, false).taking(),
        identity
    );
    assert!(!CameraParams::default().spectral);
    let p: CameraParams = toml::from_str("tube = \"saticon\"\nspectral = true").unwrap();
    assert!(p.spectral);
}

/// The whole camera on colour bars: the studio camera with spectral taking restores the colours the
/// ideal camera reads, and the consumer camera, without a matrix, reads the taking's mixture (green read
/// in red's channel on the green bar, and so on).
#[test]
fn a_studio_camera_restores_saturated_colours_and_a_consumer_camera_does_not() {
    let level = level_for(BAR_LIGHT);
    let l = oetf_inverse(level as f64 / 255.0);
    let frame = bars(level);
    let run = |processing: Processing, spectral: bool| {
        let mut cam = camera(TubeKind::Plumbicon, processing, spectral);
        let mut v = cam.video(&frame, &frame);
        for _ in 1..BAR_FRAMES {
            v = cam.video(&frame, &frame);
        }
        (v.rows, *cam.channels())
    };
    let (ideal, chs) = run(Processing::Studio, false);
    let (studio, _) = run(Processing::Studio, true);
    let (consumer, _) = run(Processing::Consumer, true);
    let t = taking_matrix(materials(TubeKind::Plumbicon));
    let m = colour_matrix(materials(TubeKind::Plumbicon));
    let read = |ch: usize, light: f64| photo_current(&chs[ch], light) / chs[ch].peak_signal_na;
    for i in 0..4 {
        let cols = bar_columns(i);
        let lights = bar_light(i, l);
        let target: [f64; 3] =
            std::array::from_fn(|ch| read(ch, (0..3).map(|c| t[ch][c] * lights[c]).sum()));
        for ch in [R, G, B] {
            let s = mean_video(&studio, ch, cols);
            let want: f64 = (0..3).map(|j| m[ch][j] * target[j]).sum();
            let want = gamma_correct(want);
            assert!(
                (s - want).abs() <= VIDEO_TOLERANCE,
                "studio, bar {i} ch{ch}: {s:.4} against {want:.4}"
            );
            let ideal_v = mean_video(&ideal, ch, cols);
            assert!(
                (gamma_inverse(s) - gamma_inverse(ideal_v)).abs() <= RESTORED_TOLERANCE,
                "studio, bar {i} ch{ch}: {s:.4} against the ideal camera's {ideal_v:.4}"
            );
            let c = mean_video(&consumer, ch, cols);
            let want = gamma_correct(target[ch]);
            assert!(
                (c - want).abs() <= VIDEO_TOLERANCE,
                "consumer, bar {i} ch{ch}: {c:.4} against {want:.4}"
            );
        }
    }
}
