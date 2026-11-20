# VHS-90 — vhs-main OFX/Metal source audit and final native migration

Date: 2026-11-20  
Archive path: `vhs/github repos/vhs-main/`  
Inventory: **33 archive entries: 28 files + 5 directories**  
License: **GNU AGPL-3.0-or-later** (source README and supplied LICENSE).

## 1. Full file disposition

| File | Disposition |
|---|---|
| `.gitignore` | Native CMake/build exclusions. |
| `build_and_install.py` | macOS OFX build/install helper; host-specific. |
| `CMakeLists.txt` | C++17 OFX plugin build; fetches/uses OFX SDK and configures Metal/CUDA paths. Not an AE SDK project. |
| `Info.plist` | macOS OFX bundle metadata. |
| `LICENSE` | AGPL-3.0-or-later; preserve separately and do not silently mix source files into a differently licensed plugin. |
| `LUTs/Looks/VHS Faded Tape.cube` | 2×2 identity placeholder LUT, not a usable color look. |
| `pack_luts.py` | Build-time .cube/.meta parser and XOR resource packer; Python packaging utility. |
| `presets/blank_slate.preset` | Neutral reference recipe. |
| `presets/VHS Camcorder.preset` | Consumer VHS profile. |
| `presets/Hi8 Camcorder.preset` | Higher-bandwidth, lower-noise camcorder profile. |
| `presets/Betacam SP.preset` | Higher-bandwidth broadcast tape profile. |
| `presets/Damaged VHS.preset` | Severe tracking/dropout/geometry profile. |
| `README.md` | OFX/macOS product overview, feature list, build/install and AGPL notice. |
| `src/CubeParser.h` | .cube parser and directory discovery; AGPL source. Not copied. |
| `src/CudaKernels.cpp` | CUDA dispatch stub/fallback in supplied build. |
| `src/CudaKernels.h` | CUDA API declarations. |
| `src/LofiFxVhsPlugin.cpp` | OFX host parameter registration, render callbacks, preset/OSD/LUT/Metal/CUDA host glue. Not portable to AE. |
| `src/LutPack.h` | XOR-packed LUT resource reader; AGPL source. Not copied. |
| `src/MetalKernels.h` | Metal dispatch declarations. |
| `src/MetalKernels.mm` | Objective-C++ Metal resource/texture/command queue implementation; macOS only. |
| `src/MetalKernelSource.h` | Embedded Metal kernel source and platform-specific abstraction layer. |
| `src/OsdRenderer.cpp` | 5×7 glyph and 7-segment OSD rasterizer; English uppercase/digits/symbols. |
| `src/OsdRenderer.h` | OSD rasterizer interface. |
| `src/PresetParser.cpp` | Text preset parser and directory discovery. |
| `src/PresetParser.h` | Master preset data structure and parser declaration. |
| `src/VhsKernelCore.inc` | 1,032-line shared GPU kernel core: signal, noise, dropout, geometry, bloom, color and LUT functions. |
| `src/VhsKernelCoreText.h.in` | CMake-generated kernel-source embedding header template. |
| `src/VhsParams.h` | Shared OFX/device parameter struct, parameter names and OSD enums. |

The five directory entries are the project root, `LUTs/`, `LUTs/Looks/`, `presets/` and `src/`.

## 2. Processing capabilities and native-engine comparison

The shared kernel and host parameter model cover:

- NTSC/PAL bandwidth and chroma bandwidth, composite encoding/decoding, dot crawl, cross-colour and composite noise.
- Independent luma/chroma noise, noise hue tint and deterministic seeded noise.
- Interlace mode, field order and field displacement.
- Head-switch band, tracking band/drift and deterministic stochastic dropouts.
- Per-line horizontal jitter, top skew with decay and edge pincushion.
- Red/blue chroma registration offsets.
- Color fade, hue tint, black lift, white knee and dynamic-range trim.
- CRT mist/bloom, highlight threshold, spread and veiling glare.
- 3D LUT and input/output color-space transform pipeline.
- OSD with 5×7 glyphs, 7-segment numerals, transport symbols, date/time, REC blink, counter and tape-speed labels.
- OSD is rasterized into the source before the composite stage so it receives the same analog signal degradation as the footage.

Most VHS-90 signal behavior is already implemented in independent CPU-first C++ modules: composite/waveform, IIR bandwidth, sync instability, receiver/RF instability, analog noise, deterministic dropouts, tape surface/transport, arbitrary-region tracking/head switching, geometry, bloom, temporal processing, OSD and Image Grade. The OFX/Metal/CUDA host and GPU kernel code was not copied.

## 3. Actual native improvements from this source audit

### 3.1 OSD pipeline order corrected

The source's OSD-before-composite design exposed a real ordering gap in VHS-90: `applyVhsOsd` had been called after composite, tracking, static and device-connection stages, leaving the OSD unnaturally crisp.

The OSD block was moved to immediately after source-frame initialization and before the composite signal stage. Therefore OSD text is now part of the input image and is degraded by composite encode/decode, bandwidth, static, tracking/head switching and subsequent signal processing. OSD-only rendering remains supported when Master Strength is zero.

- Code commit: `34be23656a7475d3a39d0fc8f4995af8d67a0f67`.
- Added a static validator assertion that OSD invocation must precede the composite signal stage: `dfb5e574773f72a4c8b7e49bf5d92b42f1a08bfb`.

### 3.2 Independent RGB channel registration added

The source preset model exposed separate red and blue channel offsets, which are not the same as VHS-90's existing I/Q chroma displacement. A distinct native RGB registration stage was added to `VHS90ChromaShiftEngine`:

- Green remains the reference channel.
- Red and blue have independent signed horizontal pixel offsets.
- Bilinear sampling and premultiplied-alpha handling are used.
- Neutral offsets take the exact identity-copy path.
- AE exposes `Red Channel Registration` and `Blue Channel Registration` controls, each from -20 to +20 pixels, scaled by Master Strength.
- The stage runs at the end of the image pipeline so the already signal-degraded OSD is also subject to final display/channel misregistration.

Implementation commits:
- `de1eb786c846e85955d0ff4a6075b5897bfd524b` — core API.
- `293f546ce4b424b176b1cf37a93bd0ee7cb54b5a` — RGB registration implementation.
- `41285ec3a531c952f7cd0e34216e7030fa595d52` — deterministic smoke coverage.
- `807154479f87968f5942b03331d61e558e6db2a2` — AE controls and final-stage wiring.
- `08612104786d43c8ed5c76e5a42afecba2c1fd4d` — static validator requirements.
- `9a6922f3246e9d3febd61e66ec29fc98de411dcf` — premultiplied-alpha smoke coverage.

## 4. Deliberate non-port decisions and license boundary

- **AGPL-3.0-or-later:** no OFX, Metal, CUDA, kernel, OSD, LUT parser or preset parser source code was copied into VHS-90. The native changes above were implemented in the existing independent VHS-90 modules. Any future direct source-code port or derivative redistribution must be reviewed for AGPL obligations or separately licensed permission.
- **3D LUT pack:** the only supplied `.cube` file is a 2×2 identity placeholder labelled “Faded Tape”; it has no visual look to import. The AGPL parser/packer was not copied. A real, independently authored LUT pack is a separate feature, not something this source archive actually provides.
- **Five source presets:** all five recipes were inspected. VHS Camcorder, Hi8, Betacam SP and Damaged VHS overlap substantially with existing VHS-90 style/camera/tape controls. They remain reference recipes, not byte-for-byte imported presets; exact mapping would need parameter-by-parameter render validation, particularly because this source includes white-knee/dynamic-range and transform settings that do not have direct one-to-one VHS-90 controls.
- **OSD bitmap glyphs:** the source's 5×7 uppercase/7-segment rasterizer is not imported; VHS-90 retains its own OSD renderer and the source glyph set is not a general font/shaping solution.
- **Metal/CUDA:** macOS Metal is the source's useful accelerated path; supplied CUDA files are stubs, and README notes Windows/Linux CPU pass-through. Neither provides a Windows AE CPU-first renderer.
- **CST transforms:** source transforms target DaVinci Wide Gamut/Intermediate and are not automatically meaningful for the AE plugin's current RGB/YIQ pipeline.

## 5. Verification boundary

- Source inventory: 28 files, 5 directories; prior report 14 plus this expanded review cover the full folder.
- OSD-before-composite static validator passed in CI run `44816473391`.
- The OSD reorder's CMake build and all 32/32 CTest smoke tests passed in CI run `44816473391`.
- The new RGB registration stage and premultiplied-alpha smoke test passed in CI run `44816747950`: static AE/preset validator, CMake build and all 32/32 CTest smoke tests succeeded.
- No Metal, CUDA, OFX host, DaVinci Resolve or Adobe AE Windows runtime test was performed.
- Windows/MSVC AE SDK compilation, PiPL, AEX loading, AE visual tests and MFR remain global release gates.

## 6. Final source-folder disposition

**vhs-main folder inventory and source review: complete (28/28 files).**  
**OSD ordering: corrected and core CI passed (32/32).**  
**Independent RGB channel registration: implemented and CI PASS (32/32).**  
**LUT: no usable LUT data in supplied archive; no parser code copied.**  
**AGPL-3.0-or-later: source code kept separate; no direct code port.**  
**All eight source folders in the user's queue have now been audited or verified against their earlier completed integration reports.**
