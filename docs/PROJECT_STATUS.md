# VHS-90 — Project Status

Updated: 2026-11-20

Latest verified source-pipeline fix: `eb8969184c2d80638cfcb1165dd1984ca02015c2` — moved Image Grade after decoded-frame creation so the composite pipeline no longer references `decodedView` before initialization.

## Current state

- Plugin name: VHS-90.
- The repository contains scoped audits of the supplied source archive and an expanding set of host-independent C++ processing modules. The 15 `github repos/` sources are being treated as a separate audit/migration queue. The first three folders (Analog-NTSC, BarashNTSC, ComfyUI RetroTape) now have complete file-by-file audits and portable algorithm migrations at source level; the remaining 12 folders are still in progress.
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
- Migrated the deterministic tape-coordinate defect map from the previously unbuilt local `src/engines/shared/vhs90_tape_surface` candidate into canonical `Source/VHS90Core/VHS90TapeSurfaceEngine.{h,cpp}`. The canonical version adds explicit premultiplied-alpha handling and an exact inactive bypass.
- Added canonical `VHS90TapeTransportEngine`, `VHS90BloomEngine`, `VHS90TemporalEngine` and `VHS90AnalogGeometryEngine` modules. These port deterministic transport flutter, alpha-aware multiscale bloom, previous-frame ghost/fringe, top skew, edge pincushion, row jitter and scanline modulation.
- Added chroma-under subsampling/bleed and optional vertical luma-only bandwidth. Added PAL-style alternate-line Q phase switching in the composite modem.
- Added independent deterministic luma, hue-biased chroma and fine composite noise, with separate UI controls.
- Added Analog-NTSC-derived RF drift, mains hum, AM nonlinearity, content-driven AGC pump, impulse bursts and three-tap multipath echoes as alpha-aware C++17 engines.
- Added BarashNTSC-derived Tape Detail, opposing Cr/Cb channel shift, frame-driven chroma phase drift, image grading/fade, and alternating-field interlace.
- Added NTSCRT-derived adjacent-track head read model: center/neighbor track overlap and frequency-dependent azimuth attenuation, with SP/LP/EP selection and adjustable track misalignment. Integrated before composite encoding and exposed as `Adjacent Track Crosstalk` / `Track Misalignment` AE controls.
- Extended tape surface with deterministic pinholes, flakes, mold colonies and vaseline film, and replaced uniform spacing attenuation with frequency-dependent head-gap response that softens high-frequency luma detail more than color-under chroma. Previous Frame Ghost now checks out the prior source frame via SmartFX and processes it through the composite core before mixing. GitHub Actions run `44602823951` passed 20/20 CTest tests. NTSCRT track-read integration subsequently passed run `44608850831` with 26/26 CTest tests; tape contamination and frequency-dependent head-spacing refinements passed run `44609446637`, also 26/26.
- Added receiver instability ports for AFC Hunt, chroma flutter, Y/C crosstalk, horizontal/vertical sync noise and burst-phase noise; these are connected to the shared composite engine and exposed as independent AE controls.
- Added ComfyUI RetroTape-derived ringing, two-stage signal ghost/chroma delay, dropout bands/speckles, thresholded analog glow, black lift, row-dependent chroma drift, per-scanline chroma loss, and epoch-crossfaded VHS edge-wave/servo profiles; these are connected to the shared pipeline and exposed as AE controls.
- Separated global mix from `compositeSignalStrength` so each post-signal effect can run without forcing composite encode/decode and without double-scaling its amount.
- Added arbitrary rotation to Tracking Noise / Head Switching region masks while retaining their existing normalized size/position semantics.
- The AE parameter panel now exposes the required Source input, PAL/NTSC, chroma subsample, vertical chroma/luma, luma/chroma/composite noise and hue bias, luma smear, bloom, tape surface, tape transport, geometry, scanline and Tracking Noise region angle controls. The current enum contains 58 slots including four topic-end sentinels, leaving 54 real parameters matching the 54 `PF_ADD_*` calls; Source remains `PARAM_INPUT=0` and is checked out through the SmartFX layer checkout ID.
- Previous-frame ghost is now exposed as `Previous Frame Ghost`. SmartFX requests the immediately preceding source frame with a second checkout ID, checks it in/out per render, and sends it through the same composite core before temporal mixing. This is render-local and avoids mutable global history; Windows/MSVC and live AE/MFR validation remain pending.
- Analog-NTSC repository audit: 31/31 files inventoried and categorized; see `docs/source-audits/36-analog-ntsc-deep-audit.md`. Its raw 525-line/910-sample waveform encode/effects/decode path is now integrated as an optional full-waveform NTSC mode; limitations (8-bit 720×480 internal intermediate, NTSC-only, CPU profiling and reference-image validation) are documented in the audit.
- Added smoke assertions for variable-tap smear/highlight trail, vertical chroma blending/luma preservation, alpha preservation and adjacent-frame servo continuity. GitHub Actions run `44592423967` on commit `9e1b5b8` passed the core build and all 6/6 CTest tests. After migrating the tape-surface engine, run `44593216735` on commit `4f7e34f` passed the build and all 7/7 CTest tests. The source audits and commits are recorded in `docs/source-audits/35-github-repositories-batch-1.md`.
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
8. Complete the full audit and applicable code port for each of the 15 bundled GitHub repositories one folder at a time. Analog-NTSC is the first repository in this 15-folder queue to reach a complete 31/31 file audit and an integrated raw-waveform NTSC mode (CI 20/20). BarashNTSC is the second repository with a complete 10/10 file audit and its portable algorithm set connected, including previous-frame ghost through a second SmartFX checkout ID. Core CI passed 20/20; Windows AE SDK build and live MFR validation remain pending. ComfyUI RetroTape is the third folder with a complete 25/25 file audit and its portable signal kernels migrated; core CI passed 24/24 at run `44605607911`. Host-specific ComfyUI/Python/CUDA UI and preview infrastructure is excluded. Static AE parameter count is 78/78. NTSCRT has a new expanded audit checkpoint at `docs/source-audits/41-ntscrt-expanded-audit.md`: current upstream tree 359 files / 59 directories, partial source review and one integrated physical track-read kernel (CI 26/26); it is not yet complete. Five repository folders have reached their current audit/port completion checkpoint; NTSCRT is the sixth folder under active partial audit, and the remaining nine folders are still queued. The `vhs-main` folder is confirmed (28 files) and has two detailed source-audit reports; its OFX/Metal host layer is not directly portable, but its PAL, geometry, scanline, bandwidth, OSD ordering and bloom concepts are being selectively ported. The upstream remote URL remains unverified from the recovered context.

Do not call VHS-90 test-ready until the Windows build and in-host validation gates pass.


## NTSCRT integration update — 2026-11-20

- Verified the upstream NTSCRT Git tree is complete (359 file blobs, 59 directory entries, not truncated); parsed all 45 preset JSON files.
- Added a source-matched three-track overlap/azimuth-loss adaptation to `VHS90TrackReadEngine`.
- Added deterministic render-local control-track PI servo replay to `VHS90TapeTransportEngine`, wired to the AE Tracking Servo control, including pulse dropout/false pulse handling and search/relock approximation.
- Upgraded `VHS90HeadConditionEngine` to two alternating head states with deterministic debris pickup/lodging/clean-off and grease accumulation/decay. This uses proxy tape-shed/film inputs, not the upstream 12-position physical tape sampling.
- Preserved NTSCRT MIT attribution under `ThirdParty/NTSCRT/`.
- Latest Linux core CI run 449 passed the C++ core build and CTest 27/27: https://github.com/vamprieieuu/vhs---90/actions/runs/44619561972.
- NTSCRT is **not complete**: the 359-file content audit and disposition ledger, physical tape map, full VCR RF/FM/color-under signal chain, receiver parity port, source parity/visual tests, Windows/MSVC build and live AE/MFR validation remain open. See `docs/source-audits/45-ntscrt-same-session-port-status.md`.


## NTSCRT continuation — 2026-11-20

- Ported the NTSCRT physical tape coordinate sampler into `VHS90TapeSurfaceEngine`: tape geometry, active-line/VBI mapping, SP/LP/EP field advance, four-point head-width sampling, source-derived wear/dropout/scratch/edge/crease curves and tape-seed transform. The sampler includes pinholes, flakes/shed, scratches, creases, edge lift, mold and Vaseline film; deck/control-head sampling uses source-compatible detail-off/flake-on mode.
- Enabled optional physical tape sampling in the two-head deck recurrence (12 points per field) and physical control-track pulse detection from the AE parameter adapter. Full source deck history/snapshot parity and Windows/AE execution remain open.
- Corrected three-track read to use absolute source overlap shares, source azimuth attenuation at FM/color-under frequencies, and VBI-scaled adjacent-track line offsets.
- Added `VHS90VcrSignalEngine` coefficient helpers for NTSCRT's 28.636 MHz signal path, 64×81 spacing bank, 95-tap analytic FM bandpass and supporting biquad/shelf/trap filters. The full nine-stage VCR record/playback pipeline is not yet ported.
- Hosted Linux core CI run [520](https://github.com/vamprieieuu/vhs---90/actions/runs/44623626784) passed 28/28 tests. Windows/MSVC, AE SDK, PiPL, AEX loading, live AE and MFR are not validated.
