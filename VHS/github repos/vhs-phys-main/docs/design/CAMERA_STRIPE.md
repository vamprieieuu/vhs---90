# CONTRACT

# The stripe colour camera

Part of the camera design (`docs/design/CAMERA_SCAN.md`).

## Optics and decoder

Source: `docs/CAMERA_RESEARCH.md`, "Single-tube camcorders trade colour bandwidth for a striped
carrier" (Panasonic WV-3110, frequency separation with cyan and yellow stripes). One tube sees the
scene through a stripe filter on its faceplate; scanning the stripes turns colour into a carrier on
the line signal, and the camera's decoder takes colour back out of it. This stage builds the optics
and the decoder as their own module; joining them to the scanned target and the processing amplifier
is the next stage.

- **Carrier.** `f_c = w / (p t)` with `w = 7.49 mm`, `p = 40 um`, `t = 52.3 us`: `STRIPE_CARRIER_HZ =
  3.58e6`, the same in both engines (a home camera's stripes are fixed in its optics).
- **Stripes.** Cyan stripes block red, yellow stripes block blue, green passes both. The beam reads
  only the stripes' fundamental (the harmonics lie far above its spot), so the filter is
  `T_G = 1`, `T_R = (1 + cos th_R) / 2`, `T_B = (1 + cos th_B) / 2` with
  `th = 2 pi f_c x / FS_BB + phi(r)`. The two stripe sets slant oppositely: `phi_R(r) = +pi r / 4`,
  `phi_B(r) = -pi r / 4` over active frame rows `r`, so from one line of a field to the next (two frame
  rows) red's carrier turns +90 deg and blue's -90 deg. The element's light is `sum_c T_c L_c`.
- **Optical low-pass.** A birefringent plate before the stripes splits the image into two copies
  displaced along the line by half a carrier period, `d = 1 / (2 f_c)` of line time:
  `L'(x) = (L(x - d/2) + L(x + d/2)) / 2` per colour, with fractional positions interpolated on the row
  and the row ends repeating their end sample. Its response `cos(pi f d)` is zero at the carrier, so
  scene detail there makes no false colour, at a cost in sharpness. This acts on the scene's light before
  the target, as the optics do.
- **Decoder** (on the clamped line signal `s`, one active row at a time, filters linear-phase with
  their delay compensated, row ends repeating their end sample):
  1. Chroma `c = BP(s)`: Gaussian band-pass at `f_c`, -3 dB at `f_c +- 0.5 MHz`.
     Luminance `y = LP_2.8(s - c)`: the trap is the band-pass's complement, then a Gaussian low-pass
     with -3 dB at 2.8 MHz.
  2. 1H separation. With `c_0` this row's chroma, `c_1` the chroma of the previous line of the same
     field (frame row `r - 2`; blank before a field's first line) and `D` a delay of a quarter carrier
     period (`1 / (4 f_c)`, fractional positions interpolated): red carrier `D(c_0) + c_1`, blue carrier
     `D(c_0) - c_1`.
  3. Detection. Each carrier's envelope is the magnitude of its analytic signal (the carrier and its
     Hilbert transform, a linear-phase FIR), times `carrier_gain`, then a Gaussian low-pass with -3 dB at
     0.5 MHz: `R_c`, `B_c`. (A rectifier would alias: at 525 lines the carrier is a quarter of the sample
     rate.)
  4. Matrix with mixed highs. `y_c = LP_0.5(y)`, `G_c = y_c - (R_c + B_c) / 2`, `h = (y - y_c) / 2`
     (white light of 1 in every colour gives `y = 2`, so halving puts the highs on the channels' scale);
     the outputs are `R_c + h`, `G_c + h`, `B_c + h`. A flat field of light `(R, G, B)` decodes to
     `(R, G, B)`, and a grey edge to a grey edge.
  `carrier_gain` is the camera's alignment: the reciprocal of the response of the read (spot and head
  amplifier) at the carrier, set by whoever joins the decoder to a tube; 1 for an ideal read.
- **What follows from it** (and the tests check): colour about 0.5 MHz wide, luminance 2.8 MHz; a
  colour change between lines mixes red and blue on the line after it; fine detail near the carrier
  makes false colour without the optical low-pass; a target that cannot follow the carrier's peaks (the
  beam limit) loses carrier and the highlight turns green.

Interfaces, in a new engine module `stripe.rs`:

```rust
pub const STRIPE_CARRIER_HZ: f64 = 3.58e6;
pub const LUMA_BAND_HZ: f64 = 2.8e6;
pub const COLOUR_BAND_HZ: f64 = 0.5e6;
pub const CHROMA_HALF_WIDTH_HZ: f64 = 0.5e6;
/// Stripe transmittances `[R, G, B]` at sample `x` of active frame row `r`.
pub fn stripe_transmittance(r: usize, x: usize) -> [f64; 3];
/// One colour of one row of scene light behind the optical low-pass.
pub fn optical_lowpass(row: &[f64]) -> Vec<f64>;
/// The light reaching the target at `(r, x)` from scene light `rgb` there: `sum_c T_c rgb_c`.
pub fn target_light(r: usize, x: usize, rgb: [f64; 3]) -> f64;
pub struct StripeDecoder { .. }
impl StripeDecoder {
    pub fn new(carrier_gain: f64) -> Self;
    /// Decode a frame of clamped line signals (`rows[r][x]`, active rows in frame order) into
    /// `[R, G, B]` rows on the same scale.
    pub fn decode(&self, rows: &[Vec<f32>]) -> Vec<[Vec<f32>; 3]>;
}
```

## Joined to the tube

Sources: `docs/CAMERA_RESEARCH.md`, "Single-tube camcorders trade colour bandwidth for a striped
carrier" (Panasonic WV-3110 and PK-958, JVC GR-C1) and "A physically grounded camera model" (single-tube
presets: stripes behind an optical low-pass on the photocurrent, head amplifier and clamp on the line
signal, then the stripe decoder). A home camcorder has one tube. The scene's light passes the
birefringent plate and the stripes onto its one target, the beam reads that target as the scan engine of
`docs/design/CAMERA_SCAN.md` reads any target, one head amplifier amplifies the one line signal, and the
decoder takes R, G and B out of it before the processing amplifier corrects them and the encoder encodes
them. Everything acts on the light, the target or the line signal; nothing is added to a picture.

### Decisions

- **Layout is chosen apart from the material.** `CameraParams.layout` (config key `layout`): `three-tube`
  (the default: everything as before) or `stripe`. `tube` still names the target material; with
  `stripe` it is the material of the one target. Any `TubeKind` other than `Off` may carry stripes.
- **Materials.** The data's single-tube cameras: the WV-3110, whose stripes and decoder these are, on a
  2/3-inch Sb2S3 vidicon of gamma 0.65 [DS]; the PK-958 on a 2/3-inch Newvicon [DS] (a single 5 MHz
  carrier, another stripe system); JVC's 1984 GR-C1 on a 1/2-inch Saticon [SEC]; Sony's Betamovie on a
  Saticon-target Trinicon [SEC] (an index system, out of scope). The colour checks use `saticon`, a
  linear target: the decoder's colours come out as they went in. The lag checks use `vidicon`, the
  WV-3110's own tube (20 % at 50 ms). When the front ends offer `stripe`, its default material is
  `saticon` [decision]. With `vidicon` the 0.65 transfer acts on the stripes' light before the decoder,
  so the carrier of a saturated colour is compressed less than white's, and after white balance the
  colour bars come out about 30 % more saturated [measured on the scan engine]. The WV-3110 corrected
  this with separate R and B gamma tracking, which is not modelled.
- **The one target** is the kind's green tube: `kind.channels_at(beam_ratio, temperature_c)?[1]`, with
  `bending` from `blooming` as for every channel. For the non-PbO kinds the three channels are equal; for
  the Plumbicon kinds it is the green target (300 nA, 1.15 nF, no traps, ACT level 7 V on
  `plumbicon-act`). The spot keeps the kind's widths; the 2/3-inch and 1/2-inch formats wait for Geometry
  [est].

### Signal path

1. **Scene light.** Per colour, the linear light of the input level `L_c = oetf_inverse(v / 255) 2^iris`
   (as for three tubes), each active row through `optical_lowpass`, then
   `target_light(r, x, [L'_R, L'_G, L'_B])`. Every sub-row takes its row's light, so the stripe phase of a
   sub-row is its row's (as all light on the target is per row).
2. **Photocurrent.** `photo_current(ch, target_light / STRIPE_WHITE_LIGHT)`, `STRIPE_WHITE_LIGHT = 2`: the
   mean of `T_R + T_G + T_B` over the stripes. So a white scene gives the tube's peak current on average,
   and the transfer exponent acts on the light each element receives, stripes included. White's stripe
   peaks reach 1.5 times peak current (rows where the red and blue carriers add), so with the default beam
   of twice peak the peaks pass the beam at 4/3 of white.
3. **Scan.** The scan engine as it stands (lag, traps, ACT flyback, dynamic beam control, bending, plate
   limit) on the one target, which sits in slot `STRIPE_SLOT = 1` of `TargetSignal`. Slots 0 and 2 hold
   empty rows: `read_currents` calls `photo` with `ch = STRIPE_SLOT` only, and no other target is scanned.
4. **Head amplifier.** `HeadAmplifier::amplify` as it stands. It skips empty rows, so it adds noise and
   band to the one line signal with the noise of `(seed, frame, row, STRIPE_SLOT)`. Noise near 3.58 MHz,
   where the FET's noise is strongest, becomes colour noise in the decoder.
5. **Clamp and scale.** `s = (i - black) STRIPE_WHITE_LIGHT / peak_signal_na`: the dark reading off,
   and the decoder's scale on which white averages 2. Dark current has no carrier, so it is taken out
   here with the luminance, before the decoder.
6. **Decoder.** `StripeDecoder::new(carrier_gain)` on the rows of `s`, with the gain from the alignment.
7. **Processing amplifier** on the decoded R, G and B: clamp at the capped-lens `floor`, gain to the white
   card's `white` (white balance), linearise (`vidicon`), knee, gamma, aperture correction from G, clip:
   the existing chain, through `VideoProcessor::with_levels(processing, &[ch; 3], floor, white)`. Then
   the encoder as now. `processing` stays a separate key; the checks use `consumer`.

**The clamp after detection.** An envelope detector rectifies the amplifier's noise: with no carrier
its output is the noise's mean envelope, about 0.03 of white with the consumer amplifier [measured]. A
camera clamps its R and B channels after detection at a level without carrier, which is the line
blanking; here the alignment measures that level on a capped lens. Without that clamp, black comes out
magenta (about 30 levels of 255 in R and B). A consequence [DER]: a weak carrier is little above the
noise, its envelope is close to the floor, and the clamp takes most of it off. Dim areas lose colour
first and lean green: a flat 18 % grey reads about 0.05 greener than red and blue in the gamma-corrected
picture, 50 % about 0.012, white not at all [measured]. The WV-3110 reports weak modulation in dim areas
and corrects it with R/B gamma tracking (not modelled). The tests do not judge greys below 50 %.

### Alignment: the carrier gain and white balance

`CAMERA_STRIPE.md` left `carrier_gain` to this stage: the reciprocal of the read's response at the
carrier. The response is the horizontal spot (`exp(-2 pi^2 sigma_h^2 f_c^2)`, 0.67 for a 40 ns spot), the
amplifier's band (0.92 at 5.5 MHz), and also the beam's vertical overlap of the other field's lines,
whose slanted stripes sit 45 degrees off (about 0.88, depending on the landing law), and the target's
transfer (0.65 compresses the carrier). The overlap and the transfer have no closed form. So the camera
is aligned as its technician aligned it, on a white card and a capped lens, once at construction (after
the dark settle; the camera's own state is left as it was):

- **White card.** A clone of the camera, at iris 0 stops whatever `iris_stops` says, reads
  `ALIGN_FRAMES = 8` frames of a flat picture of level 255 (light 1 in every colour). Its last read passes
  the head amplifier as frame `ALIGN_WHITE_FRAME = -1`. Eight frames bring a vidicon's carrier response
  within 0.2 % of its value after 40 [measured].
- **Capped lens.** A `TargetSignal` whose slot 1 holds `black` at every sample passes the head amplifier
  as frame `ALIGN_CAPPED_FRAME = -2`. Both frame indices are before any frame of a session, so their
  noise is fixed by the seed and is not reused.
- Both are clamped and scaled (step 5) and decoded with gain 1. Over the middle half of the rows and of
  the line (rows `ACTIVE_H/4 .. 3 ACTIVE_H/4`, samples `ACTIVE_W/4 .. 3 ACTIVE_W/4`) their means are `W1`
  and `C1`. The read's response is `a = ((W1_R - C1_R) + (W1_B - C1_B)) / 2`, and
  `carrier_gain = 1 / a`.
- Both are decoded again with `carrier_gain`, giving means `W` and `C`: `floor = C` and `white = W - C`,
  per channel.

Values at 525 lines [measured]: Saticon `carrier_gain` about 1.83 (white within 0.3 % of 1 in every
colour, floor about 0.028 in R and B and -0.028 in G); vidicon about 2.8. Both engines align the same
way, and the 625-line carrier, at 4.95 samples a cycle, balances as well.

### What follows (and `tests/stripe_tube.rs` checks)

- The one target's line signal carries the 3.58 MHz carrier, weakened by the spot and the vertical
  overlap (0.61 of a red field's light against a spot response of 0.67) and nothing else in the band. A
  still white reads white, and 50 % grey reads grey to 0.025 of peak white.
- Colour bars keep their hue (within 5 degrees) and saturation (0.9 to 1.05 times). Colour edges stay
  about 0.5 MHz wide (10-90 % in 0.4 to 1.3 us) and luminance edges about 2.8 MHz (0.05 to 0.35 us)
  through the whole read.
- The stripe camera and three tubes of the same material give a flat field the same luminance, within
  0.01.
- Lag. Red and blue ride one carrier off one charge, so their trails keep the same share of their lit
  reading, within 0.02. Three PbO tubes do not: blue's traps keep at least 1.3 times red's share. The
  carrier's residue fades before the luminance's, so a trail is greener than the picture it follows
  [DER]. Lag is longer at lower current (Philips 1963), and the troughs of the stripes carry less current
  than the peaks, so what the target keeps is less modulated than the light was. On a weak residue the
  floor clamp adds to this. The vidicon's trail on its second dark frame holds about 0.42 of its lit
  green and 0.02 of its red and blue [measured]; the Newvicon and the Saticon go the same way, less
  strongly. The research's guess of colour lagging behind luminance ([DER, unsourced]: "nothing documents
  how lag interacts with the stripe carrier") is not borne out by the scan.
- Highlights. Two stops over white put every element past the beam: the line signal is the beam's, flat,
  and the carrier is gone. Red and blue decode to about 0, green to the beam's 4, and the gamma-corrected
  picture is green (WV-3110: "the bright picture parts turn into an unnatural green color"). One stop over
  clips only the stripe peaks: red and blue stop near 1 where they would be 2. In the picture the clip
  then hides most of it.

### Interfaces

In `stripe.rs`:

```rust
/// Mean of `T_R + T_G + T_B` over the stripes: the target light of white, which gives the tube's peak
/// current on average.
pub const STRIPE_WHITE_LIGHT: f64 = 2.0;
```

In `camera.rs` (both engines):

```rust
/// How the camera's tubes see colour (config `layout`: `three-tube`, `stripe`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "kebab-case")]
pub enum Layout { #[default] ThreeTube, Stripe }
// CameraParams gains `pub layout: Layout` (default `ThreeTube`).
/// The slot of `TargetSignal` that carries the stripe camera's one target.
pub const STRIPE_SLOT: usize = 1;
/// Frames of white card read before the alignment measures it.
pub const ALIGN_FRAMES: usize = 8;
/// Head-amplifier frame indices of the alignment's white card and capped lens.
pub const ALIGN_WHITE_FRAME: i64 = -1;
pub const ALIGN_CAPPED_FRAME: i64 = -2;
/// The stripe camera's alignment: the decoder's carrier gain, and per channel `[R, G, B]` the decoded
/// level of a capped lens (`floor`) and of a white card above it (`white`).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct StripeAlignment { pub carrier_gain: f64, pub floor: [f64; 3], pub white: [f64; 3] }
impl CameraTube {
    pub fn layout(&self) -> Layout;
    /// `None` with three tubes.
    pub fn stripe_alignment(&self) -> Option<StripeAlignment>;
    /// The stripe camera's colour of one frame: read, head amplifier, clamp and scale, decoder, then
    /// `(v - floor) / white` per channel. That is linear light with white 1.0 (in the tube's own
    /// transfer for a tube that linearises), before linearise, knee, gamma, aperture correction and clip.
    /// Advances the frame as `video` does. `None` with three tubes, which then reads nothing.
    pub fn stripe_colour(&mut self, f0: &Frame, f1: &Frame) -> Option<Vec<[Vec<f32>; 3]>>;
}
```

With `stripe`, `channels()` returns the one target's channel in all three slots and `black()` its dark
reading in all three. `read` and `read_currents` fill slot 1 only. `video`, `encode_fields`,
`encode_baseband` and `process_frame` take the path above. `set_iris_stops` changes the scene's light,
not the alignment.

In `camera_processing.rs`:

```rust
impl VideoProcessor {
    /// As `new` with the gain's level per channel given: `new(p, ch, black)` is
    /// `with_levels(p, ch, black, [ch[0].peak_signal_na, ch[1].peak_signal_na, ch[2].peak_signal_na])`.
    pub fn with_levels(processing: Processing, channels: &[TubeChannel; 3], black: [f64; 3],
        peak: [f64; 3]) -> Self;
}
```

### Defaults and what stays

- `layout` defaults to `three-tube`, which takes exactly the present path. The camera off (the golden
  test) and every existing test stay bit-identical. `HeadAmplifier::amplify`, `VideoProcessor::new` and
  the scan engine do not change numerically. The present camera tests and the golden test pass with the
  join added [checked on a copy of the engine with Highlights].
- The stripe camera is off unless `layout = "stripe"` and `tube` names a material.

### Cost

- Per frame: one target scanned instead of three (about a third of a three-tube frame's scan). Added to
  that: the optical low-pass (three colours per row), the stripe light per element and the decoder (a few
  FIRs per row). These are small next to the scan.
- At construction: the dark settle as now (once is enough, as the three slots hold one target) and the
  alignment, which costs `ALIGN_FRAMES` reads of the one target and four decodes, about 8/3 of a
  three-tube frame. The one target's arrays may be the only ones allocated.
- `tests/stripe_tube.rs`: each test under 7 s in release on a trial build of the join.

### Out of scope

- The front ends (command line, GUI, presets and `docs/CAMERA.md`'s "Using it"): a later stage.
- The WV-3110's corrections: R/B gamma tracking for dim areas, the high-Y chroma clip that limits green
  highlights, and the dynamic focus against colour shading. Also left out: left-edge and colour shading
  (Geometry), green warm-up (Retention), and the dark shading in luminance only.
- The stripes' dye spectra and the target's spectral response (Spectra). Index systems (Trinicon), the
  JVC G/cyan/clear stripes, the PK-958's single 5 MHz carrier, and two-tube cameras.
- A stripe phase that varies continuously within a row's sub-rows; the 2/3-inch and 1/2-inch spot and
  format.
