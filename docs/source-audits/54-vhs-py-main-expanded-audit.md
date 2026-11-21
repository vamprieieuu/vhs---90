# VHS-90 — vhs.py-main expanded source audit and native migration confirmation

Date: 2026-11-20  
Archive path: `vhs/github repos/vhs.py-main/`  
Inventory: **13 entries: 9 files + 4 directories**  
Primary engine: `vhs.py` (260 lines); desktop/video wrapper: `index.py` (98 lines); browser bridge: `ui/js/core.js` (121 lines).

## 1. File-by-file disposition

| File | Disposition |
|---|---|
| `.gitignore` | Python/cache/build exclusions. |
| `README.md` | Documents a basic Python analog videotape emulator; explains luma/chroma separation and notes video output has no audio. |
| `requirements.txt` | OpenCV, NumPy, MoviePy, Eel, pydub, blendmodes and a large set of conflicting duplicate OpenCV distributions. Not suitable for the native AE runtime. |
| `vhs.py` | 260-line VHS class; reviewed all methods and the processing order. |
| `index.py` | Eel/OpenCV/Tkinter/FFmpeg-adjacent wrapper; loads a video, forces 640×480, writes MP4, displays preview, and does not include audio in its output. |
| `ui/index.html` | Desktop browser UI, parameter sliders and embedded base64 artwork; not a native AE panel. |
| `ui/js/core.js` | 121-line Eel bridge for sliders, preview, reset and JSON settings export. |
| `ui/js/FileSaver.js` | Third-party FileSaver implementation; header identifies X11/MIT, but no standalone license file was supplied. Not copied. |
| `ui/css/water.css` | 1,690-line Water.css theme/framework stylesheet; UI-only. |

The four directory entries are the project root, `ui/`, `ui/css/` and `ui/js/`. No root LICENSE file is present. No upstream Python, JavaScript, HTML or CSS source was copied during this pass.

## 2. Signal algorithm inventory

The source class separates image components using YCrCb and provides:

- Horizontal luma downsample/upsample using OpenCV Lanczos4, with Gaussian luma noise (mean and sigma).
- Horizontal chroma downsample/upsample using Lanczos4, with independently generated per-channel chroma noise.
- Generation multiplier that scales luma noise mean/sigma and chroma noise intensity.
- Row-dependent chroma wave displacement; a second lower-amplitude wave variant and a separate switch-noise displacement.
- Optional box blur, HSV saturation adjustment, Gaussian unsharp mask and high-pass/Soft Light sharpening.
- A fixed right-edge black border and a second right-edge green-channel zeroing artifact.
- Additional YIQ conversion and DFT ringing helpers that are not correctly wired into the normal processing path. In particular, `applyRinging` calls the defined `ringing` helper with an incompatible signature; do not port this broken/dead path as a working feature.
- Processing order: `sharpen_image → switchNoise → processFrame → waves → waves2 → blur → sharpen2`.

The desktop wrapper rescales every frame to 640×480, writes a silent MP4 and has no After Effects SmartFX, arbitrary-size, alpha, HDR or MFR support.

## 3. Canonical VHS-90 integration already present

The main portable source behavior was migrated earlier into `VHS90PyVhsGenerationEngine` and is already in the canonical CMake core and AE adapter. This expanded audit confirms the mapping:

| vhs.py behavior | VHS-90 implementation |
|---|---|
| Lanczos4 horizontal luma reduction/reconstruction | `resizeHorizontal` + Lanczos4 kernel in `VHS90PyVhsGenerationEngine`. |
| Independent chroma reduction/reconstruction | Separate Cr/Cb planes compressed independently in the same engine. |
| Gaussian luma noise mean/sigma | Deterministic Gaussian noise generated from the render-local seed/frame hash. |
| Independent chroma noise | Independent deterministic Cr/Cb noise streams. |
| Generation scaling | `generation` multiplies luma noise mean/sigma and chroma noise. |
| Row-dependent chroma wave | Deterministic per-row chroma wave with independent frequency and temporal phase controls. |
| Separate YIQ conversion | Shared VHS-90 RGB↔YIQ functions; the native engine uses normalized float values rather than the source's 8-bit integer planar buffers. |
| Switch noise / large raster displacement | Shared arbitrary-position Tracking Noise and Head Switching region engines; the physical source's fixed raster shift is not copied as another global warp. |
| Box blur / vertical luma softness | Existing vertical luma blur, luma smear and tape-detail stages. |
| Unsharp/edge sharpening | Existing Image Grade edge enhancement and CRT equalizer; not claimed as exact parity with the source's two different sharpening formulas. |
| Fixed right-side border and green-channel cut | Excluded; use native frame/overscan/geometry controls rather than baking a one-sided green stripe into every render. |
| Browser/desktop UI, video I/O and audio | Excluded; AE owns panel, frame scheduling, source dimensions and host rendering. |

The AE adapter already exposes six PyVHS controls: Luma Compression, Chroma Compression, Luma Noise, Chroma Noise, Chroma Wave and Generation. The native module handles arbitrary image dimensions, deterministic frame/seed processing and premultiplied alpha.

## 4. Migration decision

No second VHS generation engine was added. The useful core algorithm was already ported; duplicating it would create conflicting compression/noise controls. The source's fixed border, hard-coded 640×480 wrapper, non-deterministic global random calls and broken DFT ringing helper are not copied. The two source sharpening passes are represented by the existing general Image Grade/CRT controls; a pixel-identical Soft Light high-pass variant is not claimed.

No source-level runtime test was run. The earlier VHS-90 PyVHS engine was already included in the core CMake smoke-test target. The current core CI after the Vaporwave color-grade addition passed the static AE validator, CMake build and all 32/32 CTest smoke tests: https://github.com/vamprieieuu/vhs---90/actions/runs/44816026051.

## 5. Folder closure

**Inventory: complete (9/9 files).**  
**Useful luma/chroma compression/noise/wave behavior: already integrated in canonical C++ and AE controls.**  
**No duplicate engine or Python/UI runtime added.**  
**Broken/dead ringing helper, fixed green border and silent video wrapper: excluded.**  
**License: no root LICENSE file supplied; no source code copied.**  
**Core CI: PASS (32/32).**

Next folder in the user's eight-folder sequence: `vhs-main`.
