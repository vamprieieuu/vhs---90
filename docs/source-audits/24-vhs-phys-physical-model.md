# VHS-90 — vhs-phys Physical Record/Playback Engine Audit

Date: 2026-11-19
Input: `vhs/github repos/vhs-phys-main/` from supplied `vhs.zip`.
Inventory: 202 files, about 7.85 MB. Includes a Rust physical VHS core, CLI, Python/PySide GUI, docs, tests, presets, examples and image assets.
Scope in this checkpoint: core crate architecture and its main physical modules (`pipeline`, `record`, `tape`, `head`, `tracking`, `mech`, `electronics`, `decode`, `video`, `condition`, `format`, `composite`), plus the plugin-hosting and performance documents. The CLI, GUI, camera/audio/interference submodules and all tests/docs have not yet been exhaustively audited.

## Core model

This is a physical record/playback simulator, not a collection of VHS image filters.

`RGB frame → video raster/baseband → record electronics → FM luma + color-under chroma → magnetic transition recording → physical tape map → playback head → RF electronics → discriminator/de-emphasis → sync/AFC/chroma decoder → RGB frame`

The engine models each field as one helical track. It is written once and compiled against separate 525-line NTSC and 625-line PAL constants.

## Signal timing and raster

- Default NTSC RF rate: 12× color subcarrier, approximately 42.95 MS/s.
- Baseband: 4× color subcarrier, approximately 14.32 MS/s.
- 2,730 RF samples per line and 910 baseband samples per line.
- One field is 262.5 lines; one frame is 525 lines.
- PAL has a separate 625-line engine with its own timing, carrier and V-switch behavior.
- Active frame size is fixed: 754×480 NTSC or 740×576 PAL.
- Input/output of the core is RGB8; the CLI resamples arbitrary source media to the selected standard and back.

## Physical record stage

- Converts RGB to IRE-based Y/U/V planes with setup, sync, blanking and burst structure.
- Records luma as an FM carrier, with nominal sync-tip carrier and luma-dependent deviation.
- Applies record luma low-pass, pre-emphasis, clipping, FM high-pass and modulator asymmetry.
- Records chroma as a lower-frequency color-under QAM carrier.
- SP/LP/EP use different tape geometry and sub-pre-emphasis; LP/EP playback applies the inverse de-emphasis after the main chain.
- The tape relay stores magnetic transition reversals rather than a blurred RGB image.

## Physical tape surface

The tape is a deterministic map in coordinates along the tape and across its width. Defects are generated on demand from spatial hashes, so the same tape location reproduces the same flaw regardless of render order.

- Point defects have a power-law size distribution.
- Missing oxide (voids) reduces magnetization.
- Debris/bumps lift the tape away from the head and increase head-to-tape spacing.
- Repeated playbacks remove loose debris but not missing oxide.
- Tape-end hub pressure changes defect density.
- Longitudinal creases, roughness, edge curl and tape tension add local spacing/timebase effects.
- A defect's position in tape coordinates maps to a fixed picture height over every field it spans; a point defect travels through the raster as tape advances.

This is a strong basis for persistent Tape Wear and Dropout behavior without image overlays.

## Playback head and track geometry

- Each playback head reads Lorentzian pulses generated from recorded magnetic transitions.
- Pulse half-width combines head gap, recorded transition width, depth and head-to-tape spacing.
- Wallace spacing loss is frequency-dependent; extra spacing attenuates high-frequency FM much more strongly.
- The head reads neighboring tracks with azimuth-dependent crosstalk.
- Head parameters include gain/wear, CNR, resonance frequency/Q/gain, azimuth error, width, media AM noise, extra spacing, pole length and contour echo.
- SP/LP/EP change track pitch, angle, advance and neighboring-track overlap.

## Tracking and time-base error

### Tracking
- Lateral head position determines the weights of the recorded track and its neighbors.
- Offset, wander and slope are separate physical parameters.
- A lateral slope across the track causes the noise/crosstalk band to sweep vertically through the picture; the supplied test explicitly verifies this behavior.
- The physical tracking control is measured in micrometres of head offset, not screen pixels.

### Mechanics
- Slow capstan wow / servo hunting (0.3–8 Hz).
- Drum eccentricity at the frame rate and harmonics.
- Tape-path resonance with a second-order spectrum.
- Stick-slip flutter from 100 Hz to 3 kHz.
- Independent per-line timebase jitter, smoothly interpolated to avoid discontinuous sync spikes.
- Head-to-head timing offset.
- Tape tension kick / flagging at track start.
- Head-switch timing error at the end of the track, with configurable amplitude and decay in lines.

The mechanical Head Switching event is a timebase error around the physical head-change point. It is not a free image-space region with user-controlled X/Y/width/height. VHS-90 should preserve the physical timing event as one mode, then add the requested arbitrary-region Head Switching mask/height control independently.

Likewise, physical tracking is a head-to-track alignment error. It should inform VHS-90's tracking physics, but it does not replace the user's custom-position Tracking Noise region engine.

## Deck electronics and TV decoding

The reviewed architecture and source modules cover:
- Playback equalization and AGC.
- Power-supply ripple, ESR-related failure and mains hum.
- RF/head-amplifier noise and resonance.
- FM discriminator, dropout detection/compensation and de-emphasis.
- Sync separator and horizontal AFC.
- Chroma demodulation and burst reference.
- NTSC composite decoding and PAL delay-line color decoding.
- Optional additional composite roundtrip after the deck.

The condition model maps human-readable physical conditions (tape age/storage/quality/passes, head wear/clog, deck age/temperature/humidity, tracking error, tape tension, mains and outside interference) into the raw physical parameters.

## Format / condition presets

Supported television standards:
- NTSC: 525 lines, 59.94 fields/s, 29.97 frames/s.
- PAL: 625 lines, 50 fields/s, 25 frames/s.
- PAL-60: NTSC tape played into a PAL television.

Tape speeds:
- NTSC: SP, LP, EP.
- PAL: SP, LP (EP is rejected).

Condition presets include pristine, good, worn, bad, ruined, dry-caps, dirty-heads, mistracked, stretched, humid-attic and isolated external-interference cases (phone/radio/mains/magnet/dimmer). These are physical condition recipes, not simple visual LUT presets.

## Determinism and plugin-hosting model

The source's plugin document is unusually explicit:
- The core has no media I/O; the CLI is a thin FFmpeg wrapper.
- `Session::with_start(params, absolute_frame)` makes tape position, defects and timebase functions of absolute frame index.
- The same seed reproduces the same physical tape and timebase behavior.
- A session requires three-frame lookahead before an output frame is ready.
- A sequential per-instance cache is recommended for a host; after a seek, recreate a session at the requested absolute position and pre-roll three frames.
- Stateless short-lived sessions can render arbitrary frames but cost approximately 6–7× more per frame.
- Camera pickup tubes add temporal memory and need several frames of pre-roll.
- The core has a field-level parallel task graph and caches intermediate transitions/RF/segments.
- The plugin document specifically discusses After Effects, Premiere and OpenFX as possible hosts.

This is valuable MFR design guidance, but AE's out-of-order parallel frame requests require a careful cache/session policy. A single mutable sequential session cannot be shared unsafely across concurrent render calls.

## Performance and integration cost

- README reports approximately 11–14 frames/s on 8 CPU cores for the full physical chain.
- Optional pickup-tube camera simulation is about 2 frames/s.
- Hot spots are Lorentzian pulse rendering, RF filtering and the discriminator.
- Lower RF sampling (8× fsc), SIMD and caching are suggested by the source.
- The full model is much more expensive than a real-time post-process and uses fixed standard rasters.

## VHS-90 integration decision

This is the strongest reference in the supplied set for the physical mechanisms behind tape wear, tracking, head response, signal loss and timebase error. It is not a drop-in plugin core: it is Rust, RGB8, fixed-raster, stateful/lookahead and computationally expensive.

Port selected physics into the C++17 CPU engine:
1. Tape-coordinate spatial hash and persistent oxide void/debris defects.
2. Head spacing/Wallace attenuation and azimuth crosstalk.
3. SP/LP/EP track geometry and chroma-under bandwidth.
4. Physical tracking offset/slope as a signal-domain model.
5. Timebase spectra: wow, drum eccentricity, tape resonance, stick-slip and line jitter.
6. Head-switch timing event and top flagging.
7. FM luma/color-under record/playback stages and sync/decoder behavior, initially as an optional high-fidelity mode.

Keep the user's image-space Tracking Noise and arbitrary-position Head Switching region controls as separate VHS-90 features. Preserve alpha, arbitrary AE frame dimensions and MFR-safe per-render state. Avoid porting the GUI, FFmpeg wrapper, audio chain or pickup-tube camera model into the initial VHS-90 core.
