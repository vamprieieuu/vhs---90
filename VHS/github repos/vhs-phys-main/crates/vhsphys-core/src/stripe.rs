//! Single-tube stripe colour camera optics and frequency-separation decoder.
//!
//! Sources: `docs/design/CAMERA_STRIPE.md` (contract), `docs/CAMERA_RESEARCH.md` ("Single-tube
//! camcorders trade colour bandwidth for a striped carrier"), Panasonic WV-3110 (3.58 MHz
//! cyan/yellow stripe carrier, 2.8 MHz luminance, 0.5 MHz colour, birefringent optical low-pass).

use super::consts::FS_BB;
use rayon::prelude::*;
use std::f64::consts::PI;

/// Stripe carrier frequency, Hz (w = 7.49 mm, p = 40 um, t = 52.3 us -> 3.58 MHz;
/// `docs/CAMERA_RESEARCH.md`, Panasonic WV-3110).
pub const STRIPE_CARRIER_HZ: f64 = 3.58e6;
/// Luminance -3 dB low-pass band, Hz (`docs/CAMERA_RESEARCH.md`, Panasonic WV-3110).
pub const LUMA_BAND_HZ: f64 = 2.8e6;
/// Colour -3 dB low-pass band, Hz (`docs/CAMERA_RESEARCH.md`, Panasonic WV-3110).
pub const COLOUR_BAND_HZ: f64 = 0.5e6;
/// Chroma band-pass half-width at -3 dB around `STRIPE_CARRIER_HZ`, Hz.
pub const CHROMA_HALF_WIDTH_HZ: f64 = 0.5e6;
/// Mean of `T_R + T_G + T_B` over the stripes: the target light of white, which gives the tube's peak
/// current on average.
pub const STRIPE_WHITE_LIGHT: f64 = 2.0;

/// Frequency-sampling grid size for the linear-phase Gaussian FIRs.
const FIR_GRID: usize = 4096;
/// Tap counts (odd, centred) and Kaiser window shapes for each decoder FIR.
const LUMA_LP_TAPS: usize = 25;
const COLOUR_LP_TAPS: usize = 65;
const BANDPASS_TAPS: usize = 65;
const HILBERT_TAPS: usize = 63;
const GAUSS_FIR_BETA: f64 = 4.0;
const HILBERT_FIR_BETA: f64 = 5.0;

/// Stripe transmittances `[R, G, B]` at sample `x` of active frame row `r`.
pub fn stripe_transmittance(r: usize, x: usize) -> [f64; 3] {
    let base = 2.0 * PI * STRIPE_CARRIER_HZ * x as f64 / FS_BB;
    let phi = PI * r as f64 / 4.0;
    let t_r = 0.5 * (1.0 + (base + phi).cos());
    let t_b = 0.5 * (1.0 + (base - phi).cos());
    [t_r, 1.0, t_b]
}

/// One colour of one row of scene light behind the optical low-pass.
pub fn optical_lowpass(row: &[f64]) -> Vec<f64> {
    if row.is_empty() {
        return Vec::new();
    }
    // Birefringent plate splits along the line by half a carrier period d = 1 / (2 f_c),
    // so each copy shifts by +-d/2 = +-FS_BB / (4 f_c) samples.
    let half_d = FS_BB / (4.0 * STRIPE_CARRIER_HZ);
    (0..row.len())
        .map(|x| {
            let xf = x as f64;
            0.5 * (sample_cubic_f64(row, xf - half_d) + sample_cubic_f64(row, xf + half_d))
        })
        .collect()
}

/// The light reaching the target at `(r, x)` from scene light `rgb` there: `sum_c T_c rgb_c`.
pub fn target_light(r: usize, x: usize, rgb: [f64; 3]) -> f64 {
    let t = stripe_transmittance(r, x);
    t[0] * rgb[0] + t[1] * rgb[1] + t[2] * rgb[2]
}

/// Frequency-separation decoder for the cyan/yellow stripe colour camera.
#[derive(Clone, Debug)]
pub struct StripeDecoder {
    carrier_gain: f64,
    quarter_delay: f64,
    bp_taps: Vec<f64>,
    luma_lp_taps: Vec<f64>,
    colour_lp_taps: Vec<f64>,
    hilbert_taps: Vec<f64>,
}

impl StripeDecoder {
    /// Build a decoder with `carrier_gain` compensating the read response at `STRIPE_CARRIER_HZ`.
    pub fn new(carrier_gain: f64) -> Self {
        let mut luma_lp_taps = frequency_sampled_fir(LUMA_LP_TAPS, FIR_GRID, GAUSS_FIR_BETA, |f| {
            gaussian_mag(f, 0.0, LUMA_BAND_HZ)
        });
        normalise_dc(&mut luma_lp_taps);

        let mut colour_lp_taps =
            frequency_sampled_fir(COLOUR_LP_TAPS, FIR_GRID, GAUSS_FIR_BETA, |f| {
                gaussian_mag(f, 0.0, COLOUR_BAND_HZ)
            });
        normalise_dc(&mut colour_lp_taps);

        let mut bp_taps = frequency_sampled_fir(BANDPASS_TAPS, FIR_GRID, GAUSS_FIR_BETA, |f| {
            gaussian_mag(f, STRIPE_CARRIER_HZ, CHROMA_HALF_WIDTH_HZ)
                + gaussian_mag(f, -STRIPE_CARRIER_HZ, CHROMA_HALF_WIDTH_HZ)
        });
        // Remove residual DC leakage without perturbing the passband, then lock carrier gain to 1.
        let dc_leak: f64 = bp_taps.iter().sum();
        for (tap, &lp) in bp_taps.iter_mut().zip(colour_lp_taps.iter()) {
            *tap -= dc_leak * lp;
        }
        let carrier_mag = symmetric_gain(&bp_taps, STRIPE_CARRIER_HZ);
        for tap in bp_taps.iter_mut() {
            *tap /= carrier_mag;
        }

        let hilbert_taps = build_hilbert_fir(HILBERT_TAPS, HILBERT_FIR_BETA, STRIPE_CARRIER_HZ);
        let quarter_delay = FS_BB / (4.0 * STRIPE_CARRIER_HZ);

        Self {
            carrier_gain,
            quarter_delay,
            bp_taps,
            luma_lp_taps,
            colour_lp_taps,
            hilbert_taps,
        }
    }

    /// Decode a frame of clamped line signals (`rows[r][x]`, active rows in frame order) into
    /// `[R, G, B]` rows on the same scale.
    pub fn decode(&self, rows: &[Vec<f32>]) -> Vec<[Vec<f32>; 3]> {
        // Pass 1: extract each row's bandpass chroma and trapped luminance independently.
        let chroma_and_luma: Vec<(Vec<f64>, Vec<f64>)> = rows
            .par_iter()
            .map(|row| {
                let s: Vec<f64> = row.iter().map(|&v| v as f64).collect();
                let c = convolve_centred(&s, &self.bp_taps);
                let trapped: Vec<f64> = s.iter().zip(c.iter()).map(|(&sv, &cv)| sv - cv).collect();
                let y = convolve_centred(&trapped, &self.luma_lp_taps);
                (c, y)
            })
            .collect();

        // Pass 2: 1H chroma separation (reads row r - 2), envelope detection, and mixed-highs matrix.
        (0..rows.len())
            .into_par_iter()
            .map(|r| {
                let (ref c0, ref y) = chroma_and_luma[r];
                let w = c0.len();
                if w == 0 {
                    return [Vec::new(), Vec::new(), Vec::new()];
                }

                let mut red_carrier = Vec::with_capacity(w);
                let mut blue_carrier = Vec::with_capacity(w);
                let c1_opt = if r >= 2 { Some(&chroma_and_luma[r - 2].0) } else { None };
                for x in 0..w {
                    let dc0 = sample_cubic_f64(c0, x as f64 - self.quarter_delay);
                    let c1 = c1_opt.and_then(|v| v.get(x).copied()).unwrap_or(0.0);
                    red_carrier.push(dc0 + c1);
                    blue_carrier.push(dc0 - c1);
                }

                let r_c = self.detect_envelope(&red_carrier);
                let b_c = self.detect_envelope(&blue_carrier);
                let y_c = convolve_centred(y, &self.colour_lp_taps);

                let mut out: [Vec<f32>; 3] = std::array::from_fn(|_| vec![0.0f32; w]);
                for x in 0..w {
                    let g_c = y_c[x] - 0.5 * (r_c[x] + b_c[x]);
                    let h = 0.5 * (y[x] - y_c[x]);
                    out[0][x] = (r_c[x] + h) as f32;
                    out[1][x] = (g_c + h) as f32;
                    out[2][x] = (b_c[x] + h) as f32;
                }
                out
            })
            .collect()
    }

    fn detect_envelope(&self, carrier: &[f64]) -> Vec<f64> {
        let hilb = convolve_centred(carrier, &self.hilbert_taps);
        let mag: Vec<f64> = carrier
            .iter()
            .zip(hilb.iter())
            .map(|(&re, &im)| re.hypot(im) * self.carrier_gain)
            .collect();
        convolve_centred(&mag, &self.colour_lp_taps)
    }
}

/// Gaussian magnitude with -3 dB (`1 / sqrt(2)`) at `f_center +- half_width_hz`.
#[inline]
fn gaussian_mag(f: f64, f_center: f64, half_width_hz: f64) -> f64 {
    let ratio = (f - f_center) / half_width_hz;
    (-0.5 * 2.0_f64.ln() * ratio * ratio).exp()
}

/// Zeroth-order modified Bessel function of the first kind `I_0(x)`.
fn bessel_i0(x: f64) -> f64 {
    let (mut sum, mut term, mut k) = (1.0, 1.0, 1.0);
    while term > 1e-12 * sum {
        term *= (x / (2.0 * k)).powi(2);
        sum += term;
        k += 1.0;
    }
    sum
}

/// Kaiser-windowed linear-phase symmetric FIR by inverse real DFT of `mag(f)` on `grid` points.
fn frequency_sampled_fir(taps_len: usize, grid: usize, beta: f64, mag: impl Fn(f64) -> f64) -> Vec<f64> {
    let half_grid = grid / 2;
    let h_mag: Vec<f64> = (0..=half_grid)
        .map(|k| mag(k as f64 * FS_BB / grid as f64))
        .collect();
    let half_taps = taps_len / 2;
    let i0_beta = bessel_i0(beta);
    let mut taps = vec![0.0; taps_len];
    for (i, tap) in taps.iter_mut().enumerate() {
        let m = i as isize - half_taps as isize;
        let nyq_sign = if m & 1 == 0 { 1.0 } else { -1.0 };
        let mut sum = h_mag[0] + nyq_sign * h_mag[half_grid];
        let step = 2.0 * PI * m as f64 / grid as f64;
        for k in 1..half_grid {
            sum += 2.0 * h_mag[k] * (step * k as f64).cos();
        }
        let h_ideal = sum / grid as f64;
        let r = m as f64 / half_taps as f64;
        let w = bessel_i0(beta * (1.0 - r * r).max(0.0).sqrt()) / i0_beta;
        *tap = h_ideal * w;
    }
    taps
}

/// Kaiser-windowed FIR Hilbert transformer stored in convolution order (`taps[half + d] = h[-d]`),
/// normalised to unity gain at `ref_hz`.
fn build_hilbert_fir(taps_len: usize, beta: f64, ref_hz: f64) -> Vec<f64> {
    let half = taps_len / 2;
    let i0_beta = bessel_i0(beta);
    let mut taps = vec![0.0; taps_len];
    for (i, tap) in taps.iter_mut().enumerate() {
        let d = i as isize - half as isize;
        if d != 0 && d.unsigned_abs() % 2 == 1 {
            let h_ideal = -2.0 / (PI * d as f64);
            let r = d as f64 / half as f64;
            let w = bessel_i0(beta * (1.0 - r * r).max(0.0).sqrt()) / i0_beta;
            *tap = h_ideal * w;
        }
    }
    let gain = antisymmetric_gain(&taps, ref_hz);
    for tap in taps.iter_mut() {
        *tap /= gain;
    }
    taps
}

fn normalise_dc(taps: &mut [f64]) {
    let dc: f64 = taps.iter().sum();
    for tap in taps.iter_mut() {
        *tap /= dc;
    }
}

/// Zero-phase frequency response magnitude of an odd-length symmetric FIR at `f` Hz.
fn symmetric_gain(taps: &[f64], f: f64) -> f64 {
    let half = (taps.len() / 2) as isize;
    let w = 2.0 * PI * f / FS_BB;
    taps.iter()
        .enumerate()
        .map(|(k, &t)| t * (w * (k as isize - half) as f64).cos())
        .sum()
}

/// Quadrature gain (`H{cos} = gain * sin`) of the convolution-ordered Hilbert FIR at `f` Hz.
fn antisymmetric_gain(taps: &[f64], f: f64) -> f64 {
    let half = (taps.len() / 2) as isize;
    let w = 2.0 * PI * f / FS_BB;
    -taps
        .iter()
        .enumerate()
        .map(|(k, &t)| t * (w * (k as isize - half) as f64).sin())
        .sum::<f64>()
}

/// Apply an odd-length centred FIR; row ends repeat their end sample.
fn convolve_centred(x: &[f64], taps: &[f64]) -> Vec<f64> {
    let n = x.len();
    if n == 0 {
        return Vec::new();
    }
    let half = taps.len() / 2;
    let last = (n - 1) as isize;
    let mut out = vec![0.0; n];
    for i in 0..n {
        let acc = if i >= half && i + half < n {
            x[i - half..=i + half]
                .iter()
                .zip(taps.iter())
                .map(|(&a, &b)| a * b)
                .sum()
        } else {
            let mut s = 0.0;
            for (k, &tap) in taps.iter().enumerate() {
                let j = (i as isize + k as isize - half as isize).clamp(0, last) as usize;
                s += tap * x[j];
            }
            s
        };
        out[i] = acc;
    }
    out
}

/// Sample `x` at fractional index `pos` with Catmull-Rom cubic interpolation; ends clamp.
#[inline]
fn sample_cubic_f64(x: &[f64], pos: f64) -> f64 {
    let n = x.len();
    if n == 0 {
        return 0.0;
    }
    if pos <= 0.0 {
        return x[0];
    }
    let last = (n - 1) as f64;
    if pos >= last {
        return x[n - 1];
    }
    let i = pos.floor() as i64;
    let t = pos - i as f64;
    let max_idx = (n - 1) as i64;
    let g = |k: i64| x[k.clamp(0, max_idx) as usize];
    let (p0, p1, p2, p3) = (g(i - 1), g(i), g(i + 1), g(i + 2));
    let a = -0.5 * p0 + 1.5 * p1 - 1.5 * p2 + 0.5 * p3;
    let b = p0 - 2.5 * p1 + 2.0 * p2 - 0.5 * p3;
    let c = -0.5 * p0 + 0.5 * p2;
    ((a * t + b) * t + c) * t + p1
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn filter_magnitudes_match_contract() {
        let dec = StripeDecoder::new(1.0);
        let half_power = std::f64::consts::FRAC_1_SQRT_2;

        // Luminance low-pass: 0 dB at DC, -3 dB at 2.8 MHz.
        assert!((symmetric_gain(&dec.luma_lp_taps, 0.0) - 1.0).abs() < 1e-12);
        let luma_3db = symmetric_gain(&dec.luma_lp_taps, LUMA_BAND_HZ);
        assert!((luma_3db - half_power).abs() < 0.02, "luma at 2.8 MHz: {luma_3db}");

        // Colour low-pass: 0 dB at DC, -3 dB at 0.5 MHz.
        assert!((symmetric_gain(&dec.colour_lp_taps, 0.0) - 1.0).abs() < 1e-12);
        let colour_3db = symmetric_gain(&dec.colour_lp_taps, COLOUR_BAND_HZ);
        assert!((colour_3db - half_power).abs() < 0.02, "colour at 0.5 MHz: {colour_3db}");

        // Chroma band-pass: zero at DC, 1.0 at carrier, -3 dB at carrier +- 0.5 MHz.
        assert!(symmetric_gain(&dec.bp_taps, 0.0).abs() < 1e-12);
        assert!((symmetric_gain(&dec.bp_taps, STRIPE_CARRIER_HZ) - 1.0).abs() < 1e-12);
        for f in [STRIPE_CARRIER_HZ - CHROMA_HALF_WIDTH_HZ, STRIPE_CARRIER_HZ + CHROMA_HALF_WIDTH_HZ]
        {
            let g = symmetric_gain(&dec.bp_taps, f);
            assert!((g - half_power).abs() < 0.02, "bp at {f}: {g}");
        }
        // Complementary trap at carrier is zero.
        let trap_carrier = 1.0 - symmetric_gain(&dec.bp_taps, STRIPE_CARRIER_HZ);
        assert!(trap_carrier.abs() < 1e-12, "trap at carrier: {trap_carrier}");

        // Hilbert transformer: zero at DC, unity across the chroma band.
        assert!(antisymmetric_gain(&dec.hilbert_taps, 0.0).abs() < 1e-12);
        assert!((antisymmetric_gain(&dec.hilbert_taps, STRIPE_CARRIER_HZ) - 1.0).abs() < 1e-12);
        for f in [STRIPE_CARRIER_HZ - CHROMA_HALF_WIDTH_HZ, STRIPE_CARRIER_HZ + CHROMA_HALF_WIDTH_HZ]
        {
            let g = antisymmetric_gain(&dec.hilbert_taps, f);
            assert!((g - 1.0).abs() < 0.01, "hilbert at {f}: {g}");
        }
    }

    #[test]
    fn empty_inputs_do_not_panic() {
        assert!(optical_lowpass(&[]).is_empty());
        let dec = StripeDecoder::new(1.0);
        assert!(dec.decode(&[]).is_empty());
        let out = dec.decode(&[Vec::new(), Vec::new(), Vec::new()]);
        assert_eq!(out.len(), 3);
        assert!(out.iter().all(|ch| ch.iter().all(|r| r.is_empty())));
    }
}
