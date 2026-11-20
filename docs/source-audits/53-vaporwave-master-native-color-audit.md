# VHS-90 — Vaporwave / 蒸気機 1.0 source audit and native color integration

Date: 2026-11-20  
Archive path: `vhs/github repos/vaporwave-master/`  
Inventory: **12 entries: 11 files + 1 directory**  
Primary processing code: `vaporwave.js` (574 lines); UI/state/preset code: `document.js` (449 lines), `styles.js` (348 lines), `index.html` (543 lines).

## 1. Source architecture and file disposition

The project is a browser-based image stylizer built around Canvas 2D, a Vue 2 UI and JavaScript typed pixel buffers. It is not a video-processing engine and has no temporal frame model. The application is designed for still images, local browser-side image loading, parameterized preview, local preset storage and JPEG export. It uses a fixed preview canvas (4:3 by default, with 16:9, 21:9 and square alternatives), optional image resizing/cropping, and browser-side sample images.

All 11 files were inventoried and their roles reviewed:

| File | Disposition |
|---|---|
| `README.md` | Chinese-language project purpose, usage, public preset sharing and YUV/convolution references. |
| `index.html` | 543-line Vue UI; 34 style-bound controls including image fit/ratio, watermark, overlays and processing settings. Browser-only UI, not portable. |
| `vaporwave.js` | 574-line pixel processor; YUV transform, independent U/V grading, quantization, chroma plane offset, 4:2:0 chroma averaging, interlace-like line treatment, row displacement, noise and convolution. |
| `document.js` | 449-line image loading, clipboard/drop handling, browser API requests, localStorage user presets, Vue state and JPEG export. Host-specific. |
| `styles.js` | 19 built-in artistic look recipes. Reviewed; no recipe is a self-contained renderer. |
| `images.js` | Curated sample-image filename list and lazy loading. Media catalog only. |
| `document.css` | 599-line layout/theme/slider styling. UI-only. |
| `manifest.json` | PWA metadata and remote icon reference. |
| `logo.png` | Source application's logo/watermark asset; not part of the signal processor. |
| `logo-white.png` | Alternate source logo asset; not part of the signal processor. |
| `.gitignore` | Repository housekeeping. |

The folder contains no `LICENSE` file and its README does not state a license. No upstream JavaScript, CSS, UI, image, preset or asset was copied into VHS-90. Preserve this license uncertainty in any future distribution review; the native algorithm implementation below is a clean C++ adaptation of the general color-processing behavior, not a verbatim code port.

## 2. Processing algorithm inventory

The image processor performs the following operations:

1. Converts 8-bit RGB to limited-range-style YUV-like values using a fixed matrix and integer floor, then converts back with clipping.
2. Applies luma contrast/brightness, black/white clipping fades and independent U/V saturation multipliers.
3. Applies independent U and V color-axis offsets, followed by chroma quantization/solarization at a user-selected step.
4. Performs spatial U/V displacement by separate index offsets; the two color planes can drift by different pixel/row offsets.
5. Optionally averages U and V over 2×2 blocks (4:2:0-like chroma resolution reduction), with a row displacement/noise option.
6. Applies a scanline/interlace-inspired luma operation, with line period, horizontal luma displacement and per-line light variation.
7. Applies row-dependent horizontal displacement using a power curve and random per-row perturbation.
8. Supports luma inversion, bright-region noise and shadow-weighted noise.
9. Offers four 3×3 convolution kernels: directional right/left edge emphasis, box blur and emboss-like response.
10. Includes a generated snow/static mode, crop/contain/cover fitting, aspect/ratio and border controls, logo watermark and optional style-name overlay.

The browser output's JPEG quality, image-size resizing, remote public-preset submission, image-gallery assets and localStorage preset persistence are not signal algorithms and are not brought into the AE render path.

## 3. Native VHS-90 overlap and migration decisions

| Source behavior | VHS-90 implementation | Disposition |
|---|---|---|
| RGB/YUV-like luma/chroma transform | Shared RGB↔YIQ conversion | Existing shared color-space layer reused; YUV U/V are not falsely treated as numerically identical to YIQ I/Q. |
| Independent chroma-axis gain | Previously only global saturation existed in Image Grade | **New independent Chroma I Gain and Chroma Q Gain controls added.** |
| Independent chroma-axis offset | No direct grade control; spatial chroma shift is a different operation | **New Chroma I Offset and Chroma Q Offset controls added.** |
| Chroma quantization/posterization | No independent chroma-axis quantizer | **New Chroma Quantization control added.** |
| 4:2:0-like chroma downsampling | `processChromaSubsample` already performs configurable area-based I/Q subsampling and interpolation | Reuse the existing general engine; do not add a second 4:2:0 implementation. |
| U/V spatial displacement | `VHS90ChromaShiftEngine` and chroma-sub-sample I/Q offsets | Reuse existing spatial-shift stages; keep them separate from color-axis offsets. |
| Brightness, contrast, fade, saturation, vignette, flicker | `VHS90ImageGradeEngine` | Existing controls reused. |
| Luma edge convolution/sharpening | `VHS90ImageGradeEngine` edge enhancement | Existing general edge engine reused; source's directional/emboss kernels are not claimed as exact matches. |
| Interlace/scanline appearance | Temporal field weave, geometry interlace and scanline engine | Reuse existing signal stages; source's canvas luma-only line write is not a full field simulation. |
| Row displacement / horizontal tearing | Geometry line jitter, edge wave, tracking and arbitrary-region engines | Reuse the native geometry/region modules; do not duplicate a second row-warp engine. |
| Bright/shadow noise and snow | Analog noise and Static engine | Existing deterministic frame/seed noise and static pattern engines reused. |
| Crop, fit, aspect ratio, watermark, text overlay | AE layer dimensions and native OSD/geometry controls | Browser/export UI behavior excluded; source logo and watermark are not imported. |
| 19 named recipes | Existing VHS Style presets | Not imported wholesale: many depend on browser crop/fit, watermarks, style text, JPEG quality, fixed preview sizes and source-only random operations. Exact native translation needs per-look render comparisons rather than copying raw objects into unrelated AE controls. |

## 4. Actual C++/AE integration in this pass

Extended the canonical `VHS90ImageGradeEngine` rather than creating a duplicate Vaporwave renderer:

- `ImageGradeParams` now has independent `chromaIGain`, `chromaQGain`, `chromaIOffset`, `chromaQOffset` and `chromaQuantization` values.
- Processing applies independent I/Q gain and offsets after the existing global saturation/fade stage, then optionally quantizes each chroma axis independently. Luma and alpha remain unchanged by the new color-axis controls.
- Exact neutral defaults preserve the existing image-grade identity bypass.
- AE exposes five controls: Chroma I Gain, Chroma Q Gain, Chroma I Offset, Chroma Q Offset and Chroma Quantization. The controls are read into RenderSettings and mapped into the canonical Image Grade stage.
- Added deterministic smoke coverage for repeatable color processing, non-neutral I/Q color behavior and the difference between quantized and non-quantized chroma output.
- Extended the static imported-preset/AE parameter validator to require all five UI reads and all five native parameter mappings.

Code commits:
- `4553e3c821ff066d59b47a52a6be984030889077` — ImageGrade parameter model.
- `5001ce8cc9600a6618afefdd252320e62c89a8f9` — independent I/Q processing and quantization.
- `6b2156014b264412c2c14d24d7eccd2dc0b7b1df` — smoke tests.
- `90afc12e7fd35e64d7a75205c1a8352cd72891fb` — AE parameter/UI wiring.
- `935f4fee5762e577d0dfa9abbf6996f2f68af473` — static validator requirements.

## 5. Verification boundary

- Static imported-preset/AE parameter validator passed in the latest run observed at audit time.
- GitHub Actions run `44816026051` completed its static validator, CMake build and all 32/32 CTest smoke tests successfully. The run was still performing post-job cleanup when last checked; all substantive build/test steps reported success.
- The source's browser UI and remote API were not executed. The original JavaScript processor was inspected statically.
- No source-vs-native pixel parity is claimed. The YUV-like matrix is not numerically identical to VHS-90's YIQ matrix; the new controls intentionally operate in the plugin's native YIQ domain.
- Windows/MSVC Adobe SDK compilation, PiPL, AEX loading, After Effects visual output and MFR remain global release gates.

## 6. Folder closure

**Inventory and algorithm review: complete (11/11 files).**  
**Useful unique behavior: integrated into canonical Image Grade (five independent I/Q controls).**  
**Duplicate chroma subsampling, spatial shift, scanline, geometry and noise engines: not added.**  
**19 browser-style recipes: documented but not falsely imported as native-equivalent presets.**  
**Source license: no LICENSE file found in the supplied folder; no upstream source code copied verbatim.**  
**Core CI: static validator PASS, CMake build PASS, CTest 32/32 PASS (run 44816026051).**

Next folder in the user's eight-folder sequence: `vhs.py-main`.
