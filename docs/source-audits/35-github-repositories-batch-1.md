# GitHub repositories audit — batch 1

Date: 2026-11-19
Scope: repositories contained in the supplied archive's `github repos/` folder. The archive is not currently mounted in the execution workspace, so original folder names were cross-matched against public GitHub repositories using the repository names retained in the project inventory. The list below records exact repository matches where verified.

## Repository inventory and tree inspection

All tree requests below returned complete recursive Git trees (`truncated=false`).

| Archive folder | Matched upstream | Tree entries | Files | Directories | Status |
|---|---|---:|---:|---:|---|
| `Analog-NTSC-master` | `kdkd/Analog-NTSC` | 42 | 31 | 11 | Tree + key C++ source inspected |
| `BarashNTSC-0.1.2` | `LittleBlabbit763/BarashNTSC` | 11 | 10 | 1 | Tree + main Python file inspected |
| `ComfyUI_RetroTape-main` | `SKBv0/ComfyUI_RetroTape` | 29 | 25 | 4 | Tree + CPU/GPU cores inspected |
| `NTSC-CRT-v.2.3.2` | `LMP88959/NTSC-CRT` | 33 | 32 | 1 | Tree + NTSC/VHS/core C source inspected |
| `NTSC-VHS-Renderer-1.3.0` | `F33RNI/NTSC-VHS-Renderer` | 41 | 32 | 9 | Tree + frame processor inspected |
| `NTSCRT-0.26.0` | `finnmckenty/NTSCRT` | 420 | 359 | 59 | Tree + tape/NTSC/glitch/preset files inspected |
| `URP-VHS-Retro-Effects-main` | `nscTechArt/URP-VHS-Retro-Effects` | 327 | 293 | 34 | Tree inspected; Unity assets dominate |
| `VHS_effect_add-on-1.0.0` | `heheda2333/VHS_effect_add-on` | 144 | 101 | 43 | Tree + README/UI inspected; processing core remains to inspect |
| `composite-video-simulator-master` | `joncampbell123/composite-video-simulator` | 36 | 36 | 0 | Tree + NTSC and VHS-LED C++ sources inspected |
| `lost-signal-vhs-main` | `AlexDarkStalker2002/lost-signal-vhs` | 40 | 34 | 6 | Tree + analog-color and composite GLSL inspected |
| `ntscqt-1.5.0` | `JargeZ/ntscqt` | 41 | 33 | 8 | Tree inspected; Python renderer still needs full source review |
| `vhs-phys-main` | `allthemalware/vhs-phys` | 225 | 202 | 23 | Detailed scoped source audit: see `34-vhs-phys-rust-simulator.md` |
| `vaporwave-master` | previously audited | — | — | — | Existing scoped audit: `26-vaporwave-image-stylizer.md` |
| `vhs.py-main` | `torrober/vhs.py` | 15 | 10 | 5 | Existing scoped audit: `27-vhs-py-processor.md` |
| `vhs-main` | **upstream match not yet verified** | — | — | — | Must resolve from original archive metadata before source audit |

## Direct code findings

### 1. Analog-NTSC — C++ composite signal pipeline

Inspected `ntsc_processor.cpp`, `ntsc_effects.cpp` (68,961 chars), and `ntsc_params.h`.

- Uses a clear encode → signal-domain effects → decode frame pipeline.
- Exposes separate parameter namespaces for decoder, effects, and encoder.
- Effect parameters include noise, multipath gain/delay/ensemble, line-time jitter, RF drift, AM nonlinearity, impulse noise, AFC hunt, chroma flutter, AGC pump, hum, Y/C crosstalk, H/V sync noise, burst noise, VHS tracking, wrinkle, head-switch and dropouts.
- Strong reference for parameter metadata/ranges and signal-domain ordering. The source file is large and has its own signal representation; no wholesale copy was made.
- The source declares BSD-2-Clause headers. Retain attribution/license if any code is ported.

### 2. BarashNTSC — Python/OpenCV batch processor

Inspected the 66 KB main script and tree. It is a desktop/batch-processing application using Python, OpenCV, NumPy and Pillow, with multiprocessing, temporary files, and UI/file management. No native AE-ready DSP module has yet been isolated. Do not port its GUI, file IO or process orchestration into the AEX.

### 3. ComfyUI RetroTape — CPU/GPU VHS implementation

Inspected `ntsc_core.py` (58 KB), `ntsc_gpu.py` (38 KB), and repository tree.

- CPU core is an adapted NTSC-QT pipeline; source itself documents the upstream attribution.
- Useful concepts include separate YIQ processing, fixed reference raster scaling, slow temporal state epochs, ringing taps, black-lift pedestal and explicit CPU/GPU parity testing.
- GPU module is CUDA/PyTorch and cannot be dropped into a CPU-first AE plugin.
- Best use: port selected mathematical kernels and tests, not Python/ComfyUI/Torch infrastructure. Verify the source license and required attribution before copying code.

### 4. NTSC-CRT — integer C NTSC/VHS signal emulation

Inspected `crt_core.c`, `crt_ntsc.c`, and `crt_ntscvhs.c`.

- Integer/fixed-point signal encoding and decoding, with a dedicated NTSC+VHS system implementation.
- Fixed-point exponential and interpolated sine/cosine helpers are potential deterministic CPU references.
- This code is structurally closer to native C++ than the Python projects, but its compile-time system macros and pixel-format assumptions need isolation before integration.
- No source file copied yet; inspect full core and test vectors before porting.

### 5. NTSC-VHS-Renderer

Inspected `FramesProcessor.py` and the full tree.

- Python GUI/video renderer, with frame buffering, multiprocessing and an optional Cython bloom module.
- Bundles FFmpeg binaries and a font; these are not VHS-90 runtime dependencies.
- Main value is visual workflow and effect staging, not a self-contained native processing kernel.

### 6. NTSCRT 0.26.0

Inspected `NtscStage.swift`, `Tape.swift`, `GlitchRenderer.swift`, `BuiltInPresets.swift`, and MIT license.

- `Tape.swift` contains physically grounded tape geometry and maps tape position/track geometry to picture lines.
- `GlitchRenderer.swift` uses forward-only simulation, deterministic replay after scrubbing, and periodic checkpoints keyed to the settings history. This is an important model for deterministic temporal effects in AE.
- `NtscStage.swift` describes a CPU NTSC stage with explicit GPU↔CPU staging and a thread-ownership rule; not directly portable to AE.
- Presets are discovered from JSON rather than hard-coded in the UI.
- MIT license; preserve copyright/license notice if code is reused.
- This is a newer, larger macOS Swift/Metal app with an ntsc-rs bridge; do not copy app/Metal bridge code into VHS-90.

### 7. URP VHS Retro Effects

Recursive tree inventory is complete (327 entries, 293 files). Most of the tree is Unity sample assets, materials, prefabs and metadata. Shader and renderer source files still need to be isolated and reviewed; no code transfer decision is final.

### 8. VHS Effect Add-on

Inspected README and GUI. It has five main UI groups: effect strength, output settings, date/time overlay, text overlay and processing log. README describes chroma bleed/subsampling, luma noise, scanlines, jitter, blur, glow, saturation, REC/PLAY/date overlays, plus directory-tree batch processing. Repository contains a bundled OpenCV distribution and FFmpeg binary; those are packaging/runtime baggage, not suitable for the AE plugin. The actual `vhs_lib/vhs_effects.py` and `vhs_video.py` still need code-level audit.

### 9. Composite Video Simulator

Inspected `ffmpeg_ntsc.cpp` (about 90 KB), `ffmpeg_vhsled.cpp` (about 32 KB), and tree. These are FFmpeg command-line filter/processing programs with libav dependencies. They are useful for algorithm research, but the source files are not standalone AE kernels. Need isolate the independent math and remove FFmpeg I/O before porting.

### 10. Lost Signal VHS

Inspected `analog_color.glsl` and `composite.fsh` (about 30 KB).

- Explicit RGB↔YIQ conversion and I/Q phase rotation.
- Correlated smooth value noise rather than independent per-line hashes, useful for continuous time-base error.
- Shader depends on Minecraft shaderpack uniforms, depth buffers, camera matrices and persistent render targets; only independent equations are candidates for a C++ port.

## Code transfer ledger

- New code files copied into VHS-90 in this batch: **0**.
- New algorithms identified for C++ port: deterministic tape-coordinate defect map; lateral track-overlap model; stateful temporal replay/checkpoint model; selected fixed-point NTSC math; selected signal-domain effect kernels (multipath, line jitter, burst/sync instability, chroma phase drift).
- Reason for zero copy in this batch: the reviewed files are coupled to Python, Swift/Metal, CUDA, FFmpeg, Unity or repository-specific data structures. A verbatim copy would introduce non-compiling dependencies or incorrect host/runtime assumptions. Existing VHS-90 modules already implement some overlapping YIQ/IIR/composite/region behavior, so each candidate must be compared and tested before duplication.

## What remains before this 15-repository audit is complete

1. Verify the upstream identity and exact source snapshot for `vhs-main` from the original ZIP metadata.
2. Inspect the remaining files and subfolders of every repository, not just recursive tree inventories.
3. Finish `ntscqt` source review (especially renderer, templates and filters).
4. Inspect all Unity shader/renderer source files in URP VHS Retro Effects.
5. Inspect `vhs_lib/vhs_effects.py`, `vhs_video.py`, and `vhs_task.py` in VHS Effect Add-on.
6. Inspect all relevant C/C++ functions and tests in NTSC-CRT, Analog-NTSC and Composite Video Simulator.
7. Review all README, docs, presets, test suites and licenses in every repository.
8. Convert only independent useful kernels to C++17, wire them into the unified VHS-90 pipeline, and add tests for each port.

**Completion status: NOT COMPLETE.** This document records a batch of source-level inspections and a complete tree inventory for 12 matched repositories, plus two prior scoped audits. It does not claim all files in all 15 repositories have been read, nor that useful code has been transferred.
