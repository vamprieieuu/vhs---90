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
| `URP-VHS-Retro-Effects-main` | `nscTechArt/URP-VHS-Retro-Effects` | 327 | 293 | 34 | Tree + main multi-pass shader inspected; Unity assets dominate |
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

Inspected `ntsc_processor.cpp`, `ntsc_effects.cpp` (68,961 chars), `ntsc_params.h`, `ntsc_decoder.cpp` (684 lines), `ntsc_signal.cpp` (681 lines), `ntsc_encoder.cpp`, `ntsc_params.cpp`, and `rf2_ntsc.cpp` (720 lines).

- Uses a clear encode → signal-domain effects → decode frame pipeline.
- Exposes separate parameter namespaces for decoder, effects, and encoder.
- Effect parameters include noise, multipath gain/delay/ensemble, line-time jitter, RF drift, AM nonlinearity, impulse noise, AFC hunt, chroma flutter, AGC pump, hum, Y/C crosstalk, H/V sync noise, burst noise, VHS tracking, wrinkle, head-switch and dropouts.
- The main effects body was scanned through its implementation sections: DC restoration and sync-referenced AGC; causal group delay; line-time jitter; RF drift; AM compression/nonlinearity; impulse noise; hum; AFC hunting; AGC pumping; chroma flutter; Y/C crosstalk; VHS tracking; head switching; tape wrinkle; dropouts; H/V/burst lock noise; spectrally shaped noise.
- Decoder includes blank-level estimation, burst phasor/phase/SNR measurement, sync-center and V-sync detection, luma reconstruction, chroma soft limiting, and per-frame receiver instability. Signal module includes NTSC sync/burst generation, YIQ-to-composite modulation, SIMD paths (SSE/NEON), and VBI pseudo-VITC/VITS patterns. The AE adapter uses a fixed 720×480 intermediate and its own parameter/preset setup, so its host-specific scaling/parameter code is not reusable as-is.
- Strong reference for parameter metadata/ranges and signal-domain ordering. The source is coupled to a raw sampled composite frame; no wholesale copy was made.
- The source declares BSD-2-Clause headers. Retain attribution/license if any code is ported.

### 2. BarashNTSC — Python/OpenCV batch processor

Inspected the 66 KB main script, including `VHSProcessor.process` (lines 309–497), and tree. Its image path includes edge enhancement/fade, spatial noise banks, chroma shift, composite phase, contrast/brightness/saturation, tape wear/dropouts, horizontal jitter, tracking, interlace, prior-frame ghosting, scanlines and vignette. It relies on Python/OpenCV/NumPy/Pillow and UI/file management. Cached base maps/carrier patterns and noise banks are the main ideas worth retaining; no native AE-ready kernel has yet been isolated. Do not port its GUI, file IO or process orchestration into the AEX.

### 3. ComfyUI RetroTape — CPU/GPU VHS implementation

Inspected `ntsc_core.py` (1,738 lines), `ntsc_gpu.py` (38 KB), `tests/test_fidelity.py` (308 lines), and repository tree.

- CPU core is an adapted NTSC-QT pipeline; source itself documents the upstream attribution.
- Useful concepts include separate YIQ processing, fixed reference raster scaling, slow temporal state epochs, ringing taps, black-lift pedestal and explicit CPU/GPU parity testing.
- GPU module is CUDA/PyTorch and cannot be dropped into a CPU-first AE plugin.
- Core contains separate kernels for row low-pass/resampling, YIQ conversion, chroma bleed/delay/drift, ringing, composite artifacts, luma/chroma noise, luma smear, reference-line scaling, servo profile, tracking warp, interlace roll, comb filtering, dropouts, ghosting, scanlines, analog glow, edge wave and head switching. Its luma-smear kernel is more advanced than the Unity four-tap source: a variable-length causal exponential kernel plus a separate highlight-only glow trail. Keep this as the next upgrade reference for `processLumaSmear`.
- Tests cover preview/render fidelity, frame offsets, temporal transport stability, resolution scaling, bit depth, interlace pitch, and cache budgets. This is useful validation design for VHS-90.
- Best use: port selected mathematical kernels and tests, not Python/ComfyUI/Torch infrastructure. Verify the source license and required attribution before copying code.

### 4. NTSC-CRT — integer C NTSC/VHS signal emulation

Inspected `crt_core.c` (673 lines), `crt_ntsc.c` (332 lines), and `crt_ntscvhs.c` (339 lines), including integer YIQ conversion, interpolated quarter-wave sine/cosine, fixed-point exponential, IIR band-limiting, composite carrier/burst synthesis, field phase alternation, and the NTSC-VHS modulator. The integer source uses static global IIR state and `rand()` in places, so those pieces cannot be copied into an MFR render path unchanged.

- Integer/fixed-point signal encoding and decoding, with a dedicated NTSC+VHS system implementation.
- Fixed-point exponential and interpolated sine/cosine helpers are potential deterministic CPU references.
- This code is structurally closer to native C++ than the Python projects, but its compile-time system macros and pixel-format assumptions need isolation before integration.
- No source file copied yet; inspect full core and test vectors before porting.

### 5. NTSC-VHS-Renderer

Inspected `FramesProcessor.py`, `bloom/bloom.pyx` (1,843 lines), and the full tree.

- Python GUI/video renderer, with frame buffering, multiprocessing and an optional Cython bloom module.
- Bundles FFmpeg binaries and a font; these are not VHS-90 runtime dependencies.
- `bloom.pyx` includes 5×5 blur kernels, threshold-based bloom masks, fast/slow multiscale bloom paths, and masked filtering for RGB/RGBA. Its pixel kernels are Cython and use NumPy memoryviews; the actual convolution math can be ported to C++ later. Apache-2.0 source license; preserve notices.
- Main value is visual workflow and effect staging, not a self-contained native processing kernel.

### 6. NTSCRT 0.26.0

Inspected `NtscStage.swift`, `Tape.swift`, `GlitchRenderer.swift`, `BuiltInPresets.swift`, and MIT license.

- `Tape.swift` contains physically grounded tape geometry and maps tape position/track geometry to picture lines.
- `GlitchRenderer.swift` uses forward-only simulation, deterministic replay after scrubbing, and periodic checkpoints keyed to the settings history. `Receiver.swift` implements a stateful receiver simulator with horizontal AFC, vertical oscillator/sync integration, burst-key phase tracking, deck and transport state, snapshots/restores, and frame-grid resimulation. This is an important model for deterministic temporal effects in AE.
- `NtscStage.swift` describes a CPU NTSC stage with explicit GPU↔CPU staging and a thread-ownership rule; not directly portable to AE.
- Presets are discovered from JSON rather than hard-coded in the UI.
- MIT license; preserve copyright/license notice if code is reused.
- This is a newer, larger macOS Swift/Metal app with an ntsc-rs bridge; do not copy app/Metal bridge code into VHS-90.

### 7. URP VHS Retro Effects

Recursive tree inventory is complete (327 entries, 293 files). Most of the tree is Unity sample assets, materials, prefabs and metadata. Inspected `Retro.shader` (344 lines): it has downsample/upsample blur passes, two horizontal smear passes (four taps each with exponential falloff, different decay/offset constants), and a composite pass for bleed, edge, tape noise and scanline modulation. The smear pass is a small self-contained kernel; Unity pipeline wrappers were not copied.

### 8. VHS Effect Add-on

Inspected README, GUI, `vhs_lib/vhs_effects.py` (1,412 lines) and `vhs_lib/vhs_video.py`. The effects module has a reusable `VHSParams`/`VHSContext` design and separate methods for anisotropic chroma low-pass/subsampling/offset, TBC jitter and vertical roll, head switching, tracking bands, dropouts, low-resolution noise, softness/bloom/sharpen, color, scanlines and OSD glyph/date generation. Its render order is explicit. Frame processing is still Python/PIL/OpenCV; `vhs_video.py` is FFmpeg I/O and should not enter the AEX. OSD and effect-stage ordering are useful references, but no code was copied.

### 9. Composite Video Simulator

Inspected `ffmpeg_ntsc.cpp` (2,333 lines, about 90 KB), `ffmpeg_vhsled.cpp` (1,019 lines, about 32 KB), and tree. The FFmpeg input/output, audio and frame scheduling layers are host-specific. Relevant signal sections inspected include RGB↔YIQ, three-stage I/Q low-pass with distinct I/Q cutoffs and sample delays, phase-aware chroma-to-luma carrier modulation/demodulation, composite pre-emphasis, luma/chroma noise, chroma phase noise, VHS SP/LP/EP bandwidth profiles, vertical chroma delay-line blending, playback sharpening, chroma loss and head-switching ramp. These algorithms overlap VHS-90's YIQ bandwidth, Composite and Region engines; the vertical chroma blend and per-speed bandwidth calibration remain candidates for separate C++ additions after comparison with the current IIR implementation.

### 10. Lost Signal VHS

Inspected `analog_color.glsl` and `composite.fsh` (about 30 KB).

- Explicit RGB↔YIQ conversion and I/Q phase rotation.
- Correlated smooth value noise rather than independent per-line hashes, useful for continuous time-base error. Its interpolation pattern was adapted into VHS-90's C++ `valueNoise1D` helper and line-wobble correlation control.
- Shader depends on Minecraft shaderpack uniforms, depth buffers, camera matrices and persistent render targets; only independent equations are candidates for a C++ port.

## Code transfer ledger

- New code files copied verbatim into VHS-90 in this batch: **0**.
- Adapted C++ code added: `valueNoise1D` in `VHS90SignalKernels.cpp`, declared in the matching header, and wired into line-wobble generation using a new correlation-length parameter. This adapts the correlated smooth value-noise method inspected in Lost Signal VHS's `analog_color.glsl` / `composite.fsh`; it is a native C++ implementation, not a raw GLSL copy.
- Adapted C++ code added: `processLumaSmear` in `VHS90SignalKernels.cpp`, declared in the matching header. It now uses a configurable 2–64 tap causal exponential kernel and a separate highlight-only glow trail, based on the more complete ComfyUI RetroTape implementation; it keeps I/Q chroma separate and preserves alpha/premultiplication.
- Adapted C++ code added: Tracking Noise servo drift now uses deterministic row-correlated profiles crossfaded between temporal epochs, adapting the ComfyUI RetroTape servo-profile interpolation model. Added a configurable epoch length and a continuity smoke test.
- Tests added in `Tests/VHS90SignalKernelsSmoke.cpp`: deterministic repeatability/local continuity for `valueNoise1D`; variable-tap luma-smear/highlight trail output and alpha preservation for `processLumaSmear`. `Tests/VHS90RegionEngineSmoke.cpp` now checks temporal continuity of Tracking Noise servo drift across adjacent frames.
- Commits: value-noise header/source/test `41ee8ee` / `7ef93f2` / `a270b41`; luma-smear header/source/test `4844c3b` / `98f9e06` / `bbd0651`; regression fixes `4eddefd` / `730833e`.
- Validation: the first hosted run exposed two failures (RegionEngine signed-chroma expectation and StaticEngine zero-density identity). Corrected the test's signed chroma expectation and added an exact zero-density identity bypass. GitHub Actions run `44591429056` on `730833e` passed 6/6 CTest. The subsequent variable-smear and temporal-servo changes also passed GitHub Actions run `44592423967` on commit `9e1b5b8`.
- New algorithms identified for C++ port: deterministic tape-coordinate defect map; lateral track-overlap model; stateful temporal replay/checkpoint model; selected fixed-point NTSC math; selected signal-domain effect kernels (multipath, line jitter, burst/sync instability, chroma phase drift).
- Reason for zero copy in this batch: the reviewed files are coupled to Python, Swift/Metal, CUDA, FFmpeg, Unity or repository-specific data structures. A verbatim copy would introduce non-compiling dependencies or incorrect host/runtime assumptions. Existing VHS-90 modules already implement some overlapping YIQ/IIR/composite/region behavior, so each candidate must be compared and tested before duplication.

## What remains before this 15-repository audit is complete

1. Verify the upstream identity and exact source snapshot for `vhs-main` from the original ZIP metadata. Conversation history confirms the destination repo `vamprieieuu/vhs---90`, but does not contain a separate upstream URL for this bundled folder.
2. Inspect the remaining files and subfolders of every repository, not just recursive tree inventories.
3. Finish `ntscqt` source review of remaining UI/packaging/assets and tests; its main renderer, core VHS methods and preset JSON have now been inspected.
4. Inspect all Unity shader/renderer source files in URP VHS Retro Effects.
5. Inspect remaining `vhs_task.py`, preset/config files and test/self-test paths in VHS Effect Add-on.
6. Inspect remaining NTSC-CRT functions and full Composite Video Simulator math; Analog-NTSC's processor, effects, encoder, decoder, signal and AE adapter have now been reviewed at source level.
7. Review all README, docs, presets, test suites and licenses in every repository.
8. Convert only independent useful kernels to C++17, wire them into the unified VHS-90 pipeline, and add tests for each port.

**Completion status: NOT COMPLETE.** This document records a batch of source-level inspections and a complete tree inventory for 12 matched repositories, plus two prior scoped audits. It does not claim all files in all 15 repositories have been read, nor that useful code has been transferred.
