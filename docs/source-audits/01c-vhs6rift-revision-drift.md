# VHS-90 — VHS6RIFT Repository Revision / Documentation Drift Audit

Date: 2026-11-19
Input: all non-SDK root files, docs, workbench, assets manifests and CI workflow in supplied VHS6RIFT subtree.

## Source of truth in the supplied archive

The actual native implementation present in this archive is the 83-slot parameter version: one header, one C++ source, one PiPL resource and the original CMake/build scripts. Its CMake comment explicitly says the current native source does not call the optional ntscrs_c bridge.

The README and CONTENTS file describe this 83-slot implementation. Several later documents describe materially different project revisions and cannot be treated as evidence for the source currently present.

## Conflicting parameter counts / feature states

The documentation contains incompatible counts and implementation states:
- README / CONTENTS / current header: 83 parameter slots, 9 topic groups, 16 named style presets plus Custom, 10 camera profiles.
- WORK_COMPLETION_REPORT: 112 registered parameters.
- FEATURE_VALIDATION: 135 registered parameters.
- FINAL_PARAMETER_INDEPENDENCE_AUDIT: 192 registered parameters.
- NTSC_BASE_PIPELINE_FIX: 192 registered parameters and an NTSC-RS base pass.
- Other feature documents claim 19 camera/look profiles, 21 embedded overlays, 15 Glitch Cam families, CRT, source-layer overlay, and a Rust ntsc-rs bridge.

Those later features are not present in the supplied current header/source. The only source file under src/ is VHS6RIFT.cpp; there is no BundledOverlayFrames.h, no Rust crate workspace, no ntscrs C ABI implementation, and no 19-profile/21-overlay parameter model in the source revision being audited.

## Workbench app.py

app.py is a small Python HTTP server that serves a large HTML/JavaScript workbench. The canvas processing and interactive controls run in browser JavaScript; they do not call or compile the C++ render implementation. Its “Live C++ Algorithm Emulation Monitor” label is therefore not proof of C++ output.

The status endpoint checks for an AEX file, but when one is absent it still returns a hard-coded fallback file size and SHA-256 string. The download route returns 404 if the source archive is not present. In this supplied archive there is no artifacts directory and no AEX deliverable. Do not use the workbench's placeholder size/hash as build evidence.

## Assets directory

The assets folder contains a README and two overlay manifest JSON files, but the referenced overlay subfolders and bundled overlay frame header are not present in this archive. The manifest says 21 embedded overlays are in src/BundledOverlayFrames.h, but that source header is absent. The source build does not implement the later overlay library described in these docs.

## CI workflow

The checked-in GitHub Actions workflow runs the three Python static/model tests and then attempts a Windows x64 CMake build. It does not invoke build.ps1, does not compile tools/test_native_render.cpp, and does not run the C++ native harness. The workflow's successful static job would not detect the ABI mismatches documented in 01b.

The current CMake does default AE_SDK_ROOT to extern/AfterEffectsSDK, and this archive includes the SDK headers and PiPLtool. However, the native source still has the conflicting exported declarations, time-structure accesses and PiPL flag mismatch described in the ABI addendum.

## SDK snapshot

The archive includes 984 files under extern/AfterEffectsSDK, including AE_Effect.h, AE_PluginData.h, Param_Utils.h, PiPLtool.exe, SDK examples and utilities. The exact headers needed to identify the ABI were present. The source's CMake discovery supports the direct Headers/Resources layout used here.

The SDK snapshot was inspected for the specific PF_InData, PF_SoundWorld, EffectMain, PluginDataEntryFunction2, outflags and PiPL contracts used in the ABI report. The 984-file SDK subtree was inventoried, but every unrelated SDK example project was not individually reverse-engineered; those examples are reference materials rather than VHS6RIFT implementation source.

## Integration decision for VHS-90

Use the native VHS6RIFT C++ signal code selectively, but do not import its later documentation claims as existing features. Rebuild the VHS-90 host project cleanly around the exact AE SDK ABI, and maintain a single source-of-truth feature matrix generated from actual registered parameters and render consumers. Keep the reference overlay library out of VHS-90's procedural engine unless the user explicitly asks for overlays.

## Verification

- The 43 documentation files were inventoried; core build, limitation, feature matrix, implementation, reference, camera, parameter and completion documents were sampled and compared against the actual source tree.
- The three current static/model Python scripts pass, but they are not native C++ tests.
- app.py, assets manifests, CI workflow and SDK tree were inspected.
- No MSVC build or AE runtime render was performed.