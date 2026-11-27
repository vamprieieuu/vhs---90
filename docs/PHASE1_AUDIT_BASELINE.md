# VHS-90 Phase 1 — UI and Signal Rebuild Baseline
Date: 2026-11-27
Branch: phase1-ui-signal-rebuild-2026-11-27
Base: main @ 607f531c848db52c2e4e81179274b120379d4ac7

## Purpose
This is the first source-backed audit checkpoint for Phase 1. It is not a completion claim and does not replace visual validation. Preserve the working AE load/build path while correcting the effect architecture and behavior.

## Confirmed source findings
- The current effect registration is concentrated in Source/VHS90Effect.cpp.
- The parameter enum places Signal first, then VHS Style much later; the requested VHS Style / Presets-first hierarchy is not present.
- Master Strength is registered with a 0..100 range. RenderWorld currently normalizes it with clamp01(master / 100), so simply raising the UI maximum would saturate the current master mix at 100. The normalization and per-stage drive must be redesigned together.
- Color System is a two-choice popup. It must be checked against the actual composite pipeline before retaining or removing either choice.
- The current parameter inventory includes separate Tracking Layer 2/3/4 groups, Tracking Presets, a Procedural Static Pattern group, and Tear Band 2/3/4/5 groups. These are candidates for consolidation into one Tracking Noise engine and one Tear/Tracking family, not blind deletion.
- The UI inventory also includes separate VHS Playback, Composite Decoder, VHS Frame, Camcorder HUD, Frame Repeat, Particle Pattern, multiple OSD subsections, and Tape Counter. These match the owner's explicit removal/consolidation requests, subject to checking legacy parameter IDs and project compatibility.
- RenderWorld draws OSD into the working image before the signal-processing stages. Preserve that ordering so OSD is subsequently degraded by the VHS pipeline.
- Dedicated core modules already exist for static, tracking read, tear chroma, glow/bloom, ringing, tape surface/transport, waveform, and OSD. Their existence does not prove their current controls are correctly wired or visually faithful.

## Reference material located in the repository
- VHS/open source native plugin for ae/VHS6RIFT/docs/MASTER_STRENGTH_SCALE_AUDIT.md
- VHS/open source native plugin for ae/VHS6RIFT/docs/TRACKING_GRAIN_MASTER_SCALE_FIX.md
- VHS/open source native plugin for ae/VHS6RIFT/docs/PARAMETER_WIRING_AUDIT.md
- VHS/vhs damage lap/Tracking noise_/نص.txt
- VHS/vhs damage lap/Procedural Pattern Engine/
- VHS/vhs damage lap/Tear & Chroma Burst/
- VHS/Vhs cams/Fxc6/ and VHS/Vhs cams/Gen 7/
- VHS/plugins/aex/ and VHS/plugins/c+/
- VHS/github repos/ including Analog-NTSC and NTSCRT-0.26.0
- VHS/codes.txt

## Phase 1 target hierarchy
1. VHS Style
2. Presets (source-grouped, deduplicated; camera and plugin looks kept distinct)
3. Master Strength (extended range with meaningful scaling)
4. Signal (including the useful Signal Looks controls)
5. Grain & Tracking Noise
6. Chroma
7. Ringing
8. General Adjustments (exposure/brightness, contrast, saturation, vignette, flicker, luma smear where appropriate)
9. Glow
10. Tape (unified tape material/surface/transport controls)
11. Waves / Static where appropriate

## Behavioral gates
- A parameter is not complete until its value is traced from registration through parameter read, RenderSettings, engine call, and pixel output.
- Zero values must be neutral unless the source effect explicitly defines a nonzero floor.
- Temporal effects must use deterministic frame-aware randomness, not a frozen random field or uncontrolled flicker.
- Static must be generated procedurally in the image pipeline, not supplied as a video/overlay layer.
- Tracking Noise and Head Switching must support the requested position, direction, width/height, and full-frame coverage.
- Preserve stable parameter IDs and old-project compatibility when consolidating UI sections; do not renumber the whole parameter enum casually.
- Preserve the current known-good native AE build path. Do not advertise Smart Render or alter PiPL/outflags without a dedicated host-compatibility reason and tests.

## Source-derived VHS6RIFT guidance
The VHS6RIFT audit describes Master Strength as a bounded drive multiplier above the default, with zero remaining an exact bypass. It also records fixes where zero tracking/grain must be neutral and tracking strength must not be multiplied by raw master a second time. Apply those semantics only after tracing the VHS-90 engine's own scaling model; do not copy unrelated VHS6RIFT internals blindly.

## Next Phase 1 implementation order
1. Complete parameter-to-render wiring inventory and identify dead/duplicate controls.
2. Trace master scaling through all engine calls and define a non-saturating range above 100.
3. Build the source-grouped VHS Style / Presets model without breaking existing saved projects.
4. Consolidate UI groups while retaining legacy IDs as hidden/deprecated compatibility parameters if necessary.
5. Repair and test signal/chroma/grain/glow/tape behaviors one by one using supplied source references.
6. Run the Windows x64 build workflow and CPU smoke suite; host visual validation remains a separate gate.
