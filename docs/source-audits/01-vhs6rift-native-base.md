# VHS-90 — VHS6RIFT Native Base Audit

Date: 2026-11-19
Input: `vhs/open source native plugin for ae/VHS6RIFT/` from the supplied `vhs.zip`
Scope inspected in this pass: project manifest, native header/source, CMake, PowerShell build entry, PiPL resource, and the three Python validation/test scripts. This is a focused core/build audit, not a claim that every SDK example or bundled asset has been reviewed.

## Inventory and role

- Archive subtree contains 1,084 files including the checked-in `.git` metadata.
- Core native implementation: `include/VHS6RIFT.h` (452 lines), `src/VHS6RIFT.cpp` (1,098 lines).
- Build and identity: `CMakeLists.txt` (142 lines), `build.ps1` (62 lines), `resources/VHS6RIFTPiPL.r`.
- Validation: `tools/validate_project.py`, `tools/audit_parameter_wiring.py`, `tools/test_vhs6rift.py`.
- The source archive includes a substantial After Effects SDK subtree and a small optional ntscrs C header. CMake explicitly says the current native implementation does not call that bridge.

## Useful architecture to retain in VHS-90

1. Native AE effect entry point and PiPL-based `.aex` packaging.
2. Central `RenderContext` populated once per render, then passed to pixel kernels.
3. Deterministic hash functions keyed by seed, frame/time, and coordinates.
4. YIQ-domain composite processing, chroma/luma bandwidth separation, chroma displacement, ringing, line jitter, dropouts, noise and ghosting.
5. 8-bit, 16-bit and float pixel-path separation, with rowbytes-aware row traversal.
6. Master-strength exact-copy bypass in the 8/16/32-bit paths.
7. Preset/profile parameterization layered over common processing.
8. A CMake + MSVC x64 + PiPLtool build structure as a starting reference, subject to the ABI and source corrections below.

## Blocking correctness/build findings

### 1. EffectMain declaration/definition mismatch

The header declares `EffectMain` with six parameters, including the SDK's final `void *extra`; the source definition has only five parameters. This is a compile-time conflicting declaration in a normal C++ build. The source also needs to be checked against the exact SDK ABI rather than relying on static scripts.

### 2. The local fallback SDK declarations are not ABI-compatible with the real SDK

The audio callback's `PF_SoundWorld` members (`fi`, `dataP`) and audio checkout/get-data macros do exist in the supplied Adobe SDK, so the callback is not inherently from a different SDK revision. However, the fallback `PF_SoundWorld` declaration in the plugin header uses a different layout (`data[2]`, `rate`, `num_channels`) and must never be used to build a production plugin. More importantly, the fallback `PF_InData` models `current_time` as a struct with its own `scale`, while the actual SDK uses an integer `current_time` and a separate `time_scale`. The source currently accesses `in_data->current_time.scale`, which is incompatible with the actual SDK declaration and is a compile-time error in the intended build. Remove the fake compatibility layer from the production target and compile strictly against the exact SDK headers.

### 3. PiPL flags do not match the actual SDK flags returned by GlobalSetup

Against the supplied SDK's actual enum values, `GlobalSetup` requests WIDE_TIME_INPUT (bit 1), DEEP_COLOR_AWARE (bit 25), AUDIO_FLOAT_ONLY (bit 27), and AUDIO_EFFECT_TOO (bit 30), which combine to `0x4A000002`. The PiPL resource instead declares global outflags `0x48000046`. Its outflags2 value is `0x04000000`, while the supplied SDK defines SUPPORTS_THREADED_RENDERING as bit 27 (`0x08000000`). The existing validator only checks that one hard-coded value appears and does not compare actual SDK constants with the C++ setup. Rebuild PiPL values from the exact SDK definitions and add a test that computes and compares both sets.

### 4. The 16-bit and float renderers are not equivalent to the 8-bit renderer

The 8-bit path implements most of the advertised damage stack. The 16-bit path only performs a small subset (basic jitter, luma softness, saturation); the float path performs an even smaller subset. Both omit many enabled features and use different scaling/alpha-source choices. A single shared normalized processing kernel is needed so bit depths produce consistent looks.

### 5. Head switching is hard-coded to the bottom edge

The current context exposes only `bottomDamageAmount` and `bottomDamageHeight`; rendering computes the boundary as `height - bottomDamageHeight`. This directly conflicts with VHS-90's required arbitrary vertical position/region and adjustable height, including the ability to expand to the entire image. Replace with normalized region geometry (center/top/height or start/end) and a shared region evaluator.

### 6. Tracking noise / damage bands are not a general region system

Moving bands are vertical scanline bands with a time-derived Y coordinate. The model is not a general placement system supporting independent regions anywhere in the frame, left/right placement, custom dimensions and direction. VHS-90 needs a reusable region model with orientation, normalized bounds, motion vector, seed, intensity and behavior.

### 7. Continuous/event master toggles are applied before their values are populated

At the start of `PopulateContext`, the code conditionally clears fields such as `ctx.lumaSoftness`, `ctx.chromaBleed`, `ctx.movingBandAmount`, etc. Those fields are populated later in the same function, overwriting the clears. Therefore these toggles do not reliably disable their corresponding modules. Apply gating only after all parameter values are loaded and preset/profile merging is complete.

### 8. Preset and camera enum comments/counts need reconciliation

The header labels the style list as 17 choices including Custom, while comments and reports variously call it 16 presets. The UI popup starts at `STYLE_CUSTOM=1` and the camera enum uses `CAM_OFF=1`; verify popup indexing against AE's 1-based choice values and test each choice in-host. Do not trust enum presence as proof of correct mapping.

### 9. Several parameter fields require functional verification

The static reference-count audit does not establish that a parameter affects pixels. In the 8-bit renderer, fields such as `trackingFreq`, `noiseDetail`, `ghostingDecay`, and `signalTrails` appear not to drive the corresponding effect math. The audio downsample rate is used, but the audio path itself is API-inconsistent. Build a semantic parameter-to-kernel audit, not a token-occurrence counter.

### 10. Alpha and premultiplication require a deliberate policy

The 8-bit path copies alpha from the unshifted original pixel while RGB can be sampled from displaced coordinates. The 16/32-bit paths copy alpha from the displaced sample. The paths do not consistently define straight vs premultiplied processing. VHS-90 must explicitly unpremultiply/process/repremultiply where needed, preserve alpha, and test transparent edges.

### 11. Static validation scripts overstate their coverage

The three Python checks passed locally (validator, parameter reference counter, and 14-gate functional script). However, they primarily inspect source strings or reimplement simplified equations; they do not compile the plugin, invoke the actual render functions, or load it in After Effects. In particular, the reference counter reports a parameter as wired merely because its token occurs twice. Treat the 14/14 result as script-level checks only, not evidence of a successful native build or runtime.

## Local checks run

- `python tools/validate_project.py`: PASS (83 enum slots, 9 topic pairs).
- `python tools/audit_parameter_wiring.py`: PASS (64 active controls by occurrence count).
- `python tools/test_vhs6rift.py`: PASS (14/14 script-level gates).
- No MSVC build or After Effects runtime test was run in this environment.

## VHS-90 adaptation decision

Use VHS6RIFT as an architectural reference and selective source base, not as an unmodified production-ready plugin. Preserve its useful signal math and deterministic CPU-first pipeline ideas. Refactor the render core into a shared normalized pixel engine; implement a general region system for Tracking Noise and Head Switching; fix gating, parameter semantics, alpha handling, ABI, and the audio decision before treating it as a build base. Keep VHS-90's UI identity and preset names independent of the source project.
