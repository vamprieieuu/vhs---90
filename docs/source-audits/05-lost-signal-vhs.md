# Source Audit 05 — Lost Signal VHS

**Folder:** `github repos/lost-signal-vhs-main`  
**Status:** current source and all ten packaged versions inventoried; latest GLSL path reviewed.

## Complete inventory
- Main source includes two README documents, a release-build PowerShell script, language files, settings, common color code and GLSL passes.
- Ten version ZIP archives were opened and inventoried: v1.0, v1.1, v1.2, v1.3, v1.6, v1.6.1, v1.7, v1.8, v1.8.1 and v1.9.
- The archives contain eight shader files in earlier releases and fourteen in later releases. The v1.9 package is the newest supplied implementation.
- All 6 GLSL/fragment/vertex shader files in the current source tree and the settings/property configuration were inspected.

## Latest v1.9 signal architecture
The shader is a multi-stage, procedural analog-video system:
- Recording pass and playback/display pass are separate.
- Persistent frame history is paired with a metadata buffer that stores logarithmic view depth and AGC gain state.
- Motion-aware history reprojects prior processed frames through camera/depth matrices and rejects disoccluded or mismatched surfaces.
- Signal noise uses deterministic hashes and smooth correlated value noise; no external noise texture is required.
- Tape time-base error is correlated across neighboring scanlines using slow and fast noise components.
- Tracking tears use feathered envelopes and smoothly varying displacement rather than rectangular digital blocks.
- A narrow moving tracking line is independently generated.
- Rare vertical-sync loss can cause raster roll or a held frame.
- Recording stage uses separate luma and chroma bandwidth, YIQ conversion, phase instability, tape generation, and automatic gain.
- Playback stage performs one selected composite decoder: bypass, consumer notch, or NTSC two-line comb. The notch path deliberately creates luma/chroma leakage; the comb path supports dot crawl and cross-color.
- Persistent temporal ghosting, chroma-only persistence, frame repetition, interlaced field weave and chroma-killer behavior.
- Playback includes 4:3 framing, overscan, pixel-scale sampling, lens distortion, flicker, washout, tint, vignette, OSD and camera-generation treatment.
- Persistent oxide defects are attached to a moving virtual tape coordinate rather than regenerated as independent screen overlays.
- Optional depth fog is inserted before YIQ/tape processing so the fog inherits analog degradation.

## Relevant controls found in settings
The supplied settings expose controls for noise, scanlines, tracking-tear frequency/strength, time-base error, static-burst frequency, tape format, generation loss, tape wear, chroma phase error, automatic exposure, chroma killer, vertical-sync glitches, chroma persistence, decoder mode, dot crawl, cross-color, cross-luma, chroma delay, lens distortion, flicker, frame jitter, wobble, motion smear, ghost strength, history stabilization, spatial echo, pixel scale, tape softness, chroma bleed, black crush, saturation, head switching, halation, ringing, manual tracking, RF interference, tape chew, defect memory, defect density/strength and frame repeat.

## Preset inventory
The v1.9 README and properties define thirteen named looks: Reference VHS, Real VHS, Backrooms, Poolrooms, Liminal Night, Subtle, Found Footage, Damaged Tape, Rental Tape, Camcorder 1996, Broken Signal, Composite Decode and Analog Fog. These names are source references only; VHS-90 will create its own preset names and will not expose the source project name in the UI.

## Important transfer boundaries
- GLSL shader logic can be ported to C++ CPU kernels, but GLSL syntax and texture sampling must be translated into pixel/scanline loops and explicit buffers.
- Depth reprojection, camera matrices and Minecraft world fog depend on a 3D renderer/depth buffer. AE footage generally has no corresponding world-depth/camera metadata, so these cannot be transplanted literally; only screen-space temporal history, deterministic noise and tape/signal behavior are directly transferable.
- Persistent history and frame-field weaving need a host-safe, frame-index-addressable design in AE. Do not use a mutable global previous-frame buffer that breaks random access or MFR.
- The shader's head switching is tied to field timing and a bottom playback region. VHS-90 must extend this to user-positionable regions and an adjustable height that can expand to the entire frame.
- Current OSD glyphs are procedural shader shapes. VHS-90's separate Text system will use only the user-supplied bundled fonts, including Arabic shaping/RTL and Arabic digit/date localization as requested.
- This source is a shader pack, not an AE plug-in. Do not copy its Minecraft integration, GLSL UI, or shader-pack preset names into VHS-90.

## VHS-90 implementation targets
Prioritize the procedural hash/value-noise functions, correlated line timing error, feathered tracking tears, phase-aware YIQ path, single composite decoder, notch/comb decoding, dot crawl/cross-color/cross-luma, persistent oxide defect coordinates, chroma-only temporal persistence, frame-field weave, automatic gain response and directional tape smear. Reuse the shared VHS-90 signal/tape modules instead of duplicating them.

No claim is made that the GLSL has already been converted to C++ in this commit.
