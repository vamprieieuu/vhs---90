//! Camera geometry and optics: deflection distortion, three-tube registration, black and white
//! shading, lens veiling glare, faceplate halation, and corner focus.
//!
//! Sources: `docs/design/CAMERA_SCAN.md` ("Geometry"), `docs/CAMERA_RESEARCH.md` ("Interlaced
//! readout with an oversized spot", "Beam capacity of twice peak white turns highlights into comet
//! tails", "Head-amplifier noise and processing lie mostly above 2.5 MHz"), Philips T10, Philips
//! T12, Hitachi H8397A, RCA 1977, Heimann 1978, US 4032974, and US 4001502.

use crate::tube::{TubeKind, BEAM_RATIO_DEFAULT, TARGET_ASPECT};
use rayon::prelude::*;
use std::f64::consts::PI;

/// Half-width of the 4:3 picture in picture heights (PH) (`docs/design/CAMERA_SCAN.md`).
pub const X_EDGE_PH: f64 = TARGET_ASPECT / 2.0;
/// Corner distance from the picture centre `sqrt((2/3)^2 + (1/2)^2)` in PH (`docs/design/CAMERA_SCAN.md`).
pub const RHO_CORNER_PH: f64 = 5.0 / 6.0;
/// Picture height per nanosecond of line time (0.00256 % PH/ns; Philips T10, `docs/CAMERA_RESEARCH.md`) [DS].
pub const PH_PER_NS: f64 = 2.56e-5;
/// Black-square flare test target side length, PH (1/10 picture height; Hitachi H8397A Table 1, Philips T10) [DS].
pub const BLACK_SQUARE_PH: f64 = 0.1;
/// Refractive index of the pickup tube faceplate glass (`docs/design/CAMERA_SCAN.md`) [est].
pub const GLASS_INDEX: f64 = 1.5;
/// Anti-halation disc thickness on 30 mm PbO tubes, mm (Philips T10, `docs/CAMERA_RESEARCH.md`) [DS].
pub const ANTI_HALATION_DISC_MM: f64 = 6.0;

/// Studio deflection linearity and pincushion: at most 0.48 % inside diameter H and 0.90 % outside
/// (Philips deflection units: 0.5 % and 1.0 % bounds, Philips T10) [DS bounds].
const STUDIO_LINEARITY: f64 = -0.012;
const STUDIO_PINCUSHION: f64 = 0.0155;
/// Consumer deflection linearity and pincushion: 1.6 % inside diameter H and 1.8 % outside
/// (vidicon coil units: 2.0 % bound, Philips T12) [DS bound].
const CONSUMER_LINEARITY: f64 = -0.045;
const CONSUMER_PINCUSHION: f64 = 0.031;

/// Studio registration residuals (red; blue negated): up to 33, 60 and 101 ns in zones A, B and C
/// (matched triplets: 25-40, 40-80, 80-120 ns, Philips T10) [DS bands].
const STUDIO_REG_SIZE: f64 = 8.7e-4;
const STUDIO_REG_PINCUSHION: f64 = 3.2e-3;
/// Consumer registration residuals (red; blue negated): 40, 72 and 120 ns at the zone bands' upper
/// ends (`docs/design/CAMERA_SCAN.md`) [est].
const CONSUMER_REG_SIZE: f64 = 1.12e-3;
const CONSUMER_REG_PINCUSHION: f64 = 3.69e-3;

/// Black shading components: +12.5 % at bottom-right, +7.5 % at top-left, -5 % at centre, mean 0
/// (up to 12.5 % at corners, parabolic and sawtooth in both directions, Philips T10; split [est]).
const BLACK_SHADING_H_SAW: f64 = 0.0125;
const BLACK_SHADING_V_SAW: f64 = 0.0125;
const BLACK_SHADING_H_PAR: f64 = 0.075;
const BLACK_SHADING_V_PAR: f64 = 0.075;

/// White shading corner fall in PbO red and green ("a few per cent", blue flat, Philips T10) [est].
const PBO_WHITE_FALL: f64 = 0.03;
/// White shading corner fall on Saticon (up to 15 %, RCA 1977) [DS].
const SATICON_WHITE_FALL: f64 = 0.15;

/// Saticon black-square veil (1.00 % centre, 0.50 % corner, Hitachi H8397A Table 1) [DER on DS];
/// also used for PbO, Newvicon and Chalnicon where no separate veil data exist [est].
const SATICON_VEIL_FRACTION: f64 = 0.0215;
const SATICON_VEIL_SCALE_PH: f64 = 0.40;
/// Vidicon black-square veil (5.0 % centre through its 0.65 gamma, Philips T10 / H8397A) [DER on DS].
const VIDICON_VEIL_FRACTION: f64 = 0.0116;
const VIDICON_VEIL_SCALE_PH: f64 = 0.15;

/// PbO target diffuse reflectance `[R, G, B]` ("highest in PbO, especially in red",
/// `docs/CAMERA_RESEARCH.md`) [est].
const PBO_HALATION_REFLECTANCE: [f64; 3] = [0.15, 0.08, 0.05];
/// PbO faceplate thickness, mm (`docs/design/CAMERA_SCAN.md`) [est].
const PBO_FACEPLATE_MM: f64 = 1.5;
/// 30 mm PbO scanned target height, mm (Philips T10, `docs/CAMERA_RESEARCH.md`) [DS].
const PBO_TARGET_HEIGHT_MM: f64 = 12.8;

/// Corner focus parameter for vidicon and Newvicon (31 % loss: XQ1274 650 to 450 TVL, within
/// 25-35 %, Philips T12) [DS].
const VIDICON_CORNER_FOCUS: f64 = 0.45;
/// Corner focus parameter for Chalnicon (20 % loss, Philips T12 / Heimann 1978) [DS].
const CHALNICON_CORNER_FOCUS: f64 = 0.25;
/// Corner focus parameter for PbO and Saticon (magnetic-focus studio tubes) [est].
const STUDIO_CORNER_FOCUS: f64 = 0.25;

/// Mean Euclidean distance from the origin over a unit square `[-1/2, 1/2]^2`:
/// `(sqrt(2) + ln(1 + sqrt(2))) / 6`, used for the zero-offset cell average of `veil_kernel`.
const UNIT_SQUARE_MEAN_R: f64 = 0.382_597_858_232_106_3;

/// Deflection distortion shared by the camera's tubes.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Deflection {
    pub linearity: f64,
    pub pincushion: f64,
}

impl Deflection {
    pub const NONE: Deflection = Deflection { linearity: 0.0, pincushion: 0.0 };
}

/// A tube's size and pincushion against green.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Registration {
    pub size: f64,
    pub pincushion: f64,
}

impl Registration {
    pub const NONE: Registration = Registration { size: 0.0, pincushion: 0.0 };
}

/// Target dark-current and bias-light shading over the active picture.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Shading {
    pub h_saw: f64,
    pub h_par: f64,
    pub v_saw: f64,
    pub v_par: f64,
}

impl Shading {
    pub const NONE: Shading = Shading { h_saw: 0.0, h_par: 0.0, v_saw: 0.0, v_par: 0.0 };
}

/// Shift-invariant exponential veiling glare of the lens and target-lens reflections.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Veil {
    pub fraction: f64,
    pub scale_ph: f64,
}

impl Veil {
    pub const NONE: Veil = Veil { fraction: 0.0, scale_ph: 0.0 };
}

/// Faceplate total-internal-reflection halation of a pickup target.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Halation {
    pub reflectance: f64,
    pub faceplate_mm: f64,
    pub disc_mm: f64,
    pub target_height_mm: f64,
}

impl Halation {
    pub const NONE: Halation =
        Halation { reflectance: 0.0, faceplate_mm: 0.0, disc_mm: 0.0, target_height_mm: 0.0 };
}

/// Full geometry, shading, flare and focus specification of a camera.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct GeometrySpec {
    pub deflection: Deflection,
    pub registration: [Registration; 3],
    pub black_shading: Shading,
    /// Dark plus bias current at 25 deg C per channel, nA: the black corrector's reference.
    pub black_reference_na: [f64; 3],
    /// White shading: the fall of sensitivity at the corner per channel.
    pub white_fall: [f64; 3],
    pub veil: [Veil; 3],
    pub halation: [Halation; 3],
    pub corner_focus: [f64; 3],
}

impl GeometrySpec {
    pub const NONE: GeometrySpec = GeometrySpec {
        deflection: Deflection::NONE,
        registration: [Registration::NONE; 3],
        black_shading: Shading::NONE,
        black_reference_na: [0.0; 3],
        white_fall: [0.0; 3],
        veil: [Veil::NONE; 3],
        halation: [Halation::NONE; 3],
        corner_focus: [0.0; 3],
    };
}

/// `(dx, dy)` of tube `ch`, PH, at nominal `(x, y)`.
pub fn displacement_ph(g: &GeometrySpec, ch: usize, x: f64, y: f64) -> (f64, f64) {
    let rho2 = x * x + y * y;
    let (k, p) = (g.deflection.linearity, g.deflection.pincushion);
    let (s, q) = (g.registration[ch].size, g.registration[ch].pincushion);
    let dx = k * (x * x - X_EDGE_PH * X_EDGE_PH) / (2.0 * X_EDGE_PH) + (p + q) * x * rho2 + s * x;
    let dy = (p + q) * y * rho2 + s * y;
    (dx, dy)
}

/// Horizontal derivative `d(dx)/dx` of tube `ch` at nominal `(x, y)`.
#[inline]
pub(crate) fn displacement_deriv_x(g: &GeometrySpec, ch: usize, x: f64, y: f64) -> f64 {
    let (k, p) = (g.deflection.linearity, g.deflection.pincushion);
    let (s, q) = (g.registration[ch].size, g.registration[ch].pincushion);
    k * x / X_EDGE_PH + (p + q) * (3.0 * x * x + y * y) + s
}

/// `J_ch` at nominal `(x, y)`.
pub fn scan_gain(g: &GeometrySpec, ch: usize, x: f64, y: f64) -> f64 {
    let (p, q, s) = (g.deflection.pincushion, g.registration[ch].pincushion, g.registration[ch].size);
    let d_dx = displacement_deriv_x(g, ch, x, y);
    let d_dy = (p + q) * (x * x + 3.0 * y * y) + s;
    (1.0 + d_dx) * (1.0 + d_dy)
}

/// `b(u, v)` at `(x, y)`.
pub fn black_shading_factor(s: &Shading, x: f64, y: f64) -> f64 {
    let u = x / X_EDGE_PH;
    let v = 2.0 * y;
    s.h_saw * u + s.h_par * (u * u - 1.0 / 3.0) + s.v_saw * v + s.v_par * (v * v - 1.0 / 3.0)
}

/// `1 + w` for a corner fall `fall`.
pub fn white_shading_factor(fall: f64, x: f64, y: f64) -> f64 {
    let rho2 = x * x + y * y;
    1.0 - fall * (rho2 / (RHO_CORNER_PH * RHO_CORNER_PH))
}

/// `F` for `focus`.
pub fn focus_factor(focus: f64, x: f64, y: f64) -> f64 {
    let rho2 = x * x + y * y;
    1.0 + focus * (rho2 / (RHO_CORNER_PH * RHO_CORNER_PH))
}

/// `K(d)`, per PH^2.
pub fn veil_kernel(v: &Veil, d_ph: f64) -> f64 {
    if v.scale_ph <= 0.0 || v.fraction <= 0.0 {
        return 0.0;
    }
    (-d_ph / v.scale_ph).exp() / (2.0 * PI * v.scale_ph * v.scale_ph)
}

/// `h` of the halation ring: diffuse reflectance outside the total-internal-reflection critical angle.
pub fn halation_share(h: &Halation) -> f64 {
    h.reflectance * (1.0 - 1.0 / (GLASS_INDEX * GLASS_INDEX))
}

/// Inner radius of the halation ring, PH.
pub fn halation_radius_ph(h: &Halation) -> f64 {
    if h.target_height_mm <= 0.0 {
        return 0.0;
    }
    let t = (h.faceplate_mm + h.disc_mm) / h.target_height_mm;
    let tan_c = (1.0 / GLASS_INDEX).asin().tan();
    2.0 * t * tan_c
}

/// `K_h(d)`, per PH^2 (unit integral over the plane).
pub fn halation_kernel(h: &Halation, d_ph: f64) -> f64 {
    if h.target_height_mm <= 0.0 {
        return 0.0;
    }
    let t = (h.faceplate_mm + h.disc_mm) / h.target_height_mm;
    if t <= 0.0 {
        return 0.0;
    }
    let tan_c = (1.0 / GLASS_INDEX).asin().tan();
    let two_t = 2.0 * t;
    if d_ph < two_t * tan_c {
        return 0.0;
    }
    let ratio = d_ph / two_t;
    let denom = 1.0 + ratio * ratio;
    (1.0 + tan_c * tan_c) / (PI * two_t * two_t * denom * denom)
}

/// Build the profile's `GeometrySpec` for `kind` (`GeometrySpec::NONE` for `Off`).
pub(crate) fn profile_geometry(studio: bool, kind: TubeKind) -> GeometrySpec {
    let Some(chs) = kind.channels(BEAM_RATIO_DEFAULT) else {
        return GeometrySpec::NONE;
    };
    let deflection = if studio {
        Deflection { linearity: STUDIO_LINEARITY, pincushion: STUDIO_PINCUSHION }
    } else {
        Deflection { linearity: CONSUMER_LINEARITY, pincushion: CONSUMER_PINCUSHION }
    };
    let (reg_s, reg_q) = if studio {
        (STUDIO_REG_SIZE, STUDIO_REG_PINCUSHION)
    } else {
        (CONSUMER_REG_SIZE, CONSUMER_REG_PINCUSHION)
    };
    let registration = [
        Registration { size: reg_s, pincushion: reg_q },
        Registration::NONE,
        Registration { size: -reg_s, pincushion: -reg_q },
    ];
    let black_shading = Shading {
        h_saw: BLACK_SHADING_H_SAW,
        h_par: BLACK_SHADING_H_PAR,
        v_saw: BLACK_SHADING_V_SAW,
        v_par: BLACK_SHADING_V_PAR,
    };
    let black_reference_na = std::array::from_fn(|k| chs[k].dark_na + chs[k].bias_na);
    let is_pbo = matches!(
        kind,
        TubeKind::Plumbicon
            | TubeKind::PlumbiconBias
            | TubeKind::PlumbiconAct
            | TubeKind::PlumbiconDbc
    );
    let white_fall = if is_pbo {
        [PBO_WHITE_FALL, PBO_WHITE_FALL, 0.0]
    } else if kind == TubeKind::Saticon {
        [SATICON_WHITE_FALL; 3]
    } else {
        [0.0; 3]
    };
    let veil = if kind == TubeKind::Vidicon {
        [Veil { fraction: VIDICON_VEIL_FRACTION, scale_ph: VIDICON_VEIL_SCALE_PH }; 3]
    } else {
        [Veil { fraction: SATICON_VEIL_FRACTION, scale_ph: SATICON_VEIL_SCALE_PH }; 3]
    };
    let halation = if is_pbo {
        PBO_HALATION_REFLECTANCE.map(|reflectance| Halation {
            reflectance,
            faceplate_mm: PBO_FACEPLATE_MM,
            disc_mm: ANTI_HALATION_DISC_MM,
            target_height_mm: PBO_TARGET_HEIGHT_MM,
        })
    } else {
        [Halation::NONE; 3]
    };
    let focus = match kind {
        TubeKind::Vidicon | TubeKind::Newvicon => VIDICON_CORNER_FOCUS,
        TubeKind::Chalnicon => CHALNICON_CORNER_FOCUS,
        _ => STUDIO_CORNER_FOCUS,
    };
    GeometrySpec {
        deflection,
        registration,
        black_shading,
        black_reference_na,
        white_fall,
        veil,
        halation,
        corner_focus: [focus; 3],
    }
}

// ------------------------------------------------------------------ 2D FFT optics convolution

/// Coarse-grid dimensions covering `[-X_EDGE_PH, X_EDGE_PH] x [-0.5, 0.5]` with ~0.0078 PH
/// square cells, zero-padded to `(NX, NY) = (512, 256)` for exact linear convolution.
const OPTICS_CW: usize = 171;
const OPTICS_CH: usize = 128;
const OPTICS_NX: usize = 512;
const OPTICS_NY: usize = 256;

/// Precomputed kernel FFTs for veil and halation across the three channels.
#[derive(Clone)]
pub(crate) struct OpticsPlan {
    veil_fft: [Option<Vec<(f64, f64)>>; 3],
    veil_frac: [f64; 3],
    halation_fft: [Option<Vec<(f64, f64)>>; 3],
    halation_share: [f64; 3],
}

impl OpticsPlan {
    /// Build an `OpticsPlan` for `g`, or `None` when neither veil nor halation is active.
    pub(crate) fn new(g: &GeometrySpec) -> Option<Self> {
        let mut has_any = false;
        let mut veil_fft: [Option<Vec<(f64, f64)>>; 3] = [None, None, None];
        let mut veil_frac = [0.0; 3];
        let mut halation_fft: [Option<Vec<(f64, f64)>>; 3] = [None, None, None];
        let mut hal_share = [0.0; 3];

        for ch in 0..3 {
            let v = g.veil[ch];
            if v.fraction > 0.0 && v.scale_ph > 0.0 {
                has_any = true;
                veil_frac[ch] = v.fraction;
                let dup = (0..ch).find(|&prev| g.veil[prev] == v);
                veil_fft[ch] = Some(match dup {
                    Some(prev) => veil_fft[prev].as_ref().unwrap().clone(),
                    None => build_kernel_fft(|d| veil_kernel(&v, d)),
                });
            }
            let h = g.halation[ch];
            let share = halation_share(&h);
            if share > 0.0 && h.target_height_mm > 0.0 {
                has_any = true;
                hal_share[ch] = share;
                let dup = (0..ch).find(|&prev| {
                    let ph = g.halation[prev];
                    ph.faceplate_mm == h.faceplate_mm
                        && ph.disc_mm == h.disc_mm
                        && ph.target_height_mm == h.target_height_mm
                        && halation_share(&ph) > 0.0
                });
                halation_fft[ch] = Some(match dup {
                    Some(prev) => halation_fft[prev].as_ref().unwrap().clone(),
                    None => build_kernel_fft(|d| halation_kernel(&h, d)),
                });
            }
        }

        has_any.then_some(Self { veil_fft, veil_frac, halation_fft, halation_share: hal_share })
    }

    /// Apply veil then halation in place on `channels` (row-major `h x w`, `0..3`).
    pub(crate) fn apply_three(&self, channels: &mut [Vec<f64>; 3], w: usize, h: usize) {
        self.apply_subset(channels, w, h, &[0, 1, 2]);
    }

    /// Apply veil then halation in place on a single `channel` (`w x h`).
    pub(crate) fn apply_one(&self, slot: usize, channel: &mut [f64], w: usize, h: usize) {
        if let Some(ref k_fft) = self.veil_fft[slot] {
            let coarse = splat_to_coarse(channel, w, h);
            let conv = convolve_coarse_one(&coarse, k_fft);
            mix_from_coarse(channel, &conv, 1.0 - self.veil_frac[slot], self.veil_frac[slot], w, h);
        }
        if let Some(ref k_fft) = self.halation_fft[slot] {
            let coarse = splat_to_coarse(channel, w, h);
            let conv = convolve_coarse_one(&coarse, k_fft);
            mix_from_coarse(channel, &conv, 1.0, self.halation_share[slot], w, h);
        }
    }

    /// Apply veil only to a single `channel` (`w x h`), used before stripe low-pass and filter.
    pub(crate) fn apply_veil_one(&self, slot: usize, channel: &mut [f64], w: usize, h: usize) {
        if let Some(ref k_fft) = self.veil_fft[slot] {
            let coarse = splat_to_coarse(channel, w, h);
            let conv = convolve_coarse_one(&coarse, k_fft);
            mix_from_coarse(channel, &conv, 1.0 - self.veil_frac[slot], self.veil_frac[slot], w, h);
        }
    }

    /// Apply halation only to a single `channel` (`w x h`).
    pub(crate) fn apply_halation_one(&self, slot: usize, channel: &mut [f64], w: usize, h: usize) {
        if let Some(ref k_fft) = self.halation_fft[slot] {
            let coarse = splat_to_coarse(channel, w, h);
            let conv = convolve_coarse_one(&coarse, k_fft);
            mix_from_coarse(channel, &conv, 1.0, self.halation_share[slot], w, h);
        }
    }

    fn apply_subset(&self, channels: &mut [Vec<f64>; 3], w: usize, h: usize, idxs: &[usize]) {
        // Group channels that share the same kernel FFT so two real convolutions run in one complex FFT.
        let mut veil_done = [false; 3];
        for &i in idxs {
            if veil_done[i] {
                continue;
            }
            let Some(ref k_fft) = self.veil_fft[i] else {
                veil_done[i] = true;
                continue;
            };
            let partner = idxs
                .iter()
                .copied()
                .find(|&j| j > i && !veil_done[j] && self.veil_fft[j].as_ref() == Some(k_fft));
            if let Some(j) = partner {
                let ca = splat_to_coarse(&channels[i], w, h);
                let cb = splat_to_coarse(&channels[j], w, h);
                let (ra, rb) = convolve_coarse_two(&ca, &cb, k_fft);
                mix_from_coarse(
                    &mut channels[i],
                    &ra,
                    1.0 - self.veil_frac[i],
                    self.veil_frac[i],
                    w,
                    h,
                );
                mix_from_coarse(
                    &mut channels[j],
                    &rb,
                    1.0 - self.veil_frac[j],
                    self.veil_frac[j],
                    w,
                    h,
                );
                veil_done[i] = true;
                veil_done[j] = true;
            } else {
                let ca = splat_to_coarse(&channels[i], w, h);
                let ra = convolve_coarse_one(&ca, k_fft);
                mix_from_coarse(
                    &mut channels[i],
                    &ra,
                    1.0 - self.veil_frac[i],
                    self.veil_frac[i],
                    w,
                    h,
                );
                veil_done[i] = true;
            }
        }

        let mut hal_done = [false; 3];
        for &i in idxs {
            if hal_done[i] {
                continue;
            }
            let Some(ref k_fft) = self.halation_fft[i] else {
                hal_done[i] = true;
                continue;
            };
            let partner = idxs
                .iter()
                .copied()
                .find(|&j| j > i && !hal_done[j] && self.halation_fft[j].as_ref() == Some(k_fft));
            if let Some(j) = partner {
                let ca = splat_to_coarse(&channels[i], w, h);
                let cb = splat_to_coarse(&channels[j], w, h);
                let (ra, rb) = convolve_coarse_two(&ca, &cb, k_fft);
                mix_from_coarse(&mut channels[i], &ra, 1.0, self.halation_share[i], w, h);
                mix_from_coarse(&mut channels[j], &rb, 1.0, self.halation_share[j], w, h);
                hal_done[i] = true;
                hal_done[j] = true;
            } else {
                let ca = splat_to_coarse(&channels[i], w, h);
                let ra = convolve_coarse_one(&ca, k_fft);
                mix_from_coarse(&mut channels[i], &ra, 1.0, self.halation_share[i], w, h);
                hal_done[i] = true;
            }
        }
    }
}

fn fft_1d(buf: &mut [(f64, f64)], inverse: bool) {
    let n = buf.len();
    debug_assert!(n.is_power_of_two());
    let mut j = 0usize;
    for i in 1..n {
        let mut bit = n >> 1;
        while j & bit != 0 {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if i < j {
            buf.swap(i, j);
        }
    }
    let sign = if inverse { 1.0 } else { -1.0 };
    let mut len = 2;
    while len <= n {
        let half = len >> 1;
        let ang = sign * 2.0 * PI / len as f64;
        let wlen = (ang.cos(), ang.sin());
        for i in (0..n).step_by(len) {
            let mut w = (1.0, 0.0);
            for k in 0..half {
                let u = buf[i + k];
                let v = buf[i + k + half];
                let tv = (v.0 * w.0 - v.1 * w.1, v.0 * w.1 + v.1 * w.0);
                buf[i + k] = (u.0 + tv.0, u.1 + tv.1);
                buf[i + k + half] = (u.0 - tv.0, u.1 - tv.1);
                w = (w.0 * wlen.0 - w.1 * wlen.1, w.0 * wlen.1 + w.1 * wlen.0);
            }
        }
        len <<= 1;
    }
    if inverse {
        let inv = 1.0 / n as f64;
        for c in buf.iter_mut() {
            c.0 *= inv;
            c.1 *= inv;
        }
    }
}

fn fft_2d(buf: &mut [(f64, f64)], inverse: bool) {
    buf.par_chunks_mut(OPTICS_NX).for_each(|row| fft_1d(row, inverse));
    let mut trans = vec![(0.0, 0.0); OPTICS_NX * OPTICS_NY];
    for y in 0..OPTICS_NY {
        let row = &buf[y * OPTICS_NX..(y + 1) * OPTICS_NX];
        for x in 0..OPTICS_NX {
            trans[x * OPTICS_NY + y] = row[x];
        }
    }
    trans.par_chunks_mut(OPTICS_NY).for_each(|col| fft_1d(col, inverse));
    for x in 0..OPTICS_NX {
        let col = &trans[x * OPTICS_NY..(x + 1) * OPTICS_NY];
        for y in 0..OPTICS_NY {
            buf[y * OPTICS_NX + x] = col[y];
        }
    }
}

fn build_kernel_fft(eval: impl Fn(f64) -> f64) -> Vec<(f64, f64)> {
    let dx_c = (2.0 * X_EDGE_PH) / OPTICS_CW as f64;
    let dy_c = 1.0 / OPTICS_CH as f64;
    let d0 = UNIT_SQUARE_MEAN_R * 0.5 * (dx_c + dy_c);
    let mut buf = vec![(0.0, 0.0); OPTICS_NX * OPTICS_NY];
    let max_dy = OPTICS_CH as isize - 1;
    let max_dx = OPTICS_CW as isize - 1;
    for dy in -max_dy..=max_dy {
        let iy = dy.rem_euclid(OPTICS_NY as isize) as usize;
        let py = dy as f64 * dy_c;
        for dx in -max_dx..=max_dx {
            let ix = dx.rem_euclid(OPTICS_NX as isize) as usize;
            let d = if dx == 0 && dy == 0 { d0 } else { (dx as f64 * dx_c).hypot(py) };
            buf[iy * OPTICS_NX + ix] = (eval(d), 0.0);
        }
    }
    fft_2d(&mut buf, false);
    buf
}

#[inline]
fn bilinear_indices(pos: f64, n: usize) -> (usize, usize, f64) {
    let fl = pos.floor();
    let t = pos - fl;
    let i0 = (fl as isize).clamp(0, n as isize - 1) as usize;
    let i1 = (fl as isize + 1).clamp(0, n as isize - 1) as usize;
    (i0, i1, t)
}

fn splat_to_coarse(src: &[f64], w: usize, h: usize) -> Vec<f64> {
    let pixel_area = (TARGET_ASPECT / w as f64) * (1.0 / h as f64);
    let sx = OPTICS_CW as f64 / w as f64;
    let sy = OPTICS_CH as f64 / h as f64;
    let x_map: Vec<(usize, usize, f64)> =
        (0..w).map(|s| bilinear_indices((s as f64 + 0.5) * sx - 0.5, OPTICS_CW)).collect();
    let mut coarse = vec![0.0; OPTICS_CW * OPTICS_CH];
    for r in 0..h {
        let (iy0, iy1, ty) = bilinear_indices((r as f64 + 0.5) * sy - 0.5, OPTICS_CH);
        let wy0 = (1.0 - ty) * pixel_area;
        let wy1 = ty * pixel_area;
        let row_src = &src[r * w..(r + 1) * w];
        let row0 = iy0 * OPTICS_CW;
        let row1 = iy1 * OPTICS_CW;
        for (s, &v) in row_src.iter().enumerate() {
            if v == 0.0 {
                continue;
            }
            let (ix0, ix1, tx) = x_map[s];
            let wx0 = 1.0 - tx;
            coarse[row0 + ix0] += v * wy0 * wx0;
            coarse[row0 + ix1] += v * wy0 * tx;
            coarse[row1 + ix0] += v * wy1 * wx0;
            coarse[row1 + ix1] += v * wy1 * tx;
        }
    }
    coarse
}

fn convolve_coarse_one(a: &[f64], k_fft: &[(f64, f64)]) -> Vec<f64> {
    let mut buf = vec![(0.0, 0.0); OPTICS_NX * OPTICS_NY];
    for y in 0..OPTICS_CH {
        let src = &a[y * OPTICS_CW..(y + 1) * OPTICS_CW];
        let dst = &mut buf[y * OPTICS_NX..y * OPTICS_NX + OPTICS_CW];
        for (d, &v) in dst.iter_mut().zip(src.iter()) {
            *d = (v, 0.0);
        }
    }
    fft_2d(&mut buf, false);
    for (b, &k) in buf.iter_mut().zip(k_fft.iter()) {
        *b = (b.0 * k.0 - b.1 * k.1, b.0 * k.1 + b.1 * k.0);
    }
    fft_2d(&mut buf, true);
    let mut out = vec![0.0; OPTICS_CW * OPTICS_CH];
    for y in 0..OPTICS_CH {
        let src = &buf[y * OPTICS_NX..y * OPTICS_NX + OPTICS_CW];
        let dst = &mut out[y * OPTICS_CW..(y + 1) * OPTICS_CW];
        for (d, s) in dst.iter_mut().zip(src.iter()) {
            *d = s.0.max(0.0);
        }
    }
    out
}

fn convolve_coarse_two(a: &[f64], b: &[f64], k_fft: &[(f64, f64)]) -> (Vec<f64>, Vec<f64>) {
    let mut buf = vec![(0.0, 0.0); OPTICS_NX * OPTICS_NY];
    for y in 0..OPTICS_CH {
        let sa = &a[y * OPTICS_CW..(y + 1) * OPTICS_CW];
        let sb = &b[y * OPTICS_CW..(y + 1) * OPTICS_CW];
        let dst = &mut buf[y * OPTICS_NX..y * OPTICS_NX + OPTICS_CW];
        for ((d, &va), &vb) in dst.iter_mut().zip(sa.iter()).zip(sb.iter()) {
            *d = (va, vb);
        }
    }
    fft_2d(&mut buf, false);
    for (c, &k) in buf.iter_mut().zip(k_fft.iter()) {
        *c = (c.0 * k.0 - c.1 * k.1, c.0 * k.1 + c.1 * k.0);
    }
    fft_2d(&mut buf, true);
    let mut oa = vec![0.0; OPTICS_CW * OPTICS_CH];
    let mut ob = vec![0.0; OPTICS_CW * OPTICS_CH];
    for y in 0..OPTICS_CH {
        let src = &buf[y * OPTICS_NX..y * OPTICS_NX + OPTICS_CW];
        let da = &mut oa[y * OPTICS_CW..(y + 1) * OPTICS_CW];
        let db = &mut ob[y * OPTICS_CW..(y + 1) * OPTICS_CW];
        for ((a_out, b_out), s) in da.iter_mut().zip(db.iter_mut()).zip(src.iter()) {
            *a_out = s.0.max(0.0);
            *b_out = s.1.max(0.0);
        }
    }
    (oa, ob)
}

fn mix_from_coarse(dst: &mut [f64], coarse: &[f64], direct_w: f64, conv_w: f64, w: usize, h: usize) {
    let sx = OPTICS_CW as f64 / w as f64;
    let sy = OPTICS_CH as f64 / h as f64;
    let x_map: Vec<(usize, usize, f64)> =
        (0..w).map(|s| bilinear_indices((s as f64 + 0.5) * sx - 0.5, OPTICS_CW)).collect();
    dst.par_chunks_mut(w).enumerate().for_each(|(r, row_dst)| {
        let (iy0, iy1, ty) = bilinear_indices((r as f64 + 0.5) * sy - 0.5, OPTICS_CH);
        let r0 = &coarse[iy0 * OPTICS_CW..(iy0 + 1) * OPTICS_CW];
        let r1 = &coarse[iy1 * OPTICS_CW..(iy1 + 1) * OPTICS_CW];
        for (s, d) in row_dst.iter_mut().enumerate() {
            let (ix0, ix1, tx) = x_map[s];
            let v0 = r0[ix0] + tx * (r0[ix1] - r0[ix0]);
            let v1 = r1[ix0] + tx * (r1[ix1] - r1[ix0]);
            let conv = v0 + ty * (v1 - v0);
            *d = direct_w * *d + conv_w * conv;
        }
    });
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn scan_gain_matches_finite_difference_jacobian() {
        let g = profile_geometry(true, TubeKind::Plumbicon);
        let h = 1e-6;
        for ch in 0..3 {
            for &(x, y) in &[(0.0, 0.0), (-0.4, 0.25), (0.6, -0.45)] {
                let dx_p = displacement_ph(&g, ch, x + h, y).0;
                let dx_m = displacement_ph(&g, ch, x - h, y).0;
                let dy_p = displacement_ph(&g, ch, x, y + h).1;
                let dy_m = displacement_ph(&g, ch, x, y - h).1;
                let want = (1.0 + (dx_p - dx_m) / (2.0 * h)) * (1.0 + (dy_p - dy_m) / (2.0 * h));
                let got = scan_gain(&g, ch, x, y);
                assert!((got - want).abs() <= 1e-9, "ch{ch} ({x}, {y}): {got} vs {want}");
            }
        }
    }

    #[test]
    fn kernels_integrate_to_unity_over_the_plane() {
        let v = Veil { fraction: 0.0215, scale_ph: 0.40 };
        let dr = 1e-3;
        let r_max = 10.0;
        let n = (r_max / dr) as usize;
        let veil_int: f64 = (0..n)
            .map(|i| {
                let r = (i as f64 + 0.5) * dr;
                veil_kernel(&v, r) * 2.0 * PI * r * dr
            })
            .sum();
        assert!((veil_int - 1.0).abs() <= 1e-3, "veil integral {veil_int}");

        let h = Halation {
            reflectance: 0.15,
            faceplate_mm: 1.5,
            disc_mm: 0.0,
            target_height_mm: 12.8,
        };
        let hal_int: f64 = (0..n)
            .map(|i| {
                let r = (i as f64 + 0.5) * dr;
                halation_kernel(&h, r) * 2.0 * PI * r * dr
            })
            .sum();
        assert!((hal_int - 1.0).abs() <= 5e-3, "halation integral {hal_int}");
    }

    #[test]
    fn optics_grid_convolves_flat_field_within_tolerance() {
        let v = Veil { fraction: 0.0215, scale_ph: 0.40 };
        let mut g = GeometrySpec::NONE;
        g.veil = [v; 3];
        let plan = OpticsPlan::new(&g).unwrap();
        let (w, h) = (160, 120);
        let mut img = vec![1.0; w * h];
        plan.apply_one(0, &mut img, w, h);
        // At the picture centre, veil integral over the 4/3 x 1 rectangle is slightly below 1.0
        let centre = img[(h / 2) * w + (w / 2)];
        assert!(centre > 0.985 && centre < 1.0, "centre {centre}");
    }
}
