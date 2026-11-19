# VHS-90 — vhs-phys Physical Tape / Deck Model Audit

Date: 2026-11-19
Input: vhs/github repos/vhs-phys-main/
Scope: 202-file inventory; detailed review of architecture, plugin-hosting design, tape geometry/defects, lateral tracking, playback head pulse rendering, speed model, condition mapping and the field pipeline. The camera, audio, interference and every test file are inventoried but are not all fully reverse-engineered in this checkpoint.

## Why this source is important

Unlike image-domain VHS filters, this Rust engine models a recording and playback chain:
- RGB picture is encoded to baseband luma/chroma.
- Recording electronics pre-emphasize and FM-modulate luma onto tape.
- Tape geometry, oxide defects, head spacing and lateral tracking alter the recorded transitions.
- Playback heads reconstruct an RF waveform from magnetization transitions.
- Deck electronics discriminate/de-emphasize the RF and recover baseband.
- Composite encoding/decoding and TV display processing can follow.

The repository describes this as a physical-model simulator, not an overlay or generic noise stack.

## Timing and processing architecture

- NTSC/PAL engines are compiled separately against their own timing constants.
- Default RF domain is 12× subcarrier (42.95 MS/s, 2,730 samples per line); baseband is 4× subcarrier (14.32 MS/s, 910 samples per line).
- A field is one helical track. Global sample indices form the absolute time axis, making timebase error and tape position reproducible and independent of render order.
- The pipeline runs record-track generation, parallel RF playback-head rendering, head switching/deck electronics segmentation, then sync/AFC/colour decoding.
- The RNG is seeded and order-independent, keyed by seed, stream and global index.
- SP/LP/EP change physical track pitch, track angle, tape advance and head width; slow speeds are not implemented as merely a lower-resolution blur.

## Tape geometry and defects

The tape model stores defects in tape coordinates, not screen coordinates:
- Roughness creates time-correlated head-to-tape spacing variation.
- Edge/tension profiles add spacing near track ends.
- Point defects include debris (extra spacing) and voids (reduced magnetization).
- Defect maps are generated deterministically in spatial cells.
- Longitudinal creases are modeled as localized Gaussian spacing lift with a shallow halo. Because the crease is fixed in tape coordinates and the head follows helical tracks, it can appear at a consistent picture height across successive fields.
- Tape stretch and speed alter the relationship between tape coordinates and the visible track.

This is a valuable reference for authentic tape-wear/dropout behavior: different defects should have different signal mechanisms, not all be white lines.

## Tracking geometry

The playback head reads a weighted mixture of five neighboring helical tracks (n−2 through n+2). Lateral weights are calculated from overlap between head width and track pitch:
- Mean offset (offset_um).
- Start-to-end slope (slope_um) from tracking/track-angle mismatch or stretched tape.
- Parabolic bow (bow_um) along the track.
- Seeded per-track wander (wander_um).

As the head crosses into an adjacent track, opposite azimuth and track content alter the recovered signal, naturally creating a tracking/noise band. The model supports drift across a field and is not a bottom-only raster strip.

This is richer than adding random noise inside a screen-space rectangle. VHS-90 can use the concept and possibly port a simplified neighbor-track mixing model, while still exposing user-friendly arbitrary region controls for the visual effect.

## Playback head model

- Each magnetization reversal is rendered as a Lorentzian pulse with an effective half-width derived from gap, transition width, spacing and recorded depth.
- Pulse width controls high-frequency attenuation through the Wallace spacing relationship.
- Extra head-to-tape spacing widens pulses and reduces high-frequency signal.
- Head width and azimuth determine how adjacent tracks contribute.
- Head amplifier noise, media AM noise and head resonance are separate front-end stages.
- Head wear changes spacing, gain, azimuth error and carrier-to-noise ratio.

This is not a direct pixel kernel. It is a high-fidelity reference for a future physical-tape engine mode; it is too expensive and structurally different to replace the fast procedural VHS-90 look engine wholesale.

## SP / LP / EP behavior

The format module changes tape speed and advance per field, track angle and pitch (58.0 µm SP, 28.9 µm LP, 19.3 µm EP), head width and overlap with adjacent tracks, noise/crosstalk penalties from narrower tracks, LP/EP carrier interleave and burst amplitude rules, and LP/EP sub-pre-emphasis and playback sub-de-emphasis.

The source documents these as format-derived behaviors, not arbitrary preset color changes.

## Head switching and UI implications

The field pipeline switches between head tracks around the vertical interval. This is a physical head handoff and can create discontinuity/noise at the switch. The source's physical switch is tied to field timing and tape/head geometry, not a general freely positioned screen rectangle.

For VHS-90's visual module, preserve the user requirement: Head Switching must have independently adjustable vertical position, region height and intensity, with the ability to expand from the bottom to the full image. The physical model can inform authentic signal texture but must not constrain user-facing placement.

## Condition presets / parameter mapping

The Condition layer maps tape age, storage, quality, pass count, stretch, head wear/clog, drum wear, deck age/temperature/humidity, tracking offset/slope, mains sag and other conditions into lower-level physical parameters. Named condition examples include pristine, good, worn, bad, ruined, dry-caps, dirty-heads, mistracked, stretched, humid-attic and interference conditions.

VHS-90 can translate the high-level condition idea into optional preset recipes, but should not force the user to understand every physical parameter to create an effect.

## Plugin-hosting feasibility

The core's plugin-hosting document explicitly states:
- Core has no I/O or global state.
- It can be wrapped by After Effects, Premiere or OpenFX.
- Input is fixed at 754×480 RGB8 and the host must resample to/from that format.
- The streaming session has three-frame latency.
- Arbitrary-time host requests need either a sequential per-instance cache or a short-lived session per frame; the latter costs roughly 6–7× more.
- A camera tube adds temporal pre-roll requirements.
- The source estimates about 13 fps on 8 CPU cores for the full chain, and ~2 fps with camera stage enabled.

This is valuable architecture research but not a drop-in AE render kernel: fixed input dimensions, RGB8-only API, multi-frame state and heavy RF processing require significant adaptation.

## VHS-90 integration decision

1. Treat this as the reference for a separate high-fidelity physical tape mode, not the default fast VHS engine.
2. Port selected ideas first: tape-coordinate defect generation, seeded lateral tracking weights, crease/spacing-to-dropout response, head wear/noise curves, and timebase error.
3. Preserve deterministic global-frame/time indexing and avoid mutable global state.
4. Keep the current procedural signal/image engine for practical real-time use; physical modeling can be an optional high-quality/slow mode only if performance allows.
5. Expose a simplified user interface, with advanced physical controls tucked behind optional advanced settings.
6. Rebuild screen-space Tracking Noise and Head Switching as arbitrary-region modules; the physical source's head handoff timing must not override the user's position/height controls.
7. Maintain alpha outside the RGB signal chain and resample to arbitrary AE dimensions/rowbytes.

## Verification status

- Rust core modules and architecture documentation were inspected statically.
- The archive includes extensive tests (bit identity, schedule invariance, tracking, head bumps, speed details, tape speed, composite, decode and pipeline); tests were inventoried but not run.
- Cargo was not available in this environment; no Rust build or render output is claimed.
- This is a high-value algorithm audit, not a completed C++ port.