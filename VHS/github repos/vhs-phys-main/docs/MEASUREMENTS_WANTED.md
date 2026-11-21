# Measurements wanted

This register lists every parameter and effect in the vhs-phys physical model that has no recorded measurement behind it. Every value listed here is either an estimate (`[est]`), a derivation from other estimates, or an effect left unmeasured by the published sources. If you have access to camera tubes, camcorders, VHS decks, or test tapes and can supply a measurement with test conditions, please send it.

Contact: (to be added)

## How to read this

Evidence marks used across the codebase and design notes:

- `[est]`: estimated value or assumed functional form with no direct measurement in the sources.
- `[DER]`: derived mathematically from other documented constants or measurements.
- `[DS]`: taken from a manufacturer data sheet.
- `[FIG]`: read from a published graph or figure.

Only `[est]` items and items that a source explicitly leaves unmeasured (including effects dropped for want of magnitudes) are listed here.

## Camera tubes: target materials

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| PbO dark current | `1.0` nA | `DARK_NA` in `crates/vhsphys-core/src/tube.rs` | 30 mm or 2/3-in PbO Plumbicon (e.g. XQ1020), capped dark current at 25 C and as a function of faceplate temperature (20-50 C) | Black-level pedestal drift and warm-up dark-current rise |
| Chalnicon peak-white signal current | `200.0` nA | `NEW_PEAK_SIGNAL_NA` in `crates/vhsphys-core/src/tube.rs` | 2/3-in Chalnicon (e.g. E5060 series), signal current at standard peak-white faceplate illuminance (in lx) | Target voltage swing and beam-lag operating point |
| Sb2S3 vidicon target voltage | `30.0` V | `VIDICON_TARGET_VOLTAGE_V` in `crates/vhsphys-core/src/tube.rs` | Named consumer/industrial Sb2S3 vidicon deck or camera service manual, regulated target bias voltage under normal auto-sensitivity and manual operation | Target capacitance, dark current, and highlight compression |
| Newvicon target voltage | `25.0` V | `NEWVICON_TARGET_VOLTAGE_V` in `crates/vhsphys-core/src/tube.rs` | Named Newvicon camera (e.g. Panasonic/National 2/3-in Newvicon), operating target voltage at standard scene illumination | Target capacitance and lag |
| Chalnicon target capacitance | `6.4e-9` F | `CHALNICON_CAPACITANCE_F` in `crates/vhsphys-core/src/tube.rs` | 2/3-in or 1-in Chalnicon, direct target capacitance measurement (pF) and 3rd-field decay lag at stated signal current (nA) and target voltage | Capacitive discharge lag and highlight decay rate |
| Newvicon and Chalnicon layer permittivity and thickness | not separately modelled | `crates/vhsphys-core/src/tube.rs` (`CAMERA_RESEARCH.md`) | Photoconductive layer thickness (um) and dielectric constant for ZnSe/ZnCdTe (Newvicon) and CdSe (Chalnicon) targets | First-principles target capacitance per unit scanned area |
| PbO blue trap parameters (two traps) | `[(0.12, 0.37, 0.066), (0.035, 3.9, 0.45)]` (capture share, capacity nA s, release time s) | `PBO_BLUE_TRAPS` in `crates/vhsphys-core/src/tube.rs` | PbO Plumbicon blue channel, step-off decay tail from 50 ms to 10 s at several highlight currents and temperatures | Blue-channel long tail and colour shift in dim decay |
| Saticon trap parameters (two traps) | `[(0.95, 0.14, 0.047), (0.027, 20.0, 0.10)]` (capture, capacity nA s, release s; with `SATICON_CAPACITANCE_F = 0.80e-9` F `[DER]` fitted to BC4908; fast capture `0.95` `[DER]` where `0.6` fits almost as well; second trap capacity `20.0` nA s `[est]`) | `SATICON_TRAPS`, `SATICON_CAPACITANCE_F` in `crates/vhsphys-core/src/tube.rs` | 2/3-in Saticon (BC4908 or H8397A), 50 ms lag at 30 / 50 / 200 nA with 0 / 5 / 10 nA bias on a real tube, plus step-off decay from 50 ms to 5 s after 5-30 s illumination | Saticon build-up/decay lag tails, fast-trap capture, second-trap capacity, and settling the BC4908 vs H8397A Fig. 6 disagreement and BC4391 absolute levels (>2x offset) |
| Saticon BC4391 tube size and gun geometry | not modelled | `docs/design/CAMERA_RETENTION.md` | BC4391 data sheet: scanned target dimensions, gun type, and lag curve vs H8397A/BC4908 | Whether BC4391 lag data can be scaled directly to 2/3-in Saticon |
| Sb2S3 vidicon trap parameters | `[(0.95, 5.0, 0.25), NONE]` | `VIDICON_TRAPS` in `crates/vhsphys-core/src/tube.rs` | Sb2S3 vidicon at 30 V target, step-off decay from 50 ms to 20 s at 50 nA and 200 nA signal current | Vidicon multi-second comet-tail decay |
| Electron beam vertical spot sigma | `0.65` frame-line pitches | `SIGMA_V_LINES` in `crates/vhsphys-core/src/tube.rs` | 1-in and 2/3-in PbO/Saticon/Vidicon tubes, vertical MTF (400 TVL modulation depth and full CTF curve) with diameter convention stated | Vertical sharpness and line-to-line beam overlap |
| Electron beam horizontal spot sigma | `40e-9` s | `SIGMA_H_S` in `crates/vhsphys-core/src/tube.rs` | 30 mm Plumbicon and 2/3-in tubes, horizontal square-wave CTF across 100-600 TVL at beam current 200-600 nA | Horizontal resolution and edge roll-off |
| ACT flyback beam current | `100_000.0` nA | `ACT_BEAM_NA` in `crates/vhsphys-core/src/tube.rs` | Plumbicon XQ1020/XQ1410 with ACT gun, fraction of ~100 uA cathode current actually landing on the target during flyback | Highlight compression knee in ACT mode |
| ACT flyback active duration | `8.0e-6` s | `ACT_FLYBACK_S` in `crates/vhsphys-core/src/tube.rs` | ACT camera service manual, duration of the grid-1 flyback pulse within the 10.9-12 us line blanking interval | Total charge discharged per line during ACT |
| ACT vertical beam span | `1.0 / 30.0` PH | `ACT_SPAN_PH` in `crates/vhsphys-core/src/tube.rs` | ACT Plumbicon, vertical defocus width (in active lines or picture height) of the flyback flood beam | Vertical halo around clipped highlights in ACT mode |
| Beam-bending search radius bound | `4.0` frame-line pitches | `BEND_RADIUS_MAX_LINES` in `crates/vhsphys-core/src/tube.rs` | Plumbicon/vidicon under 10x-100x overload spot, measured deflection of bright edge position (in lines or um) vs target potential step | Maximum highlight blooming displacement |
| Highlight blooming extent (`blooming`) | no measured extent | `CameraParams::blooming` in `crates/vhsphys-core/src/camera.rs` (`docs/CAMERA_RESEARCH.md`) | Point or bar highlight at 10x-100x peak white on PbO/Saticon/Vidicon: diameter growth (TVL or % PH) vs overload factor | Calibrated highlight halo and bloom width |
| 60 Hz Plumbicon lag curves | not modelled | `docs/CAMERA_RESEARCH.md` | 525-line / 59.94 Hz Plumbicon build-up and decay lag at 3rd field (50 ms) vs 50 Hz (60 ms) | Accurate 60 Hz vs 50 Hz lag comparison |
| Red-specific comet-tail mechanism | not modelled | `docs/design/CAMERA_SCAN.md` | Red-channel vs green/blue-channel comet-tail length on overloaded PbO Plumbicon at matched target current | Red highlight trailing asymmetry |
| Hybrid field/frame integration window | inferred, no measurement | `docs/CAMERA_RESEARCH.md` | Interlaced 2/3-in or 1-in tube under stroboscopic pulse: charge read in field 1 vs field 2 as a function of beam current | Motion-blur and interlace-aliasing accuracy |

## Camera tubes: retention

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| PbO held-charge collection capacity and release time | `[10.0, 10.0, 1.0]` nA s, `2.0` s | `PBO_COLLECTION`, `release_s` in `crates/vhsphys-core/src/tube.rs` | XQ1020 (or equivalent PbO Plumbicon) R/G/B channels, 45 V target, high-key highlight for 5-10 s: residual signal 1-10 s after capping as % of peak | Temporary PbO image retention ("burn" after bright static scenes) |
| Saticon held-charge capacity and release time | `14.0` nA s, `0.25` s | `SATICON_COLLECTION` in `crates/vhsphys-core/src/tube.rs` | 2/3-in Saticon (H8397A or BC4908), high-key highlight for 5 s: residual signal at 0.1-2 s after capping vs target voltage | Saticon short-term retention and sticking |
| Tube warm-up time constant | `600.0` s | `WARM_UP_S` in `crates/vhsphys-core/src/tube.rs` | Cold-start dark current and sensitivity drift sampled every 30 s for 30 min after power-on | Warm-up black-level and colour-balance drift |
| Sb2S3 burn-in rate, recovery time, and dark gain | `rate_per_s = 5.5e-6`, `recovery_s = 600.0`, `dark_gain = 0.75` (at `3500x` peak white) | `VIDICON_BURN` in `crates/vhsphys-core/src/tube.rs` (`docs/CAMERA.md`) | Sb2S3 vidicon exposed to calibrated static bar chart at 10x, 100x, and 1000x peak white for 10 s to 10 min: retained positive/negative signal and recovery curve over 30 min | Vidicon burn-in speed, sign, and fade rate |
| Burn-in kinetics for non-Sb2S3 materials | not modelled | `docs/CAMERA_RESEARCH.md`, `docs/design/CAMERA_RETENTION.md` | Same static overload exposure test on PbO, Saticon, Newvicon, and Chalnicon | Whether and how fast non-vidicon tubes retain high-dose burns |
| Permanent damage thresholds on model light scale | not modelled | `docs/design/CAMERA_RETENTION.md` ("Out of scope") | Calibrated flash or solar overload dose (in model peak-white-seconds) at which permanent target damage occurs for Sb2S3, Newvicon, and Saticon | Threshold where burn stops recovering |
| Sign of Saticon/Chalnicon retained image vs target voltage | not modelled | `docs/design/CAMERA_RETENTION.md` ("Out of scope") | Retained image polarity (positive vs negative) on Saticon and Chalnicon across target voltages (e.g. 20 V vs 50 V) | Polarity of retained ghost |
| Burn driven by infrared vs visible light | not modelled | `docs/design/CAMERA_RETENTION.md` ("Out of scope") | Burn-in rate with and without an IR-cut filter at equal visible illuminance | Sensitivity of burn-in to tungsten/sunlight IR |
| Tube ageing (burn fade, blue slump, red build-up) | not modelled | `docs/design/CAMERA_RETENTION.md` ("Out of scope") | Comparative lag, dark current, and burn sensitivity on new vs 500-2000 hour aged PbO and Saticon tubes | Long-term tube wear behaviour |
| Lag vs temperature beyond dark current | not modelled | `docs/design/CAMERA_RETENTION.md` ("Out of scope") | PbO and Saticon 3rd-field decay lag and held-charge release time measured at 20 C, 35 C, and 50 C | Warm-camera smear increase |
| Beam temperature vs beam current ("reduced beam") | not modelled | `docs/design/CAMERA_RETENTION.md` ("Out of scope") | Lag curve on a single tube at nominal beam vs reduced beam current (RCA Fig. 11-63 style) | Beam-landing lag under low beam reserve |

## Camera: optics and geometry

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| Faceplate glass refractive index | `1.5` | `GLASS_INDEX` in `crates/vhsphys-core/src/geometry.rs` | Manufacturer glass specification for PbO/Saticon/Vidicon faceplate window | Halation ring radius |
| PbO faceplate thickness | `1.5` mm | `PBO_FACEPLATE_MM` in `crates/vhsphys-core/src/geometry.rs` | Direct thickness measurement of 30 mm and 2/3-in Plumbicon faceplate windows | Halation ring radius around bright spots |
| PbO halation reflectance (R, G, B) | `[0.15, 0.08, 0.05]` | `PBO_HALATION_REFLECTANCE` in `crates/vhsphys-core/src/geometry.rs` | Point-source halo intensity vs radius at 450 nm, 550 nm, and 650 nm on a PbO Plumbicon | Red/green/blue halation ring brightness |
| Optical veil fraction and scale | `0.0215`, `0.40` PH | `SATICON_VEIL_FRACTION`, `SATICON_VEIL_SCALE_PH` in `crates/vhsphys-core/src/geometry.rs` | Black-patch contrast ratio (small black square on white field) on PbO, Newvicon, and Chalnicon tubes | Veiling glare lifting deep blacks |
| PbO white-shading edge fall-off | `0.03` (0.0 for vidicon, Newvicon, Chalnicon) | `PBO_WHITE_FALL` in `crates/vhsphys-core/src/geometry.rs` | Uniform white-field corner-to-centre signal ratio on PbO, Sb2S3, Newvicon, and Chalnicon with lens shading removed | Edge brightness and colour shading |
| Black shading sawtooth vs parabolic split | `0.0125` H/V sawtooth, `0.075` H/V parabolic | `BLACK_SHADING_H_SAW`, `BLACK_SHADING_V_SAW`, `BLACK_SHADING_H_PAR`, `BLACK_SHADING_V_PAR` in `crates/vhsphys-core/src/geometry.rs` | Capped-lens waveform monitor trace (horizontal line and vertical field) on 3-tube and single-tube cameras | Shape of dark-field shading |
| Consumer 3-tube registration size and pincushion | `1.12e-3`, `3.69e-3` (R `+`, B `-`, zones A `<= 0.5`, B `<= 2/3`, C `> 2/3`) | `CONSUMER_REG_SIZE`, `CONSUMER_REG_PINCUSHION` in `crates/vhsphys-core/src/geometry.rs` (`docs/design/CAMERA_SCAN.md`) | Registration error (ns or % PH) measured in zones A, B, and C across multiple consumer 3-tube cameras | Colour-fringing magnitude and pattern |
| Studio corner focus softening | `0.25` | `STUDIO_CORNER_FOCUS` in `crates/vhsphys-core/src/geometry.rs` | Resolution chart (400 TVL) modulation depth at screen corners vs centre on studio PbO and Saticon cameras | Corner softness |
| Out-of-picture target illumination | nearest active-picture sample | `crates/vhsphys-core/src/camera.rs` (`docs/design/CAMERA_SCAN.md`) | Over-scanned target border brightness when a bright object sits just outside the active raster | Edge flare from off-screen highlights |

## Camera: spectral taking

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| Wavelength integration bounds | `380.0`-`780.0` nm (`N_LAMBDA = 81`, `LAMBDA_STEP_NM = 5.0`) | `N_LAMBDA`, `LAMBDA_MIN_NM`, `LAMBDA_STEP_NM` in `crates/vhsphys-core/src/spectral.rs` | UV (< 380 nm) and near-IR (780-1100 nm) response of Newvicon, Chalnicon, and Sb2S3 with actual camera lens and IR-cut filter | IR/UV contamination under tungsten or daylight |
| Dichroic prism B/G and G/R crossover edges | `480.0` nm, `580.0` nm, width `5.0` nm | `PRISM_BG_EDGE_NM`, `PRISM_GR_EDGE_NM`, `PRISM_EDGE_WIDTH_NM` in `crates/vhsphys-core/src/spectral.rs` | Spectrophotometer transmission curves of a 3-tube broadcast prism splitter (R, G, B ports) | Colour matrixing and metamerism |
| Camera IR-cut filter edge and width | `680.0` nm, `10.0` nm | `IR_CUT_EDGE_NM`, `IR_CUT_WIDTH_NM` in `crates/vhsphys-core/src/spectral.rs` | Spectral transmission curve of camera IR-cut filter from 600 nm to 1000 nm | Red saturation and tungsten skin tones |
| Material spectral response `[est]` fill knots | unmeasured boundary/tail knots in `KNOTS_PBO`, `KNOTS_PBO_EXTENDED_RED`, `KNOTS_SB2S3`, `KNOTS_NEWVICON`, `KNOTS_SATICON`, `KNOTS_CHALNICON` | `crates/vhsphys-core/src/spectral.rs` | Monochromator quantum efficiency or spectral sensitivity curves from 380 nm to 780 nm (and 700-1000 nm for Newvicon/Saticon) | Spectral taking curves and colour fidelity |

## Camera: stripe filter

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| 2/3-in and 1/2-in stripe spot width and format geometry | keeps tube kind's widths | `docs/design/CAMERA_STRIPE.md` | Horizontal/vertical MTF and scanned raster width (mm) of 2/3-in and 1/2-in stripe-filter vidicon/Saticon/Newvicon camcorders | Stripe carrier modulation depth and chroma resolution |
| Trinicon stripe pitch, index frequency, and target data | not modelled | `docs/CAMERA_RESEARCH.md`, `docs/design/CAMERA_STRIPE.md` | Service manual / optical microscope measurement of Sony Trinicon stripe pitch, index electrode frequency, and target bias | Ability to model Trinicon single-tube cameras |
| Consumer camcorder S/N and resolution benchmarks | not modelled | `docs/CAMERA_RESEARCH.md` | Measured luminance S/N (dB), chroma S/N, and horizontal TVL for named Sony/JVC/RCA tube camcorders | Validation targets for consumer stripe cameras |
| WV-3110 R/B gamma tracking and high-Y chroma clip | not modelled | `docs/CAMERA_RESEARCH.md`, `docs/design/CAMERA_STRIPE.md` | Chroma-to-luma amplitude vs scene reflectance (10-100 IRE and highlight) on Panasonic WV-3110 or similar stripe camcorder | Highlight desaturation and colour tracking |
| Stripe-camera dynamic focus, left-edge/colour shading, and green warm-up | not modelled | `docs/design/CAMERA_STRIPE.md` | Corner focus, horizontal edge colour tint, and dark-shading R/G/B balance during warm-up on a single-tube stripe camera | Edge tint and warm-up colour shift |
| Stripe dye spectral transmission curves | not modelled | `docs/design/CAMERA_STRIPE.md` | Microspectrophotometer transmission (380-780 nm) of cyan, yellow, and clear/white stripe filter elements | Stripe-camera colour matrix accuracy |

## Camera electronics

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| Detail coring thresholds (studio and consumer) | `0.008` (studio), `0.015` (consumer) | `STUDIO_CORING`, `CONSUMER_CORING` in `crates/vhsphys-core/src/camera_processing.rs` | Sweep low-amplitude high-frequency detail signal into camera detail processor and measure dead-zone threshold (% of peak white) | Suppression of fine texture and grain |
| Era-specific knee, aperture boost (dB), and auto-iris loop constants | undocumented | `docs/CAMERA_RESEARCH.md` | Service manual schematics for studio and consumer cameras: knee slope/point, aperture boost (dB at 3-5 MHz), and auto-iris attack/decay time constants | Highlight roll-off, edge enhancement, and exposure pumping |
| Consumer processing defaults (`matrix`, `knee_range`, `corrections`) | `matrix = false`, `knee_range = None`, `flare/black_shading/white_shading = false` | `Processing::Consumer` in `crates/vhsphys-core/src/camera_processing.rs` | Service manuals for 1970s-1980s consumer cameras confirming which corrections are absent or simplified | Consumer colour fidelity and highlight clipping |
| Flare corrector APL integrator | frame mean `APL_ch` | `crates/vhsphys-core/src/camera_processing.rs` (`docs/design/CAMERA_SCAN.md`) | Time constant (fields/frames) and gain of analog flare-correction feedback loop in studio cameras | Black-level lift under bright scenes |

## Tape and transport

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| Electrolytic capacitor ageing curve and duty cycle | `0.14` powered duty, `c_ratio` `0.30 * a^0.9`, `esr_mult` `1 + 14 * a^2` | `electrolytic` in `crates/vhsphys-core/src/condition.rs` | Capacitance and ESR (100 Hz / 100 kHz) measured across dated VHS decks of known age and operating hours | Power-supply ripple, coupling roll-off, and servo drift |
| Resistor drift vs age and humidity | `(0.004 + 0.012 * humidity) * sqrt(age_years)` | `resistor_drift` in `crates/vhsphys-core/src/condition.rs` | Resistance drift of carbon-film/metal-film resistors in aged decks stored in dry vs humid climates | Bias, sync, and colour-circuit mistuning |
| Power supply load, capacitance, ESR, voltage, and PSRR | `i_load = 0.35` A, `c0 = 2200e-6` F, `esr0 = 0.06` ohm, `v_nom = 17.0` V, `v_need = 14.0` V, PSRR `62.0` dB, `hum_ire = leak + deficit * 7.0`, `smps_ire = 0.05 * esr_mult + deficit * 3.0` | `supply` in `crates/vhsphys-core/src/condition.rs` | Deck main-rail DC current, filter capacitor values, ESR, and measured 50/60 Hz hum bar and SMPS ripple (IRE) at video out | Hum bars and switching noise in picture and audio |
| `Condition::to_params` wear, clog, stretch, tension, and deck drift laws | phenomenological laws in `to_params` | `Condition::to_params` in `crates/vhsphys-core/src/condition.rs` | Before/after RF envelope, dropout rate, skew, and flutter measurements on decks with quantified head wear, clogged heads, and stretched tapes | Realistic ageing progression of presets |
| Head width matching at LP and EP | `h.width_um = self.tape_speed.track_pitch_um()` | `Condition::to_params` in `crates/vhsphys-core/src/condition.rs` (`docs/TAPE_SPEEDS.md`) | Measured video head pole-tip width (um) across multi-speed SP/LP/EP VHS decks (2-head vs 4-head) | Track-to-head noise and crosstalk at LP/EP |
| Media-noise CNR and AM scaling with track width | `CNR = cnr_db + 10 log10(width / 58 um)`, media AM factor `sqrt(58 / width)` | `docs/TAPE_SPEEDS.md` | RF carrier-to-noise ratio (dB) and AM noise spectrum measured on the same tape recorded at SP, LP, and EP on a calibrated deck | Luminance/chroma noise penalty at slower tape speeds |
| LP/EP sub-emphasis network form | high-pass + gain + diode limiter | `docs/RESEARCH.md` | Schematic and swept-sine transfer curve at multiple input levels (-20 dB to 0 dB) of the ST 32 LP/EP sub-emphasis stage | High-frequency transient overshoot and edge noise at LP/EP |
| Tension-to-spacing map and magnetic decay parameters | phenomenological map, `[est]` demagnetisation/thermal decay scale | `crates/vhsphys-core/src/condition.rs` (`docs/RESEARCH.md`) | RF envelope loss vs back-tension (g) and vs storage temperature/time on standard ferric/cobalt VHS tapes | High-frequency roll-off and dropout sensitivity |
| Record/playback emphasis non-linearity, head resonance, EQ shapes, dropout length stats, and TBE spectrum | open questions | `docs/RESEARCH.md` | Swept RF head response, deck EQ schematics, dropout duration histograms per tape grade, and horizontal line-timing jitter PSD (0-10 kHz) | FM distortion, ringing, dropout length distribution, and timebase wobble |

## Deck electronics, heads and the television

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| Head-wear automation track coefficients | `0.06` um extra spacing, `0.25` um pulse half-width (`h0_um`), `0.35` gain loss per unit of `head_wear` | `render_rf` in `crates/vhsphys-core/src/pipeline.rs` | Spacing loss, pulse width, and output of a head against its measured wear | HF roll-off, pulse broadening, and RF level loss as head wear increases |
| Tension automation track line displacement | `0.8e-6` s per unit of slack (`(-tension).max(0.0)`) | `render_rf` in `crates/vhsphys-core/src/pipeline.rs` | Skew at the head switch in microseconds against back tension in grams | Top-of-field flagging and horizontal line skew when tape tension drops |
| Neighbouring-track lateral weight cutoff | `0.003` | `render_rf` in `crates/vhsphys-core/src/pipeline.rs` | Weakest crosstalk that is visible on a monitor | Threshold below which adjacent-track crosstalk is not rendered |
| Debris particle tape-lift law | `0.25 * (r / 20.0)^0.6 * exp(0.5 * ln)` um, clamped to `0.03`..`3.0` um | `Tape::cell_defects` in `crates/vhsphys-core/src/tape.rs` | RF dropout depth against particle size | Head-to-tape spacing lift and dropout depth from debris particles |
| Debris cleaning over repeated passes | `exp(-passes / wash_passes) * 0.9 + 0.1` (`wash_passes = 3.0`) | `Tape::cell_defects`, `TapeParams` in `crates/vhsphys-core/src/tape.rs` | Dropout counts over repeated plays of one tape | Fraction of loose debris surviving repeated playbacks |
| Oxide-void demagnetised fraction | `0.8 + 0.2 * stick` (range `0.8` to `1.0`, leaving `0.0` to `0.2` of signal) | `Tape::cell_defects` in `crates/vhsphys-core/src/tape.rs` | RF amplitude inside a scratch | Residual RF amplitude inside missing-oxide voids |
| Tape crease length, slope, width, and lift | length `10_000.0`..`120_000.0` um, slope sigma `0.002`, `crease_lift_um = 0.8` um, `crease_width_um = 60.0` um (log-normal `0.4` sigma; halo `0.25 * lift` at `3.0 * sigma`) | `Tape::paint`, `TapeParams` in `crates/vhsphys-core/src/tape.rs` | Lift profiles of creased tape | Crease persistence, vertical drift, width, and dropout halo |
| Tape-end defect clustering scale | `10.0` m (`10.0e6` um at each end; `bathtub = 3.0`) | `Tape::bathtub`, `TapeParams` in `crates/vhsphys-core/src/tape.rs` | Dropout density against distance from the tape's ends | End-of-tape dropout clustering near hub and leader |
| Dropout compensator detector decay and release hold | `0.35e-6` s envelope decay, `6` RF samples release hold | `process_segment` in `crates/vhsphys-core/src/electronics.rs` | Response of a deck's dropout compensator to RF dips of known length | Minimum dropout duration that trips DOC and hold-off before release |
| Playback RF AGC gain floor and initial window | `0.25` minimum gain, `4000` RF samples initial level window | `process_segment` in `crates/vhsphys-core/src/electronics.rs` | Attenuation limit and attack of a deck's playback RF AGC | Maximum RF attenuation by AGC and initial gain settling |
| Supply hum gain-modulation share and ripple harmonics | `0.8` per 100 IRE gain share; `0.35` (2nd harmonic) and `0.2` (3rd harmonic) ripple weights | `supply_disturbance` in `crates/vhsphys-core/src/electronics.rs` | Hum bars on peak white against hum at blanking, and the ripple spectrum of an aged supply | Contrast modulation of hum bars vs additive hum, and harmonic shape of aged-capacitor ripple |
| Luma low-pass stop-band attenuation | `55.0` dB | `process_segment` in `crates/vhsphys-core/src/electronics.rs` | A deck's luma response above 4 MHz | Stop-band rejection of high-frequency demodulator products and noise |
| Media AM noise bandwidth and gain factor | `0.5e6` Hz (one-pole low-pass), `0.8` gain factor | `head_front_end` in `crates/vhsphys-core/src/head.rs` | Spectrum of the RF envelope's amplitude noise | Bandwidth and RMS level of multiplicative media AM noise |
| Pole-face contour echo low-band corner | `1.2e6` Hz | `CONTOUR_CORNER_HZ` in `crates/vhsphys-core/src/head.rs` | Low-band response of a head against its pole length | Upper frequency limit of pole-face contour echo (head bumps) |
| Readback pulse support | `8.0 * h` samples clamped to `16..56` | `render_pulses` in `crates/vhsphys-core/src/head.rs` | numerical, no measurement needed | Truncation of Lorentzian pulse tails under heavy head-to-tape lift |
| Time-base error frequency bands | slow drift `0.3`..`8.0` Hz (`1/f`), tension resonance `0.3`..`3.0` x `tension_hz`, stick-slip flat `100.0`..`3000.0` Hz | `Mech::new` in `crates/vhsphys-core/src/mech.rs` | Spectrum of line jitter from 0 to 5 kHz | Frequency distribution of capstan wow, tape-path resonance, and scrape flutter |
| Drum eccentricity harmonics and scale factor | `1 / k^2` over `3` harmonics, factor `1.3` | `Mech::base` in `crates/vhsphys-core/src/mech.rs` | Jitter amplitudes at the drum rate and its second and third harmonics | Harmonic roll-off and RMS scaling of drum-eccentricity jitter |
| Sync separator thresholds and search range | fall `>= 14.0` IRE, tip in `-65.0..=-15.0` IRE over samples `8..24`, at least `44` of `50` samples low, search `45` samples each way | `find_sync`, `decode_planes` in `crates/vhsphys-core/src/decode.rs` | Sync slicing level, noise immunity, and pull-in range of a consumer television | Horizontal sync detection, noise rejection, and pull-in range |
| Back-porch clamp gate | `120` samples after sync edge, `14` samples long | `decode_planes` in `crates/vhsphys-core/src/decode.rs` | Clamp pulse timing of a consumer television | Back-porch DC restoration window and susceptibility to burst bleed |
| Colour killer threshold and ACC/APC loop smoothing | `kill_threshold = 0.18` (no hysteresis), `acc_alpha = 0.1`, `apc_alpha = 0.2` | `DecodeParams`, `decode_planes` in `crates/vhsphys-core/src/decode.rs` | Burst level at which a deck's killer cuts in and out, and the settling time of its ACC and APC in lines | Colour mute threshold/chatter on weak bursts, and chroma gain/phase tracking speed |

## Audio

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| Linear audio ALC, saturation, bias coupling, low-cut, gap, spacing, wow/flutter, and hum parameters | `alc_threshold = 0.5`, `alc_ratio = 4.0`, `alc_attack_ms = 10.0`, `alc_release_ms = 500.0`, `sat_drive = 1.0`, `bias_coupling = 1.0`, `low_cut_hz = 80.0`, `gap_um = 1.0`, `nominal_spacing_um = 0.36`, `wow_gain = 65.0`, `flutter_gain = 4.0`, `max_delay_ms = 1.5`, `transport_step = 16`, `hum_per_ire = 1.3e-4`, `hum_fundamental = 0.4` | `AudioParams` in `crates/vhsphys-core/src/audio.rs` (`docs/AUDIO.md`) | Linear audio deck measurements: ALC attack/release (ms) and ratio, 3% THD saturation level, gap width (um), head-to-tape spacing loss, wow/flutter spectrum (DIN/WRMS), and power-supply hum feedthrough | Linear audio pumping, distortion, HF roll-off, pitch wobble, and hum |
| Hi-Fi audio noise floor, dropout hold, recovery, and crossfade | `HIFI_NOISE_RMS = 1.0e-4`, `HIFI_HOLD_MAX_S = 0.002`, `HIFI_RECOVER_FIELDS = 30`, `HIFI_XFADE_S = 0.005` | `crates/vhsphys-core/src/audio.rs` (`docs/AUDIO.md`) | VHS Hi-Fi deck: uncompanded FM carrier noise floor (dBFS), maximum dropout hold duration (ms), linear fallback recovery time (fields), and crossfade duration (ms) | Hi-Fi noise floor and dropout switching clicks/mutes |

## PAL

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| PAL luma pre-emphasis ratio | `5.0` | `PRE_EMPH_RATIO` in `crates/vhsphys-core/src/consts_pal.rs` | PAL VHS deck schematic or swept-frequency luma record pre-emphasis curve (dB, 0.1-5 MHz) | Edge overshoot and FM deviation |
| PAL white and dark clip levels | `WHITE_CLIP_IRE` (1.6x span), `DARK_CLIP_IRE` (-0.4x span) | `WHITE_CLIP_IRE`, `DARK_CLIP_IRE` in `crates/vhsphys-core/src/consts_pal.rs` | Oscilloscope measurement at FM modulator input on a PAL VHS deck under over-driven step pulse | Peak white/black FM over-modulation clipping |
| PAL head switch lead | `SWITCH_LEAD_LINES = 6.5 + 2.5` (6.5 H lead) | `SWITCH_LEAD_LINES` in `crates/vhsphys-core/src/consts_pal.rs` | Head-switching pulse timing relative to vertical sync on PAL VHS decks | Vertical position of head-switching tear |
| PAL LP slow-speed sub-emphasis rules | `ST32_SLOW_SPEED_RULES = false` (no 625-line source) | `ST32_SLOW_SPEED_RULES` in `crates/vhsphys-core/src/consts_pal.rs` (`docs/PAL.md`) | PAL LP VHS deck service manual: sub-emphasis network values and clip thresholds vs PAL SP | PAL LP sharpness and noise |
| PAL SP head CNR reference width | `49` um uses same `cnr_db` as 525-line `58` um | `docs/PAL.md` | Calibrated RF CNR (dB) on PAL SP (49 um) vs NTSC SP (58 um) using identical tape formulation | Baseline PAL SP noise level |
| PAL deck circuit figures and ageing laws | carried over from 525-line model | `docs/PAL.md` | PAL deck playback EQ peaks, head resonance, FM/luma/chroma filters, DOC, AGC, APC/ACC loop speeds, and ageing behaviour | PAL-specific colour/luma response and lock speed |

## Interference

| Quantity | Value used | Where | Measurement that would settle it | What it changes |
| --- | --- | --- | --- | --- |
| Head ferrite core magnetic coupling | `0.1` | `HeadMagnet::core_coupling` in `crates/vhsphys-core/src/interference_params.rs` | Induced head voltage under a calibrated 50/60 Hz Helmholtz coil magnetic field (uT) | Mains magnetic hum pickup in video and audio heads |
| Outside-world EMI coupling constants (all 21 fields) | `video_ire_per_vpm2 = 0.12`, `audio_fs_per_vpm2 = 8.0e-4`, `rf_loop_area_m2 = 5.0e-4`, `head_signal_v = 1.0e-3`, `preamp_lpf_hz = 8.0e6`, `ground_to_video = 0.15`, `clamp_residual = 0.1`, `ground_gain_per_ire = 0.008`, `ground_to_audio = 0.15`, `head_emf_v_per_ut_hz = 9.5e-8`, `audio_head_ref_v = 1.0e-4`, `dimmer_video_ire = 4.0`, `dimmer_ring_hz = 80.0e3`, `dimmer_ring_tau_s = 40.0e-6`, `dimmer_rf_ratio = 0.05`, `dimmer_rf_hz = 2.0e6`, `dimmer_rf_tau_s = 2.0e-6`, `dimmer_audio_fs = 0.02`, `dimmer_audio_hz = 5.0e3`, `dimmer_audio_tau_s = 300.0e-6` | `Coupling` in `crates/vhsphys-core/src/interference_params.rs` (`docs/INTERFERENCE.md`) | Bench tests on a VHS deck with calibrated E-field, B-field, ground-loop injection, and TRIAC dimmer: video IRE pickup, RF preamp interference, ring frequencies, decay time constants, and audio hum/buzz levels | Amplitude and waveform of external EMI, ground loops, and dimmer hash |
| CRT tube depth, deflection distance, and stripe pitch | `depth_m = 0.35`, `deflection_z_m = 0.27`, `stripe_mm = 0.7 / 3.0` | `CrtParams` in `crates/vhsphys-core/src/interference_params.rs` (`docs/INTERFERENCE.md`) | Physical measurements of target consumer CRT (screen depth, deflection-yoke centre to screen distance, and aperture-grille/shadow-mask pitch) | CRT geometry distortion and phosphor stripe moire |

## Index by file

- `crates/vhsphys-core/src/tube.rs`: PbO dark current, Chalnicon peak-white signal current, Sb2S3 vidicon target voltage, Newvicon target voltage, Chalnicon target capacitance, Newvicon and Chalnicon layer permittivity and thickness, PbO blue trap parameters, Saticon trap parameters, Sb2S3 vidicon trap parameters, electron beam vertical/horizontal spot sigmas, ACT flyback beam current, ACT flyback duration, ACT vertical beam span, beam-bending search radius bound, PbO held-charge collection capacity and release time, Saticon held-charge capacity and release time, tube warm-up time constant, Sb2S3 burn-in rate/recovery/dark gain.
- `crates/vhsphys-core/src/camera.rs`: Highlight blooming extent (`blooming`), out-of-picture target illumination.
- `crates/vhsphys-core/src/geometry.rs`: Faceplate glass refractive index, PbO faceplate thickness, PbO halation reflectance, optical veil fraction and scale, PbO white-shading edge fall-off, black shading sawtooth vs parabolic split, consumer 3-tube registration size and pincushion, studio corner focus softening.
- `crates/vhsphys-core/src/spectral.rs`: Wavelength integration bounds, dichroic prism B/G and G/R crossover edges, camera IR-cut filter edge and width, material spectral response `[est]` fill knots.
- `crates/vhsphys-core/src/camera_processing.rs`: Detail coring thresholds (studio and consumer), consumer processing defaults, flare corrector APL integrator.
- `crates/vhsphys-core/src/condition.rs`: Electrolytic capacitor ageing curve and duty cycle, resistor drift vs age and humidity, power supply load/capacitance/ESR/voltage/PSRR, `Condition::to_params` wear/clog/stretch/tension/deck drift laws, head width matching at LP and EP, tension-to-spacing map and magnetic decay parameters.
- `crates/vhsphys-core/src/pipeline.rs`: Head-wear automation track coefficients, tension automation track line displacement, neighbouring-track lateral weight cutoff.
- `crates/vhsphys-core/src/tape.rs`: Debris particle tape-lift law, debris cleaning over repeated passes, oxide-void demagnetised fraction, tape crease length/slope/width/lift, tape-end defect clustering scale.
- `crates/vhsphys-core/src/electronics.rs`: Dropout compensator detector decay and release hold, playback RF AGC gain floor and initial window, supply hum gain-modulation share and ripple harmonics, luma low-pass stop-band attenuation.
- `crates/vhsphys-core/src/head.rs`: Media AM noise bandwidth and gain factor, pole-face contour echo low-band corner, readback pulse support.
- `crates/vhsphys-core/src/mech.rs`: Time-base error frequency bands, drum eccentricity harmonics and scale factor.
- `crates/vhsphys-core/src/decode.rs`: Sync separator thresholds and search range, back-porch clamp gate, colour killer threshold and ACC/APC loop smoothing.
- `crates/vhsphys-core/src/audio.rs`: Linear audio `AudioParams` (ALC, saturation, bias coupling, low-cut, gap, spacing, wow/flutter, hum), Hi-Fi audio noise floor/dropout hold/recovery/crossfade.
- `crates/vhsphys-core/src/consts_pal.rs`: PAL luma pre-emphasis ratio, PAL white and dark clip levels, PAL head switch lead, PAL LP slow-speed sub-emphasis rules.
- `crates/vhsphys-core/src/interference_params.rs`: Head ferrite core magnetic coupling, outside-world EMI coupling constants (all 21 `Coupling` fields), CRT tube depth/deflection distance/stripe pitch.
- `docs/CAMERA.md`: Sb2S3 burn-in lamp irradiance (`3500x` peak white).
- `docs/CAMERA_RESEARCH.md`: Newvicon/Chalnicon layer permittivity and thickness, highlight blooming extent, 60 Hz Plumbicon lag curves, hybrid field/frame integration window, burn-in kinetics for non-Sb2S3 materials, Trinicon stripe pitch/index frequency/target data, consumer camcorder S/N and resolution benchmarks, WV-3110 R/B gamma tracking and high-Y chroma clip, era-specific knee/aperture boost/auto-iris loop constants.
- `docs/design/CAMERA_SCAN.md`: Consumer registration signs and zone radii, out-of-picture target illumination, red-specific comet-tail mechanism, flare corrector APL integrator.
- `docs/design/CAMERA_RETENTION.md`: Saticon BC4391 tube size and gun geometry, burn-in kinetics for non-Sb2S3 materials, permanent damage thresholds on model light scale, sign of Saticon/Chalnicon retained image vs target voltage, burn driven by infrared vs visible light, tube ageing, lag vs temperature beyond dark current, beam temperature vs beam current.
- `docs/design/CAMERA_STRIPE.md`: 2/3-in and 1/2-in stripe spot width and format geometry, Trinicon stripe pitch/index frequency/target data, WV-3110 R/B gamma tracking and high-Y chroma clip, stripe-camera dynamic focus/left-edge and colour shading/green warm-up, stripe dye spectral transmission curves.
- `docs/AUDIO.md`: Linear audio and Hi-Fi `[est]` parameter documentation.
- `docs/TAPE_SPEEDS.md`: Head width matching at LP and EP, media-noise CNR and AM scaling with track width.
- `docs/RESEARCH.md`: LP/EP sub-emphasis network form, tension-to-spacing map and magnetic decay parameters, record/playback emphasis non-linearity, head resonance, EQ shapes, dropout length stats, and TBE spectrum.
- `docs/PAL.md`: PAL LP slow-speed sub-emphasis rules, PAL SP head CNR reference width, PAL deck circuit figures and ageing laws.
- `docs/INTERFERENCE.md`: Outside-world EMI coupling estimates and CRT shape estimates.
