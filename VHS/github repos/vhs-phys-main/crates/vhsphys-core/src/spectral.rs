//! Spectral taking and the colour matrix: BT.709 scene primary spectra, CIE 1931 observer fit,
//! dichroic prism, IR-cut filter, photoconductor spectral responses, and the inverse colour matrix.
//!
//! Sources: `docs/design/CAMERA_SCAN.md` ("Spectral taking and the colour matrix"),
//! `docs/CAMERA_RESEARCH.md` ("Six photoconductors differ in gamma, dark current and lag mechanism"
//! and "Head-amplifier noise and processing lie mostly above 2.5 MHz"), ITU-R BT.709, Wyman, Sloan
//! and Shirley 2013 (JCGT 2(2), multi-lobe CIE 1931 2-degree fit), and Jones 1968 (BBC Plumbicon
//! matrix).

/// Number of wavelength samples on the 380-780 nm grid (`docs/design/CAMERA_SCAN.md`).
pub const N_LAMBDA: usize = 81;
/// Shortest wavelength of the spectral grid, nm (`docs/design/CAMERA_SCAN.md`).
pub const LAMBDA_MIN_NM: f64 = 380.0;
/// Spacing of the spectral grid, nm (`docs/design/CAMERA_SCAN.md`).
pub const LAMBDA_STEP_NM: f64 = 5.0;

/// ITU-R BT.709 primary luminance shares of D65 white `[R, G, B]` [DS].
pub const BT709_LUMINANCE: [f64; 3] = [0.2126, 0.7152, 0.0722];

/// CIE 1931 2-degree multi-lobe analytic fit parameters `(weight, mu_nm, sigma_lo_nm, sigma_hi_nm)`
/// from Wyman, Sloan and Shirley 2013 (JCGT 2(2)) [DS].
const CIE_X_LOBES: [(f64, f64, f64, f64); 3] = [
    (1.056, 599.8, 37.9, 31.0),
    (0.362, 442.0, 16.0, 26.7),
    (-0.065, 501.1, 20.4, 26.2),
];
const CIE_Y_LOBES: [(f64, f64, f64, f64); 2] = [
    (0.821, 568.8, 46.9, 40.5),
    (0.286, 530.9, 16.3, 31.1),
];
const CIE_Z_LOBES: [(f64, f64, f64, f64); 2] = [
    (1.217, 437.0, 11.8, 36.0),
    (0.681, 459.0, 26.0, 13.8),
];

/// Scene primary spectral shape parameters fitted to the BT.709 primaries' CIE (x, y) [DER].
const SCENE_RED_FLOOR: f64 = 0.0118;
const SCENE_RED_EDGE_NM: f64 = 619.0;
const SCENE_RED_WIDTH_NM: f64 = 15.0;
const SCENE_GREEN_PEAK_NM: f64 = 539.1;
const SCENE_GREEN_SIGMA_NM: f64 = 33.2;
const SCENE_BLUE_EDGE_NM: f64 = 474.2;
const SCENE_BLUE_WIDTH_NM: f64 = 18.8;

/// Dichroic prism edges (nm) and transition width (nm) fitted to the BBC Plumbicon matrix [est].
const PRISM_BG_EDGE_NM: f64 = 480.0;
const PRISM_GR_EDGE_NM: f64 = 580.0;
const PRISM_EDGE_WIDTH_NM: f64 = 5.0;

/// Lens IR-cut filter edge (nm) and transition width (nm) fitted to the BBC Plumbicon matrix [est].
const IR_CUT_EDGE_NM: f64 = 680.0;
const IR_CUT_WIDTH_NM: f64 = 10.0;

/// Standard PbO relative spectral response knots `(nm, response)`: 490 and 650 nm from Philips T10
/// Fig. 1 [FIG], remaining points [est].
const KNOTS_PBO: &[(f64, f64)] = &[
    (380.0, 0.55),
    (400.0, 0.70),
    (450.0, 0.92),
    (490.0, 1.00),
    (550.0, 0.72),
    (600.0, 0.35),
    (630.0, 0.10),
    (650.0, 0.00),
];

/// Sulphur-doped red-extended PbO relative spectral response knots `(nm, response)`: 780 nm tail
/// from Philips T10 Fig. 1 (tails to 850-950 nm) [FIG], remaining points [est].
const KNOTS_PBO_EXTENDED_RED: &[(f64, f64)] = &[
    (380.0, 0.55),
    (400.0, 0.70),
    (450.0, 0.92),
    (490.0, 1.00),
    (550.0, 0.80),
    (600.0, 0.55),
    (650.0, 0.38),
    (700.0, 0.25),
    (780.0, 0.10),
];

/// Sb2S3 vidicon relative spectral response knots `(nm, response)`: 550 and 675 nm from
/// `docs/CAMERA_RESEARCH.md` [FIG], remaining points [est].
const KNOTS_SB2S3: &[(f64, f64)] = &[
    (380.0, 0.40),
    (400.0, 0.55),
    (450.0, 0.80),
    (550.0, 1.00),
    (600.0, 0.88),
    (675.0, 0.50),
    (700.0, 0.36),
    (750.0, 0.12),
    (780.0, 0.05),
];

/// Newvicon (ZnSe/ZnCdTe) relative spectral response knots `(nm, response)`: 400 and 735 nm from
/// Philips T12 Fig. 1 (220 of 575 mA/W, peak 730-740 nm) [FIG], remaining points [est].
const KNOTS_NEWVICON: &[(f64, f64)] = &[
    (380.0, 0.36),
    (400.0, 0.38),
    (500.0, 0.55),
    (600.0, 0.78),
    (700.0, 0.96),
    (735.0, 1.00),
    (780.0, 0.92),
];

/// Saticon (Se-As-Te) relative spectral response knots `(nm, response)`: 430, 450, 595 and 700 nm
/// from Hitachi H8397A Fig. 3 [FIG], remaining points [est].
const KNOTS_SATICON: &[(f64, f64)] = &[
    (380.0, 0.65),
    (400.0, 0.80),
    (430.0, 1.00),
    (450.0, 1.00),
    (500.0, 0.82),
    (550.0, 0.62),
    (595.0, 0.40),
    (650.0, 0.12),
    (700.0, 0.03),
    (750.0, 0.00),
];

/// Chalnicon (CdSe) relative spectral response knots `(nm, response)`: 400, 710 and 770 nm from
/// Toshiba 1973 Fig. 6 (0.30 of 0.47 uA/uW) [FIG], remaining points [est].
const KNOTS_CHALNICON: &[(f64, f64)] = &[
    (380.0, 0.60),
    (400.0, 0.64),
    (500.0, 0.75),
    (600.0, 0.87),
    (710.0, 1.00),
    (740.0, 0.50),
    (770.0, 0.02),
    (780.0, 0.00),
];

/// `LAMBDA_MIN_NM + LAMBDA_STEP_NM * i`.
pub fn wavelength_nm(i: usize) -> f64 {
    LAMBDA_MIN_NM + LAMBDA_STEP_NM * i as f64
}

#[inline]
fn asym_lobe(nm: f64, mu: f64, sigma_lo: f64, sigma_hi: f64) -> f64 {
    let s = if nm < mu { sigma_lo } else { sigma_hi };
    (-0.5 * ((nm - mu) / s).powi(2)).exp()
}

/// CIE 1931 2 degree `[xbar, ybar, zbar]` at `nm`, Wyman, Sloan and Shirley 2013 multi-lobe fit.
pub fn cie_1931(nm: f64) -> [f64; 3] {
    let x = CIE_X_LOBES
        .iter()
        .map(|&(w, mu, s1, s2)| w * asym_lobe(nm, mu, s1, s2))
        .sum();
    let y = CIE_Y_LOBES
        .iter()
        .map(|&(w, mu, s1, s2)| w * asym_lobe(nm, mu, s1, s2))
        .sum();
    let z = CIE_Z_LOBES
        .iter()
        .map(|&(w, mu, s1, s2)| w * asym_lobe(nm, mu, s1, s2))
        .sum();
    [x, y, z]
}

/// Photoconductor spectral material.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Material {
    Pbo,
    PboExtendedRed,
    Sb2s3,
    Newvicon,
    Saticon,
    Chalnicon,
}

/// All photoconductor materials in order.
pub const MATERIALS: [Material; 6] = [
    Material::Pbo,
    Material::PboExtendedRed,
    Material::Sb2s3,
    Material::Newvicon,
    Material::Saticon,
    Material::Chalnicon,
];

impl Material {
    /// The knots of the table, `(nm, relative response)`.
    pub fn knots(self) -> &'static [(f64, f64)] {
        match self {
            Material::Pbo => KNOTS_PBO,
            Material::PboExtendedRed => KNOTS_PBO_EXTENDED_RED,
            Material::Sb2s3 => KNOTS_SB2S3,
            Material::Newvicon => KNOTS_NEWVICON,
            Material::Saticon => KNOTS_SATICON,
            Material::Chalnicon => KNOTS_CHALNICON,
        }
    }

    /// Linear between knots, 0 outside them.
    pub fn response(self, nm: f64) -> f64 {
        let knots = self.knots();
        let first = knots[0];
        let last = knots[knots.len() - 1];
        if nm < first.0 || nm > last.0 {
            return 0.0;
        }
        if nm == first.0 {
            return first.1;
        }
        if nm == last.0 {
            return last.1;
        }
        for pair in knots.windows(2) {
            let (l0, r0) = pair[0];
            let (l1, r1) = pair[1];
            if nm <= l1 {
                let t = (nm - l0) / (l1 - l0);
                return r0 + t * (r1 - r0);
            }
        }
        last.1
    }
}

#[inline]
fn long_pass(nm: f64, edge: f64, width: f64) -> f64 {
    1.0 / (1.0 + (-(nm - edge) / width).exp())
}

#[inline]
fn short_pass(nm: f64, edge: f64, width: f64) -> f64 {
    1.0 - long_pass(nm, edge, width)
}

fn primary_shape(c: usize, nm: f64) -> f64 {
    match c {
        0 => SCENE_RED_FLOOR + (1.0 - SCENE_RED_FLOOR) * long_pass(nm, SCENE_RED_EDGE_NM, SCENE_RED_WIDTH_NM),
        1 => (-0.5 * ((nm - SCENE_GREEN_PEAK_NM) / SCENE_GREEN_SIGMA_NM).powi(2)).exp(),
        2 => short_pass(nm, SCENE_BLUE_EDGE_NM, SCENE_BLUE_WIDTH_NM),
        _ => panic!("invalid primary index {c}"),
    }
}

/// `E_c(nm)` of primary `c` (`[R, G, B]`) at unit linear light.
pub fn primary_spectrum(c: usize, nm: f64) -> f64 {
    let mut y_sum = 0.0;
    for i in 0..N_LAMBDA {
        let l = wavelength_nm(i);
        y_sum += cie_1931(l)[1] * primary_shape(c, l);
    }
    BT709_LUMINANCE[c] * primary_shape(c, nm) / y_sum
}

/// Lens IR-cut filter transmission at `nm`.
pub fn ir_cut(nm: f64) -> f64 {
    short_pass(nm, IR_CUT_EDGE_NM, IR_CUT_WIDTH_NM)
}

/// Prism transmission into channel `ch`.
pub fn prism(ch: usize, nm: f64) -> f64 {
    match ch {
        0 => long_pass(nm, PRISM_GR_EDGE_NM, PRISM_EDGE_WIDTH_NM),
        1 => long_pass(nm, PRISM_BG_EDGE_NM, PRISM_EDGE_WIDTH_NM) * short_pass(nm, PRISM_GR_EDGE_NM, PRISM_EDGE_WIDTH_NM),
        2 => short_pass(nm, PRISM_BG_EDGE_NM, PRISM_EDGE_WIDTH_NM),
        _ => panic!("invalid prism channel {ch}"),
    }
}

/// `ir_cut(nm) * prism(ch, nm) * m.response(nm)`.
pub fn taking(ch: usize, m: Material, nm: f64) -> f64 {
    ir_cut(nm) * prism(ch, nm) * m.response(nm)
}

/// `T[ch][c]`, rows summing to 1.
pub fn taking_matrix(materials: [Material; 3]) -> [[f64; 3]; 3] {
    let mut out = [[0.0; 3]; 3];
    for ch in 0..3 {
        let m = materials[ch];
        let mut raw = [0.0; 3];
        for i in 0..N_LAMBDA {
            let l = wavelength_nm(i);
            let tau = taking(ch, m, l);
            for c in 0..3 {
                raw[c] += tau * primary_spectrum(c, l);
            }
        }
        let sum: f64 = raw.iter().sum();
        for c in 0..3 {
            out[ch][c] = raw[c] / sum;
        }
    }
    out
}

/// `T^-1`.
pub fn colour_matrix(materials: [Material; 3]) -> [[f64; 3]; 3] {
    let m = taking_matrix(materials);
    let det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
        - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
        + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    let cofactor = |i: usize, j: usize| {
        let (r0, r1) = ((i + 1) % 3, (i + 2) % 3);
        let (c0, c1) = ((j + 1) % 3, (j + 2) % 3);
        m[r0][c0] * m[r1][c1] - m[r0][c1] * m[r1][c0]
    };
    std::array::from_fn(|i| std::array::from_fn(|j| cofactor(j, i) / det))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn grid_and_knots_behave_as_specified() {
        assert_eq!(wavelength_nm(0), 380.0);
        assert_eq!(wavelength_nm(N_LAMBDA - 1), 780.0);
        for m in MATERIALS {
            let k = m.knots();
            assert_eq!(m.response(k[0].0 - 1.0), 0.0);
            assert_eq!(m.response(k[k.len() - 1].0 + 1.0), 0.0);
            assert!((m.response(k[0].0) - k[0].1).abs() < 1e-12);
            assert!((m.response(k[k.len() - 1].0) - k[k.len() - 1].1).abs() < 1e-12);
        }
    }

    #[test]
    fn taking_and_colour_matrix_are_inverses_with_unit_row_sums() {
        let t = taking_matrix([Material::PboExtendedRed, Material::Pbo, Material::Pbo]);
        let m = colour_matrix([Material::PboExtendedRed, Material::Pbo, Material::Pbo]);
        for i in 0..3 {
            assert!((t[i].iter().sum::<f64>() - 1.0).abs() < 1e-12);
            assert!((m[i].iter().sum::<f64>() - 1.0).abs() < 1e-12);
            for j in 0..3 {
                let prod: f64 = (0..3).map(|k| m[i][k] * t[k][j]).sum();
                let want = if i == j { 1.0 } else { 0.0 };
                assert!((prod - want).abs() < 1e-12);
            }
        }
    }
}
