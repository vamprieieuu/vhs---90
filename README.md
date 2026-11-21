# VHS-90

Native C++ Adobe After Effects VHS / analog signal processing plugin project.

**Current state:** the Windows x64 / MSVC 2022 AEX workflow has completed successfully. It builds the native plugin, runs the CPU smoke tests, verifies the single-file output and publishes a raw `.aex` release asset. After Effects host loading/rendering still requires an in-host test.

**[Download the latest VHS-90.aex](https://github.com/vamprieieuu/vhs---90/releases/latest/download/VHS-90.aex)**

Use **Actions → Build** for the Windows AEX build. **Core Tests (Linux - no AEX)** is only the host-independent test workflow and does not produce an AEX.

## Engineering status

See [Project Status](docs/PROJECT_STATUS.md) for verified work, limitations and next gates.

## Source research

See [Source Audit Index](docs/source-audits/INDEX.md) for scoped audits of the supplied C++/C/Rust/GLSL/WGSL sources, AEX references, font pack, camera clips and VHS Damage Lab reference prompts.

## Current CPU engines

- `Source/VHS90Core/VHS90SignalKernels.h/.cpp` — source-matched RGB↔YIQ conversion and initial signal kernels.
- `Source/VHS90Core/VHS90RegionEngine.h/.cpp` — arbitrary horizontal/vertical/rectangular Tracking Noise and Head Switching regions, smooth servo drift, segmented tears, deterministic events and alpha-aware compositing.
- `Source/VHS90Core/VHS90StaticEngine.h/.cpp` — procedural static/snow patterns, top/bottom weighting, split-band chroma and static-to-blackout events.
- `Source/VHS90Core/VHS90IIRFilter.h/.cpp` — constant-K and Butterworth YIQ bandwidth filtering.
- `Source/VHS90Core/VHS90CompositeEngine.h/.cpp` — four-phase NTSC composite encode/decode with Box, One-Line Comb and Two-Line Comb demodulators.
- `Source/VHS90Core/VHS90SyncEngine.h/.cpp` — procedural time-base instability, tracking tears, tracking line and sync vertical roll.
- `Source/VHS90Core/VHS90TapeTransportEngine.h/.cpp` — deterministic tape transport warp with a render-local replay of control-track PI servo, pulse dropout/false-pulse handling and search mode.
- `Source/VHS90Core/VHS90TrackReadEngine.h/.cpp` — three-track read overlap and frequency-dependent opposite-azimuth cancellation.
- `Source/VHS90Core/VHS90HeadConditionEngine.h/.cpp` — two-head field alternation with deterministic debris/grease accumulation and head wear/gap response; optional 12-point physical tape sampling.
- `Source/VHS90Core/VHS90VcrSignalEngine.h/.cpp` — NTSCRT sample-rate, channel-gain, spacing-bank, analytic FM-bandpass, low-pass, emphasis/de-emphasis and input-trap coefficient generators.

## Initial AE host adapter

`Source/VHS90Effect.cpp` registers one VHS-90 effect with initial controls for Composite Signal, Sync Collapse, Static, Tracking Noise and Head Switching. Tracking Noise exposes region shape, center X/Y, width/height, motion X/Y, servo drift and tear probability. Head Switching exposes bottom-edge position, height and horizontal shift; a 100% height with bottom edge at 100% covers the full frame.

The adapter contains SmartFX pre-render/render handling, a legacy render fallback, ARGB32/ARGB64/ARGB128 conversion, full-frame input checkout for neighborhood-dependent kernels, output-region cropping and per-render immutable settings. This host source has not yet been compiled against MSVC/AE SDK.

## CPU-only build and tests

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVHS90_BUILD_AE_PLUGIN=OFF
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The Windows Build workflow runs the CPU CTest suite as part of the native AEX build. The separate Linux core workflow does not produce an AEX.

## Windows AEX build

Requirements and SDK setup are documented in [Windows AEX Build](docs/WINDOWS_AEX_BUILD.md). The checked-in VHS6RIFT SDK and approved font files are used by default; a separate SDK archive is not normally required.

```powershell
 .\build.ps1 -BuildDir build -Configuration Release
```

The output is the bare file `build/VHS-90.aex`. The workflow uploads it as `VHS-90-AEX` and publishes it as a raw release asset. A successful build is not proof that the plugin loads or renders correctly in After Effects; host verification remains a separate gate.
