# Phosphor schematic design specification

## Intent

The vhs-phys interface replaces a multi-group parameter form with a three-page `1180 x 620` technical schematic rendered in the style of a monochrome green-phosphor cathode-ray monitor and a 1980s vector explainer diagram. The first page (`python/vhsgui/schematic/`) depicts the internal helical-scan NTSC signal chain (`20` controls across `8` blocks); the camera page (`python/vhsgui/schematic/camera/`) depicts the camera ahead of the recorder, from the lens to the encoder (`13` controls across `7` blocks); the outside page (`python/vhsgui/schematic/outside/`) depicts external physical interference sources acting on the deck and picture tube from outside the circuit (`18` controls across `7` blocks). All three pages share the same canvas, phosphor palette, font stack, animation parameters, bottom hint bar, and right-hand television monitor preview, so turning any control shows its effect on the live picture immediately. Each adjustable parameter sits directly below the physical assembly it governs as a monospace row (`LABEL ........ VALUE`) underlined by a 1 px proportional gauge. Hierarchy comes from phosphor brightness and stroke weight, with one colour and few type sizes.

## Palette

All drawing uses a single P1/P31 green phosphor ramp on a near-black CRT faceplate, plus one amber accent reserved exclusively for modified values and fault indicators.

| Token | Hex | Hue (deg) | Saturation | Contrast vs `bg` | Role |
|---|---|---|---|---|---|
| `bg` | `#010804` | 146 | 0.88 | 1.0 : 1 | Unlit CRT glass background |
| `faint` | `#0e4a22` | 140 | 0.81 | 1.9 : 1 | Shape fills, reference axes, leader lines, unlit gauge track (never text) |
| `dim` | `#1c9a45` | 140 | 0.82 | 5.5 : 1 | Secondary chassis outlines, block captions, callouts, and legend lines |
| `phosphor` | `#39ff6e` | 136 | 0.78 | 15.1 : 1 | Primary vector drawings, active control labels, wire labels, lit gauge bar |
| `hot` | `#d8ffe3` | 137 | 0.15 | 18.6 : 1 | Overdriven beam core: head gaps, cathode beam, canvas title, hovered rows |
| `warn` | `#ffb347` | 35 | 0.72 | 11.4 : 1 | Single amber accent: non-default edited values and fault markers |

Body text (`phosphor` on `bg`) achieves 15.1 : 1 WCAG contrast; secondary labels and captions (`dim` on `bg`) achieve 5.5 : 1; hovered or active values (`hot` on `bg`) achieve 18.6 : 1. Because `faint` measures 1.9 : 1 against `bg`, it is restricted to non-text geometry.

## Typography

- **Family stack**: `["JetBrains Mono", "DejaVu Sans Mono", "monospace"]`
- **Base size**: `12 px` for all control rows, block titles, wire labels, and the bottom hint bar; `14 px` for the top canvas title; `11 px` (the strict minimum on the canvas) for block captions, glyph callouts, and technical legend lines.
- **Character advance and tracking**: Monospace advance is `0.602 * size` plus `0.6 px` letter tracking (`7.824 px` per character at `12 px`; `7.222 px` per character at `11 px`). Every control row leaves at least 3 leader dots between its uppercase label and its widest formatted value within `w - 4 px`.
- **Block captions**: Printed at `(x + 8, y + h - 7)` in the `dim` tier at `11 px` (`7.222 px` per character) and must fit within `w - 16 px`: at most `25` characters in a `200 px` block, `28` characters in the `224 px` cassette block, `30` characters in a `236 px` block, and `35` characters in a `270 px` block (`tv` carries no caption on either page).
- **Brightness tiers in text**:
  - `hot`: canvas title, active or hovered control row, `REC` indicator, and capacitor `C` callout.
  - `norm` (`phosphor`): standard control labels, block titles, wire labels (`VIDEO`, `SOUND`, `DUB`, `MAINS`), and primary component or physics identifiers (`COPY N+1`, `T-120 VHS`, `OXIDE`, `+/-6 DEG`, `CTL`, `RIPPLE`, `1H`, `Y/C`, `217 HZ`, `3.4-4.4 Y FM`, `GND`, `4X -> 1X`, `V X B`, `SAT / ERASE`).
  - `dim`: block captions, secondary callouts (`1VPP`, `SYNC`, `FM MOD`, `DUB`, `CREASE`, `180 DEG`, `A`, `B`, `CLOG`, `DX`, `PINCH`, `TENS`, `ESR`, `AC`, `RCA`, `PWR`, `CH 3`, `HOLD`, `900 MHZ`, `577 US // 1/8`, `0.63 C`, `LOOP`, `59.94 HZ`, `TRIAC`, `CAGE`, `GUN 0`, `35 CM`, `B FLUX`), and technical legend lines.
  - `faint`: never used for text.
- **Static constants vs. controls**: Every numeric or format text in the glyphs and legend lines states a fixed NTSC VHS physical constant, carrier band, or coordinate endpoint (`59.94 HZ`, `217 HZ`, `577 US // 1/8`, `0.63 C`, `3.4-4.4 Y FM`, `GUN 0 .. 35 CM`, `4X -> 1X`) rather than a selectable mode or distance, so no static label can ever disagree with an adjustable control setting.

## Stroke system

Every vector primitive is painted without blur shaders by stacking three concentric glow passes (`w: 9.0, a: 0.05` -> `w: 5.0, a: 0.13` -> `w: 2.6, a: 0.32`) under the crisp core stroke, overlaid with a `3 px` scanline grating (`alpha: 0.16`) and a `0.35` corner vignette.

- **Weights**:
  - `hair` (`1.0 px`): leader lines from part to control row, dimension ticks, internal reference axes, reel tape-pack rings, and spring zigzags.
  - `thin` (`1.6 px`): standard component outlines, cassette windows, reels, signal waveforms, RF envelopes, and connector barrels.
  - `bold` (`2.6 px`): primary silhouettes (cassette shell, rotating drum cylinder, ferrite head tips, capstan and pinch roller, capacitor plates, transformer core, GSM handset, CRT side envelope, permanent magnets, CRT cabinet).
- **Brightness tiers**: `faint`, `dim`, `norm`, and `hot` scale beam intensity from background structure up to active contact points (tape wrap arcs, ferrite gaps, active TDMA burst slots, and deflected electron beams).
- **Dash patterns**:
  - `[5, 3]` and `[4, 3]`: mechanical hinge seams, cassette throat guides, and Faraday shield cage walls.
  - `[4, 2]` and `[3, 2]`: internal rotor bridges, mistracked helical scan paths, copy-loop feedback paths, RF modulation envelopes, ground return loops, and magnetic flux lines.
  - `[3, 3]` and `[2, 2]`: zero-volt signal baselines, CRT optical centerlines, and vertical azimuth reference axes.

## Page 1: Circuit schematic (`python/vhsgui/schematic/`)

### Grid

The circuit page divides the `1180 x 620 px` canvas into five vertical columns separated by `20 px` gutters:

| Column | X (px) | Width (px) | Blocks (`y .. y+h`) | Control row `x, w` |
|---|---|---|---|---|
| 1 | 20 | 200 | `source` (`40..236`), `record_deck` (`252..520`) | `x: 26, w: 188` |
| 2 | 240 | 224 | `camera` (`40..180`), `cassette` (`196..571`) | `x: 246, w: 211 / 212` |
| 3 | 484 | 200 | `head_drum` (`40..284`), `transport` (`300..520`) | `x: 490, w: 188` |
| 4 | 704 | 200 | `power` (`40..284`), `output` (`344..520`) | `x: 710, w: 188` |
| 5 | 924 | 236 | `tv` (`40..400`) + `VIDEO` bus (`424`) + legend (`450..530`) | `x: 930, w: 224` (`STANDARD`) |

- **Vertical rhythm**: Stacked blocks in columns 1 and 3 are separated by a `16 px` gutter; in column 4, `power` (`40..284`) aligns its bottom edge and lower three control rows (`186, 208, 230`) with `head_drum` (`40..284`). Every block reserves its top `28 px` for the title header and at least `32 px` at the bottom for its caption.
- **Control rows**: Each row is `18 px` tall (`16 px` text plus `1 px` gauge line) on a `22 px` vertical pitch (`4 px` clear between rows), aligned to `block.x + 6` and `block.x + block.w - 6`.
- **Bottom bands**: `y = 520..580` is the horizontal wire raceway for the `SOUND` and `DUB` buses; `y = 594..611` is the exclusive band for the `hint` explanation line (baseline `y = 606`).

### Symbol vocabulary (circuit page)

| Glyph | Physical part depicted | Block | Adjacent controls |
|---|---|---|---|
| `source_gate` | 35 mm 4:3 film aperture with sprocket holes, lens reticle, and 1 Vpp sync step waveform | `source` | `PRESET`, `SEED` |
| `pickup_tube` | Camera pickup tube showing lens, photoconductive target, deflection yoke, and electron gun | `camera` | none: a `dim` label `SET ON THE CAMERA PAGE` at `(246, 142)` |
| `rec_deck` | Top view of VCR cassette throat, `REC` counter window, `FM MOD` modulator, and record head amp | `record_deck` | `REC AGE`, `REC TEMP` |
| `dub_loop` | Coaxial dubbing input jack and `COPY N+1` cascade feedback loop (`DUB`) | `record_deck` | `GENERATIONS` |
| `cassette` | 188 x 104 mm VHS shell (`1.86:1`) with turning supply/take-up reels, window, write-protect tab, and `T-120 VHS` label | `cassette` | `SPEED`, `PLAYS`, `POSITION` |
| `tape_strip` | Magnified 12.7 mm tape ribbon cross-section showing `OXIDE` layer, Mylar base, and edge `CREASE` | `cassette` | `TAPE AGE`, `STORAGE`, `QUALITY`, `STRETCH` |
| `drum` | 62 mm rotating upper head drum seen from above with `180 DEG` M-wrap tape path and two heads | `head_drum` | `HEAD WEAR`, `DRUM WEAR` |
| `azimuth_pair` | Opposed ferrite head gaps (`A`, `B`) tilted at `+/-6 DEG` azimuth with gap `CLOG` debris | `head_drum` | `HEAD CLOG` |
| `ctl_head` | Stationary control-track sync head (`CTL`) and skewed helical scan track offset (`DX`) | `transport` | `TRACK ERROR`, `TRACK SLOPE` |
| `capstan` | Spring-loaded tension arm (`TENS`), guide posts, steel capstan shaft, and rubber pinch roller (`PINCH`) | `transport` | `TENSION` |
| `capacitor` | Polarized electrolytic capacitor marked `C` and series `ESR` with thermometer and heat wave | `power` | `DECK AGE`, `CAP TEMP`, `HUMIDITY` |
| `transformer` | Two-prong `AC` mains plug, iron-core transformer windings, and supply `RIPPLE` waveform | `power` | `SMPS PSU` |
| `comb_filter` | 1H delay line (`1H`), summer node, and notch/comb passband dip curve | `output` | `SIGNAL` |
| `av_socket` | 4-pin mini-DIN S-Video socket (`Y/C`) and coaxial RCA composite/audio jack (`RCA`) | `output` | `SOUND` |
| `tv` | CRT monitor cabinet with chamfered bezel, `204 x 153` (4:3) `screen`, speaker grille, `PWR` switch, and `CH 3`/`HOLD` knobs | `tv` | live preview window |

### Wire language (circuit page)

- `video`: Primary picture and sync signal path flowing left to right from `source` (`w_source`) through `record_deck` (`w_record`), `cassette` (`w_tape`, `w_ctl`), `head_drum`/`transport` (`w_servo`, `w_heads`), and exiting `output` (`w_out`, labelled `VIDEO`) horizontally into the bottom center of `tv`.
- `audio` (`SOUND`): Linear edge-track audio path (`w_audio`) exiting the bottom of `cassette` (`x = 352`), running along the `y = 546` raceway, and entering the bottom of `output` (`x = 760`).
- `dub` (`DUB`): Multi-generation copy loop (`w_dub`) exiting the bottom of `output` (`x = 848`), running left along `y = 574` outside the audio bus, and re-entering the bottom of `record_deck` (`x = 120`) directly below `GENERATIONS`.
- `power`: Supply rails (`w_mech`, `w_power_out`) carrying regulated DC and capacitor ripple from `power` left into `head_drum` (`y = 100`) and down from `y = 284` into `output` (`x = 804, y = 344`).

### Camera block and standard labels

The `camera` block sits at the top of column 2, between the input `source` and the recording deck, with a video feed (`w_camera`) into the recorder. It keeps its `pickup_tube` drawing and caption but no control rows: the camera's settings live on the camera page, and the block says so with the `dim` 11 px label `SET ON THE CAMERA PAGE` at `(246, 142)` where its `TUBE` and `IRIS` rows were. In the `tv` block, `STANDARD` selects `ntsc`, `pal`, or `pal60`. The technical spec labels along the right column and top header switch dynamically using `when: {"standard": [...]}` to display the exact parameters of the active television standard: raster line count and frame rate, luma FM carrier band (3.4-4.4 MHz vs 3.8-4.8 MHz), colour-under subcarrier (629 kHz vs 627 kHz), drum speed (1800 rpm vs 1500 rpm), and tape speeds (SP/EP vs SP/LP).

## Page 2: Outside the circuit (`python/vhsgui/schematic/outside/`)

### Grid

The outside-world page uses four equal columns of `270 px` width separated by `20 px` gutters (`x = 20, 310, 600, 890`), with a `16 px` vertical gutter between the upper and lower block in each of the first three columns:

| Column | X (px) | Width (px) | Blocks (`y .. y+h`) | Control row `x, w` |
|---|---|---|---|---|
| 1 | 20 | 270 | `phone` (`40..254`), `radio` (`270..520`) | `x: 26, w: 258` |
| 2 | 310 | 270 | `mains` (`40..320`), `coupling` (`336..520`) | `x: 316, w: 258` |
| 3 | 600 | 270 | `crt_magnet` (`40..298`), `head_magnet` (`314..520`) | `x: 606, w: 258` |
| 4 | 890 | 270 | `tv` (`40..400`) + `MAINS` bus (`424`) + legend (`450..530`) | none (preview screen) |

- **Horizontal alignment across columns**: All three upper blocks (`phone`, `mains`, `crt_magnet`) start at `y = 40`, place their `100 px` tall glyph drawings at `y = 70..170`, and align their first two control rows at `y = 178` and `y = 200`. All three lower blocks (`radio`, `coupling`, `head_magnet`) end at `y = 520`, align their bottom two control rows at `y = 444` and `y = 466`, and align their captions at baseline `y = 513`.
- **Visual hierarchy (squint test)**: The `tv` cabinet (`254 x 300` with a `216 x 162` 4:3 screen) reads first, followed by the three primary physical silhouettes built with `bold` closed outlines and `faint`/`dim` fills: the flared CRT side-elevation assembly (`crt_tube`), the rotating head drum and U-magnet (`drum_magnet`), and the GSM handset with its burst frame (`phone_gsm`). The remaining glyphs (`radio_rf`, `mains_ground`, `mains_dimmer`, `shield_cage`) use open `thin` and `hair` linework so they never compete with the primary forms.

### Symbol vocabulary (outside page)

| Glyph | Physical part and mechanism depicted | Block | Adjacent controls |
|---|---|---|---|
| `phone_gsm` | 900 MHz GSM handset radiating wavefront arcs, 8-slot TDMA frame with one active `577 US` burst (`217 HZ`, `1/8`), and distance dimension line | `phone` | `MODE`, `DISTANCE` |
| `radio_rf` | Whip antenna with ground plane, AM-modulated RF carrier envelope, head-lead pickup `LOOP` feeding the ferrite head gap, and spectrum strip marking the `0.63 C` chroma-under and `3.4-4.4 Y FM` luma beat bands | `radio` | `MODULATION`, `FREQUENCY`, `FIELD` |
| `mains_ground` | Wall socket and two-prong plug, drifting `60 Hz` vs `59.94 HZ` crystal sine waves, and deck-to-TV earth symbols linked by a `GND` loop | `mains` | `MAINS`, `SAG`, `GROUND LOOP` |
| `mains_dimmer` | Mains transformer core radiating stray flux arcs into the audio head gap, paired with a `TRIAC` symbol and phase-cut sine wave exhibiting post-step ringing | `mains` | `STRAY FIELD`, `DIMMER` |
| `shield_cage` | Approach vector arrow closing in from `4X -> 1X` range toward a dashed Faraday `CAGE` enclosure that attenuates incident RF waves | `coupling` | `SHIELDING`, `APPROACH` |
| `crt_tube` | Side view of CRT neck, electron gun (`GUN 0`), deflection yoke, flared funnel, phosphor faceplate (`35 CM`), external bar magnet, Lorentz-bent electron beam (`V X B`), and polar angle reticle | `crt_magnet` | `TUBE MAGNET`, `POSITION`, `DISTANCE`, `ANGLE` |
| `drum_magnet` | Top view of 62 mm head drum with `180 deg` tape wrap and ferrite heads next to a U-magnet driving `B FLUX` loops that saturate the head core and erase the tape (`SAT / ERASE`) | `head_magnet` | `HEAD MAGNET`, `DISTANCE` |
| `tv` | CRT monitor cabinet (`254 x 300`) with chamfered bezel, `216 x 162` (4:3) live `screen`, speaker grille, `PWR` switch, and `CH 3`/`HOLD` knobs | `tv` | live preview window |

### Wire and motion language (outside page)

- **Wires**: Electromagnetic and magnetic fields (`phone`, `radio`, `crt_magnet`, `head_magnet`) act across free space and are depicted by radiated wavefront arcs and dashed flux loops rather than schematic wires. Only the physical AC supply and ground-loop connection is wired: a `power` bus (`w_mains`, labelled `MAINS`) exits the right side of `mains` at `(580, 306)`, runs along the center of the `y = 306` horizontal gutter between `crt_magnet` and `head_magnet`, drops down the `x = 880` vertical gutter to `y = 424`, and enters the bottom center of `tv` at `(1025, 400)`.
- **Motion**: Shares the exact `animation` object of the circuit page (`pulse_speed: 90`, `pulse_spacing: 70`, `reel_rpm: 12`, `flicker: 0.03`, `boot_ms: 1800`, `refresh_flash_ms: 250`). Signal pulses travel along `w_mains` from `mains` into `tv`; no `reel` primitives are used on the outside page.

## Page 3: The camera (`python/vhsgui/schematic/camera/`)

The camera page draws the camera ahead of the recorder in the order the light and then the signal travel through it: scene and lens with its iris, the colour split (a dichroic prism, or a stripe filter), the pickup tube(s), the head amplifier, the processing amplifier, the encoder, and the TV that shows the result. It holds every camera setting of the window and nothing else; the circuit page keeps only the drawing of its `camera` block. The data files are `python/vhsgui/schematic/camera/layout.json` and `glyphs.json`; the tables below say what they contain.

### Grid

The camera page uses the outside page's grid: four `270 px` columns at `x = 20, 310, 600, 890` with `20 px` gutters, a `16 px` vertical gutter between the upper and lower block of each of the first three columns, control rows at `block.x + 6`, `w = 258`, and the same `tv` block, `tv` glyph, `VIDEO` wire route and legend positions as the outside page.

| Column | Blocks (`y .. y+h`) | Drawings (`x, y`) | Control rows (`y`) |
|---|---|---|---|
| 1 | `lens` LENS AND IRIS (`40..254`), `split` COLOUR SPLIT (`270..520`) | `lens_iris` (`28, 70`); `prism` or `stripe_filter` (`28, 300`) | `IRIS` 178, `LENS CAP` 200; `LAYOUT` 444, `SPECTRAL COLOUR` 466 |
| 2 | `tubes` PICKUP TUBES (`40..386`), `head_amp` HEAD AMPLIFIER (`402..590`) | `tubes_three` or `tube_one` (`318, 70`); `head_amp` (`318, 432`) | `TUBE` 178, `TUBE TEMP` 200, `SCAN GEOMETRY` 222, `BEAM` 244, `BLOOMING` 266, `COLD START` 288, `START TEMP` 310, `BEAM OFF` 332; none |
| 3 | `processing` PROCESSING AMP (`40..386`), `encoder` ENCODER (`402..590`) | `proc_amp` (`608, 70`); `encoder` (`608, 432`) | `ELECTRONICS` 178; none |
| 4 | `tv` (`40..400`) + `VIDEO` wire (`424`) + legend (`450..530`) | `tv` (`898, 48`) | none (preview screen) |

- **Flow**: `w_light` drops from `lens` into `split` (`x = 127`); `w_targets` leaves `split` on the right (`y = 352`), climbs the `x = 300` gutter and enters `tubes` at the targets (`y = 114`); `w_signal` drops from the targets' signal lead into `head_amp` (`x = 348`); `w_head` leaves `head_amp` on the right (`y = 462`), climbs the `x = 590` gutter and enters `processing` (`y = 100`); `w_proc` drops into `encoder` (`x = 858`); `w_out` (`VIDEO`) leaves `encoder` at `y = 458`, climbs the `x = 880` gutter and runs along `y = 424` into the bottom centre of `tv` (`1025, 400`). Every wire is of kind `video`.
- **Block centres** read left to right: `lens` = `split` < `tubes` = `head_amp` < `processing` = `encoder` < `tv`.
- **Captions** (at most `35` characters): `EXPOSES THE TARGETS IN STOPS`, `SPLITS THE LIGHT INTO COLOURS`, `STORE CHARGE, LAG AND SMEAR`, `LIFTS NANOAMPS TO VOLTS`, `CLAMPS, MATRIXES, KNEES, CORRECTS`, `ADDS COLOUR BURST AND SYNC`.

### Controls (camera page)

| Line | id | Block | Kind | Range or choices |
|---|---|---|---|---|
| `IRIS` | `iris_stops` | `lens` | slider | `-2 .. 6` stops, step `0.5`, 1 decimal (`IRIS_STOPS_RANGE`) |
| `LENS CAP` | `camera_idle_s` | `lens` | slider | `0 .. 1800` s, step `10`, 0 decimals (`CAMERA_IDLE_RANGE`): capped time before the shot |
| `LAYOUT` | `camera_layout` | `split` | choice | `three-tube`, `stripe` (`CAMERA_LAYOUTS`) |
| `SPECTRAL COLOUR` | `camera_spectral` | `split` | toggle | |
| `TUBE` | `camera_tube` | `tubes` | choice | `CAMERA_TUBES`, in order |
| `TUBE TEMP` | `camera_temperature_c` | `tubes` | slider | `0 .. 60` °C, step `1`, 0 decimals (`CAMERA_TEMPERATURE_RANGE`) |
| `SCAN GEOMETRY` | `camera_geometry` | `tubes` | toggle | |
| `BEAM` | `camera_beam_ratio` | `tubes` | slider | `0.5 .. 4.0` x white, step `0.1`, 1 decimal (`CAMERA_BEAM_RATIO_RANGE`) |
| `BLOOMING` | `camera_blooming` | `tubes` | slider | `0 .. 3`, step `0.1`, 1 decimal (`CAMERA_BLOOMING_RANGE`) |
| `COLD START` | `camera_cold_start` | `tubes` | toggle | |
| `START TEMP` | `camera_cold_start_c` | `tubes` | slider | `-20 .. 60` °C, step `1`, 0 decimals (`CAMERA_COLD_START_RANGE`); works only while `COLD START` is on |
| `BEAM OFF` | `camera_beam_off_s` | `tubes` | slider | `0 .. 60` s, step `0.5`, 1 decimal (`CAMERA_BEAM_OFF_RANGE`): the beam is held off for the start of the shot |
| `ELECTRONICS` | `camera_processing` | `processing` | choice | `studio`, `consumer` (`CAMERA_PROCESSING`) |

Each setting has one line on one page: `TUBE` and `IRIS` moved here from the circuit page's `camera` block. `SPECTRAL COLOUR` sits with the prism because spectral taking is the prism's, the infrared filter's and the targets' colour response; `SCAN GEOMETRY` sits with the tubes because deflection, registration and shading belong to the yokes and targets.

**Without a tube the camera's lines are inert**, as the window disables its own camera controls: while `camera_tube` is `off`, the window calls `set_enabled(id, False)` for the six other lines. A disabled line is drawn in the `dim` tier (label, leader, value and gauge; no edited mark, no hover brightening), ignores clicks, drags, the wheel, the keys and double clicks, and still shows its help in the hint band when hovered, so the user reads why nothing happens. `TUBE` itself is always live.

### The layout switch

The layout switch changes the drawing itself. Two pairs of instances share a place and carry `when: {"camera_layout": [...]}` (see `docs/design/FORMAT.md`):

| `camera_layout` | `split` block | `tubes` block | Top-right label (`1160, 24`, right aligned) | Legend line at `y = 510` |
|---|---|---|---|---|
| `three-tube` | `prism`: infrared cut filter, pentagonal dichroic prism with crossed `hot` layers, three exits `R`, `G`, `B` | `tubes_three`: three small tubes (target, envelope, yoke, gun) with their targets' leads joined into one signal lead | `THREE TUBES BEHIND A PRISM` | `PRISM: ONE TUBE PER COLOUR` |
| `stripe` | `stripe_filter`: optical low-pass plate and a faceplate of alternating `norm`/`dim` stripes (`CY / YE`), one exit (`1 TARGET`) | `tube_one`: one larger tube with a striped target, its beam drawn to the target | `ONE TUBE BEHIND A STRIPE FILTER` | `STRIPES CODE COLOUR AT 3.58 MHZ` |

The other legend lines (`dim`, 11 px, `x = 890`) are fixed physics: `CLICK SCREEN TO ENLARGE` (450), `1 STOP = 2X LIGHT ON TARGET` (470), `DARK CURRENT DOUBLES WITH HEAT` (490), `THE BEAM READS EACH LINE'S CHARGE` (530). No static text names a tube, a profile or a temperature, so nothing printed can disagree with a control.

### Symbol vocabulary (camera page)

| Glyph | Part depicted | Block | Adjacent controls |
|---|---|---|---|
| `lens_iris` | A lit object (`SCENE`), its rays through a biconvex `LENS` and past two iris blades (`IRIS`) to an image point | `lens` | `IRIS`, `LENS CAP` |
| `prism` / `stripe_filter` | See "The layout switch" | `split` | `LAYOUT`, `SPECTRAL COLOUR` |
| `tubes_three` / `tube_one` | See "The layout switch"; callouts `TARGETS`, `YOKES`, `GUNS` or `STRIPED TARGET`, `YOKE`, `GUN` | `tubes` | `TUBE`, `TUBE TEMP`, `SCAN GEOMETRY`, `BEAM`, `BLOOMING`, `COLD START`, `START TEMP`, `BEAM OFF` |
| `head_amp` | The target's signal lead into a `LOAD` resistor and a `FET` amplifier, with its `NOISE` | `head_amp` | none |
| `proc_amp` | A chain `CLAMP`, `MATRIX`, `GAMMA`, `DETAIL` with sketches of the black level (`BLACK`, `0 V`), the knee, the gamma curve and the aperture overshoot | `processing` | `ELECTRONICS` |
| `encoder` | `R G B` into a `MATRIX`, a summing node adding the `BURST` and `SYNC` | `encoder` | none |
| `tv` | The outside page's TV, unchanged | `tv` | live preview window |

All callouts are 11 px in the `dim` or `norm` tier; nothing is `faint` or scaled below 1.0.

### Reaching the page

- A checkable `Camera` button (object name `cameraButton`) joins `Circuit` and `Outside` in the exclusive page group, in the order Camera, Circuit, Outside (the order the signal travels). Like them it is visible in schematic mode only.
- `MainWindow.camera` is the page's `SchematicView` (object name `cameraView`); `set_schematic_page("camera")` shows it; the window still starts on `circuit`. Each page plays its start-up sequence on its first visit only.
- `python -m vhsgui --page camera` starts on the camera page (`--page` takes `camera`, `circuit` or `outside`).
- The camera page's TV shows the same picture as the other two.

## Rules

- **Do**:
  - Keep every control row within a block aligned to the same `x` and `w`.
  - Place each control row directly below the glyph depicting the part it modifies, linked by a `hair` `faint` leader line that stops clear of all text.
  - Keep block captions (`11 px`, `dim`, at `(x + 8, y + h - 7)`) within `w - 16 px`: at most `25` characters in `200 px` blocks, `28` in `224 px` blocks, `30` in `236 px` blocks, and `35` in `270 px` blocks.
  - Use `bold` strokes only for primary physical silhouettes and active contact points.
- **Do not**:
  - Set any piece of text in the `faint` tier (`1.9 : 1` contrast) or smaller than `11 px`.
  - Allow any two pieces of text, control rows, or the hint band to overlap, or allow any text to straddle a block frame.
  - Allow any wire or glyph stroke to run through or touch any text.
  - Print any static number or mode name that an adjustable control can contradict.
  - Introduce any colour outside the six-token green phosphor and single amber (`warn`) palette, or place any element within `4 px` of the canvas border or inside the `y = 594..611` hint band.

## Rejected ideas

1. **Decorative PCB trace background**: Considered filling gutters with 45-degree circuit traces and via dots; rejected because fake traces compete with the real signal wires and obscure the left-to-right flow.
2. **Third track-footprint glyph in the cassette block**: Considered adding a separate `tape_tracks` glyph at the bottom of `cassette`; rejected because `ctl_head` in `transport` already draws the helical slant tracks and control track 40 px to the right.
3. **Side-elevation CRT electron-gun diagram below the TV**: Considered filling the lower right column with a second large `crt_neck` box; rejected because in the 60% squint test it made the TV column look like two stacked boxes instead of an unmistakable television monitor.
4. **Internal bounding boxes around sub-glyphs**: Considered wrapping `rec_deck`, `dub_loop`, and `azimuth_pair` in their own rectangular frames; rejected to avoid nested boxes inside block frames.
5. **Split-width control rows beside narrow drawings**: Considered narrowing control rows to `110 px` to sit horizontally next to tall glyphs; rejected because `12 px` monospace labels and units would truncate or break the single left/right alignment edge per block.
6. **High-speed 45 rpm reels and rapid wire pulses**: Considered faster reel rotation and `250 px/s` wire pulses; rejected as distracting during long editing sessions in favour of a calm `12 rpm` and `90 px/s`.
7. **Gutter wire labels**: Considered placing labels (`MASTER`, `FM REC`, `RF PB`, `CTL`, `SERVO`, `Y/C`, `DC`) on short inter-block wires inside the `20 px` vertical and `16 px` horizontal gutters; rejected because a `20 px` gutter fits at most two `12 px` characters, causing labels to straddle block frames and collide with wires.
8. **Schematic wires connecting wireless interference blocks**: Considered linking `phone`, `radio`, `crt_magnet`, and `head_magnet` to `tv` with `video` or `power` wires; rejected because radiated RF and magnetic fields couple through free space rather than cables, so only the physical AC cord and ground loop (`w_mains`) is drawn as a wire.
9. **Miniature second CRT silhouette inside `mains_dimmer`**: Considered drawing a small 18 px picture tube beside the audio head in `mains` to illustrate stray hum deflection; rejected because it read as visual clutter at 60% zoom and duplicated the large `crt_tube` side-elevation drawing immediately to the right in `crt_magnet`.
10. **Static numeric distance or frequency readouts on dimension lines**: Considered printing `20 CM` on the phone distance line or `3.8 MHZ` on the radio carrier; rejected because fixed numbers would contradict the live `DISTANCE` and `FREQUENCY` sliders, whereas physical constants (`217 HZ`, `577 US // 1/8`, `0.63 C`, `3.4-4.4 Y FM`, `59.94 HZ`, `GUN 0 .. 35 CM`) remain true at every setting.
11. **Uniform heavy silhouettes across all seven outside blocks**: Considered giving `radio_rf`, `mains_ground`, `mains_dimmer`, and `shield_cage` closed `bold` frames with fills; rejected so that in the squint test the `tv` monitor and the three primary assemblies (`crt_tube`, `drum_magnet`, `phone_gsm`) read in strict visual order without secondary blocks competing.
12. **The camera's new settings in the circuit page's camera block**: Considered growing the `camera` block to take `LAYOUT`, `ELECTRONICS`, `TUBE TEMP`, `SPECTRAL COLOUR` and `SCAN GEOMETRY`; rejected because column 2 is full (`cassette` fills `196..571`) and its drawings carry 11 px text that cannot be scaled down, so the camera got a page of its own.
13. **`TUBE` and `IRIS` on both the circuit and the camera page**: Considered keeping them on the circuit page as well; rejected so that every setting has exactly one line on one page, and the circuit's `camera` block points to the camera page instead.
