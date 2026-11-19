# VHS-90 — Project Status

Updated: 2026-11-19

## Current state

- Plugin name: VHS-90.
- The current repository contains source-audit documents and host-independent C++ signal/region kernels.
- A root CMake project now builds the host-independent CPU core and two smoke tests. No complete Adobe After Effects plugin target exists yet: no build.ps1, no PiPL resource, no AE EffectMain implementation and no AEX build workflow.
- Therefore there is no test-ready AEX artifact and no claim of AE load/render success.

## Work completed in this pass

- Inventoried all 15 bundled GitHub source repositories and wrote scoped source audits for their relevant VHS/NTSC engines.
- Audited the seven supplied AEX references and their available Ghidra decompilation/metadata.
- Audited the VHS6RIFT native base, its AE SDK declarations, build scripts, PiPL, tests and documentation drift.
- Audited all 20 VHS Damage Lab prompt documents and all 19 associated visual references.
- Audited the supplied font pack metadata and Arabic glyph/shaping coverage.
- Audited the VHS camera before/after references and the available processed audio stream.
- Added host-independent C++ signal kernels, a generalized multi-region Tracking Noise / Head Switching engine, and a procedural static-pattern engine.

## Important VHS6RIFT findings

- VHS6RIFT provides useful YIQ/composite, deterministic-noise and AE build architecture references, but its current source has conflicting exported function declarations, invalid PF_InData time member accesses and PiPL/global flag mismatches.
- Its fallback PF_* structs do not match the bundled SDK ABI.
- Its three Python scripts pass, but they are static/model checks; the native render harness is not invoked by the build workflow and fails to compile against the fallback declarations.
- Several documentation files describe later source revisions (112/135/192 parameter slots, NTSC-RS bridge, overlays and CRT modules) that are not present in the supplied 83-slot source snapshot.

## C++ tests run

- CMake configure/build: PASS for the host-independent VHS90Core static library and all three smoke-test executables.
- CTest: 3/3 PASS.
- VHS90SignalKernelsSmoke, VHS90RegionEngineSmoke and VHS90StaticEngineSmoke: PASS under g++ C++17 with ASan/UBSan.
- VHS6RIFT static validator, parameter occurrence audit and Python 14-gate model tests: PASS.
- VHS6RIFT native harness compile attempt: FAIL, with SDK fallback/ABI/API declaration errors.
- No Windows/MSVC build or After Effects runtime test has been performed.

## Next engineering gates

1. Finish the deeper source and reference audits, keeping each source's algorithms and limitations separate.
2. Build a clean VHS-90 native AE project using exact SDK declarations and verified PiPL flags; do not copy VHS6RIFT's ABI mistakes.
3. Integrate the signal, region and static kernels into a shared normalized pixel pipeline.
4. Add 8/16/32-bit adapters, SmartFX checkout, alpha/premultiplication policy and MFR-safe immutable frame context.
5. Port the highest-value algorithms from Analog NTSC, ntsc-rs, RetroTape, NTSCRT, vhs-phys and Lost Signal in isolated modules.
6. Build with Windows/MSVC 2022, then validate load/apply/render inside AE 23.2.1.
7. Compare actual AE renders against the supplied reference clips/stills before marking visual matching complete.