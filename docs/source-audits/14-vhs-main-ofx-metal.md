# VHS-90 — vhs-main OFX / Metal Reference Audit

Date: 2026-11-19
Input: `vhs/github repos/vhs-main/` from supplied `vhs.zip`
Scope: full file inventory; detailed inspection of README, shared parameter struct, core shader/kernel processing path, OSD rasterizer, OFX host entry and bundled preset files.

## Project architecture

- Native C++ OFX effect for DaVinci Resolve, with Metal compute on macOS and a CUDA source/stub.
- README states that Linux/Windows compile but use CPU passthrough fallback in this version; the useful GPU render path is macOS Metal.
- The plugin is not an Adobe AE SDK plugin and does not provide a native Windows AE rendering path suitable for direct reuse.
- Shared `VhsParams` struct is designed to have the same layout in C++ and Metal/CUDA kernels. The kernel source contains the actual processing logic and a per-frame render parameter block.

## Signal / image processing modules

- NTSC/PAL selection, luma bandwidth and chroma bandwidth controls.
- Composite encode/decode strength with dot crawl, cross-colour and composite-domain noise.
- Independent luma and chroma noise, including hue bias for chroma noise.
- Interlace mode, field order and field displacement.
- Magnetic-tape head-switch noise band, tracking band with vertical drift, and stochastic dropouts.
- Per-line horizontal jitter, top-of-frame skew with exponential decay and edge pincushion.
- Color fade, hue tint, black lift, white knee and dynamic-range control.
- Red/blue chroma registration offsets.
- Optional CRT mist/bloom with highlight threshold, spread and veiling glare.
- Optional input/output color-space transforms and 3D LUT look packs.

The core sampling path is arranged so OSD is burned into the source before the composite stage. The resulting text therefore receives the same bandwidth blur, dot crawl and signal artifacts as the footage, rather than being composited after the VHS damage.

## Important region geometry limitations

- Head switching computes `bandTop = imageHeight - headSwitchHeight`; it is hard-coded to the bottom edge.
- Tracking computes a vertical band from frame index × speed and a configured half-height; it drifts vertically but the model does not expose arbitrary X position, width, orientation or multiple independent rectangles.
- Dropouts are procedural blocks with density, horizontal length, vertical height, opacity and feathering. Their decisions are deterministic from frame/pixel hash inputs.
- VHS-90 should reuse the procedural dropout construction and noise/hash techniques but implement a shared arbitrary-region system for Tracking Noise and Head Switching.

## OSD renderer

The OSD is a hand-authored bitmap rasterizer:
- 5×7 uppercase glyphs, digits and punctuation.
- Dedicated symbols for play, pause, fast-forward, rewind, stop and REC dot.
- A separate 7-segment digit set.
- Date/time formatting, counter, REC blink, playback mode and tape speed label.
- Nine anchor positions, per-element position controls, scale and optional shadow.
- OSD mask is built procedurally and applied before composite encoding, so it degrades with the rest of the signal.

This is a strong structural reference for VHS-90's procedural OSD stage. Its built-in glyphs are limited to English uppercase/digits and do not provide Arabic shaping, bidi or font-file support. VHS-90 must use the user's supplied 15 fonts and a proper Windows Arabic text shaping path instead.

## Bundled presets

Five preset files were found:
- Blank Slate
- VHS Camcorder
- Hi8 Camcorder
- Betacam SP
- Damaged VHS

They store explicit parameter values for composite, noise, interlace, head-switch, tracking, dropouts, geometry, channel registration, aging/color, mist, OSD, and color transforms. Their values can be imported as reference recipes; VHS-90 must rename the looks and adapt values to its own parameter units and frame-size normalization.

Notable preset data:
- VHS Camcorder uses NTSC, composite strength 1, luma/chroma noise 0.5, 12 px head-switch band, dropouts around 6.05/frame and line jitter around 4.16 px.
- Damaged VHS uses composite noise 0.5, full tracking amount, dropout density around 21.16/frame, longer/wider dropouts, larger line jitter and enabled mist.
- Hi8 Camcorder uses lower noise and much lighter dropouts than the VHS Camcorder profile.
- Betacam SP uses higher bandwidth, low noise and a 60 px head-switch setting, showing that the parameter set was also intended to cover non-consumer VHS formats.

## VHS-90 adaptation decisions

1. Port the shader's procedural composite, noise, tracking drift, dropout, geometric wobble and bloom concepts into the CPU-first VHS-90 kernel.
2. Keep OSD generation before VHS processing so the OSD itself can acquire authentic signal damage.
3. Replace bottom-only head switching with user-positionable regions and full-frame height support.
4. Upgrade tracking from one drifting horizontal band to independently configurable regions anywhere in the image.
5. Use the supplied font pack, Arabic shaping and localized digits/months; do not reuse this project's limited bitmap font as the main VHS-90 font system.
6. Treat this repository's OFX host layer, Metal-specific implementation and macOS build configuration as reference only. Keep VHS-90's Adobe AE 23.2.1 / Windows x64 / C++17 host architecture separate.

## Verification status

- The repository's C++ host/kernel interfaces, OSD implementation and all five preset files were inspected.
- No Metal compilation, CUDA compilation, OFX host test or AE runtime test was performed here.
