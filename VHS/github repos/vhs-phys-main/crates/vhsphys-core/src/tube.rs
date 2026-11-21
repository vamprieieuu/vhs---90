//! Camera pickup tube: the photoconductive target of a three-tube Plumbicon camera, the
//! source of the lag and the comet tails of 1970s and 1980s studio and outside-broadcast
//! video (the "trails" behind lamps and highlights).
//!
//! Each element of the target is a small capacitor. Light discharges it through the
//! photoconductor between scans (here: adds charge, counted positive); the scanning beam
//! recharges it to the cathode potential, and the charge it delivers is the signal. Two
//! physical limits of the beam make the picture remember the past:
//!
//! * **Discharge (capacitive) lag.** Near the cathode potential only the faster electrons
//!   of the beam can land (their energies are spread thermally, kT/e of a few tenths of a
//!   volt), so the landing current falls exponentially as the element approaches it:
//!   `i = I_b exp(u / V_T)` for `u <= 0`, `i = I_b` above. An element left slightly
//!   charged after a scan gives up the rest over the next scans. In target-equivalent nA s
//!   the scan of one element is, exactly:
//!     1. above the cathode potential the full beam lands: `r1 = min(q, I_b)`, `q -= r1`;
//!     2. with the beam charge left, `I_b - r1`, below it: `exp(-q'/K) = exp(-q/K) + (I_b - r1)/K`
//!   where `K = C V_T`. The plate limit is `C V_target`; the signal is charge removed.
//!   Light bias adds a steady charge that keeps elements off the slow part of the curve.
//! * **Comet tails.** The beam is set to twice what peak white needs (data sheet). A
//!   highlight beyond that leaves charge the scan cannot remove; it accumulates (up to the
//!   point where the element reaches the signal-plate voltage and photoconduction stops,
//!   `C V_target`) and drains at the beam current after the highlight has
//!   gone. An anti-comet-tail (ACT) gun recharges, during line flyback, every element above
//!   the level the read beam can stabilise, so nothing is left over.
//!
//! The target capacitance per channel is fitted on the scanned engine to handbook lag.
//! Source: Philips Data Handbook T10, Plumbicon camera tubes, 1985 (XQ1020, XQ1410, XQ1520),
//! lag at Is/Ib = 20/300 nA, low key, 50 Hz. See docs/CAMERA.md.

use crate::spectral::Material;
use serde::{Deserialize, Serialize};

/// Gamma of the light transfer characteristic (all three types: 0.95 +/- 0.05).
pub const GAMMA: f64 = 0.95;
/// The camera's gamma corrector follows the BT.709 transfer: linear (slope 4.5) below
/// 0.018, then `1.099 L^0.45 - 0.099`. The input video is taken through its inverse to get
/// the scene light and the tube's signal through it again, so a tube with gamma 1 and no
/// lag returns the input.
pub fn oetf(l: f64) -> f64 {
    let l = l.clamp(0.0, 1.0);
    if l < 0.018 { 4.5 * l } else { 1.099 * l.powf(0.45) - 0.099 }
}

/// Inverse of `oetf`.
pub fn oetf_inverse(v: f64) -> f64 {
    let v = v.clamp(0.0, 1.0);
    if v < 0.081 { v / 4.5 } else { ((v + 0.099) / 1.099).powf(1.0 / 0.45) }
}
/// Peak-white signal current per channel `[R, G, B]`, nA (30 mm tubes).
pub const PEAK_SIGNAL_NA: [f64; 3] = [150.0, 300.0, 150.0];
/// Beam current as a multiple of what peak white needs, as the data sheet sets it.
pub const BEAM_RATIO_DEFAULT: f64 = 2.0;
/// Dark current, nA. The data sheets give maxima of 1 to 3 nA. [est]
pub const DARK_NA: f64 = 1.0;
/// Internal light bias of the XQ1410, nA per channel `[R, G, B]` (data sheet setting).
pub const LIGHT_BIAS_XQ1410_NA: [f64; 3] = [3.0, 2.0, 3.5];
/// Signal-plate (target) voltage, V.
pub const TARGET_VOLTAGE_V: f64 = 45.0;
/// Triode gun beam temperature of 1900 to 2350 K with crossover, US 4,388,556.
pub const V_T_TRIODE_GUN_V: f64 = 0.17;
/// Diode gun beam temperature of about 1200 K, US 4,388,556.
pub const V_T_DIODE_GUN_V: f64 = 0.10;
/// Fitted target capacitance per colour `[R, G, B]`, F; see docs/CAMERA.md.
/// R and G are unchanged from the capacitive Plumbicon fit (`docs/CAMERA_RESEARCH.md`, "One
/// constant reproduces the handbook curves"); B is refitted with traps within 15 % of the R/G
/// mean (`docs/CAMERA_RESEARCH.md`, "Traps add an exposure-dependent tail, strongest in blue PbO
/// and in Sb2S3").
pub const TARGET_CAPACITANCE_F: [f64; 3] = [1.38e-9, 1.15e-9, 1.32e-9];
/// Maximum trap populations per target channel.
pub const MAX_TRAPS: usize = 2;

/// One photoconductor trap population on a target channel.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Trap {
    /// Share of light current (photocurrent plus bias) captured while empty (0 to 1).
    pub capture: f64,
    /// Trap capacity in target-equivalent nA s.
    pub capacity_nas: f64,
    /// Exponential release time constant, s.
    pub release_s: f64,
}

impl Trap {
    /// An unused trap slot.
    pub const NONE: Trap = Trap { capture: 0.0, capacity_nas: 0.0, release_s: 0.0 };
}

/// Fitted PbO blue trap populations, capacities on the 30 mm target (`docs/CAMERA_RESEARCH.md`,
/// "Traps add an exposure-dependent tail, strongest in blue PbO and in Sb2S3"): fitted to the
/// XQ1410B floor against bias (Mullard 1987 Figs. 12 and 14), the XQ1427B exposure family (RCA
/// handbook Figs. 11-61/11-62) and the XQ1020/XQ1520 blue table. The first fills within a few
/// fields and carries the 50 ms excess; the second fills over about a second and holds the floor
/// that bias light does not remove.
pub const PBO_BLUE_TRAPS: [Trap; MAX_TRAPS] = [
    Trap { capture: 0.12, capacity_nas: 0.37, release_s: 0.066 },
    Trap { capture: 0.035, capacity_nas: 3.9, release_s: 0.45 },
];

/// Peak-white signal current for the 2/3-inch and 18 mm non-PbO tubes, nA.
/// Source: `docs/CAMERA_RESEARCH.md` ("Six photoconductors differ in gamma, dark current and lag
/// mechanism"); Hitachi H8397A (0.2 uA), Matsushita S4075 / Philips T12 (0.2 uA), home vidicons
/// (200 nA); Chalnicon `[est]`.
pub const NEW_PEAK_SIGNAL_NA: f64 = 200.0;
/// Transfer exponents (`gamma`) of the non-PbO materials.
/// Source: `docs/CAMERA_RESEARCH.md` ("Six photoconductors differ in gamma, dark current and lag
/// mechanism"); Philips T12 / Panasonic WV-3110 (Sb2S3 0.65), Philips T12 (Newvicon 1.0), Hitachi
/// H8397A (Saticon 1.0), Toshiba 1973 (Chalnicon 0.90-0.95).
pub const VIDICON_GAMMA: f64 = 0.65;
pub const NEWVICON_GAMMA: f64 = 1.0;
pub const SATICON_GAMMA: f64 = 1.0;
pub const CHALNICON_GAMMA: f64 = 0.925;
/// Signal-plate (target) voltages, V.
/// Source: `docs/CAMERA_RESEARCH.md` ("Six photoconductors differ in gamma, dark current and lag
/// mechanism"); Philips T12 / XQ1271 (vidicon 10-60 V `[est]`, Newvicon 10-35 V `[est]`),
/// Hitachi H8397A (Saticon 50 V), Toshiba 1973 (Chalnicon 35 V).
pub const VIDICON_TARGET_VOLTAGE_V: f64 = 30.0;
pub const NEWVICON_TARGET_VOLTAGE_V: f64 = 25.0;
pub const SATICON_TARGET_VOLTAGE_V: f64 = 50.0;
pub const CHALNICON_TARGET_VOLTAGE_V: f64 = 35.0;
/// Dark current at 25 deg C (nA) and temperature doubling step (deg C).
/// Source: `docs/CAMERA_RESEARCH.md` ("Six photoconductors differ in gamma, dark current and lag
/// mechanism"); Philips T12 / XQ1271 (vidicon 20 nA, doubles every 7-8 deg C; Newvicon 5 nA,
/// doubles every 7-8 deg C), Hitachi H8397A (Saticon 0.15 nA at 20 deg C, 2.7 nA at 60 deg C),
/// Toshiba 1973 (Chalnicon 0.1 / 1 / 10 nA at 0 / 30 / 60 deg C).
pub const VIDICON_DARK_25_NA: f64 = 20.0;
pub const VIDICON_DARK_DOUBLING_C: f64 = 7.5;
pub const NEWVICON_DARK_25_NA: f64 = 5.0;
pub const NEWVICON_DARK_DOUBLING_C: f64 = 7.5;
pub const SATICON_DARK_25_NA: f64 = 0.215;
pub const SATICON_DARK_DOUBLING_C: f64 = 9.6;
pub const CHALNICON_DARK_25_NA: f64 = 0.681;
pub const CHALNICON_DARK_DOUBLING_C: f64 = 9.0;
/// Fitted target capacitances for the non-PbO materials, F, with `V_T = 0.17` V, on the scanned
/// target against the lag data of `docs/CAMERA_RESEARCH.md` ("Six photoconductors differ in gamma,
/// dark current and lag mechanism", "Traps add an exposure-dependent tail"). Saticon [DER]: fitted
/// with its traps to the BC4908 family [FIG] (RCA 1977, Neuhauser 1978 Fig. 12: 50 ms lag at 30 / 50 /
/// 200 nA with 0 / 5 / 10 nA bias, 800 nA beam; `docs/design/CAMERA_RETENTION.md`), every point
/// within a factor 1.2; `K = C V_T` is 0.136 nA s. It lies inside the layer's 1000-2000 pF/cm^2
/// [FIG] (Hitachi Hyoron 1976 Fig. 10, 0.58-1.16 nF on 6.6 x 8.8 mm [DER]). Newvicon: Matsushita
/// S4075 (10 % at 50 ms) and Philips T12 (8-13 % at 60 ms). Chalnicon: Toshiba 1973 (10 % at 50 ms)
/// [est conditions]. Vidicon: home-camera 20 % at 50 ms, Philips XQ1240 and RCA's bias cut.
pub const VIDICON_CAPACITANCE_F: f64 = 9.9e-9;
pub const NEWVICON_CAPACITANCE_F: f64 = 7.6e-9;
pub const SATICON_CAPACITANCE_F: f64 = 0.80e-9;
pub const CHALNICON_CAPACITANCE_F: f64 = 6.4e-9;
/// Fitted trap populations for the non-PbO materials, from the same lag data
/// (`docs/CAMERA_RESEARCH.md`, "Traps add an exposure-dependent tail"). The vidicon's single strong
/// trap carries most of its lag and is what bias light cuts.
///
/// The Saticon's traps are fitted [DER] with `SATICON_CAPACITANCE_F` to the BC4908 family [FIG]
/// (RCA 1977, Neuhauser 1978 Fig. 12; `docs/design/CAMERA_RETENTION.md`). The first is small and
/// fast and takes nearly all the light while it has room, so a few nA of bias keep it full: it is
/// part of what bias light removes at 30 and 50 nA. The second is large and stays proportional to
/// the light, so bias light does not touch it: it is the tail the family keeps at 200 nA with
/// 10 nA of bias (1.85 %), about 1.5 % at 50 ms [DER]. Its capacity is [est], only large enough
/// that a 200 nA picture fills a few per cent of it. Checks the fit was not made on: the BC4390's
/// 1 % at 150 ms with 6 nA of bias (0.72 %) [DS], the shape of the BC4391 exposure family [FIG],
/// and US 4,330,733's sticking band with `SATICON_COLLECTION` [FIG]. The BC4391's absolute levels
/// (about 2 % at 50 ms at 30 nA, 5 nA bias) are not met: the two RCA families disagree by more than
/// a factor 2 there, and the H8397A's 6.7 % unbiased at 0.2 uA (Fig. 6) is not met either
/// (`docs/design/CAMERA_RETENTION.md`, "The Saticon's lag does not rise with signal").
pub const VIDICON_TRAPS: [Trap; MAX_TRAPS] =
    [Trap { capture: 0.95, capacity_nas: 5.0, release_s: 0.25 }, Trap::NONE];
pub const SATICON_TRAPS: [Trap; MAX_TRAPS] = [
    Trap { capture: 0.95, capacity_nas: 0.14, release_s: 0.047 },
    Trap { capture: 0.027, capacity_nas: 20.0, release_s: 0.10 },
];

/// How completely the layer sweeps out its carriers as its field falls, and the carriers it holds
/// when it does not. `FULL`: complete collection, no held charge.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Collection {
    pub zero_v: f64,
    pub scale_v: f64,
    pub knee_v: f64,
    /// Held-charge capacity, target-equivalent nA s.
    pub capacity_nas: f64,
    /// Release time of the held charge, s.
    pub release_s: f64,
}

impl Collection {
    pub const FULL: Collection = Collection {
        zero_v: 0.0,
        scale_v: 1.0,
        knee_v: f64::NEG_INFINITY,
        capacity_nas: 0.0,
        release_s: 1.0,
    };

    pub fn is_full(&self) -> bool {
        !self.knee_v.is_finite()
    }

    /// `eta(V)`, 0 to 1.
    pub fn efficiency(&self, layer_v: f64) -> f64 {
        if self.is_full() || layer_v >= self.knee_v {
            return 1.0;
        }
        if layer_v <= self.zero_v {
            return 0.0;
        }
        let num = 1.0 - (-(layer_v - self.zero_v) / self.scale_v).exp();
        let den = 1.0 - (-(self.knee_v - self.zero_v) / self.scale_v).exp();
        num / den
    }
}

/// Lead-oxide collection laws `[R, G, B]` (RCA AN-5042 Fig. 2, Philips 5b 1975 XQ1020 high key).
pub const PBO_COLLECTION: [Collection; 3] = [
    Collection { zero_v: 5.0, scale_v: 10.0, knee_v: 45.0, capacity_nas: 10.0, release_s: 2.0 },
    Collection { zero_v: 5.0, scale_v: 10.0, knee_v: 45.0, capacity_nas: 10.0, release_s: 2.0 },
    Collection { zero_v: 3.0, scale_v: 6.0, knee_v: 45.0, capacity_nas: 1.0, release_s: 2.0 },
];

/// Saticon Te-doped layer collection law (Hitachi Hyoron 1976 Fig. 8, US 4,330,733 Fig. 6).
pub const SATICON_COLLECTION: Collection =
    Collection { zero_v: 8.0, scale_v: 8.0, knee_v: 38.0, capacity_nas: 14.0, release_s: 0.25 };

/// Reversible burn: drive per second per unit of `p / peak_signal_na`, recovery time, and the dark
/// current's relative rise per unit of burn.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Burn {
    pub rate_per_s: f64,
    pub recovery_s: f64,
    pub dark_gain: f64,
}

impl Burn {
    pub const NONE: Burn = Burn { rate_per_s: 0.0, recovery_s: 1.0, dark_gain: 0.0 };
}

/// Antimony trisulphide temporary burn (US 3,546,520).
pub const VIDICON_BURN: Burn =
    Burn { rate_per_s: 5.5e-6, recovery_s: 600.0, dark_gain: 0.75 };

/// Faceplate warm-up time constant, s (`docs/design/CAMERA_RETENTION.md`) [est].
pub const WARM_UP_S: f64 = 600.0;
/// Vertical beam spot width (Gaussian sigma) in frame-line pitches [est].
/// Source: `docs/CAMERA_RESEARCH.md` ("Interlaced readout"), 0.55 to 0.73 across tube formats,
/// derived from Philips T10 and RCA 1977 400 TVL modulation depths.
pub const SIGMA_V_LINES: f64 = 0.65;
/// Horizontal beam spot width (Gaussian sigma) in seconds [est].
/// Source: `docs/CAMERA_RESEARCH.md` ("Interlaced readout"), 36 to 44 ns at 52 us active scan
/// for 30 mm Plumbicon tubes (XQ1520 ACT 36 ns, XQ1020 G 44 ns).
pub const SIGMA_H_S: f64 = 40e-9;
/// ACT cutting level per channel `[R, G, B]`, V: XQ1520 cathode-pulse presets
/// (Mullard 1987, `docs/CAMERA_RESEARCH.md`) [DS].
pub const ACT_LEVEL_V: [f64; 3] = [3.5, 7.0, 3.5];
/// ACT flyback beam current, nA: grid 1 raised to about 100 uA during flyback
/// (Mullard 1987, `docs/CAMERA_RESEARCH.md`) [DS], all taken to reach the target [est].
pub const ACT_BEAM_NA: f64 = 100_000.0;
/// Duration of the ACT line flyback beam, s [est, inside the 10.9 to 12 us line blanking].
pub const ACT_FLYBACK_S: f64 = 8.0e-6;
/// Height of the ACT flyback band, as a fraction of the picture height
/// ([est]: "many scanning lines", US 4,143,305; 16 lines at 525, 19.2 at 625).
pub const ACT_SPAN_PH: f64 = 1.0 / 30.0;
/// Element potential above cathode at which beam bending starts, V
/// (Philips Tech. Rev. 1963: kept target swing within about 10 V) [DS].
pub const V_BEND_ONSET_V: f64 = 10.0;
/// Maximum beam-bending capture radius in frame-line pitches [est, bounds the search].
pub const BEND_RADIUS_MAX_LINES: f64 = 4.0;
/// Scanned target picture aspect ratio (4:3 broadcast raster).
pub const TARGET_ASPECT: f64 = 4.0 / 3.0;
/// Bisection tolerance for dynamic beam control beam current, nA [est].
pub const DBC_TOLERANCE_NA: f64 = 0.01;
/// Dynamic beam control: `headroom` as a fraction of `beam_na` (US 4,229,767: 300 nA over a
/// 600 nA base) [DS], the largest beam as a multiple of `peak_signal_na` ([DER]: 16x peak white
/// handled per Mullard 1987 plus one peak of headroom).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Dbc {
    pub headroom: f64,
    pub max_ratio: f64,
}
/// Diode-gun dynamic beam control parameters (`docs/design/CAMERA_SCAN.md`, "Dynamic beam control").
pub const DBC: Dbc = Dbc { headroom: 0.5, max_ratio: 17.0 };
/// Which camera tube the picture was shot through.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "kebab-case")]
pub enum TubeKind {
    /// No camera stage: the input is the video signal itself.
    #[default]
    Off,
    /// Plumbicon, standard gun, no light bias (XQ1020): lag and comet tails.
    Plumbicon,
    /// Plumbicon with internal light bias (XQ1410): less lag, the same comet tails.
    PlumbiconBias,
    /// Plumbicon with an anti-comet-tail gun (XQ1520): lag, no comet tails.
    PlumbiconAct,
    /// Plumbicon diode gun with dynamic beam control (XQ3440): cooler beam, higher capacitance.
    PlumbiconDbc,
    /// Antimony trisulphide (Sb2S3) 2/3-inch home-camera vidicon: heavy trap-dominated lag.
    Vidicon,
    /// ZnSe/ZnCdTe 2/3-inch Newvicon: linear transfer, high capacitance, no traps.
    Newvicon,
    /// Se-As-Te 2/3-inch Saticon (H8397A): linear transfer, low dark current, traps.
    Saticon,
    /// CdSe Chalnicon: near-linear transfer, capacitive lag.
    Chalnicon,
}

/// One tube of the camera (one colour channel) at its operating point.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct TubeChannel {
    /// Signal current at peak white, nA.
    pub peak_signal_na: f64,
    /// Beam current, nA.
    pub beam_na: f64,
    /// Target capacitance, F.
    pub capacitance_f: f64,
    /// Beam temperature voltage, V.
    pub v_t: f64,
    /// Charge constant `K = C V_T` in target-equivalent nA s.
    pub k_nas: f64,
    /// Vertical beam spot Gaussian sigma in frame-line pitches.
    pub sigma_v_lines: f64,
    /// Horizontal beam spot Gaussian sigma in seconds.
    pub sigma_h_s: f64,
    /// Plate limit `C * target_voltage_v`, nA s.
    pub plate_nas: f64,
    pub dark_na: f64,
    pub bias_na: f64,
    /// Anti-comet-tail gun.
    pub act: bool,
    /// ACT cutting level, V (`ACT_LEVEL_V[k]` for `PlumbiconAct`, `0.0` otherwise).
    pub act_level_v: f64,
    /// Dynamic beam control (`Some(DBC)` for `PlumbiconDbc`, `None` otherwise).
    pub dbc: Option<Dbc>,
    /// Beam-bending strength (`0.0` from `TubeKind`; set by `CameraTube` from `CameraParams::blooming`).
    pub bending: f64,
    /// Light transfer exponent.
    pub gamma: f64,
    /// Whether the processing amplifier linearises `1 / gamma` before the knee.
    pub linearise: bool,
    /// Signal-plate (target) voltage, V.
    pub target_voltage_v: f64,
    /// Temperature step for dark-current doubling in deg C (`f64::INFINITY` for none).
    pub dark_doubling_c: f64,
    /// Photoconductor trap populations on this channel.
    pub traps: [Trap; MAX_TRAPS],
    /// Layer collection efficiency and held-charge parameters.
    pub collection: Collection,
    /// Reversible burn parameters.
    pub burn: Burn,
    /// Faceplate temperature at switch-on less operating temperature, deg C (`0.0` for none).
    pub cold_offset_c: f64,
    /// Fixed retention sub-step, s, for a numerical reference; `0.0` takes the held-charge step.
    pub retention_step_s: f64,
}

impl TubeChannel {
    /// Element potential above cathode for stored charge `q_nas` (nA s), V: `q / (capacitance_f * 1e9)`.
    pub fn potential_v(&self, q_nas: f64) -> f64 {
        q_nas / (self.capacitance_f * 1e9)
    }

    /// `target_voltage_v - q / (capacitance_f * 1e9)`, V.
    pub fn layer_voltage_v(&self, q_nas: f64) -> f64 {
        self.target_voltage_v - self.potential_v(q_nas)
    }

    /// Dark current at session time `t_s` (the warm-up law), nA.
    pub fn dark_na_at(&self, t_s: f64) -> f64 {
        if self.cold_offset_c == 0.0 || self.dark_doubling_c.is_infinite() {
            return self.dark_na;
        }
        let dt = self.cold_offset_c * (-t_s.max(0.0) / WARM_UP_S).exp();
        self.dark_na * (dt / self.dark_doubling_c).exp2()
    }

    /// ACT cutting level in target-equivalent charge, nA s: `capacitance_f * act_level_v * 1e9`.
    pub fn act_level_nas(&self) -> f64 {
        self.capacitance_f * self.act_level_v * 1e9
    }

    /// Beam-bending capture radius `rho(V)` for this channel's `bending`, in frame-line pitches.
    pub fn bend_radius_lines(&self, potential_v: f64) -> f64 {
        if self.bending <= 0.0 || potential_v <= V_BEND_ONSET_V {
            return 0.0;
        }
        (self.bending * (potential_v - V_BEND_ONSET_V) / V_BEND_ONSET_V)
            .min(BEND_RADIUS_MAX_LINES)
    }
}

fn dark_law(kind: TubeKind) -> Option<(f64, f64)> {
    match kind {
        TubeKind::Off => None,
        TubeKind::Plumbicon
        | TubeKind::PlumbiconBias
        | TubeKind::PlumbiconAct
        | TubeKind::PlumbiconDbc => Some((DARK_NA, f64::INFINITY)),
        TubeKind::Vidicon => Some((VIDICON_DARK_25_NA, VIDICON_DARK_DOUBLING_C)),
        TubeKind::Newvicon => Some((NEWVICON_DARK_25_NA, NEWVICON_DARK_DOUBLING_C)),
        TubeKind::Saticon => Some((SATICON_DARK_25_NA, SATICON_DARK_DOUBLING_C)),
        TubeKind::Chalnicon => Some((CHALNICON_DARK_25_NA, CHALNICON_DARK_DOUBLING_C)),
    }
}

/// Dark current of `kind`'s channels at `temperature_c` (deg C), nA, or `None` for `Off`.
pub fn dark_at(kind: TubeKind, temperature_c: f64) -> Option<f64> {
    let (dark_25, doubling) = dark_law(kind)?;
    if doubling.is_infinite() {
        Some(dark_25)
    } else {
        Some(dark_25 * ((temperature_c - 25.0) / doubling).exp2())
    }
}

impl TubeKind {
    /// The three channels `[R, G, B]` at `temperature_c` (deg C), or `None` for `Off`.
    /// `beam_ratio` scales the beam current to that multiple of each channel's peak-white current.
    pub fn channels_at(self, beam_ratio: f64, temperature_c: f64) -> Option<[TubeChannel; 3]> {
        let (dark_na, dark_doubling_c) = (dark_at(self, temperature_c)?, dark_law(self)?.1);
        let make_uniform = |peak: f64,
                            capacitance_f: f64,
                            gamma: f64,
                            linearise: bool,
                            target_voltage_v: f64,
                            traps: [Trap; MAX_TRAPS],
                            collection: Collection,
                            burn: Burn| {
            let ch = TubeChannel {
                peak_signal_na: peak,
                beam_na: beam_ratio * peak,
                capacitance_f,
                v_t: V_T_TRIODE_GUN_V,
                k_nas: capacitance_f * V_T_TRIODE_GUN_V * 1e9,
                sigma_v_lines: SIGMA_V_LINES,
                sigma_h_s: SIGMA_H_S,
                plate_nas: capacitance_f * target_voltage_v * 1e9,
                dark_na,
                bias_na: 0.0,
                act: false,
                act_level_v: 0.0,
                dbc: None,
                bending: 0.0,
                gamma,
                linearise,
                target_voltage_v,
                dark_doubling_c,
                traps,
                collection,
                burn,
                cold_offset_c: 0.0,
                retention_step_s: 0.0,
            };
            [ch, ch, ch]
        };
        match self {
            TubeKind::Off => None,
            TubeKind::Plumbicon
            | TubeKind::PlumbiconBias
            | TubeKind::PlumbiconAct
            | TubeKind::PlumbiconDbc => {
                let is_dbc = self == TubeKind::PlumbiconDbc;
                let v_t = if is_dbc { V_T_DIODE_GUN_V } else { V_T_TRIODE_GUN_V };
                Some(std::array::from_fn(|k| {
                    let capacitance_f = if is_dbc {
                        TARGET_CAPACITANCE_F[k] * V_T_TRIODE_GUN_V / V_T_DIODE_GUN_V
                    } else {
                        TARGET_CAPACITANCE_F[k]
                    };
                    TubeChannel {
                        peak_signal_na: PEAK_SIGNAL_NA[k],
                        beam_na: beam_ratio * PEAK_SIGNAL_NA[k],
                        capacitance_f,
                        v_t,
                        k_nas: capacitance_f * v_t * 1e9,
                        sigma_v_lines: SIGMA_V_LINES,
                        sigma_h_s: SIGMA_H_S,
                        plate_nas: capacitance_f * TARGET_VOLTAGE_V * 1e9,
                        dark_na,
                        bias_na: if self == TubeKind::PlumbiconBias {
                            LIGHT_BIAS_XQ1410_NA[k]
                        } else {
                            0.0
                        },
                        act: self == TubeKind::PlumbiconAct,
                        act_level_v: if self == TubeKind::PlumbiconAct {
                            ACT_LEVEL_V[k]
                        } else {
                            0.0
                        },
                        dbc: if is_dbc { Some(DBC) } else { None },
                        bending: 0.0,
                        gamma: GAMMA,
                        linearise: false,
                        target_voltage_v: TARGET_VOLTAGE_V,
                        dark_doubling_c,
                        traps: if k == 2 { PBO_BLUE_TRAPS } else { [Trap::NONE; MAX_TRAPS] },
                        collection: PBO_COLLECTION[k],
                        burn: Burn::NONE,
                        cold_offset_c: 0.0,
                        retention_step_s: 0.0,
                    }
                }))
            }
            TubeKind::Vidicon => Some(make_uniform(
                NEW_PEAK_SIGNAL_NA,
                VIDICON_CAPACITANCE_F,
                VIDICON_GAMMA,
                true,
                VIDICON_TARGET_VOLTAGE_V,
                VIDICON_TRAPS,
                Collection::FULL,
                VIDICON_BURN,
            )),
            TubeKind::Newvicon => Some(make_uniform(
                NEW_PEAK_SIGNAL_NA,
                NEWVICON_CAPACITANCE_F,
                NEWVICON_GAMMA,
                false,
                NEWVICON_TARGET_VOLTAGE_V,
                [Trap::NONE; MAX_TRAPS],
                Collection::FULL,
                Burn::NONE,
            )),
            TubeKind::Saticon => Some(make_uniform(
                NEW_PEAK_SIGNAL_NA,
                SATICON_CAPACITANCE_F,
                SATICON_GAMMA,
                false,
                SATICON_TARGET_VOLTAGE_V,
                SATICON_TRAPS,
                SATICON_COLLECTION,
                Burn::NONE,
            )),
            TubeKind::Chalnicon => Some(make_uniform(
                NEW_PEAK_SIGNAL_NA,
                CHALNICON_CAPACITANCE_F,
                CHALNICON_GAMMA,
                false,
                CHALNICON_TARGET_VOLTAGE_V,
                [Trap::NONE; MAX_TRAPS],
                Collection::FULL,
                Burn::NONE,
            )),
        }
    }

    /// The three channels `[R, G, B]` at 25 deg C, or `None` for `Off`. `beam_ratio` scales the
    /// beam current to that multiple of each channel's peak-white current.
    pub fn channels(self, beam_ratio: f64) -> Option<[TubeChannel; 3]> {
        self.channels_at(beam_ratio, 25.0)
    }

    /// Photoconductor material of each channel `[R, G, B]`, or `None` for `Off`.
    pub fn materials(self) -> Option<[Material; 3]> {
        match self {
            TubeKind::Off => None,
            TubeKind::Plumbicon
            | TubeKind::PlumbiconBias
            | TubeKind::PlumbiconAct
            | TubeKind::PlumbiconDbc => {
                Some([Material::PboExtendedRed, Material::Pbo, Material::Pbo])
            }
            TubeKind::Vidicon => Some([Material::Sb2s3; 3]),
            TubeKind::Newvicon => Some([Material::Newvicon; 3]),
            TubeKind::Saticon => Some([Material::Saticon; 3]),
            TubeKind::Chalnicon => Some([Material::Chalnicon; 3]),
        }
    }
}

/// Photo current for a linear scene light (1.0 = the light that gives peak white), nA.
pub fn photo_current(ch: &TubeChannel, light: f64) -> f64 {
    ch.peak_signal_na * light.max(0.0).powf(ch.gamma)
}

/// The scan of one element holding charge `q`: the charge it holds afterwards.
pub fn scan(beam: f64, scale: f64, q: f64) -> f64 {
    if beam <= 0.0 {
        return q;
    }
    // above the cathode potential the whole beam lands, up to the charge there is
    let landed = q.clamp(0.0, beam);
    let q = q - landed;
    let left = beam - landed;
    if left <= 0.0 {
        return q;
    }
    // below it, exp(-q'/K) = exp(-q/K) + left/K
    let x = -q / scale;
    if x < 700.0 {
        return -scale * (x.exp() + left / scale).ln();
    }
    // an element left far below the cathode potential would overflow the exponential: the
    // same sum as a log-sum-exp
    let y = (left / scale).ln();
    let m = x.max(y);
    -scale * (m + ((x - m).exp() + (y - m).exp()).ln())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn an_element_far_below_the_cathode_potential_still_scans() {
        let ch = TubeKind::Plumbicon.channels(BEAM_RATIO_DEFAULT).unwrap()[1];
        // exp(-q/K) is out of range here; the beam takes the element to about -K ln(beam/K)
        let after = scan(ch.beam_na, ch.k_nas, -10_000.0);
        assert!(after.is_finite() && after <= -10_000.0 && after > -10_001.0, "{after}");
    }

    #[test]
    fn the_scan_removes_all_the_charge_the_beam_can_reach() {
        // a charge below the beam is taken off entirely, and the rest of the beam only pushes
        // the element a few K below the cathode potential
        let after = scan(600.0, 5.64, 100.0);
        assert!(after < 0.0 && after > -40.0, "{after}");
        // a charge above the beam loses exactly the beam
        assert_eq!(scan(600.0, 5.64, 1000.0), 400.0);
    }
}
