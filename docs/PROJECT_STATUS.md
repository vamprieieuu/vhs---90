# VHS-90 — Project Status

Updated: 2026-11-21

Latest verified source-pipeline fix: `eb8969184c2d80638cfcb1165dd1984ca02015c2` — moved Image Grade after decoded-frame creation so the composite pipeline no longer references `decodedView` before initialization.

## Current state

- Plugin name: VHS-90.
- The 15 `github repos/` source folders are inventoried. The URP-VHS-Retro-Effects audit is now closed as a full file-disposition audit: all 291 archive entries were enumerated (144 Unity `.meta` files + 147 substantive files), all four C# runtime/example scripts and the complete 343-line HLSL shader were inspected, the MIT license was retained, and portable effects were mapped to existing VHS-90 engines without importing Unity-only assets. See `docs/source-audits/48-urp-vhs-retro-effects.md` and `docs/URP-VHS-Retro-Effects-Integration.md`. Audit closure does not mean every source algorithm is ported or visually matched; URP's feature disposition and visual parity are explicitly scoped.
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
- The AE parameter panel now exposes the required Source input, PAL/NTSC, chroma subsample, vertical chroma/luma, luma/chroma/composite noise and hue bias, luma smear, bloom, tape surface, tape transport, geometry, scanline and Tracking Noise region angle controls. The current enum contains 96 slots including four topic-end sentinels, leaving 92 real parameters matching the 92 `PF_ADD_*` calls; Source remains `PARAM_INPUT=0` and is checked out through the SmartFX layer checkout ID. Nine NTSCRT-derived Tape Material controls now expose independent wear, dropouts, scratches, creases, edge damage, mold, Vaseline/binder film, sticky shed and cassette seed, with tested source-curve/master scaling in the core mapping helper.
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
5. Continue remaining source-exact ports: full composite sync/burst/AFC/PLL, dropout/blackout/hold, remaining tape-physics/receiver parity, bundled font loading and OSD visual parity, and Blue Screen mode.
6. Finish the URP-VHS-Retro-Effects deep file audit where relevant, complete applicable preset migration and connect the modules to unified presets and the final UI.
7. Compare actual AE renders against the supplied reference clips/stills before marking visual matching complete.
8. Source audit status: 14/15 supplied `github repos/` folders have full file-disposition audits: Analog-NTSC (31), BarashNTSC (10), ComfyUI RetroTape (25), NTSC-CRT (32), NTSC-VHS-Renderer (32), NTSCRT (359 blobs: 314 content-reviewed + 45 semantic-reviewed), VHS Effect Add-on (18), Composite Video Simulator (36), Lost Signal VHS (31), NTSCQT (23), vaporwave (11), vhs.py (9), vhs-main (28) and vhs-phys (202). URP-VHS-Retro-Effects is the remaining fast/partial audit (Unity archive 291 entries; its C# scripts and main shader were inspected). These are audit counts, not integration counts.

Do not call VHS-90 test-ready until the Windows build and in-host validation gates pass.


## NTSCRT integration update — 2026-11-20 (current)

### Source audit closure

- Upstream recursive tree: 359 file blobs, 59 directory entries, not truncated.
- All 359 blobs now have a final content disposition: 314 source/app/test/support/documentation/dependency/license/asset rows are content-reviewed; all 45 preset JSON rows are semantic-reviewed. Zero pending rows remain in the ledger.
- The 45 presets are classified per look in docs/source-audits/47-ntscrt-preset-semantic-migration.md. The audit distinguishes active stages from serialized disabled-stage defaults, whole-state NTSC/shader keyframes, digital-only looks and five Screen Loop looks.
- The audit closure means every upstream file was inspected and dispositioned. It does not mean every upstream subsystem has been ported.

### NTSCRT-derived code currently in VHS-90

- VHS90TrackReadEngine: three-track absolute overlap shares, source track selection and separate FM/color-under azimuth attenuation.
- VHS90TapeTransportEngine: deterministic render-local control-track PI replay, physical control-head tape sampling and pulse dropout/false-pulse approximations. It is not the full line-domain capstan/guide/stick-slip model.
- VHS90TapeSurfaceEngine: physical tape sampler with NTSCRT millimetre coordinates, helical track/active-line/VBI mapping, SP/LP/EP advance, four-point head-width sampling, source deterministic hash/value noise, remanence, pinholes, flakes/shed, scratches, creases, edge lift, mold and Vaseline. The source track origin is exactly 1.0 mm; a prior 1.52 mm assumption was corrected. Source wear/dropout/scratch/edge curves, crease top-end and tape-seed transform are exposed.
- VHS90HeadConditionEngine: two-head recurrence with 12 physical tape samples per active field, detail-off/flake-on sampling, debris pickup/lodging/shedding/clean-off and grease transfer/decay. The upstream rolling 12-field snapshot/history behavior is not yet reproduced.
- VHS90VcrSignalEngine: coefficient/helper layer for the 28.636 MHz / 1820 samples-per-line RF path, spacing bank, analytic FM bandpass and supporting filters. It is not yet a complete record/playback signal pipeline.

### Remaining implementation gaps

1. Finish source-exact TapePosition/field-history parity and independent AE controls for each physical tape condition, including user-selectable tape seed and sticky-binder/transport stick-slip coupling.
2. Port and integrate all nine RF/FM/color-under stages (record, tape, magnetization, playback, FM band, line, comb, ACC and output) so the coefficient helpers are actually consumed by a sample-domain render path.
3. Complete receiver parity: line/field sync, AFC/VCO, vertical integrator, burst phase, chroma PLL, ACC/color killer and deterministic history/resimulation. Current sync/composite effects are not a complete source receiver port.
4. Convert the selected analog NTSCRT presets into native VHS-90/AE parameter presets and convert timeline snapshot keys/easing into host keyframes. Digital-only and Screen Loop presets are intentionally excluded from the analog tape engine.
5. Perform deterministic source-vs-C++ sample/frame parity and visual comparisons.
6. Compile with Windows/MSVC and the exact AE SDK, validate PiPL, AEX loading, SmartFX, MFR and live AE rendering. No successful Windows/AE-host validation is claimed.

### CI and host boundary

- The previous Linux core workflow passed run 561 after the tape geometry helper was added. A later correction changed the track origin to the upstream 1.0 mm value; runs 567/568 exposed a stale/incorrect test expectation, which was corrected in commit 7c07cfe. Follow-up Linux core CI run 572 passed after the correction, with all 28/28 CTest tests: https://github.com/vamprieieuu/vhs---90/actions/runs/44779518318.
- Linux core CI only validates host-independent C++ modules and smoke tests. It does not compile the AE adapter or establish Windows/MSVC, PiPL, AEX loading, After Effects rendering or MFR success.
- NTSCRT MIT attribution remains under ThirdParty/NTSCRT. Per-component upstream shader/runtime licenses were reviewed; GPL shader source is excluded from reuse by default.


## Source audit 48 — URP-VHS-Retro-Effects

- Fast-audited `vhs/github repos/URP-VHS-Retro-Effects-main/` from the user Library `vhs.zip`: 291 archive entries, including 144 Unity `.meta` sidecars and 147 substantive files.
- Inspected all four C# scripts and the complete 343-line HLSL shader. Unity URP renderer plumbing and scene/assets were classified as host-specific/non-portable.
- Ported the source's distinct multi-scale animated horizontal tape-noise algorithm into `VHS90AnalogNoiseEngine`; exposed `Tape Noise (Multi-Scale)` as a native AE Signal parameter and added deterministic/time-varying smoke coverage.
- Existing VHS-90 chroma bleed/bandwidth, luma smear, edge enhancement/ringing, interlace/geometry and scanline engines already cover the other major source effects; no duplicate engines were added.
- MIT attribution and license preserved in `ThirdParty/URP-VHS-Retro-Effects/`; full disposition in `docs/source-audits/48-urp-vhs-retro-effects.md`.
- Linux core CI run 613 passed 28/28 tests: https://github.com/vamprieieuu/vhs---90/actions/runs/44783681639. Windows/MSVC and AE-host validation are not claimed.


## Source audit 49 — VHS Effect Add-on 1.0.0 (2026-11-20)

- Completed file-disposition audit of `vhs/github repos/VHS_effect_add-on-1.0.0/`: 20 archive entries (18 files + 2 directories). See `docs/source-audits/49-vhs-effect-addon-full-audit.md`.
- Reviewed the Python image-processing core, five source presets, processing order, OSD, time-base jitter, chroma bleed/subsampling/offset, head switching, moving tracking band, dropouts, noise, optical processing, CLI/GUI wrappers, dependencies, FFmpeg path and self-test.
- Reused existing VHS-90 native engines rather than duplicating already-implemented chroma bandwidth, geometry/transport, arbitrary-region tracking/head switching, dropout, analog noise, tape detail, bloom, image grade and OSD systems.
- Added five source-derived entries to the native `VHS Style Preset` popup: Add-on Light, Add-on Classic, Add-on Heavy, Add-on Worn and Add-on Clean TV. The popup now has 22 entries; new cases 18–22 blend through the existing Style Intensity control. Source behavior is mapped to VHS-90's native controls, not claimed as pixel-identical.
- Extended `tools/validate_imported_presets.py` to keep the original 16 VHS6RIFT checks and additionally require Add-on cases 18–22 and 22 popup entries.
- Source self-test passed 30/30 checks using generated image/video fixtures. The archive omitted its documented Material reference folder; generated fixtures are functional smoke tests, not visual-parity evidence.
- No project LICENSE file was present; no Python source or GPL FFmpeg binary was copied into the native plugin. README's third-party license notes were recorded.
- First CI attempt failed only because the static validator still expected exactly 16 VHS6RIFT styles; it did not reach C++ compilation/tests. The validator was corrected in commit `f79b45763c7a53a3407fa740b5efa116aca7e434`. Follow-up run: https://github.com/vamprieieuu/vhs---90/actions/runs/44806070384 (check final conclusion before declaring the repository closed).
- The AE adapter change has not been compiled against the exact Adobe SDK; Windows/MSVC, PiPL, AEX loading, in-host rendering and MFR are not verified. This remains a global release gate.


## Source audit 50 — Composite Video Simulator (2026-11-20)

- Completed file-disposition audit of `vhs/github repos/composite-video-simulator-master/`: 36/36 files, including all 20 C++ translation units. See `docs/source-audits/50-composite-video-simulator-full-audit.md`.
- Main VHS/composite path reviewed in `ffmpeg_to_composite.cpp` and `ffmpeg_ntsc.cpp`: YUV composite carrier encode/decode, NTSC/PAL phase, cascaded chroma low-pass, VHS SP/LP/EP bandwidth, chroma delay/vertical blend, head switching, noise and separate luma/chroma playback sharpening.
- Reused existing VHS-90 composite/waveform, tape speed/transport, head/tracking region, noise, dropout, chroma subsample/vertical blend, luma edge and grade engines. Did not duplicate existing modules.
- Added independent `Chroma Edge Enhancement` control under new `VHS Playback` group. Native ImageGrade implementation uses three cascaded first-order I/Q low-pass stages and separate chroma unsharp gain; tape speed maps cutoff to SP 640 kHz, LP 600 kHz, EP 560 kHz. Luma Edge Enhancement remains separate.
- Added `CVS VHS SP`, `CVS VHS LP`, `CVS VHS EP` entries to the unified `VHS Style Preset` menu. These represent the source's three VHS speed modes, not original upstream named preset files. The unified popup now has 25 entries.
- Static validator confirms 153 AE parameter slots and all 25 styles; Linux CMake build and 32/32 core smoke tests passed in run https://github.com/vamprieieuu/vhs---90/actions/runs/44807065869.
- The source folder is GPL-2.0. No upstream source files were copied verbatim; nevertheless, the algorithmic adaptation's distribution/licensing implications must be reviewed before any proprietary/closed-source release.
- Raw 28.6 MHz decoder expects raw composite samples, Scanimate is CRT phosphor display, cassette tool is audio-only, and the remaining FFmpeg utilities are timing/palette/keying/gamma/conversion shells; all were classified and intentionally not inserted into the VHS render chain.
- Final-state CI passed: https://github.com/vamprieieuu/vhs---90/actions/runs/44807131889 (153 AE parameter slots validated, CMake build passed, 32/32 core smoke tests passed). Windows/MSVC AE SDK build, PiPL, AEX load and AE runtime/MFR remain unverified global gates.


## Source audit 51 — Lost Signal VHS v1.9 (2026-11-20)

- Closed the full `lost-signal-vhs-main` folder audit: 31/31 files, including all 10 historical release archives. v1.9 is the authoritative source. See `docs/source-audits/51-lost-signal-vhs-full-audit.md`.
- Parsed all 13 v1.9 profiles and 77 unique public settings; source release script validates 75 preset-owned options. MIT license recorded and attribution retained; no GLSL copied verbatim.
- Added 13 `LS` profiles to the unified VHS Style Preset list. Unified list now has 38 entries: 16 VHS6RIFT + 5 VHS Effect Add-on + 3 Composite Video Simulator + 13 Lost Signal + Custom.
- Added Composite Decoder selector (Consumer Notch / One-Line Comb / Two-Line Comb); implemented and tested five-tap Consumer Notch. Existing one-/two-line comb paths exposed.
- Added VHS 4:3 crop, final rounded overscan mask, radial lens distortion and two-axis digital zoom in the native geometry/composite path.
- Added three-segment OSD battery indicator and control; Camcorder 1996 preset enables it.
- Added deterministic frame-repeat frequency/strength using previous-frame data and frame/tape-seed hash; connected interlaced profile values to temporal field weaving.
- Reused existing composite/waveform, RF AGC, tape-surface persistent-defect, tracking/head, temporal, OSD and noise engines. Persistent tape-defect master now derives from wear/defect density/defect strength so clean-wear profiles can still retain defect memory.
- Static validator: 165 AE parameter slots, UI order matches ParamDiskID. Linux CMake build passed and 32/32 core smoke tests passed: https://github.com/vamprieieuu/vhs---90/actions/runs/44810036059.
- Depth-reprojected history and depth fog require Minecraft depth/camera data and cannot be reproduced exactly from a standard AE RGB input. Liminal fog is only approximated with available color/glow controls; exact focus-hunt/camera-era behavior remains outstanding. Windows/MSVC Adobe SDK/PiPL/AEX/AE runtime/MFR gates remain outstanding.
- Next folder in the user's specified eight-folder order: `ntscqt-1.5.0`. The first two listed folders are already closed in reports 49 and 50; this report closes the third.


## Source audit 52 — ntscQT 1.5.0 (2026-11-20)

- Expanded the earlier ntscQT report against the supplied `vhs.zip` folder: 28 archive entries (23 files + 5 directories); reviewed the 745-line YIQ/composite/VHS core, GUI/control model, render pipeline, dependency/build files, template data, license and bundled spectral ringing curve.
- Apache-2.0 license recorded. Python/PyQt/OpenCV/SciPy/FFmpeg GUI and media orchestration are host-specific and were not copied into the native AE plugin.
- Confirmed the signal behaviors are already represented by canonical VHS-90 engines: composite YIQ/waveform, IIR bandwidth, SP/LP/EP playback, vertical chroma blend, luma/chroma sharpening, noise, phase instability, arbitrary-region head/tracking, deterministic frame/seed processing. No duplicate ntscQT engine was added.
- Explicit remaining parity candidates: ntscQT's pre-modulation luma pre-emphasis signal-order stage and its 720-sample frequency-domain ringing response. The latter is not identical to VHS-90's spatial `VHS90RingingEngine`; an efficient validated transform is needed before porting. These were not falsely marked as integrated.
- Full report: `docs/source-audits/52-ntscqt-1.5.0-expanded-audit.md`. Audit index updated. This closes the fourth consecutive folder in the current source-audit run; verify the next exact source folder against the original eight-folder order before proceeding.
- No new core code or parameter slots were added in this audit; existing Linux core test results remain unchanged. Windows/MSVC AE SDK/PiPL/AEX and live AE/MFR validation remain global release gates.


## Eight-folder source queue — vhs-phys and Vaporwave (2026-11-20)

- `vhs-phys-main` was already fully inventoried and integrated in the earlier `docs/vhs-phys-main-integration.md` pass: 202 files; all Rust source module responsibilities and Rust test assertions indexed; Python GUI/lab/CLI/audio/packaging excluded from the native render path. Its unique SI-derived magnetic interference, head-core saturation/tape erasure, mains/ground-loop and RF pickup model is integrated as `VHS90PhysicalInterferenceEngine`, with six AE controls and dedicated C++ smoke test. The zero-field erasure tail was corrected to exact pass-through; Linux core CI runs 631/632 passed 29/29 tests. Source camera-tube acquisition remains a distinct follow-on requiring history/pre-roll; the full RF/audio record-playback pipeline was not transplanted wholesale.
- `vaporwave-master` audited as 11 files + one directory. Added independent YIQ Chroma I/Q Gain, I/Q Offset and Chroma Quantization to the canonical Image Grade engine and AE UI, with deterministic smoke assertions and validator wiring. No upstream source code/assets copied; supplied archive has no LICENSE file and no explicit README license. See `docs/source-audits/53-vaporwave-master-native-color-audit.md`.
- Latest workflow run `44816026051` passed the imported-preset/AE static validator, CMake build and all 32/32 CTest smoke tests; GitHub Actions was still completing post-job cleanup at last poll: https://github.com/vamprieieuu/vhs---90/actions/runs/44816026051.
- The next folder in the user's eight-folder sequence is `vhs.py-main`. Its archive contains a Python VHS image processor and browser UI; audit and reuse only native-compatible signal behavior, not the UI/runtime.
- Windows/MSVC AE SDK build, PiPL, AEX loading, AE rendering and MFR remain global release gates.


## Eight-folder queue — vhs.py-main closed (2026-11-20)

- Expanded the earlier vhs.py audit to all 9 files. Confirmed the portable luma/chroma Lanczos compression, independent deterministic noise streams, generation scaling and row-dependent chroma wave are already integrated in `VHS90PyVhsGenerationEngine` and exposed by six AE controls. The 260-line Python engine was reviewed; UI, OpenCV/Eel/Tkinter wrapper, silent MP4 output and browser dependencies are not copied.
- Confirmed the source's broken/dead DFT ringing helper is not a usable renderer stage; fixed right-edge green/black border is excluded; sharpening/blur behavior is covered by the existing Image Grade/CRT/luma-smear controls without claiming exact pixel parity. No duplicate engine added.
- Report: `docs/source-audits/54-vhs-py-main-expanded-audit.md`. CI run `44816026051` passed validator, CMake build and 32/32 CTest tests.
- The next and final folder in the user's eight-folder sequence is `vhs-main`. Inspect the exact `vhs-main` directory separately from `lost-signal-vhs-main`, which was already closed in report 51.


## Eight-folder source audit — final folder vhs-main (2026-11-20)

- Completed expanded inventory of `vhs-main`: 28 files + 5 directories, AGPL-3.0-or-later. No upstream OFX/Metal/CUDA/kernel/OSD/LUT/preset-parser code was copied; the supplied `VHS Faded Tape.cube` is an identity placeholder. Existing CPU-first VHS-90 modules already cover the majority of signal/tape processing.
- Corrected OSD order: OSD is now rasterized into the source immediately before composite signal processing, so it acquires composite/bandwidth/static/tracking damage. Static validator and CMake/CTest 32/32 passed in run `44816473391`.
- Added independent RGB channel registration to `VHS90ChromaShiftEngine`: red and blue signed pixel offsets with green as reference, bilinear sampling and premultiplied-alpha preservation; added AE controls (-20..+20 px), Master Strength scaling and smoke coverage. Latest run `44816747950` completed successfully: static AE/preset validation, CMake build and CTest 32/32 passed: https://github.com/vamprieieuu/vhs---90/actions/runs/44816747950.
- Full report: `docs/source-audits/55-vhs-main-expanded-final-audit.md`. The eight-folder queue is now audited or verified against earlier completed integration reports. The latest CI is green; remaining work is global release verification: Windows/MSVC AE SDK build, PiPL, AEX load, AE visual testing and MFR.


## Final status — supplied eight-folder VHS source queue (2026-11-20)

1. `VHS Effect Add-on` — closed; report 49.
2. `Composite Video Simulator` — closed; report 50.
3. `Lost Signal VHS v1.9` — closed; report 51.
4. `ntscqt-1.5.0` — expanded audit 52; source-specific pre-emphasis and spectral ringing parity remain documented follow-ups, not falsely marked as ports.
5. `vhs-phys-main` — earlier full 202-file integration closed in `docs/vhs-phys-main-integration.md` and referenced by audit index.
6. `vaporwave-master` — audit 53; five independent YIQ color controls integrated.
7. `vhs.py-main` — expanded audit 54; existing deterministic PyVHS engine confirmed complete for portable core behavior.
8. `vhs-main` — expanded audit 55; OSD moved before composite and independent RGB channel registration integrated.

**Source-queue state: all eight folders audited or verified against completed integrations. Latest native Linux CI: PASS (32/32 CTest tests). No Windows AE SDK/PiPL/AEX/After Effects runtime or MFR result is claimed.**


## Tracking variants and VHS Damage Lab continuation (2026-11-20)

- Re-opened the actual vhs.zip from the Library (2,919 entries) and reconfirmed VHS Damage Lab and VHS Cameras archive paths. Extracted and visually inspected all six Damage Lab still references. Existing Damage Lab audits 27 and 28 already contain the 20 text specs and 19 media-reference inventory/analysis; reports 26 and 29 contain the first-pass VHS Cameras FXC6/Gen 7 analysis.
- Added five genuinely different Tracking Noise renderer branches to VHS90RegionEngine: ServoSweep (VHS6RIFT active C++ line jitter/moving-band behavior), RfWave (NTSC-R tracking wave/anisotropic snow parameter model), CompositeRibbon (Crates multi-scale tracking bands), TapeFlutter (VHSPro source-inspired model; exact kernel not recoverable from decompilation), FourEdgeCrawl (Damage Lab arbitrary-edge geometry). UI exposes a neutral-name Tracking Profile popup, read by both SmartFX and legacy paths.
- The core region API supports multiple independent regions; the current AE UI still exposes one tracking region plus Head Switching. A true multi-layer AE stack allowing multiple different tracking renderers simultaneously remains pending and is explicitly documented.
- Initial profile distinctness smoke test exposed an invalid uniform-color fixture; replaced it with a spatial RGB gradient so displacement differences can be measured. Latest CI run `44818220252` passed imported AE parameter validation, CMake build and 32/32 CTest smoke tests: https://github.com/vamprieieuu/vhs---90/actions/runs/44818220252.
- Full checkpoint: `docs/source-audits/56-tracking-variants-and-damage-lab-checkpoint.md`. Remaining Damage Lab work: multi-layer Tracking UI; Particle Pattern Engine; dedicated Tear & Chroma hybrid; remaining multi-phase/sync-collapse/angular/diagonal procedural patterns. Only after those gates should the VHS Cameras folder be closed with deeper FXC6/Gen 7 analysis.


## VHS Damage Lab integration closed; next VHS Cameras (2026-11-20)

- Tracking Noise now has five distinct native renderer types (Servo Sweep, RF Wave, Composite Ribbon, Tape Flutter, Four-Edge Crawl), eight editable recipes, and four independently configurable simultaneous Tracking regions in AE. Each layer has its own type, preset, geometry, amount, servo, displacement, wave and tear controls. Head Switching remains separate.
- Added procedural Tear & Chroma Process with five modes and up to five independently adjustable bands (base plus Bands 2–5). Spectral smear and tear bands have independent activation behavior; the combined mode intentionally runs both.
- Added Particle Pattern Engine with five modes and separate density, particle size, streak length, distribution, chroma, speed, seed, source mix, brightness and contrast controls.
- Extended Static/Procedural Pattern Engine from seven to eleven modes, including Dark-Field Luma Streaks, Multi-Phase Green Line, Two-Stage Angular Ribbon and Diagonal Sync Collapse. AE exposes the mode, angle, ribbon width, green-line intensity and phase count.
- All modules are procedural C++ and are integrated into the AE render path. Source references are not used as overlays. Detailed report: `docs/source-audits/57-vhs-damage-lab-engines-and-camera-handoff.md`; tracking source evidence and architecture: `docs/source-audits/56-tracking-variants-and-damage-lab-checkpoint.md`.
- Latest Linux CI run `44819741465` passed imported AE parameter validation, CMake build and all 34/34 CTest tests: https://github.com/vamprieieuu/vhs---90/actions/runs/44819741465.
- The Damage Lab implementation checkpoint is closed for the next folder transition. Next: reopen and deeply analyze all six files in `vhs/Vhs cams/` (FXC6 and Gen 7 clips + notes), building on first-pass reports 26 and 29. Windows/MSVC AE SDK/PiPL/AEX loading and After Effects visual/MFR testing remain release gates.


## VHS Cameras — FXC6 and Gen 7 deep analysis (2026-11-20)

- Re-opened all six files in vhs/Vhs cams/: two FXC6 split-screen HEVC clips and note; Gen 7 clean source clip, processed clip and note. All four videos were re-probed and sampled across their timelines.
- FXC6: the processed side is a cool blue/cyan-to-violet, grainy, softened direct-capture look; it does not show the severe tracking/tear/static behavior of Gen 7. The split-screen halves are not aligned source/output frames, so pixel-parity or direct edge-detail claims are not valid. See report 58.
- Gen 7: before is 828x1086/30fps/45 frames/no audio; after is 828x1078/299 frames/~33.43fps/8.96s with AAC stereo 44.1kHz. The processed clip cycles between teal/yellow colorized frames, dark wavy monochrome static and cleaner recovery states. It is not a simple monotonic amount ramp. After-only audio RMS is ~0.0706 with energy concentrated in low/mid frequencies; original has no audio, so no before/after muffling curve can be measured.
- Added two Camera Profile recipes: FXC6 Direct Capture (restrained cool cast, fine grain, softened detail, no severe tracking bars) and Gen 7 Reference (stronger teal/yellow response, chroma instability, correlated noise and moderate static/tracking/tear). These are source-inspired profiles; visual calibration in AE remains required. UI menu now has Off plus 11 reference profiles.
- Report: docs/source-audits/58-fxc6-gen7-camera-reference-deep-analysis.md. Earlier report 26 was amended with measurement/alignment caveats.
- Camera-profile code build/test passed in run 44820324482. The latest validator/menu-count follow-up run 44820427161 passed: 286 AE parameter slots validated, CMake build passed and all 34/34 CTest tests passed: https://github.com/vamprieieuu/vhs---90/actions/runs/44820427161.
- Remaining camera-folder work: controlled native AEX before/after renders and tuning against the supplied footage; separate audio engine required if the Gen 7 note's noise-rise/muffling behavior is to be implemented.


## Current checkpoint — Damage Lab and supplied camera folder analyzed (2026-11-20)

- Damage Lab engines: five Tracking types, eight editable tracking recipes, four simultaneous independently controlled Tracking layers; separate Head Switching; Tear & Chroma five modes with five independently configurable bands; Particle Pattern five modes; Procedural Static eleven modes. Core and AE parameter/render wiring documented in report 57.
- Camera folder: all six files re-opened, four clips re-probed and sampled, both Arabic notes analyzed. FXC6 and Gen 7 have separate reference reports and separate Camera Profile recipes. Fxc6 split-screen is not aligned for pixel-difference metrics; Gen 7 after clip cycles between colorized, dark static and recovery phases. After-only audio exists; before clip has no audio, so no before/after muffling measurement is possible. See report 58 and addendum to report 26.
- Latest CI run `44820427161`: validator PASS (286 AE parameter slots; UI order matches all disk IDs), CMake PASS, CTest PASS (34/34): https://github.com/vamprieieuu/vhs---90/actions/runs/44820427161.
- Remaining work is controlled native AEX visual calibration against the supplied references, Windows/MSVC Adobe SDK/PiPL/AEX verification, MFR, and a separate audio-processing design if the Gen 7 audio requirement is retained. The folder-analysis queue is complete.


## Preset migration and pre-AEX gap audit (2026-11-21)

- Reopened the supplied `vhs.zip` from the Library (2,919 entries) and verified the seven supplied native AEX plugin binaries. No file/plugin named `Quicks Master` or `Quick Master` is present; `CratesVHSMaster.aex` is present and separately audited. Do not silently equate the names.
- Expanded `VHS Style Preset` from 38 to 72 popup entries: 71 named recipes plus Custom. Current source families represented: 16 VHS6RIFT, 5 VHS Effect Add-on, 3 Composite Video Simulator speed modes, 13 Lost Signal VHS profiles, 16 Crates VHS Master, 10 VHSPro and 8 VHSCraft.
- Added 16 Crates recipes using 44 selected source values recovered from the embedded JSON in the supplied AEX binary (16×44 native mapping). The source contains 66 controls; 22 fields have no direct native equivalent and are listed in audit 59. The mapper is a semantic adaptation, not pixel-identical parity.
- Added ten renamed VHSPro and eight renamed VHSCraft source-inspired recipes. Their menu names are recovered, but full source parameter tuples are not reliably available from the supplied decompilation; visual calibration and exact value reconstruction remain open. VHSCraft's Classic composite and Cinematic RGB render paths remain separate source-port gaps.
- Renamed source-family labels in the VHS Style menu to VHS-90-native labels. The camera profile and NTSC-RS look menus remain separate from the 72-entry VHS Style list.
- Inspected all 15 supplied font binaries. Ten have Arabic codepoints and OpenType joining features; all ten support Arabic-Indic digits; six also support Persian digits. Font licensing metadata reveals explicit redistribution restrictions on several files, so a font inventory is complete but packaging all binaries into a distributable AEX is not authorized by the metadata alone.
- Reopened `VHS90OsdEngine.cpp`: it still uses a fixed 5×7 bitmap glyph table for Latin letters/digits and fixed REC/PLAY/SP/time/battery elements. Supplied font loading, arbitrary text, Arabic shaping/RTL/diacritics, Arabic month/date localization and manufacturer-camera font matching are not implemented.
- Added full gap checkpoint `docs/source-audits/59-preset-font-and-pre-aex-gap-audit.md`, including the seven source AEX inventory, preset migration counts, font/licensing status, and outstanding technical/host release gates.
- The 45 NTSCRT presets are semantically audited but not all ported: analog timeline/keyframe migration remains; digital-only and Screen Loop presets remain intentionally outside the analog VHS engine. NTSC-QT pre-emphasis/spectral ringing, NTSCRT full RF/FM/color-under playback and 12-field history, Signal render reconstruction, Universe procedural transition parity, camera visual calibration and optional Gen 7 audio remain open.
- The first 72-entry migration commit passed Linux CI run `44821704478`; Crates source-value mapping and menu-label renaming were committed afterward and require a fresh CI run. No Windows/MSVC Adobe SDK build, PiPL compile, AEX artifact, AE 23.2.1 load/render or MFR success is claimed.
- **Pre-AEX status:** not ready. Close Text/font rendering, remaining required source/preset parity, and visual acceptance first; then run Windows/MSVC + exact Adobe SDK + PiPL + AEX + AE/MFR gates.


## Font manifest and recovered VHS Tape OSD detail (2026-11-21)

- Added `Resources/Fonts/manifest.json` with all 15 supplied font filenames, original internal family names, glyph counts, Arabic/Latin digit coverage, OpenType shaping features, embedding flags and explicit package/licensing status. The manifest does not copy restricted font binaries into the distributable plugin.
- Re-inspected the actual `VHSTape.aex` binary. Recovered the full deck-mode state choices (Play/Pause/Stop/Fast Forward/Rewind), independent OSD corner positions, SP/LP/EP, counter offset, Text Size, date/time modes, four date formats, full/abbreviated month names, Clock Runs, Random Seed and Mix. VHS-90's current OSD is still a smaller subset and does not yet match the full VHS Tape control/layout behavior. See audit 59.
- Latest code validator + CMake + 34/34 CTest run after the Crates 16×44 source-value table and popup-label changes: `44822675580` PASS. This is Linux core/static validation only; it does not compile the AE adapter or create/load an AEX.
- Remaining pre-AEX work remains unchanged: full Arabic/RTL font-backed Text/OSD, full VHS Tape OSD parity, VHSPro/VHSCraft exactness/visual calibration, NTSCRT selected analog preset/keyframe migration and remaining RF/FM/12-field parity, NTSC-QT pre-emphasis/spectral ringing, other source-specific parity, Windows/MSVC Adobe SDK/PiPL/AEX and AE/MFR verification.


## Continuation checkpoint — confirmed Crates VHS Master and source archive re-verification (2026-11-21)

- User confirmed that “Create VHS Master” means `CratesVHSMaster.aex`. Treat its recovered 16 source presets as the Crates family already represented in the 72-choice VHS Style selector; do not create a duplicate family under a mistaken product name.
- Reopened the supplied `vhs.zip` working copy and verified the NTSCRT and NTSCQT source trees directly. A fresh recursive count finds 39 JSON files beneath the NTSCRT 0.26.0 `presets/` directory in this exact ZIP snapshot (34 top-level look/timeline files plus five Screen Loop files). Existing audit 47 is titled 45/45 and contains six additional named entries not found in this directory snapshot. This count/provenance discrepancy is recorded in audit 60 and must be reconciled before claiming all archive presets are accounted for.
- NTSCQT source includes `app/ntsc.py`, `Renderer.py`, `ringPattern.npy`, `builtin_templates.json`, and UI/renderer files. Current VHS90RingingEngine is a spatial edge/echo implementation and is not the NTSCQT frequency-domain ringing response. Exact port and source-derived fixture comparisons remain open.
- Added `docs/source-audits/60-continuation-source-verification-and-release-gates.md` with current preset, OSD, engine and release blockers. This is a truthful checkpoint, not a completion declaration.
- Latest verified CI runs for the prior code/documentation state: `44822831371` and `44822815000` both completed successfully. These are Linux core/static checks only. Windows/MSVC, exact Adobe SDK/PiPL, AEX loading, AE 23.2.1 visual renders and MFR remain untested.
- **Pre-AEX remains blocked.** The four user-approved closure groups remain open: complete applicable preset migration; font-backed Arabic/Latin Text/OSD with VHS Tape parity; NTSCQT/NTSCRT and other required source-engine parity; and source-reference visual regression testing.


## NTSC-QT exact source re-verification — 2026-11-21

- Re-extracted the archived `ntscQT-1.5.0/app/ntsc.py` from the supplied `vhs.zip` and re-read `ringing`, `ringing2`, `composite_preemphasis` and the field-level call site. Confirmed both ringing variants operate on alternating Y, I and Q field planes; `ringing2` applies the 720-sample `ringPattern.npy` response across every vertical-frequency row after linear resize/center-crop and power shaping. The other variant applies a rectangular horizontal DFT mask with optional seeded frequency noise.
- Confirmed composite pre-emphasis is a separate Y-only high-pass-derived stage before composite modulation, not post-decode sharpening. Existing `VHS90RingingEngine` remains a spatial edge/echo effect and is not source parity.
- Added exact implementation/verification requirements in `docs/source-audits/61-ntscqt-spectral-ringing-preemphasis-exact-port-spec.md` and registered it in the source-audit index. This is a source-exact port specification only; no FFT engine, pre-emphasis code, source fixtures or UI wiring is claimed complete.
- Linux core CI runs `44825813088` and `44825794594` both completed successfully for the prior documentation-only state. A new CI run is required after subsequent code changes; Linux core CI does not validate Windows/MSVC, PiPL, AEX loading or AE/MFR behavior.


## OSD / supplied font implementation checkpoint — 2026-11-21

- Re-opened `VHS90OsdEngine.cpp`, its smoke test, and the AE adapter. Confirmed OSD is inserted into the working frame before the VHS composite/signal stages (correct behavior to preserve), but the renderer is still a fixed 5×7 ASCII bitmap table and only emits REC/PLAY/SP/date-time/battery primitives. Unicode text, Arabic shaping, font selection and full VHSTape controls/layout are absent.
- Rechecked the supplied font manifest and actual embedded license name records. `Rooyin_Free_Bold.ttf` and `Sponge_Arabic_Regular.otf` are the only supplied fonts currently cleared by the manifest for bundling under OFL terms, with complete files/notices retained. The repository currently contains only the manifest, not those font binaries or license bundle. Other fonts remain permission-restricted or unverified and must not be redistributed without permission.
- Added `docs/source-audits/62-osd-directwrite-arabic-vhs-tape-implementation-spec.md`. It defines a Windows DirectWrite renderer isolated from the portable core, UTF-8/bidi/Arabic shaping, localized months and digit systems, arbitrary user text, font enumeration/fallback, VHS Tape states/positions/date/counter controls, and regression tests.
- At the time of this earlier checkpoint, no OSD rendering code or AE UI parameters had changed. The subsequent VHS Tape OSD implementation is recorded in source audit 63 below; font packaging and AE-host validation remain open.


## VHS Tape OSD implementation increment — 2026-11-21

- Added bilingual English/Arabic VHS OSD text formatting, localized date/time and digit sets, independent 12/24-hour clock, Clock Runs, month-name style, and four date layouts (`JAN.21.1994`, `1.21.1994`, `21.1.1994`, `1994.1.21`).
- Added requested transport/status words and Arabic labels, independent per-word visibility, master Show Text, Show Date, Show Counter, text glow, shared font selection, extra-label positioning, and second-based counter offset. DirectWrite now replaces bitmap text for REC/deck/speed/date/time/counter while retaining bitmap icons and the established pre-composite insertion point.
- Added and wired the localization core/test, AE parameter controls, SmartFX checkout and Legacy reads. Static parameter map now has 336 active slots, 336 unique disk IDs and 336 registered IDs in matching order. The validator was corrected to include the persistent ID in `PF_ADD_ARBITRARY2`.
- Linux core CI passed after the battery safe-margin fix (`44834430499`); Windows + Linux OSD Core/DirectWrite smoke passed (`44834430462`). Latest code/test commit `5a717087339d01988c541793cba5217738cab2c0` passed Linux Core CI `44834780888` (36/36 CTest) and Windows + Linux OSD Core/DirectWrite CI `44834781058` (37/37 CTest, including Windows DirectWrite smoke). This still does not compile the AE host adapter or create/load an AEX.
- The font selector now lists all 15 supplied families using the actual DirectWrite family names. Font binaries are not yet embedded: the public repo manifest clears only Rooyin Free Bold and YR Sponge Arabic for OFL redistribution; other supplied files are restricted or lack verifiable redistribution rights. Bundle eligible fonts with their complete notices and implement a private DirectWrite font collection before claiming bundled-font completion.
- VHS6RIFT's actual SDK tree, `PiPLtool.exe`, `Param_Utils.h`, CMake/PiPL generation and MSVC build scripts were extracted from the user's Library archive. They provide the build reference, but this runtime has no MSVC/Windows SDK toolchain; no VHS-90 AEX build or AE 23.2.1 host test is claimed.
- See `docs/source-audits/63-vhs-osd-localization-and-words-integration.md`. Overall VHS-90 remains pre-AEX and not test-ready.


## VHS6RIFT build-reference verification — 2026-11-21

- Extracted the user's Library archive `vhs.zip` and the complete `VHS6RIFT/extern/AfterEffectsSDK` tree, including `Headers/AE_Effect.h`, `Util/Param_Utils.h` and `Resources/PiPLtool.exe`.
- Inspected the reference `CMakeLists.txt`, `build.ps1`, PiPL resource and SDK discovery/PiPL generation scripts. VHS-90's Windows-only AEX target already uses the same SDK-layout discovery and PiPL preprocessing pattern, while keeping the VHS-90 engine and its own PiPL/flags.
- Ran VHS6RIFT's reference validation locally: architecture validator PASS (83 parameter slots, nine balanced topic groups, PiPL/outflags check); parameter wiring audit PASS (64/64); functional suite PASS (14/14); SDK discovery suite PASS (6/6). This validates the source/build reference only, not a VHS-90 binary.
- Current runtime has no MSVC/Windows SDK toolchain, so no local VHS-90 PiPL/AEX build is possible. Continue with the Windows/MSVC build gate before claiming a test-ready plugin.


- Latest static/core validation after the text jitter/font-family updates: VHS-90 Core CI run `44835315872` passed on commit `c07234be26fe28f8ce17a4be07fd50e31aa04fa7`. The matching Windows + Linux OSD Core/DirectWrite workflow `44835316039` completed successfully (37/37 tests, including Windows `VHS90DirectWriteSmoke`). Neither workflow compiles the full AE effect adapter or creates an AEX.


- Latest code commit `a26540d6f3` passed VHS-90 Core CI `44835822873` and Windows + Linux OSD Core/DirectWrite CI `44835822927` (37/37 tests, including DirectWrite and OSD localization smoke). Font-weight-specific Windows DirectWrite validation passed in `44835660219`. The Core CI also passed the static AE parameter/ID audit. These workflows do not compile `VHS90Effect.cpp` into an AEX or validate AE-host behavior.
