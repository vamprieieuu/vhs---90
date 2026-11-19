# VHS-90 — Project Status

Updated: 2026-11-19

## Current state

- Plugin name: VHS-90.
- The repository contains scoped audits of the supplied source archive and six host-independent C++ processing modules.
- An initial native AE host adapter now exists in `Source/VHS90Effect.cpp`, with PiPL identity/flags, an optional Windows/MSVC CMake target, a PiPL generation script and `build.ps1`.
- The host adapter has not yet been compiled with MSVC against a local AE SDK or loaded in After Effects. There is no verified AEX artifact and no claim of AE load/render success.

## Work completed in this pass

- Inventoried the supplied archive and wrote scoped audits for the supplied C++/C/Rust/GLSL/WGSL repositories and AEX references.
- Audited the VHS6RIFT native base against the bundled AE SDK declarations, including EffectMain ABI, PF_InData time fields and PiPL/global flag mismatches.
- Audited all 20 VHS Damage Lab prompt documents and 19 associated visual references.
- Audited all 15 supplied font files for metadata, cmap coverage and Arabic OpenType shaping features.
- Audited the VHS camera before/after references and available processed audio stream.
- Added source-matched RGB↔YIQ conversion, YIQ IIR bandwidth filtering, four-phase NTSC composite encode/decode, procedural static patterns, generalized multi-region Tracking Noise / Head Switching and a procedural sync-collapse/time-base engine.
- Added a deterministic correlated 1D value-noise kernel adapted from the Lost Signal VHS shader's smooth-noise method; line-wobble now supports a configurable row-correlation length.
- Added `processLumaSmear`, a C++ CPU port upgraded to a configurable 2–64 tap causal exponential luma-smear kernel with a separate highlight-only glow trail, adapted from ComfyUI RetroTape. It retains I/Q chroma and handles premultiplied alpha.
- Adapted ComfyUI RetroTape's temporal servo-profile crossfade into Tracking Noise. Row-correlated deterministic servo profiles now interpolate between configurable temporal epochs rather than fully re-randomizing every frame.
- Added `processVerticalChromaBlend`, a C++ port of the composite-video simulator's VHS field-line chroma delay. It blends I/Q from the previous same-field line while retaining current-line luma and alpha.
- Added smoke assertions for variable-tap smear/highlight trail, vertical chroma blending/luma preservation, alpha preservation and adjacent-frame servo continuity. GitHub Actions run `44592423967` on commit `9e1b5b8` passed the core build and all 6/6 CTest tests. The source audits and commits are recorded in `docs/source-audits/35-github-repositories-batch-1.md`.
- Added an initial AE SmartFX/legacy host adapter with ARGB32/64/128 conversion and basic UI controls for Composite Signal, Sync Collapse, Static, Tracking Noise and Head Switching.
- Added an optional Windows x64/MSVC AEX target, PiPL resource generation and PowerShell build script. The target searches the local AE SDK tree for PiPLtool.

## Important VHS6RIFT findings

- VHS6RIFT provides useful YIQ/composite, deterministic-noise and AE build architecture references, but its supplied source has conflicting exported function declarations, invalid PF_InData time member accesses and PiPL/global flag mismatches.
- Its fallback PF_* structs do not match the bundled SDK ABI and must not be used for production compilation.
- Its three Python scripts pass, but they are static/model checks; the native render harness is not invoked by the build workflow and fails to compile against the fallback declarations.
- Several documentation files describe later source revisions (112/135/192 parameter slots, NTSC-RS bridge, overlays and CRT modules) that are not present in the supplied 83-slot source snapshot.

## C++ tests run

- CMake configure/build: PASS for the host-independent VHS90Core static library and all six smoke-test executables.
- CTest: 6/6 PASS in Release and Debug with AddressSanitizer + UndefinedBehaviorSanitizer.
- Release smoke-test assertions are now explicitly kept enabled with `-UNDEBUG` / `/UNDEBUG`; an optional `VHS90_ENABLE_SANITIZERS` CMake switch is available.
- Added `.github/workflows/core-ci.yml` to configure, compile and run all six host-independent smoke tests on Ubuntu for pushes and pull requests. The workflow has been committed but its first hosted run has not yet been confirmed.
- Signal-kernel, region, static, IIR, composite-modem and sync-collapse smoke tests all pass under g++ C++17. The first CI run after adding the new source kernels exposed two pre-existing smoke-test issues (signed-chroma expected direction and zero-density static identity); both were corrected. GitHub Actions run `44591429056` on commit `730833e` completed successfully with build PASS and CTest 6/6 PASS, including the new luma-smear and correlated-noise assertions.
- VHS6RIFT static validator, parameter occurrence audit and Python 14-gate model tests: PASS.
- VHS6RIFT native harness compile attempt: FAIL, with SDK fallback/ABI/API declaration errors.
- No Windows/MSVC build or After Effects runtime test has been performed.

## Next engineering gates

1. Compile the new AE host adapter on Windows x64 with MSVC 2022 and the exact AE SDK; fix every compiler/linker/PiPL error.
2. Verify AE 23.2.1 loads the AEX, applies it, previews, renders, changes parameter values and survives repeated renders/MFR.
3. Verify SmartFX full-frame checkout and output-region cropping at full resolution, downsampled previews, non-zero origins and cropped/tiled renders.
4. Verify ARGB32, ARGB64 and ARGB128 output, premultiplied alpha, transparent edges and HDR float values.
5. Continue porting the high-value source algorithms: full composite sync/burst/AFC/PLL, dropout/blackout/hold, tape-surface defects, OSD/font shaping and Blue Screen mode.
6. Complete the remaining source/reference audits and connect the modules to unified presets and the final UI.
7. Compare actual AE renders against the supplied reference clips/stills before marking visual matching complete.
8. Complete the full audit of all 15 bundled GitHub repositories; current batch-1 audit is explicitly incomplete.

Do not call VHS-90 test-ready until the Windows build and in-host validation gates pass.
