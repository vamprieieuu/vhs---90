# CONTRACT

# The scanned camera tube

Design of the camera stage rebuilt from `docs/CAMERA_RESEARCH.md`. The tube is simulated as it
works: light charges a capacitive target continuously, and an electron beam reads it line by
line, field by field, at the instant each element passes under it. The camera's signal is the
current the beam lands, a continuous signal in time; every later process (head amplifier,
noise, correction, gamma, colour encoding) acts on that signal. The picture is written only
after the recorder, as for everything else in the project.

## Decisions (lead, 2026-11-13)

- Both camera classes. Shared physics first, then the single-tube stripe camera (home
  camcorders), then the further studio tube types. Trinicon waits for data.
- The scanned model replaces the per-element frame shortcut. It is slower; the camera stays off
  by default, so renders without a tube are unchanged.
- The scene is the input pictures, each held for its own time: a line integrates whatever
  pictures cover the interval since its elements were last read. Nothing is interpolated.

## Time and raster

- Time `t` is in seconds from the start of the session. Frame `n` covers
  `[n T_frame, (n + 1) T_frame)`; its two fields are read in raster order.
- Active frame row `r` is frame line `line` of field `p` (`video::row_to_line(r)`). Sample `x`
  of the row is read at `t = n T_frame + line / FH + (ACTIVE_START + x) / FS_BB`
  (`read_time_s`). Rows of field 0 are therefore read before rows of field 1, and within a
  field from the top down.
- With `push(f)` both fields see the picture `f`. With `push_fields(f0, f1)` field 0's interval
  `[n T_frame, n T_frame + T_frame / 2)` sees `f0` and the rest of the frame sees `f1`. There is
  no weaving: the target integrates each picture over its own interval.
- Each line integrates the light of the frame period before its read, so a frame's read holds
  light from the previous picture too (most of it in the top rows): the camera runs up to a frame
  behind the scene, as a tube camera does. Before the session (`t < 0`) the scene is the first
  picture, so a camera starting on a still scene reads it in every row of its first frame.

## Target

- Per colour channel, a grid of `ACTIVE_W` columns by `SUB_ROWS * ACTIVE_H` sub-rows
  (`SUB_ROWS = 4`): finer than the line pitch, so the beam's Gaussian profile can overlap the
  other field's lines.
- Charge is in target-equivalent nA s: an element of a target lit uniformly by a photocurrent of
  `I` nA (the current the whole target would give) gains `I * dt` in `dt` seconds. In these units
  the charge constant is `K = C * V_T` (nC = nA s) and the plate limit is `C * V_target`.
- Between two visits of the beam an element gains `(j_photo + j_dark + j_bias) * dt`, the photo
  term integrated exactly over the pictures that cover the interval, up to the plate limit.

## Beam

- Each row read is one pass of the beam along the row centre. Its vertical profile is a
  Gaussian of `sigma_v` (in frame-line pitches, per channel) over the sub-rows, truncated at
  3 sigma and normalised so that a uniform target receives, over one frame, beam charge
  `I_b * T_frame` per element (target-equivalent units).
- Landing on each sub-element is the existing exact scan of `tube.rs`: the whole beam share above
  the cathode potential, then `exp(-q'/K) = exp(-q/K) + left/K` below it, with the share of beam
  charge as `left`.
- The ACT gun, when fitted, discharges highlights during the line flyback before each pass over a
  span of lines (see "Highlights"; this replaces the clip at the stabilisable charge).
- Beam charge is in the same target-equivalent units as the stored charge. With `g` the Gaussian
  and `S_k` the sum of `g` over every pass of a frame (both fields) at sub-row `k`, the pass of row
  `r` offers sub-row `k` the beam charge `I_b * T_frame * g(y_k - c_r) / S_k`.
- The signal of the row at sample `x` is the charge landed on column `x` over all sub-rows during
  the pass divided by `SUB_ROWS * T_frame`, so that a uniform steady photocurrent `I` reads
  `I + j_dark + j_bias`. `TargetSignal` holds these currents in nA: no black level is subtracted
  and nothing is normalised to peak white there (that is the electronics' job).
- The horizontal spot is the beam's width moving at scan speed: the row's signal is convolved in
  time with a Gaussian of `sigma_h_s` seconds (per channel). This acts on the line signal in
  time, before any picture exists.

## Interfaces

In `camera.rs` (both engines compile it):

```rust
/// Signal currents read from the targets in one frame: per active frame row, per channel, one
/// value per baseband sample of the active line, nA (before the head amplifier).
pub struct TargetSignal { pub rows: Vec<[Vec<f32>; 3]> }

/// Seconds from the session start at which the beam centre reads sample `x` of active row `r`
/// in frame `n`.
pub fn read_time_s(n: i64, r: usize, x: usize) -> f64;

impl CameraTube {
    /// Read one frame from photocurrents: `photo(ch, r, x, t)` is the target-equivalent photo
    /// current density, nA, at element (row r, sample x) at time t. It is constant over each
    /// field's interval (the pictures are held), so the engine integrates it in pieces split at
    /// the field boundaries, evaluating it once inside each piece. Advances the frame counter.
    pub fn read_currents(&mut self, photo: &(dyn Fn(usize, usize, usize, f64) -> f64 + Sync)) -> TargetSignal;
    /// Read one frame from pictures: field 0's interval sees `f0`, field 1's sees `f1`.
    pub fn read(&mut self, f0: &Frame, f1: &Frame) -> TargetSignal;
    /// The picture path: `read`, then the camera electronics, then the encoder.
    pub fn encode_fields(&mut self, f0: &Frame, f1: &Frame) -> Baseband;
    /// `encode_fields(f, f)`.
    pub fn encode_baseband(&mut self, f: &Frame) -> Baseband;
    /// The channels as built (operating point, `K`, spot widths).
    pub fn channels(&self) -> &[TubeChannel; 3];
}
```

`Session::push_fields(f0, f1)` calls `encode_fields(f0, f1)` directly.

## Tube constants in seconds

- The charge constant is `K = C V_T` (nA s) with both factors physical. `V_T = k T_b / e` is set by
  the gun: about 0.17 V for the triode guns with a crossover of the three Plumbicon types
  (1900 to 2350 K) and about 0.10 V for a diode gun (about 1200 K); the ACT gun is a triode.
  `C` is the target capacitance per colour (whole target, F), fitted on the scanned engine to the
  handbook lag and checked against the 1000 to 2000 pF of the data. A colour's target is the same
  in the XQ1020, XQ1410 and XQ1520, so `C` and `V_T` are shared by the three types.
- The plate limit is `C * TARGET_VOLTAGE_V`, so it follows the fitted `C`.
- No constant depends on the frame or field period: 50 and 60 Hz use the same values.
- The per-frame engine (`frame_step`, `dark_equilibrium`, `lag_measurement`, the charge scales
  in nA, `saturation_na`) is retired; the data-sheet tables move into the tests. A target starts at
  dark equilibrium by scanning it dark from the cathode potential until it settles.

In `camera.rs`, the measurement surface of the data sheets: one column of a uniformly lit target,
read by the same scan as `CameraTube` (sub-rows, beam passes at their read times, ACT clip, plate
limit) on column `ACTIVE_W / 2`.

```rust
pub struct UniformColumn { /* sub-row charges and landing times of one column, frame counter */ }
impl UniformColumn {
    /// One channel's target at dark equilibrium; the next read is frame 0.
    pub fn new(ch: &TubeChannel) -> Self;
    /// Read the next frame with photocurrent `photo(t)`, nA, the same over the whole target
    /// and held within each field's interval (evaluated once per piece, as `read_currents`).
    /// Returns the signal of every active row, nA, as `TargetSignal` would hold it.
    pub fn read(&mut self, photo: &dyn Fn(f64) -> f64) -> Vec<f64>;
}
```

In `tube.rs`, `TubeChannel` carries `capacitance_f` (F) and `v_t` (V), with `k_nas` and `plate_nas`
derived from them, and `TubeKind::channels(beam_ratio)` no longer takes a scan time.

## Camera electronics

The head amplifier and the processing amplifier act on the line signal, as the camera's circuits
did: after the tube's read and before the encoder. Source: `docs/CAMERA_RESEARCH.md`, "Head-amplifier
noise and processing lie mostly above 2.5 MHz". Order (US 4032974): head amplifier (noise, band),
clamp, gain to peak white per channel, knee, gamma, aperture correction, clip. The colour matrix
waits for the spectral taking characteristics of the target materials, and flare and shading with their corrections
go with the optics and geometry: without the error they correct, they would only add a fault.

- **Head amplifier.** The tube is a current source into the input capacitance `C_in` of tube, coil,
  FET and strays. The amplifier adds a noise current of one-sided density
  `S(f) = 2 q I + 4 k T / R_f + (2 pi f C_in)^2 4 k T R_n` (A^2/Hz): the shot noise of the signal
  current `I` (the sample's own current, so it follows the picture), the feedback resistor's thermal
  noise, and the FET's voltage noise across `C_in`, which rises in amplitude with frequency (the
  "triangular" spectrum). Signal and noise then pass the amplifier's band: the magnitude of an analog
  second-order Butterworth low-pass, `|H(f)| = 1 / sqrt(1 + (f / band_hz)^4)`, with its delay
  compensated (the camera matches the delays of its channels), realised as a short linear-phase FIR on
  the line signal whose ends repeat their end sample. A bilinear-transform biquad is not used: with its
  corner at 0.38 of the sample rate its poles sit at radius 0.6 and a highlight rings about three times
  longer than through the analog amplifier. The noise is generated along the line signal, independent from
  row to row, channel to channel and frame to frame, and fixed by `(seed, frame, row, channel)` through
  `rng::Rng` stream `STREAM_CAMERA = 61`. Its measured density must follow `S(f) |H(f)|^2`.
- **Clamp.** The dark reading of the settled target (`black`, as now) is subtracted: a capped camera
  reads 0 on average.
- **Gain.** Each channel is divided by its peak-white current, so peak white is 1.0 (white balance on
  a white scene).
- **Knee.** On the linear signal, per channel. With a knee range `R` (the 1984 practice: linear to
  100 % of rated light, 230 % reaching the white clip, US 4470067), `L <= 1` is unchanged and above it
  `1 + (L - 1) s`, `s = (gamma_inverse(WHITE_CLIP) - 1) / (R - 1)`. Without a knee the signal goes to the
  gamma corrector as it is and clips there.
- **Gamma.** `gamma_correct(L)`: `4.5 L` below `0.018` (negative values included), `1.099 L^0.45 - 0.099`
  above, with no clamp (the clip comes last). Equal to `tube::oetf` on `[0, 1]`.
- **Aperture correction.** From the gamma-corrected green signal, added equally to all three channels,
  so edges get no coloured fringe from the corrector. Horizontal: `d_h(t) = s(t) - (s(t - T) + s(t + T)) / 2`
  with the delay `T` in seconds (fractional delays interpolated on the line signal; the line ends
  repeat their end sample). Vertical: `d_v(r) = s(r) - (s(r - 2) + s(r + 2)) / 2` over frame rows, that
  is the lines one line period before and after in the same field (1H delays); rows without such a
  neighbour use their own value. Coring removes small detail, `core(d) = sign(d) max(|d| - c, 0)`.
  The detail added is `k_h core(d_h) + k_v core(d_v)`, so the horizontal response is
  `1 + k_h (1 - cos 2 pi f T)` and the vertical one `1 + k_v (1 - cos 2 pi nu)` for `nu` in cycles per
  field line. The horizontal gain restores the green channel's response at `1 / (2T)`:
  `k_h = (1 / (M(f_p) |H(f_p)|) - 1) / 2` with `M(f) = exp(-2 pi^2 sigma_h^2 f^2)` the spot's response,
  `H` the head amplifier's and `f_p = 1 / (2T)`.
- **Clip.** `[BLACK_CLIP, WHITE_CLIP] = [-0.03, 1.05]` of peak white, after the detail is added (so a
  highlight's overshoot is clipped and its dark undershoot stays).

Profiles (`CameraParams::processing`, config key `processing`, default `studio`):

| | studio (three-tube broadcast camera) | consumer (home camera) |
|---|---|---|
| `C_in`, `R_n`, `R_f`, `T` | 20 pF, 75 ohm, 1 Mohm, 300 K (ranges 15 to 30 pF, 50 to 100 ohm, 0.3 to 1 Mohm) | 30 pF, 100 ohm, 470 kohm, 300 K (the poor end of the ranges: about 45 dB at 300 nA, as the PK-958's "> 45 dB") |
| band (-3 dB) | 5.5 MHz (flat to about 5 MHz, US 3743766) | 5.5 MHz |
| knee range | 2.3 | none [est] |
| aperture `T`, `k_v`, `c` | 150 ns (BBC), 0.5 (`g` = 0.25, BBC Monograph 47), 0.008 [est] | 200 ns (0.2 us line, Panasonic), 0, 0.015 [est] |

Interfaces, in a new engine module `camera_processing.rs`:

```rust
pub const STREAM_CAMERA: u64 = 61;
pub const WHITE_CLIP: f64 = 1.05;
pub const BLACK_CLIP: f64 = -0.03;
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "kebab-case")]
pub enum Processing { #[default] Studio, Consumer }
pub struct HeadAmpSpec { pub input_c_f: f64, pub r_n_ohm: f64, pub r_f_ohm: f64, pub temp_k: f64, pub band_hz: f64 }
pub struct ApertureSpec { pub delay_s: f64, pub vertical: f64, pub coring: f64 }
pub struct ProcessingSpec { pub head_amp: HeadAmpSpec, pub knee_range: Option<f64>, pub aperture: ApertureSpec }
impl Processing { pub fn spec(self) -> ProcessingSpec; }
pub fn gamma_correct(l: f64) -> f64;
pub fn gamma_inverse(v: f64) -> f64;
/// `k_h` of the restoration rule.
pub fn horizontal_gain(delay_s: f64, sigma_h_s: f64, band_hz: f64) -> f64;
/// Gamma-corrected video of one frame: per active row, per channel, per sample; 1.0 = peak white.
pub struct CameraVideo { pub rows: Vec<[Vec<f32>; 3]> }
pub struct HeadAmplifier { .. }
impl HeadAmplifier {
    pub fn new(spec: &HeadAmpSpec, seed: u64) -> Self;
    /// Noise and band on every row of `sig` (nA in, nA out), for frame `n`.
    pub fn amplify(&self, sig: &mut TargetSignal, n: i64);
}
pub struct VideoProcessor { .. }
impl VideoProcessor {
    /// `black` is the clamp level per channel, nA; `k_h` from the green channel's spot.
    pub fn new(processing: Processing, channels: &[TubeChannel; 3], black: [f64; 3]) -> Self;
    pub fn horizontal_gain(&self) -> f64;
    /// Clamp, gain, knee, gamma, aperture correction, clip.
    pub fn process(&self, sig: &TargetSignal) -> CameraVideo;
}
```

In `camera.rs`: `CameraParams` gains `processing: Processing`; `CameraTube::seeded(params, seed)`
(`new(params)` is `seeded(params, 0)`; `Session` passes its seed); `CameraTube::black() -> [f64; 3]`;
`CameraTube::video(f0, f1) -> CameraVideo` (read, amplify with the frame index of the read, process).
`encode_fields` encodes `video`; `process_frame` writes `video(f, f)` as `(255.0f32 * v).round().clamp(0.0, 255.0) as u8`.

## Target materials and traps

Source: `docs/CAMERA_RESEARCH.md`, "Six photoconductors differ in gamma, dark current and lag
mechanism" and "Traps add an exposure-dependent tail". Four further targets join the lead-oxide
Plumbicon. Each is a material: its transfer exponent, its dark current and how fast that grows with
temperature, its layer capacitance, and its traps. Until the single-tube stripe camera, a camera with
one of them carries three identical tubes behind the prism.

- **Kinds** (`TubeKind`, config names in brackets): `Vidicon` (`vidicon`, Sb2S3, a 2/3-inch home
  camera tube), `Newvicon` (`newvicon`, ZnSe/ZnCdTe, 2/3 inch), `Saticon` (`saticon`, Se-As-Te,
  the low-lag H8397A, 2/3 inch, with its lag fitted to RCA's equivalent, the BC4908), `Chalnicon`
  (`chalnicon`, CdSe). The three Plumbicon kinds keep
  every constant they have, except blue's capacitance (below).
- **Transfer.** `photo_current = peak * light^gamma` with `gamma` per channel: PbO 0.95, Sb2S3 0.65
  (the home camera's tube, Panasonic WV-3110), Newvicon 1.0, Saticon 1.0, Chalnicon 0.925 (0.90 to
  0.95). The camera's gamma corrector is set for its tube: a vidicon camera's correction gamma is about
  0.7 on a 0.65 tube, an overall law near 0.45, so for a channel with `linearise` the processing
  amplifier raises the clamped, gained signal to `1 / gamma` (keeping its sign) before the knee. Only the
  vidicon linearises; the 0.95 of PbO and Chalnicon was left in by the cameras.
- **Operating points** (all channels of a non-PbO kind alike): peak signal 200 nA (H8397A 0.2 uA,
  S4075 0.2 uA, home vidicons 200 nA; Chalnicon [est]). Target voltage: PbO 45 V, Saticon 50 V,
  Chalnicon 35 V, Newvicon 25 V [est, 10 to 35 V], vidicon 30 V [est, 10 to 60 V]. Every gun is a
  triode: `V_T = V_T_TRIODE_GUN_V`. The beam is `beam_ratio` times peak as now (the Saticon's
  practice of 3 to 4 times is used in its lag measurement).
- **Dark current against temperature.** `dark(T) = dark_25 * 2^((T - 25) / doubling)`, `T` in deg C.
  Saticon: through the H8397A curve, 0.15 nA at 20 deg C and 2.7 nA at 60 deg C. Chalnicon: 0.1 / 1 / 10
  nA at 0 / 30 / 60 deg C. Newvicon: 5 nA at 25 deg C, doubling every 7.5 deg C (7 to 8). Vidicon: 20 nA
  at 25 deg C (the voltage is set for it), doubling every 7.5 deg C. PbO: 1 nA, no temperature law (none
  is published). `CameraParams.temperature_c` (default 25) sets it; `TubeKind::channels(beam_ratio)` is
  `channels_at(beam_ratio, 25.0)`.
- **Capacitance.** One `C` per material (per colour for PbO), fitted on the scanned engine with
  `V_T = 0.17` together with the traps. Checks against the data: Saticon within 0.58 to 1.16 nF (1000
  to 2000 pF/cm^2 on 6.6 x 8.8 mm); Newvicon more than twice PbO green ("much larger target
  capacitance"); PbO blue within 15 % of the mean of red and green (its excess was standing in for trap
  lag). Red and green PbO stay as fitted.
- **Traps.** Up to `MAX_TRAPS = 2` populations per channel, each with a capture share `f` (0 to 1),
  a capacity `N` (target-equivalent nA s) and a release time `tau` (s). Light (the photocurrent and the
  bias light; the dark current is excluded) of current `j` at an element fills them:
  `dn/dt = f j (1 - n / N) - n / tau`, and the element stores the light less what the traps keep:
  over an interval in which `j` is constant the element gains `j dt - sum(n_end - n_start)`, with
  `n(t) = n_inf + (n_0 - n_inf) exp(-b t)`, `b = f j / N + 1 / tau`, `n_inf = f j / b`. Charge is
  conserved: all trapped charge arrives, late. Each sub-element carries its own `n`, settled with the
  dark settle (bias light on) before frame 0. Traps: PbO blue, Saticon, vidicon. None in PbO red and
  green ("negligible"), Newvicon ("free of photoconductive lag") and Chalnicon ("mainly capacitive").
- **Fit data** (`tests/camera_materials.rs`; the RCA procedure is a 30 nA signal, 5 nA bias, a
  100 nA beam, 60 Hz, a dark target exposed for a number of fields, lag in percent of the signal
  current): XQ1410B blue floor at 200 ms against bias (20/300 nA, 50 Hz); XQ1427B blue exposure
  family; the BC4391 Saticon exposure family by its shape only (each reading as a share of the
  256-field one); the BC4908 Saticon family at 50 ms (30 / 50 / 200 nA with 0 / 5 / 10 nA bias,
  800 nA beam, 60 Hz; RCA 1977, Neuhauser 1978 Fig. 12, `docs/design/CAMERA_RETENTION.md`), which
  sets the Saticon's `C` (0.80 nF) and traps; the two RCA Saticon families cannot both be met in level,
  and neither can the H8397A's unbiased 6.7 % at 0.2 uA (Fig. 6); Newvicon S4075 10 % at
  50 ms (0.2 uA, 60 Hz) and T12 8 to 13 % at 60 ms (50 Hz); Chalnicon 10 % at 50 ms (200 nA, 60 Hz
  [est conditions]); home vidicon 20 % at 50 ms (200 nA, 20 nA dark, 60 Hz), XQ1240 8 % at 200 ms, and
  the bias cut of RCA's vidicon (13 to 9.2 % at 50 ms, 3.3 to 1.2 % at 200 ms with 20 nA).

Interfaces, in `tube.rs`:

```rust
pub const MAX_TRAPS: usize = 2;
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Trap { pub capture: f64, pub capacity_nas: f64, pub release_s: f64 }
impl Trap { pub const NONE: Trap; }
// TubeChannel gains: gamma, linearise, target_voltage_v, dark_doubling_c (f64::INFINITY for none),
// traps: [Trap; MAX_TRAPS]; plate_nas = capacitance_f * target_voltage_v * 1e9.
impl TubeKind {
    pub fn channels_at(self, beam_ratio: f64, temperature_c: f64) -> Option<[TubeChannel; 3]>;
}
/// Dark current of `kind`'s channels at `temperature_c`, nA.
pub fn dark_at(kind: TubeKind, temperature_c: f64) -> Option<f64>;
pub fn photo_current(ch: &TubeChannel, light: f64) -> f64; // peak * light^ch.gamma
```

`UniformColumn` and `CameraTube` integrate the traps in the same pieces as the light. `GAMMA` stays as
the PbO value.

## Highlights

Source: `docs/CAMERA_RESEARCH.md`, "Beam capacity of twice peak white turns highlights into comet
tails". A highlight the read beam cannot take off leaves charge on the target. Three things decide
what then happens: the gun (a flyback that discharges it, or a beam that rises to meet it), the
element's potential (an unstabilised element bends the beam towards itself), and the target (how much
it stores and in which colour). All three act on the target and the beam during the scan; nothing is
added to the picture.

### Decisions

- **ACT flyback over a span of lines.** The read-time clip of the ACT gun is replaced by a flyback
  before every pass: a defocused beam spread over a band of lines discharges every element above the
  cutting level, so it reaches highlights a few lines before the read beam does (US 4,143,305). Only
  active lines have a pass, so only they have a flyback: an overload at the top of the picture sees one
  flyback before it is read instead of several.
- **Dynamic beam control as its own kind.** A diode gun with dynamic beam control joins as
  `TubeKind::PlumbiconDbc`. Its beam follows the Ampex law (US 4,229,767) along the line. The diode
  gun's cooler beam, `V_T = 0.10 V`, comes with a target of more capacitance: the XQ3440 (diode) and
  XQ1520 (ACT triode) print the same low-key lag (Mullard 1987), so `C` is chosen to keep `K = C V_T`.
- **Beam bending replaces the lateral spill.** The spill moved photocurrent sideways in the input
  picture before the target. Light does not move; the beam does. The spill was keyed on the light of
  the moment, so it vanished as soon as the light went although the element stayed charged, and no gun
  could cut it. Beam bending is keyed on the stored potential of each element (Philips 1963: an
  unstabilised element "will start to attract electrons towards it when the beam approaches, and
  continue to do so after the beam should have passed"), so a bloom lasts as long as its comet tail,
  and ACT and dynamic beam control, which hold the potential down, cut it as the data say. The config
  key `blooming` stays and now sets the bending strength; `0.0` (the default) is off.
- **Trail colour needs no new mechanism.** It follows from each channel's capacitance, beam and traps:
  the red and blue PbO tubes run half the green current with half its beam on targets of about the
  same capacitance, so they store more overload per unit of beam and their tails outlast green's (red
  over green, "red-trail", RCA 1977); a Saticon camera has three identical tubes, so its residue is
  white, and its low lag clears it within about 2 s at a beam of 3 to 4 times the highlight (RCA 1977,
  Neuhauser 1978). Which of red and blue clears last depends on blue's capacitance (fitted in "Target
  materials and traps", 1.32 nF with its traps) and is not judged.

### One pass with the highlight physics

Each pass of row `r` in frame `n` (per channel, with `c_r = r + 0.5` and `y_k = (k + 0.5) / SUB_ROWS`
in frame-line pitches) runs, in this order:

1. **Flyback** (ACT only). At `t_fb = n T_frame + line / FH` (the start of the pass's line, `line` from
   `row_to_line(r)`, the same for every column), every element whose sub-row lies in the span
   `|y_k - c_r| <= h_act`, `h_act = ACT_SPAN_PH * ACTIVE_H / 2`, is integrated to `t_fb` by the usual
   rule (light in held pieces, dark, bias, traps, plate limit) and, if `q > q_act`,
   `q <- q_act + scan(b_fb, K, q - q_act)`; an element at or below `q_act` is left exactly as it is.
   The charge landed in flyback goes to the head amplifier during blanking and is not in
   `TargetSignal`.
2. **Integration** of every element of the pass's band (the sub-rows with a beam share in this pass)
   to its read time `read_time_s(n, r, x)`, as now. With bending on, the elements within
   `ceil(BEND_RADIUS_MAX_LINES * SUB_ROWS)` sub-rows above and below the band are integrated to the same
   times too (they may capture beam).
3. **Beam current per column** `I_b(x)`: `beam_na`, or the dynamic beam control's (below).
4. **Aims.** The beam aimed at element `(x, k)` of the band is `b(x, k) = I_b(x) T_frame w_rk` with
   `w_rk` the existing share `g(y_k - c_r) / S_k`.
5. **Capture** (bending on). Each aim lands on its captor (below), otherwise on its own element.
6. **Landing.** Each element receives `B_e`, the sum of the aims it captures, and lands once:
   `after = scan(B_e, K, before)`, `landed_e = before - after`. Elements of the band that receive no
   beam keep their charge.
7. **Signal.** `landed_e` is shared among the aims `e` captured in proportion to their `b`, and each
   share counts at the aim's column (the beam was there when it landed). Row `r` at column `x` reads the
   sum of the shares of the aims at column `x` divided by `SUB_ROWS * T_frame`, and the horizontal spot
   follows as now.

Without ACT, dynamic beam control or bending, steps 1, 3 and 5 do nothing and the pass is the present
one, bit for bit. Bending couples the columns of a pass, so with bending on the passes run row by row
(all columns of a pass, then the next pass); without it the column-by-column engine may stay.
`UniformColumn` runs the same pass on its one column; its neighbours are lit alike, so a capture by a
neighbouring column never beats one by its own column, and only captors in its own column count.

### ACT flyback

- Cutting level per channel `[R, G, B]`: `ACT_LEVEL_V = [3.5, 7.0, 3.5]` V, the XQ1520 cathode-pulse
  presets (Mullard 1987) [DS]. In charge, `q_act = C * act_level_v * 1e9` nA s (green 8.05, red 4.83).
- Flyback beam: `ACT_BEAM_NA = 100 000` nA, grid 1 raised to about 100 uA (Mullard 1987) [DS], all of it
  taken to reach the target [est]; for `ACT_FLYBACK_S = 8.0e-6` s [est, inside the 10.9 to 12 us line
  blanking]; spread evenly over a band of `ACT_SPAN_PH = 1/30` of the picture height [est: "many
  scanning lines", US 4,143,305; 16 lines at 525, 19.2 at 625]. Each element of the span is offered,
  per flyback, `b_fb = ACT_BEAM_NA * ACT_FLYBACK_S / ACT_SPAN_PH = 24.0` nA s in target-equivalent
  units, the same in both standards.
- Why these sizes [DER]. With `h_act` = 8 lines (525) an element is in the span of 8 passes of each
  field, 4 before its read and 4 after. A highlight's element is at the plate by the time the span
  reaches it (32 times peak white fills it in about 5 ms). Four flybacks (96 nA s) bring a full element
  to the cutting level in every channel (plate less `q_act`: green 43.7, red 57.3, blue at most 73 nA s),
  so the read beam finds at most `q_act` and the flybacks after it take what the read left: no tail at
  any overload, which covers the data's 32 times over a spot of 10 % of the picture height (5 stops).
  One flyback (24 nA s) does not: the first lines of each field are read with their overload in place,
  which is US 4,143,305's "not pre-discharged" at the top of the picture.
- With the cutting levels below the 10 V bending onset, ACT also cuts blooming wherever it has
  pre-discharged the highlight, and not at the top of the picture (US 4,143,305).

### Dynamic beam control

- `TubeKind::PlumbiconDbc` (config `plumbicon-dbc`): the PlumbiconAct channel with `act = false`,
  `v_t = V_T_DIODE_GUN_V`, `capacitance_f` times `V_T_TRIODE_GUN_V / V_T_DIODE_GUN_V` (1.7), so `k_nas`
  is unchanged and the plate charge grows by 1.7; `k_nas` and `plate_nas` recomputed from them as for
  every kind; `dbc = Some(DBC)`. The rest of the PbO material (gamma, dark current, traps) as the other
  Plumbicon kinds.
- The Ampex law: the beam stays at `beam_na` until the video passes `beam_na - headroom` and then
  stays `headroom` above it, up to the gun's largest beam. `DBC.headroom = 0.5` of `beam_na` (300 nA
  over a 600 nA base, US 4,229,767) [DS], `DBC.max_ratio = 17` times `peak_signal_na` [DER: 16 times
  peak white handled (Mullard 1987, 4 stops) needs a beam of 16 times peak plus the headroom of one
  peak]. The loop is taken as fast as the scan [est]; its delay is out of scope.
- Per column `x` of the pass, with the band's charges after step 2:
  `I_v(I) = sum_k (q_k - scan(I T_frame w_rk, K, q_k)) / (SUB_ROWS T_frame)` (the column's own reading,
  before any capture), `head = headroom * beam_na`, `max = max(max_ratio * peak_signal_na, beam_na)`,
  `g(I) = I - head - I_v(I)`. If `g(beam_na) >= 0`, `I_b = beam_na`; else if `g(max) <= 0`,
  `I_b = max`; else bisect on `[beam_na, max]` keeping `g(lo) < 0 <= g(hi)` until
  `hi - lo <= DBC_TOLERANCE_NA` (0.01 nA) and take `I_b = hi`.
- Consequence: a steady light reads linearly to 16 times peak white, a highlight up to 16 times leaves
  no tail, and beyond it the beam stops at `max`. In red and blue a full element holds more than one
  pass of the largest beam takes, so a stop beyond (32 times) leaves a tail; a green element at the
  plate fits in one pass and is cleared (the plate caps what it can store).

### Beam bending

- An element's potential above the cathode is `V = q / (capacitance_f * 1e9)` V (`q` in nA s). Above
  the onset `V_BEND_ONSET_V = 10` V (Philips kept the swing within about 10 V because more "deflects
  the approaching beam excessively", Philips 1963) [DS], it captures the beam aimed within its radius
  `rho(V) = min(bending * (V - V_BEND_ONSET_V) / V_BEND_ONSET_V, BEND_RADIUS_MAX_LINES)` frame-line
  pitches, `0` at or below the onset. `bending` is the free strength (no extent data exist, research
  "Open gaps"); at `bending = 1` an element at the 45 V plate reaches 3.5 lines. `BEND_RADIUS_MAX_LINES
  = 4` bounds the search [est].
- Distance between element `(x', k')` and aim `(x, k)`:
  `d = sqrt(((x' - x) p_h)^2 + ((k' - k) / SUB_ROWS)^2)`, `p_h = TARGET_ASPECT * ACTIVE_H / ACTIVE_W`
  (`sample_pitch_lines()`; 0.849 at 525, 1.038 at 625), `TARGET_ASPECT = 4/3`.
- The captor of an aim is, among the elements with `V > V_BEND_ONSET_V` and `d <= rho(V)` (an aim's own
  element if it is above the onset, at `d = 0`), the nearest; ties go to the higher `V`, then the smaller
  `x'`, then the smaller `k'`. An aim with no such element lands on its own element. The potentials are
  those after steps 1 and 2: each element's own read-time charge stands for its potential both as the
  beam approaches and after it has passed [est; an element that bends the beam is beyond what one pass
  stabilises, so it is still positive after the pass].
- `CameraParams.blooming` sets `bending` on every channel (negative values count as 0). `TubeKind`
  channels carry `bending = 0.0`.

### Defaults and what stays

- The camera off is untouched (the golden test). The default `blooming = 0.0` leaves `Plumbicon`,
  `PlumbiconBias` and the material kinds bit-identical to the engine before this stage. `PlumbiconAct`
  changes where highlights exceed the cutting level; below it, its readings are the plain gun's up to
  rounding, so its lag is unchanged.
- `apply_blooming`, `BLOOMING_MAX`, `BLOOM_NEAR_SHARE` and `BLOOM_FAR_SHARE` go; `frame_photo` maps
  levels to photocurrent only. `docs/CAMERA.md`'s paragraphs on ACT and blooming are rewritten to match.
  The command line and GUI learn `plumbicon-dbc` in "Front ends".
- Cost: the ACT flyback integrates about four times as many elements per pass; bending runs row by row.

### Interfaces

In `tube.rs`:

```rust
/// ACT cutting level per channel `[R, G, B]`, V (XQ1520 cathode-pulse presets).
pub const ACT_LEVEL_V: [f64; 3] = [3.5, 7.0, 3.5];
pub const ACT_BEAM_NA: f64 = 100_000.0;
pub const ACT_FLYBACK_S: f64 = 8.0e-6;
/// Height of the flyback band, as a fraction of the picture height.
pub const ACT_SPAN_PH: f64 = 1.0 / 30.0;
pub const V_BEND_ONSET_V: f64 = 10.0;
pub const BEND_RADIUS_MAX_LINES: f64 = 4.0;
pub const TARGET_ASPECT: f64 = 4.0 / 3.0;
pub const DBC_TOLERANCE_NA: f64 = 0.01;
/// Dynamic beam control: `headroom` as a fraction of `beam_na`, the largest beam as a multiple of
/// `peak_signal_na`.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Dbc { pub headroom: f64, pub max_ratio: f64 }
pub const DBC: Dbc = Dbc { headroom: 0.5, max_ratio: 17.0 };
// TubeKind gains (config name in brackets):
//     PlumbiconDbc ("plumbicon-dbc"): diode gun with dynamic beam control.
// TubeChannel gains:
//     act_level_v: f64   // ACT_LEVEL_V[k] for PlumbiconAct, 0.0 for every other kind
//     dbc: Option<Dbc>   // Some(DBC) for PlumbiconDbc only
//     bending: f64       // 0.0 from TubeKind; set by CameraTube from CameraParams::blooming
impl TubeChannel {
    /// `q / (capacitance_f * 1e9)`, V.
    pub fn potential_v(&self, q_nas: f64) -> f64;
    /// `capacitance_f * act_level_v * 1e9`, nA s.
    pub fn act_level_nas(&self) -> f64;
    /// `rho(V)` of this channel's `bending`, frame-line pitches.
    pub fn bend_radius_lines(&self, potential_v: f64) -> f64;
}
```

In `camera.rs` (both engines):

```rust
/// Charge offered to each element of the span per flyback, nA s:
/// `ACT_BEAM_NA * ACT_FLYBACK_S / ACT_SPAN_PH`.
pub fn act_flyback_nas() -> f64;
/// Horizontal sample pitch in frame-line pitches: `TARGET_ASPECT * ACTIVE_H / ACTIVE_W`.
pub fn sample_pitch_lines() -> f64;
impl UniformColumn {
    /// Read the next frame with photocurrent `photo(r, t)`, nA, the light of frame row `r` (each
    /// sub-row takes its row's light, as `CameraTube`), held within each field's interval. `read(p)`
    /// is `read_rows(|_, t| p(t))`.
    pub fn read_rows(&mut self, photo: &dyn Fn(usize, f64) -> f64) -> Vec<f64>;
}
```

`CameraParams.blooming` keeps its name and type; its doc becomes the bending strength.

### Out of scope

- The loop delay of dynamic beam control; spot growth, geometry shift and lag from a raised beam; the
  flyback current seen by the head amplifier; the ACT on vertical retrace lines (there is none).
- Bending's displacement of landing energy and any asymmetry between approach and departure; bending
  across channels.
- A red-specific trail mechanism (no data); highlight burn and sticking (Retention); optical flare and
  the anti-halation disc (Geometry).
- Highlight guns on the non-PbO kinds; return-beam tubes (ABO).

## Spectral taking and the colour matrix

Source: `docs/CAMERA_RESEARCH.md`, "Six photoconductors differ in gamma, dark current and lag
mechanism" (the spectral response of each material and the IR-cut filter it needs) and "Head-amplifier
noise and processing lie mostly above 2.5 MHz" (US 4032974's order; the linear matrix before gamma
with rows summing to 1; the BBC's Plumbicon matrix, Jones 1968). ITU-R BT.709 for the primaries, the
white and the luminance shares [DS]. The CIE 1931 2 degree observer through the analytic fit of Wyman,
Sloan and Shirley 2013 (JCGT 2(2), "multi-lobe"), within about 1 % of the tabulated functions [DS].

A camera does not see red, green and blue. Each tube sees the scene's light through the lens's IR-cut
filter and its arm of the prism, weighted by its photoconductor's spectral response; the processing
amplifier's linear matrix turns the three readings back into the system's primaries. Taking curves
are positive and overlap, so without the matrix a saturated colour is read partly in the wrong
channels: it comes out paler and turned toward its neighbour in the spectrum. This stage puts the
spectra on the light before the target and the matrix in the processing amplifier.

### Decisions

- **The scene is three spectra.** The input holds three numbers per element, so the scene has three
  degrees of freedom: the light of an element is `E(l) = sum_c L_c E_c(l)` over wavelength `l`, with
  `L_c` the linear light of primary `c` as now (`oetf_inverse(v / 255) * 2^iris_stops`). Each `E_c` is a
  smooth, non-negative spectrum with the CIE chromaticity of its BT.709 primary, scaled so that its
  luminance is the primary's share of white; the three then add to a white of D65's chromaticity.
  Light outside 380 to 780 nm is not in the picture and is not invented [est]: the IR-cut filter acts
  only on the far red inside the grid. A 3 x 3 matrix therefore inverts the taking exactly; there is no
  metameric residual (out of scope).
- **Shapes** [DER]: red is a long-pass (a red surface reflects from the orange on) on a small floor that
  pulls its chromaticity off the spectral locus to the primary's, green a Gaussian band, blue a
  short-pass. Each has two free numbers, solved so its `(x, y)` is the primary's (residual below
  0.0003).
- **Taking.** Channel `ch` with material `m`: `tau_ch(l) = ir_cut(l) prism_ch(l) response_m(l)`. No
  prism or IR-cut curve is in the research: the dichroic edges (480 and 580 nm, 5 nm wide) and the
  IR-cut (680 nm, 10 nm wide) are fitted so the Plumbicon camera's matrix comes out near the BBC's
  (within 0.075 in every term) [est, fitted to DS].
- **The red Plumbicon carries the red-extended layer** [DER]. With the standard layer, which is dark by
  650 nm, the red arm sees only 580 to 650 nm and needs a red row of `[1.69, -0.83, 0.14]`; with the
  extended-red layer it needs `[1.19, -0.23, 0.04]`, beside the BBC's `[1.14, -0.18, 0.04]` for the
  Plumbicon cameras of the day. Only the spectral response changes: the red channel keeps its fitted
  capacitance and no traps.
- **White balance on the light.** The operating point is set on white, as now: each row of the taking
  matrix is divided by its sum, so white (`L = (1, 1, 1)`) gives every tube its peak current. The
  absolute sensitivity (a Newvicon is 3 to 4 times a Plumbicon behind an IR filter) only sets the
  exposure needed and is out of scope.
- **The matrix is the inverse of the taking** and sits in the processing amplifier after the clamp, the
  gain, the correctors of "Geometry" and the linearising of a vidicon, before the knee and the gamma
  corrector (US 4032974: matrix before gamma; the knee then acts on matrixed channels). Its rows sum to
  1 because the taking's do. The studio profile carries it (US 4032974, Jones 1968); the consumer
  profile does not [est: the WV-3110 signal path has no linear matrix, and its decoder's
  `G = Y - (R + B) / 2` only separates the colours].
- **Off by default.** With `CameraParams.spectral = false` (the default) the taking is ideal (`T = I`,
  each channel sees its own primary, as now) and the matrix is the identity, which is not applied: the
  engine is bit for bit the one before this stage.

### Spectra

All spectra live on a grid of `N_LAMBDA = 81` wavelengths, `l_i = 380 + 5 i` nm, and every integral is
a sum over it. `long_pass(l, e, w) = 1 / (1 + exp(-(l - e) / w))`, `short_pass = 1 - long_pass`.

- Scene shapes: `S_R = 0.0118 + 0.9882 long_pass(l, 619.0, 15.0)`,
  `S_G = exp(-((l - 539.1) / 33.2)^2 / 2)`, `S_B = short_pass(l, 474.2, 18.8)`. Chromaticities (Wyman
  fit) `(0.6400, 0.3300)`, `(0.3002, 0.6001)`, `(0.1500, 0.0601)` against BT.709's `(0.64, 0.33)`,
  `(0.30, 0.60)`, `(0.15, 0.06)`.
- `E_c(l) = Y_c S_c(l) / sum_i ybar(l_i) S_c(l_i)` with `Y = BT709_LUMINANCE = [0.2126, 0.7152, 0.0722]`
  and `ybar` the Wyman fit's: white `sum_c E_c` sits at `(0.3129, 0.3292)` against D65's
  `(0.3127, 0.3290)`.
- Prism: `prism_R = long_pass(l, 580, 5)`, `prism_G = long_pass(l, 480, 5) short_pass(l, 580, 5)`,
  `prism_B = short_pass(l, 480, 5)`. IR-cut: `short_pass(l, 680, 10)`.
- Material responses, relative to their peak, linear between knots and 0 outside them (nm, response).
  Knots marked [FIG] are read from the research's figures (about +-10 nm); the rest are [est] fills of
  the curves' shape:

| Material (`Material`) | Knots |
|---|---|
| PbO (`Pbo`) | 380 0.55, 400 0.70, 450 0.92, **490 1.00** [FIG, peak 485-500], 550 0.72, 600 0.35, 630 0.10, **650 0.00** [FIG] |
| PbO, red-extended (`PboExtendedRed`) | 380 0.55, 400 0.70, 450 0.92, 490 1.00, 550 0.80, 600 0.55, 650 0.38, 700 0.25, **780 0.10** [FIG: tails to 850-950] |
| Sb2S3 (`Sb2s3`) | 380 0.40, 400 0.55, 450 0.80, **550 1.00** [FIG], 600 0.88, **675 0.50** [FIG], 700 0.36, 750 0.12, 780 0.05 |
| Newvicon (`Newvicon`) | 380 0.36, **400 0.38** [FIG, 220 of 575 mA/W], 500 0.55, 600 0.78, 700 0.96, **735 1.00** [FIG, 730-740], 780 0.92 |
| Saticon (`Saticon`) | 380 0.65, 400 0.80, **430 1.00**, **450 1.00** [FIG], 500 0.82, 550 0.62, **595 0.40** [FIG], 650 0.12, **700 0.03** [FIG], 750 0.00 |
| Chalnicon (`Chalnicon`) | 380 0.60, **400 0.64** [FIG, 0.30 of 0.47 uA/uW], 500 0.75, 600 0.87, **710 1.00** [FIG], 740 0.50, **770 0.02** [FIG], 780 0.00 |

- Materials of the kinds `[R, G, B]`: every Plumbicon kind `[PboExtendedRed, Pbo, Pbo]`; `Vidicon` Sb2S3,
  `Newvicon`, `Saticon` and `Chalnicon` their own material in all three.

### Equations

- Taking matrix: `T_raw[ch][c] = sum_i tau_ch(l_i) E_c(l_i)`, `T[ch][c] = T_raw[ch][c] / sum_c' T_raw[ch][c']`.
- Light on target `ch` (before the optics of "Geometry"): `l_ch = sum_c T[ch][c] L_c`; photocurrent
  `photo_current(ch, l_ch)` as now. White and every grey reach the targets exactly as with ideal taking.
- Colour matrix: `M = T^-1`. In the processing amplifier the linear signals `L'_ch` (after clamp, gain,
  correctors and linearising) become `L''_ch = sum_j M[ch][j] L'_j` before the knee.
- Resulting values (for the record; the tests judge them against the data). Taking
  `T` of the Plumbicon camera `[[0.846, 0.153, 0.001], [0.032, 0.803, 0.165], [0.016, 0.043, 0.940]]`;
  its matrix `[[1.191, -0.229, 0.039], [-0.044, 1.265, -0.222], [-0.019, -0.054, 1.073]]`. Red
  primary read on its own channel without a matrix (`T[R][R]`): Saticon 0.766, Plumbicon 0.846, vidicon
  0.854, Chalnicon 0.898, Newvicon 0.903.

### What follows from it (and the tests check)

- White balance holds for every material; a grey scene is read exactly as with ideal taking.
- No material's matrix is diagonal: the Plumbicon camera needs about the BBC's off-diagonal terms.
- Without the matrix, saturated primaries lose saturation and turn toward their spectral neighbours:
  red toward yellow (its green reading exceeds its blue), green toward yellow, blue toward cyan. The
  readings of a Plumbicon camera without matrix are close to the inverse of the BBC matrix.
- Each material needs its own matrix. A Newvicon, whose response rises to 735 nm, keeps red the most
  saturated of the materials; behind a matrix made for lead oxide its reds come out over-saturated. A
  Saticon, at 3 % by 700 nm, desaturates red the most and comes out under-corrected behind the
  lead-oxide matrix.

### Interfaces

A new shared module `spectral.rs` (no raster constant; both engines use it):

```rust
pub const N_LAMBDA: usize = 81;
pub const LAMBDA_MIN_NM: f64 = 380.0;
pub const LAMBDA_STEP_NM: f64 = 5.0;
/// `LAMBDA_MIN_NM + LAMBDA_STEP_NM * i`.
pub fn wavelength_nm(i: usize) -> f64;
pub const BT709_LUMINANCE: [f64; 3] = [0.2126, 0.7152, 0.0722];
/// CIE 1931 2 degree `[xbar, ybar, zbar]` at `nm`, Wyman, Sloan and Shirley 2013 multi-lobe fit.
pub fn cie_1931(nm: f64) -> [f64; 3];
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Material { Pbo, PboExtendedRed, Sb2s3, Newvicon, Saticon, Chalnicon }
pub const MATERIALS: [Material; 6];
impl Material {
    /// The knots of the table, `(nm, relative response)`.
    pub fn knots(self) -> &'static [(f64, f64)];
    /// Linear between knots, 0 outside them.
    pub fn response(self, nm: f64) -> f64;
}
/// `E_c(nm)` of primary `c` (`[R, G, B]`) at unit linear light.
pub fn primary_spectrum(c: usize, nm: f64) -> f64;
pub fn ir_cut(nm: f64) -> f64;
/// Prism transmission into channel `ch`.
pub fn prism(ch: usize, nm: f64) -> f64;
/// `ir_cut(nm) * prism(ch, nm) * m.response(nm)`.
pub fn taking(ch: usize, m: Material, nm: f64) -> f64;
/// `T[ch][c]`, rows summing to 1.
pub fn taking_matrix(materials: [Material; 3]) -> [[f64; 3]; 3];
/// `T^-1`.
pub fn colour_matrix(materials: [Material; 3]) -> [[f64; 3]; 3];
```

In `tube.rs`: `impl TubeKind { pub fn materials(self) -> Option<[Material; 3]>; }` (`None` for `Off`).

In `camera_processing.rs`: `ProcessingSpec` gains `matrix: bool` (studio `true`, consumer `false`).

```rust
impl VideoProcessor {
    /// The colour matrix the processor applies after linearising; `new` starts with the identity,
    /// which is not applied.
    pub fn with_matrix(self, m: [[f64; 3]; 3]) -> Self;
    pub fn matrix(&self) -> [[f64; 3]; 3];
}
```

In `camera.rs`: `CameraParams` gains `spectral: bool` (config key `spectral`, default `false`).

```rust
impl CameraTube {
    /// The taking matrix in use: `taking_matrix(kind.materials())` with `spectral`, else the identity.
    pub fn taking(&self) -> [[f64; 3]; 3];
    /// The processor's colour matrix: `colour_matrix(..)` with `spectral` and a profile that has one,
    /// else the identity.
    pub fn colour_matrix(&self) -> [[f64; 3]; 3];
    /// The photocurrent the picture `f` drives on each target element, nA, indexed
    /// `(r * ACTIVE_W + x) * 3 + ch`: the light through the taking and the optics of "Geometry", then the
    /// target's sensitivity and transfer. What `read` integrates for a picture held over a field.
    pub fn photo_currents(&self, f: &Frame) -> Vec<f64>;
}
```

`frame_photo` mixes the three linear lights through `T` per element before the transfer, so its
per-level table becomes a table of linear light per level.

### Out of scope

- Metameric failure (scene spectra outside the three) and scene IR; the illuminant and colour-temperature
  filters (the scene's white is the camera's white).
- The matrix's cost in luminance S/N: Jones's 0.3 to 0.7 dB rests on channel noise the research does not
  give (the BBC matrix itself costs 0.83 dB with equal noise in the channels [DER]); it is not judged,
  but it follows, since the matrix acts on the noisy line signals.
- The extended-red layer's own lag (fits of `K` 0.31 to 0.57 nC and its traps) and a choice of layer.
- The stripe camera's spectra: when the stripe tube is joined, the cyan and yellow stripes'
  transmissions and the lens's IR-cut take the prism's place on its one material, and a Newvicon's
  luminance then weights red by its response (its "red sensitivity" in the picture).

## Geometry

Source: `docs/CAMERA_RESEARCH.md`, "Interlaced readout with an oversized spot blurs, skews and twitters"
(signal current proportional to scan speed, Philips T10 4.2; deflection distortion of 0.5 % of the
picture height inside a circle of diameter H and 1 % outside, 2 % for vidicon coil units; matched
triplets registered to 25-40, 40-80 and 80-120 ns by zone, 1 ns about 0.00256 % of the picture height;
black shading of up to 12.5 % at the corners with parabolic and sawtooth components in both directions;
white shading of a few per cent in PbO red and green, blue flat, up to 15 % on the RCA Saticon; corner
limiting resolution), "Beam capacity of twice peak white turns highlights into comet tails" (the
black-square flare test, Saticon 1 % / 0.5 %, vidicon 5 % / 1 %; PbO's reflectance, highest in red, and
its 6 mm anti-halation disc), "Single-tube camcorders trade colour bandwidth for a striped carrier" (a
deliberately non-linear deflection, colour shading from focus, dark shading), "Head-amplifier noise and
processing lie mostly above 2.5 MHz" (US 4032974's order, US 4001502's flare correction) and "A
physically grounded camera model" (C: the scan path is a ramp plus a low-order distortion field and a
per-tube registration offset).

Every error of the picture's field acts where it happens. The lens's veiling glare and the faceplate's
halation act on the light before the target. Shading of sensitivity and of dark current is the target's.
Deflection decides where and when the beam lands, and with it the read time, the charge read per unit of
time and the spot's width in time. Corner focus is the spot's. Corrections are the processing
amplifier's, on the line signal.

### Decisions

- **One displacement field per tube.** The beam of tube `ch`, aimed at the nominal raster position
  `(x, y)`, lands at `(x + dx_ch, y + dy_ch)`: a sweep linearity and a pincushion shared by the tubes, and
  per tube a residual size and pincushion against green (registration). A target element is read when
  the beam passes it, and the line signal is the charge read per unit of time: where the beam moves fast
  it reads more current and its spot is shorter in time. Nothing is resampled in the picture.
- **The target runs past the picture.** With geometry on, the target grid extends beyond the active
  picture on every side, at least as far as any landing plus four spot widths. Elements there are lit as
  their nearest picture element (the scene goes on as its edge) [est]; they are read only by displaced
  passes.
- **Black shading is the target's dark current**, uneven over the target (bias light and dark current
  alike); **white shading is its sensitivity**, uneven over the target. Both are tube properties scaled to
  the data.
- **Flare is a veil and a ring.** The veil is light spread by the lens and by reflections between target
  and lens, a shift-invariant exponential kernel fitted per material to the black-square data. The ring is
  PbO's halation: light the target reflects into the faceplate comes back from the glass's front face by
  total reflection, as a ring around every light; the anti-halation disc moves it out of the picture.
- **Corrections are aligned once.** The studio profile fits US 4032974's black and white shading
  correctors and US 4001502's flare corrector; each is set at alignment (25 deg C, a capped target, a
  flat white field, the black-square chart) and then stays, so what changes later (temperature, the
  scene's average level) shows. The consumer profile fits none [est: its manual names dark shading and
  edge shading as visible faults].
- **Off by default.** `CameraParams.geometry = false` (the default) is `GeometrySpec::NONE`: no
  displacement, shading, flare, focus growth or corrector, and the engine is bit for bit the one before
  this stage. With `geometry = true` the profile's spec for the tube kind applies
  (`Processing::geometry(kind)`).

### Coordinates

- Positions are in picture heights (PH) from the picture centre, `x` to the right, `y` down:
  `x` in `[-X_EDGE_PH, X_EDGE_PH]`, `X_EDGE_PH = TARGET_ASPECT / 2 = 2/3`, `y` in `[-1/2, 1/2]`,
  `rho = sqrt(x^2 + y^2)`, `RHO_CORNER_PH = 5/6`. `u = x / X_EDGE_PH`, `v = 2 y` run from -1 to 1.
- Sample `s` (fractional) of active row `r`, and target element `(r, s)`, sit at
  `picture_position_ph(r, s) = (((s + 0.5) / ACTIVE_W - 0.5) TARGET_ASPECT, (r + 0.5) / ACTIVE_H - 0.5)`.
- Where data are in ns of line time: `PH_PER_NS = 2.56e-5` (the research's 0.00256 %).
- Zones [est: the research gives the registration bands by zone but not the zones]: A inside the circle of
  diameter H (`rho <= 0.5`, the deflection data's circle), B inside the circle of the picture width
  (`rho <= 2/3`), C the corners beyond.

### Deflection and registration

- Displacement of tube `ch`, PH, at nominal `(x, y)`:
  `dx = kappa (x^2 - X_EDGE_PH^2) / (2 X_EDGE_PH) + p x rho^2 + s_ch x + q_ch x rho^2`,
  `dy = p y rho^2 + s_ch y + q_ch y rho^2`. `kappa` is the sweep's linearity: the scan speed along the line
  is `1 + kappa x / X_EDGE_PH` (negative: the sweep slows along the line, as an inductive coil's does),
  with the ends of the line in place. `p` is the pincushion (positive pulls the corners out). `s_ch`,
  `q_ch` are tube `ch`'s size and pincushion against green: `s_G = q_G = 0`, red `(+s, +q)`, blue
  `(-s, -q)` [est: the signs].
- Local scan gain `J_ch = (1 + d(dx)/dx) (1 + d(dy)/dy)` at the nominal position: the area the beam
  sweeps per unit of time, against nominal. Signal current is proportional to it [DS: Philips T10 4.2,
  "signal current is a function of target illumination and of scanning speed"].
- **The scan with geometry** (replacing the read times, beam centres and horizontal spot of "Beam" and
  "Highlights"; ACT flyback spans and bending distances stay on the target grid):
  1. For row `r` and target column `j`, the nominal sample `s_j` at which the beam lands on the column:
     `x(s_j) + dx_ch(x(s_j), y_r) = x(j)` (one root: the speed stays positive; solved to 1e-6 sample).
     The column is read at `read_time_s` of `(n, r)` at the fractional sample `s_j`
     (`n / FRAME_RATE + line / FH + (ACTIVE_START + s_j) / FS_BB`).
  2. The pass's centre over column `j` is `c_r + dy_ch(x(s_j), y_r) ACTIVE_H` frame-line pitches and its
     vertical sigma `sigma_v F_ch(x(s_j), y_r)` (corner focus, below). Shares are normalised per column:
     `w_rk(j) = g_rj(y_k) / S_k(j)`, `S_k(j)` the sum over the frame's passes at sub-row `k` of column `j`,
     truncated at 3 sigma as now. Each element is still offered `I_b T_frame` per frame.
  3. The landed charge of column `j` gives `raw_j = landed / (SUB_ROWS T_frame)` as now.
  4. The line signal at sample `s`: the beam is at column position
     `X(s) = s + dx_ch(x(s), y_r) ACTIVE_W / TARGET_ASPECT` moving at `X'(s) = dX / ds`, with spot sigma
     `sigma_x = sigma_h_s FS_BB F_ch(x(s), y_r)` columns:
     `signal(s) = X'(s) sum_j raw_j G(X(s) - j) / sum_j G(X(s) - j)`, `G` the Gaussian of `sigma_x` over
     the target's columns within 4 sigma. Without geometry `X(s) = s`, `X' = 1`, `F = 1`: the present
     spot, renormalised at the line ends.
  A uniform target thus reads `J_ch` times its current, and the spot is `sigma_x / X'` long in time.
- The deflection data bound the total displacement; registration data the difference between tubes.

### Shading

- **Black.** The dark and bias currents of an element at `(x, y)` are multiplied by `1 + b(u, v)`,
  `b = h_saw u + h_par (u^2 - 1/3) + v_saw v + v_par (v^2 - 1/3)` (mean zero over the picture). Both
  profiles: `h_saw = v_saw = 0.0125`, `h_par = v_par = 0.075`: `+12.5 %` at the bottom-right corner,
  `+7.5 %` top-left, `-5 %` at the centre [DS: up to 12.5 % at the corners, parabolic and sawtooth in both
  directions; the split est]. It follows the dark current with temperature.
- **White.** The light an element converts is multiplied by `1 + w_ch`, `w_ch = -fall_ch (rho / RHO_CORNER_PH)^2`
  (centre-high). `fall`: every Plumbicon kind `[0.03, 0.03, 0.0]` [DS: "a few per cent" in red and green,
  blue flat; 3 est]; Saticon `0.15` [DS: up to 15 %]; vidicon, Newvicon, Chalnicon 0 [no data].
- The target starts at its uniform dark equilibrium; a shaded target settles in the first frames, as a
  camera switched on.

### Flare

- **Veil.** Light reaching target `ch`: `l' = (1 - f_ch) l + f_ch (K * l)`,
  `K(d) = exp(-d / a_ch) / (2 pi a_ch^2)` per PH^2 (`d` in PH), the convolution over the picture only
  (light spread beyond the target is lost; nothing enters from outside the picture).
- **The black-square chart** (`BLACK_SQUARE_PH = 0.1`): a white picture (`L = 1`, a 200 nA field on a
  2/3-inch tube) with a black square of side 0.1 PH, centred at the picture centre, or flush with a
  corner [est: "corner"]. The reading is the photocurrent at the square's centre as a share of peak.
- Fits [DER on DS]: Saticon `f = 0.0215`, `a = 0.40` PH: 1.00 % at the centre and 0.50 % at the corner.
  Vidicon `f = 0.0116`, `a = 0.15` PH: 5.0 % at the centre through its 0.65 transfer (1.0 % of light).
  Through that transfer no width of a shift-invariant veil reads a corner square below 0.51 of the centre
  square (this width is the lowest [DER]): the vidicon's corner reads 2.6 % against the data's 1 % (open). PbO,
  Newvicon and Chalnicon take the Saticon's veil [est: no data].
- **Halation** (the PbO kinds). `l' += h_ch (K_h * l)`, `h_ch = reflectance_ch (1 - 1 / n^2)` (the share
  of a diffuse reflection beyond the critical angle), with the ring kernel
  `K_h(d) = (1 + tan^2 theta_c) / (pi (2 t)^2) / (1 + (d / 2t)^2)^2` for `d >= 2 t tan theta_c`, 0 inside,
  `sin theta_c = 1 / n`, `t = (faceplate_mm + disc_mm) / target_height_mm` PH [DER: light reflected
  diffusely by the target is totally reflected by the glass's front face beyond `theta_c` and lands
  `2 t tan theta` away; per unit area `cos^4 theta`]. `n = GLASS_INDEX = 1.5` [est], reflectance
  `[0.15, 0.08, 0.05]` [est: "highest in PbO, especially in red"], faceplate 1.5 mm [est],
  `disc_mm = ANTI_HALATION_DISC_MM = 6.0` [DS], target height 12.8 mm (30 mm tube) [DS]. Without the disc
  the ring's inner edge is at 0.21 PH; the disc puts it at 1.05 PH, past the picture's half-diagonal, so a
  highlight inside the picture's middle has no halo and the rest of the ring is 25 times thinner. Other
  materials: no halation.
- Order on the light: taking (spectral), veil, halation, white shading, transfer. The optics act on each
  held picture (per field), so `photo_currents` and `read` see them. They may be computed on a coarser
  grid than the picture, to 1e-4 of peak light.

### Corner focus

- Both spot sigmas of tube `ch` at `(x, y)` are multiplied by `F_ch = 1 + focus_ch (rho / RHO_CORNER_PH)^2`.
  With the Gaussian response `exp(-2 pi^2 sigma^2 f^2)` the limiting resolution (a fixed small modulation)
  falls as `1 / F`: at the corner by `1 - 1 / (1 + focus)`.
- `focus`: vidicon and Newvicon 0.45 (31 %: XQ1274 650 to 450 TVL, within 25-35 %) [DS]; Chalnicon 0.25
  (20 %) [DS]; PbO kinds and Saticon 0.25 [est: no corner figure; the low end for magnetic-focus studio
  tubes].

### Corrections

The processing amplifier's order becomes (US 4032974, with the knee of "Camera electronics" and the matrix
of "Spectral taking and the colour matrix"): clamp with flare correction, gain, black shading correction,
white shading correction, linearise, matrix, knee, gamma, aperture correction, clip. Per channel, at
`(x, y) = picture_position_ph(r, s)`:

1. `L1 = (I - black) / peak` (clamp and gain as now).
2. Flare: `L2 = L1 - c_ch APL_ch`, `APL_ch` the mean of `L1` over the frame's active samples, `c_ch` the
   black-square chart's centre reading (through the camera's own optics) divided by the chart's mean
   light `1 - BLACK_SQUARE_PH^2 / TARGET_ASPECT`: aligned so the chart's centre square reads black
   (US 4001502 feeds the integrated picture level into the clamp) [est: the frame's own mean stands for the
   integrator].
3. Black shading: `L3 = L2 - beta_ch`, `beta_ch = ref_ch (J_ch (1 + b) - 1) / peak_ch`, `ref_ch` the dark
   plus bias current at 25 deg C (`black_reference_na`): the capped reading at alignment.
4. White shading: `L4 = L3 / (J_ch (1 + w_ch)^gamma_ch)`: the flat-field reading at alignment.
5. Linearise, matrix, knee, gamma, aperture correction, clip as before.

Without a corrector the step is skipped. `CameraTube` measures the chart reading at construction on its
own optics; the processor takes it as `flare_reading`.

### Profiles

| | studio | consumer |
|---|---|---|
| linearity `kappa`, pincushion `p` | -0.012, 0.0155: at most 0.48 % inside the circle of diameter H and 0.90 % outside (Philips deflection units, 0.5 % and 1 %) [DS bounds] | -0.045, 0.031: 1.6 % and 1.8 % (vidicon coil units, 2 %) [DS bound] |
| registration `s`, `q` (red; blue negated) | 8.7e-4, 3.2e-3: up to 33, 60 and 101 ns in zones A, B, C (matched triplets 25-40, 40-80, 80-120 ns) [DS bands] | 1.12e-3, 3.69e-3: 40, 72 and 120 ns, the bands' upper ends [est] |
| black shading | as above [DS] | as above [DS] |
| correctors | flare, black shading, white shading (US 4032974, US 4001502) | none [est] |
| matrix | yes | no |

White shading, veil, halation and corner focus are the tube kind's, in both profiles.

### The single-tube camera

The stripe camera has one target: no registration. Deflection, shading, flare and corner focus act on its
target as above; its veil acts on the scene's light before the optical low-pass and the stripes, and
corner focus lowers the carrier's modulation toward the corners, which is the WV-3110's "colour shading"
from focus with no further mechanism.

### Defaults and what stays

- The camera off is untouched (the golden test). `geometry = false` builds `GeometrySpec::NONE` and the
  engine before this stage, bit for bit. `UniformColumn` (the data sheets' measurement surface) has no
  geometry.
- Cost: per-column vertical shares; two picture convolutions per held picture for the optics.

### Interfaces

A new shared module `geometry.rs` (positions in PH; no raster constant):

```rust
pub const X_EDGE_PH: f64 = TARGET_ASPECT / 2.0;
pub const RHO_CORNER_PH: f64 = 5.0 / 6.0;
pub const PH_PER_NS: f64 = 2.56e-5;
pub const BLACK_SQUARE_PH: f64 = 0.1;
pub const GLASS_INDEX: f64 = 1.5;
pub const ANTI_HALATION_DISC_MM: f64 = 6.0;
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Deflection { pub linearity: f64, pub pincushion: f64 }
/// A tube's size and pincushion against green.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Registration { pub size: f64, pub pincushion: f64 }
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Shading { pub h_saw: f64, pub h_par: f64, pub v_saw: f64, pub v_par: f64 }
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Veil { pub fraction: f64, pub scale_ph: f64 }
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Halation { pub reflectance: f64, pub faceplate_mm: f64, pub disc_mm: f64, pub target_height_mm: f64 }
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
impl Deflection { pub const NONE: Deflection; }
impl Registration { pub const NONE: Registration; }
impl Shading { pub const NONE: Shading; }
impl Veil { pub const NONE: Veil; }
impl Halation { pub const NONE: Halation; }   // reflectance 0
impl GeometrySpec { pub const NONE: GeometrySpec; }
/// `(dx, dy)` of tube `ch`, PH, at nominal `(x, y)`.
pub fn displacement_ph(g: &GeometrySpec, ch: usize, x: f64, y: f64) -> (f64, f64);
/// `J_ch` at nominal `(x, y)`.
pub fn scan_gain(g: &GeometrySpec, ch: usize, x: f64, y: f64) -> f64;
/// `b(u, v)` at `(x, y)`.
pub fn black_shading_factor(s: &Shading, x: f64, y: f64) -> f64;
/// `1 + w` for a corner fall `fall`.
pub fn white_shading_factor(fall: f64, x: f64, y: f64) -> f64;
/// `F` for `focus`.
pub fn focus_factor(focus: f64, x: f64, y: f64) -> f64;
/// `K(d)`, per PH^2.
pub fn veil_kernel(v: &Veil, d_ph: f64) -> f64;
/// `h` and `K_h(d)` (per PH^2, unit integral over the plane), and the ring's inner radius, PH.
pub fn halation_share(h: &Halation) -> f64;
pub fn halation_kernel(h: &Halation, d_ph: f64) -> f64;
pub fn halation_radius_ph(h: &Halation) -> f64;
```

In `camera_processing.rs`:

```rust
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Corrections { pub flare: bool, pub black_shading: bool, pub white_shading: bool }
// ProcessingSpec gains `corrections: Corrections` (studio all true, consumer all false) beside `matrix`.
impl Processing {
    /// The profile's spec for a tube kind (`GeometrySpec::NONE` for `Off`).
    pub fn geometry(self, kind: TubeKind) -> GeometrySpec;
}
impl VideoProcessor {
    /// Fit the profile's correctors, aligned to `geometry`; `flare_reading` is the black-square chart's
    /// centre reading per channel, a share of peak.
    pub fn with_geometry(self, geometry: &GeometrySpec, flare_reading: [f64; 3]) -> Self;
    /// `c_ch` of the flare corrector (0 without it).
    pub fn flare_coefficient(&self) -> [f64; 3];
}
```

In `camera.rs`:

```rust
/// `(x, y)` in PH from the picture centre of fractional sample `s` of active row `r`.
pub fn picture_position_ph(r: usize, s: f64) -> (f64, f64);
// CameraParams gains `geometry: bool` (config key `geometry`, default false).
impl CameraTube {
    /// The camera with an explicit spec; `seeded(p, seed)` is
    /// `seeded_geometry(p, seed, &if p.geometry { p.processing.geometry(p.tube) } else { GeometrySpec::NONE })`.
    pub fn seeded_geometry(params: &CameraParams, seed: u64, geometry: &GeometrySpec) -> Option<Self>;
    pub fn geometry(&self) -> &GeometrySpec;
    pub fn processor(&self) -> &VideoProcessor;
}
```

`read_currents`' callback keeps its picture indices: elements beyond the picture call it with their nearest
picture element.

### Out of scope

- Lens distortion and vignetting; raster skew (0.4 to 1 %, vidicon 4 %); dynamic focus; the electronic
  registration correction of 1984 (Iwabe et al.) and the operator's alignment of centring, size and
  linearity beyond the residuals above.
- The beam reserve's change with scan speed and line density (each element is still offered
  `I_b T_frame`); spot growth with beam current (H8397A 58 % to 37 %).
- Flare from outside the picture; the vidicon's corner flare (above); glare ghosts; the halation of
  rays inside the critical angle.
- Blemishes, mesh microphonics; burn and its shading (Retention).

## The edge of the scan

A tube camera blanks its beam only for the deflection retrace, and blanks the picture for longer: "The blanking
voltage pulse should be as long as the retrace time of the deflecting circuit and narrower than the final picture
blanking. A spurious signal which is developed at the very edge of the scan ... should be blanked off on both sides
and top and bottom from the final video signal by the wider system blanking pulse. To accomplish this vertically, it
is desirable to generate a vertical system blanking pulse that starts several scan lines before the camera vertical
retrace" (*Photosensitive Camera Tubes and Devices Handbook*, 11.5.4). The target beyond the scanned area is lit and
never read, so it charges up, and the beam's wings at the outermost lines read it (US 4,404,499 calls the margin the
overscan). Measured on the engine before this section (flat cards, 6 frames, decoded, green): the first and last rows
read up to `+32` levels high and `-41` low without geometry, and up to `+103` high with it; a capped vidicon with
geometry reads `+73` in its top row. Columns are within `0.6` levels on both paths: along a line the scan is continuous.

### Rules

- **Guard lines.** The tube scans `GUARD_FIELD_LINES = 4` `[est]` ("several") field lines beyond the active picture
  at the top and the bottom of each field: `GUARD_ROWS = 2 * GUARD_FIELD_LINES = 8` frame rows at each edge, rows
  `-8..-1` and `ACTIVE_H..ACTIVE_H + 8`. Guard row `r` of field `p = r.rem_euclid(2)` is frame line
  `FIELD_FIRST_LINE[p] + r.div_euclid(2)` and is read at `read_time_s` of that line, inside the vertical blanking
  interval (`20.5` lines a field at 525 lines, `25` at 625). Active rows keep their read times.
- **Their light.** A guard row sees the nearest active row of the held picture, the out-of-picture rule
  (`docs/MEASUREMENTS_WANTED.md`). In the geometry path the pad beyond the guard rows is the unscanned border, lit
  the same way, as before.
- **The target.** Without geometry the target grid is `SUB_ROWS * (ACTIVE_H + 2 * GUARD_ROWS)` sub-rows and ends at
  the outermost guard line; the beam shares `S_k` sum over every pass, guard passes included. Nothing that reaches
  an active row (the beam's `3 sigma_v`, the bending radius `BEND_RADIUS_MAX_LINES`, the vertical aperture's two
  rows) reaches past the guard rows, so an unscanned border would change no output and is left out there.
- **Everything a scanned line has, a guard line has:** dark current, traps and held charge, the ACT flyback before
  its pass, dynamic beam control, beam bending, burn and retention, `idle`, the beam-off start and the stripe filter.
- **Electronics and blanking.** Guard rows pass the head amplifier, with noise from their own stream
  (`STREAM_CAMERA_GUARD`, so every active row's noise is unchanged), and the processing amplifier, where the
  vertical aperture corrector takes them as the neighbours of the edge rows. The system blanking then drops them:
  only active rows reach the encoder. `CameraTube::guard_rows()` keeps the last frame's processed guard rows (top
  eight, then bottom eight, each `[R, G, B]` in the processing amplifier's output scale) for the checks.
- **Black.** The clamp reference of a tube camera is the beam-off level of the blanking interval, zero signal
  current (handbook 11.5.4); the dark current then lifts the picture, and the studio operator sets black on the
  capped camera. `black` stands for that set-up: the settled dark reading averaged over the centre half of the
  active rows (`ACTIVE_H / 4..3 * ACTIVE_H / 4`), the bulk of the waveform the operator watches. A camera without a
  line-up is in `docs/MEASUREMENTS_WANTED.md`.
- **Cost.** Sixteen more rows a frame: `3.3 %` at 480 rows, `2.8 %` at 576.

### What the tests check (`tests/camera_guard_band.rs`)

- On flat cards (capped, grey, white), for every tube with and without geometry, in both layouts and both engines,
  the first and last four rows match the rows just inside them within `1.5` levels.
- The spurious signal is still made: a capped vidicon with geometry reads well above black in its outermost guard
  row, and the active rows do not.
- Rows more than 32 rows from either edge match the engine before this section on a moving scene (frozen per-row
  sums), so the interior is untouched.
- A capped camera's centre rows average black within the noise, for every tube.

### Out of scope

- Edge hooking (US 4,404,499): the beam bending towards the charged margin around the scan. It is left to what
  the geometry path's pad gives the bending model.
- The horizontal guard time (a few microseconds of video delay at the right edge): the columns measure clean.

## Stages

| Stage | Content |
|---|---|
| Scan engine | Target grid, beam passes, read times, held pictures, signal normalisation, horizontal spot; the existing tube parameters converted to `K`. |
| Tube in seconds | Tube parameters in seconds: physical `V_T` per gun, `C` fitted per channel to the handbook curves (decay against bias, build-up at 40 to 80 ms, high key) on the scanned model; the lag measurement re-run on the engine; 50 and 60 Hz from the same constants. |
| Electronics | Head amplifier noise (amplitude rising with frequency) and band, clamp, knee, gamma, aperture correction from line delays with coring, clip; studio and consumer profiles. |
| Materials | Sb2S3, Newvicon, Saticon, Chalnicon targets: transfer exponent, dark current against temperature, capacitance, trap populations. |
| Spectra | Spectral weighting of the channels per material and the colour matrix that goes with it. |
| Stripe camera | Single-tube colour: stripe filter on the target behind an optical low-pass, carrier from scan speed, decoder (trap, band-pass, 1H separation, detection, 2.8 MHz Y and 0.5 MHz colour). |
| Highlights | ACT flyback spanning several lines, dynamic beam control, beam bending from element potential, trail colour. |
| Geometry | Deflection distortion, three-tube registration, black and white shading and its correction, lens flare and its correction, corner focus. |
| Retention | Trap memory and burn-in, recovery, warm-up and temperature. |
| Front ends | Command line, configuration, GUI choices and `docs/CAMERA.md`. |
