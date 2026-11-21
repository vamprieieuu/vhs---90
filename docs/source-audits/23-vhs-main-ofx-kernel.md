# VHS-90 — Native VHS OFX Kernel Audit

Date: 2026-11-19
Input: `vhs/github repos/vhs-main/` from supplied `vhs.zip`.
Inventory: 28 files, including a C++17 OFX plugin, shared Metal/CUDA-oriented kernel source, Metal host wrapper, OSD renderer, preset parser, five preset files and a 3D LUT.

## Project architecture

This is a native DaVinci Resolve OFX effect, not an After Effects plugin. The strongest reusable component is `src/VhsKernelCore.inc`, a shared signal-kernel source consumed by a Metal runtime compiler. It is deliberately written against a small abstraction macro layer, so the same pixel math can be adapted to a C++ CPU backend.

The GPU pipeline has five passes:
1. Encode input to Y'CbCr, apply OSD burn-in, composite encode/decode artifacts, magnetic noise and dropouts.
2. Geometric warp, tracking displacement and chroma registration.
3. Horizontal bandwidth limit: separate luma/chroma Gaussian filtering.
4. Vertical luma bandwidth limit (skipped when interlace is active).
5. Color conversion/grade, CRT mist bloom and output CST/LUT.

Intermediate buffers use RGBA16F Y'CbCr. The source/destination buffers are RGBA float32.

## Signal processing

### Composite encoding / decoding
- Uses BT.601 Y'CbCr, with Cb/Cr centered around zero.
- NTSC and PAL select different carrier cycles per pixel and line phase.
- NTSC alternates line phase; PAL can enable alternate-line phase switching.
- Three horizontal samples are used to encode and decode a local composite carrier.
- Dot crawl, cross-color and composite-domain noise are independently controlled.
- Composite strength can blend between separated Y/C and composite-encoded response.

### Magnetic noise and dropouts
- Luma and chroma noise use independent deterministic hash streams, with coarse cell sizes so the noise survives later bandwidth filtering.
- Chroma noise has a user hue-bias angle.
- Dropouts are seeded horizontal impulses, with density, length, vertical height, softness and opacity.
- Dropouts are inserted before bandwidth filtering, so their edges smear naturally.
- Field parity changes the noise seed, allowing alternate fields to differ.

### Bandwidth and tape response
- Horizontal Gaussian bandwidth filtering uses separate luma and chroma sigma/radii.
- Vertical luma bandwidth is a separate pass.
- Chroma is vertically delayed by approximately one scanline relative to luma.
- Independent R/Cr and B/Cb channel-registration offsets simulate misconvergence.
- Playback grade includes color fade, hue tint, black lift, white knee, dynamic-range compression and optional highlight bloom/veiling glare.

### Geometry and tracking
- Per-line horizontal jitter is deterministic from seed, frame and row.
- Top skew uses an exponential decay from the top edge.
- Edge pincushion bends the image horizontally.
- Tracking error is a vertically moving horizontal band; its center advances using frame index and speed, with adjustable height, severity and opacity.
- Inside the tracking band, horizontal displacement and luma/chroma noise increase.

### Head Switching limitation
- Head switching is a noise band hard-coded to the bottom of the image.
- Height and intensity are configurable, but no vertical position or end coordinate exists.
- This does not satisfy VHS-90's requirement for user-adjustable vertical position/height and full-frame expansion. VHS-90 should add region start/end (or center/height) and allow the band to move anywhere.

## OSD implementation

The source has a dedicated C++ OSD mask renderer:
- 5×7 bitmap glyphs and 7-segment digits.
- Date, time, REC dot, play mode, counter and tape speed labels.
- Independent enable bits for each element.
- Nine position anchors per element (top/middle/bottom × left/center/right).
- Date/time start fields and a frame-advancing clock.
- REC blink, shadow and scale.
- Play modes include PLAY, PAUSE, FF, REW and STOP.
- Tape speed labels include SP, LP and EP.

The OSD mask is generated on the CPU, uploaded to the GPU, then burned into the signal in pass 1 before composite/noise/dropout/bandwidth processing. This matches VHS-90's required signal ordering better than sources that add clean OSD after the damage.

Limitations relative to VHS-90:
- The font is a custom Latin bitmap/7-segment set, not a selectable font pack.
- No Arabic glyphs, shaping, RTL, Arabic month names or Arabic-Indic digit support.
- Positioning is nine anchors, not independent free X/Y.
- Date formats have three menu options; no locale layer.
- VHS-90 should use the user's 15 bundled fonts and a proper native shaping/rasterization engine, then feed the glyph mask into the pre-damage stage.

## Presets

Five presets are supplied:
- VHS Camcorder
- Betacam SP
- Hi8 Camcorder
- Damaged VHS
- Blank Slate

The preset files contain complete parameter recipes. Their values are useful references for bandwidth, composite, noise, tracking, dropout, geometry, channel registration, color aging, mist and OSD. VHS-90 should translate their behavior into its own preset names and unified UI.

## Alpha / color management

- Alpha is carried as the fourth component through the Y'CbCr intermediate and written back in the final output.
- The code preserves the sampled alpha value, but the processing path does not explicitly unpremultiply and repremultiply RGB. Transparent-edge behavior must be tested and corrected in VHS-90.
- The internal signal is Rec.709 gamma-encoded with a BT.601 Y'CbCr matrix; explicit CST/LUT support converts from/to DaVinci Wide Gamut/Intermediate.
- VHS-90 should use a clearly defined AE color-space policy rather than assume the host uses the same DWG/DI transforms.

## Host/build limitations

- The plugin supports only OFX filter context, RGBA components and float32 pixel depth.
- macOS builds use Metal.
- Windows/Linux compile a CUDA dispatch stub that does nothing; CMake explicitly describes CUDA as future work.
- There is no working CPU renderer.
- The CMake project requires C++17 and fetches OpenFX SDK headers from GitHub.
- The shared kernel source is not directly a CPU implementation: it requires GPU vector types, texture sampling/writing macros and platform entry points. It can be adapted to CPU by implementing scalar/vector helpers and explicit bilinear sampling.
- Metal intermediate textures are module-level static globals (`s_texA`, `s_texB`) shared by renders. Concurrent renders can race over those textures; this is not a safe design to copy into an AE MFR renderer without per-render/per-instance resources or synchronization.
- The OSD renderer uses `std::round` but does not include `<cmath>`. A strict GCC C++17 compile of `OsdRenderer.cpp` failed with “round is not a member of std”; it compiled only after force-including `<cmath>`. VHS-90's port should include the required header explicitly.
- The code sets `frame_index = (int)time` and calls `renderOsdMask(..., 30.0)`. OFX time is expressed in seconds; unless this code converts to frame units elsewhere, the OSD clock advances at the wrong rate for typical timelines. VHS-90 must derive frame number from the actual comp time and frame rate.

## Local verification

- `OsdRenderer.cpp` and `PresetParser.cpp` were compiled with GCC C++17 after force-including `<cmath>`.
- The normal compile of `OsdRenderer.cpp` failed due the missing direct `<cmath>` include.
- The full OFX plugin was not built: this environment is not macOS/Metal, the Windows CUDA path is a stub, and the project fetches external OpenFX headers.
- No DaVinci Resolve runtime test or AE runtime test was performed.

## VHS-90 integration decision

This is a strong reference for a modular signal pipeline and pre-damage OSD ordering. Port the actual composite, deterministic noise, dropout, bandwidth, geometric warp, tracking, channel-registration and bloom math into VHS-90's C++17 CPU core. Replace bottom-only head switching and full-width tracking with the shared arbitrary-region engine. Rebuild the host adapter against the actual AE SDK, preserve alpha/premultiplication, remove module-global render textures, and implement Arabic-capable font rendering from the supplied pack.
