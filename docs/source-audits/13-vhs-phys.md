# Source Audit 13 — VHS Physical Tape Model

**Folder:** github repos/vhs-phys-main  
**Status:** complete source inventory and core VHS architecture review; Rust execution unavailable because this environment has no rustc/cargo toolchain.

## Scope and structure
The repository contains a Rust workspace with a reusable vhsphys-core and a separate FFmpeg CLI. The core has 40+ modules and a large regression suite. It is a physical record/playback simulator, not a stack of image filters. The CLI, FFmpeg/audio I/O, Qt/Python GUI, schematic viewer, packaging, screenshots and documentation images are not AE plug-in code. The C++ conversion targets are the video record/tape/head/mechanics/electronics/decode modules.

## Physical signal path
RGB frame → video raster and sync/blanking/burst → Y/C baseband → record-side luma emphasis and FM modulation + colour-under carrier → thresholded magnetic relay reversals on tape → spatial tape defects/spacing/magnetisation → playback head Lorentzian pulses and adjacent-track crosstalk → timebase/mechanical warp → head amplifier/resonance/noise → deck equaliser, AGC, dropout compensator, limiter and FM discriminator → de-emphasis/sub-emphasis → sync separator/AFC/clamp/chroma PLL/comb decoder → RGB frame.

## Core algorithms inspected
### Tape and tracking geometry
- Tape defects are generated on demand from a deterministic spatial hash in physical tape coordinates (u along the tape, v across it), so a defect remains at the same place across fields and moves naturally through the picture as the tape advances.
- Defect types: missing oxide (magnetisation reduction), debris/bump (head-to-tape spacing lift), longitudinal crease, edge/tension spacing profile and correlated surface roughness.
- Defect density uses a Poisson process per tape cell; defect size uses a power-law distribution; debris is progressively cleaned by playback passes while voids remain.
- SP/LP/EP track geometry changes tape advance, angle, pitch and adjacent-track overlap; physical track pitches are 58 / 28.9 / 19.3 µm.
- Tracking computes lateral weights across the current track and two neighbours on each side, which naturally produces crosstalk when the head is off-center.

### Record and magnetic medium
- Luma FM carrier is integrated from instantaneous luma-dependent frequency; record filters and pre-emphasis shape the signal before the modulator.
- Chroma is a colour-under carrier with line/head phase rotation, PAL V switching, burst boost and LP/EP carrier interleave.
- The sum of FM and colour-under current crosses a tape coercivity threshold. A two-state relay records each threshold crossing as a magnetisation reversal with sub-sample time refined by Catmull-Rom cubic interpolation and Newton steps.
- Slow-speed sub-preemphasis is level-dependent and undone by a mathematically inverted playback network; this reduces LP/EP noise while preserving large edges.

### Playback head and tape-to-head interaction
- Each magnetisation reversal reads as a unit-area Lorentzian pulse. Half-width combines head gap, recording depth and head-to-tape spacing.
- Pulse amplitude is scaled by magnetisation and lateral track overlap; defects therefore cause physically derived dropouts rather than arbitrary white dots.
- Smoothly tapered pulse windows prevent sample-position-dependent area errors.
- Head front end includes pole-face contour echo (low-band), head amplifier noise, media AM noise, head resonance and azimuth smoothing. Narrower LP/EP tracks increase noise and adjacent-track crosstalk.

### Transport and mechanics
- Timebase warp includes slow drift, drum eccentricity/flutter, tape-path resonance, stick-slip, per-line timing error and flagging.
- Tracking slope/tension mismatch changes lateral head position over the track and sweeps noise bands vertically through the image.
- All processes use global sample indices and seeded, order-independent random streams; output is designed to be invariant to thread scheduling and batch size.

### Deck electronics and decoding
- Playback electronics model equalisation, AGC, supply hum/ripple, dropout compensation (DOC), FM limiter/discriminator, main de-emphasis and LP/EP sub-deemphasis.
- Sync separator validates sync-tip level and pulse duration, then AFC corrects line timing.
- Chroma recovery uses burst phase/gain estimation, optional second-order PLL, colour killer, chroma LPF, line comb and PAL delay-line decoding.
- Composite output can be separated by notch, 1H comb or 2H comb, with distinct dot-crawl, rainbow, hanging-dot and vertical-softening behavior.

## Preset and condition model
Condition recipes include pristine/good/worn/bad/ruined, dry capacitors, dirty heads, mistracking, stretched tape, humid storage, phone/radio/mains/magnet/dimmer interference. A Condition maps years, temperature, humidity, storage, playback passes, head wear/clog, drum wear, tracking offset/slope, tension and recording-deck age into physical parameters. VHS-90 must translate the underlying look recipes into its own original preset names; do not show these source condition labels in the UI.

## Other modules classified
- Relevant VHS video core: consts/NTSC/PAL, video, record, emphasis, tape, tracking, head, mech, electronics, decode, composite, condition, format, pipeline, rng, dsp.
- Potential optional external interference: interference and interference_params (phone RF, radio carrier, mains/ground loop, magnetic fields near head/CRT, dimmer).
- Not part of the current VHS video effect: audio, audio I/O, rack multi-deck chain/standards conversion unless multi-generation simulation is later approved.
- Camera/CRT front-end is a separate optional model: camera, camera_processing, tube, spectral, geometry, stripe and crt. It models pickup-tube charge retention, camera optics, scan and display tube; do not merge it into the VHS tape engine unless a camera-source effect is explicitly requested.

## Verification and performance evidence from the source documentation
- The test suite includes frozen bit-identity outputs, thread/batch/start-frame schedule invariance, tape speed, head bumps, composite, sync/PLL, dropouts, interference and camera physics.
- Source documentation reports 27.6 fps for a 754x480 worn-tape clip on 8 threads and 31.7 fps on 16 threads on a Ryzen 7 5800X at 12x fsc; 8x RF is faster and 16x is slower. A camera tube can drop to roughly 2 fps. These are upstream-reported measurements, not measurements made here.
- This environment has no Rust compiler, so no cargo build/test pass is claimed.

## VHS-90 integration decision
This is the deepest physical reference in the 15-repository set. Port its signal path in staged C++ modules rather than embedding Rust or invoking an external CLI. Highest-value modules: deterministic spatial tape-defect map; tracking/adjacent-track weighting; Lorentzian head-pulse model; timebase/mechanical warp; record FM/colour-under signal; head resonance/noise; AGC/DOC/discriminator; sync/AFC/chroma decode; and composite notch/comb. Keep full physical RF simulation as an optional high-quality engine mode if performance allows, and use selected equations as shared modules in the standard CPU pipeline.

Do not port the camera-tube, CRT display, audio, GUI or FFmpeg CLI code into VHS-90's VHS processing core. Preserve alpha in the AE adapter, seed all randomness by frame/module, and make state addressable by frame index for MFR. No Rust source has yet been converted to C++ in this commit.
