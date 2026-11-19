# VHS-90

Native C++ After Effects VHS / analog signal processing plugin project.

**Current state:** source research and host-independent C++ kernels are in progress. The repository does not yet contain a complete AE plugin target or a test-ready AEX. No After Effects runtime success is claimed.

## Engineering status

See [Project Status](docs/PROJECT_STATUS.md) for the current state, verified tests and next engineering gates.

## Source research

See [Source Audit Index](docs/source-audits/INDEX.md) for the audits of the supplied C++/C/Rust/GLSL/WGSL sources, seven supplied AEX references, font pack, camera clips and VHS Damage Lab reference prompts.

## Current CPU kernels

- `Source/VHS90Core/VHS90SignalKernels.h/.cpp` — YIQ conversion, chroma blur and initial composite/VHS reference kernels.
- `Source/VHS90Core/VHS90RegionEngine.h/.cpp` — arbitrary horizontal/vertical/rectangular Tracking Noise and Head Switching regions, deterministic events and alpha-aware compositing.
- `Source/VHS90Core/VHS90StaticEngine.h/.cpp` — generated static/snow patterns, top/bottom weighting, split-band chroma, and static-to-blackout events.
- `Source/VHS90Core/VHS90IIRFilter.h/.cpp` — constant-K and Butterworth YIQ bandwidth filtering.
- `Tests/` — standalone C++ smoke tests for the host-independent kernels (4 CTest targets).

## Build status

Build and run the host-independent CPU tests:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Root CMake currently builds the host-independent CPU core and smoke tests only. It does not yet build an AEX: PiPL, AE entry point and Windows plugin target are still pending. Do not treat the standalone C++ tests as an AE plugin build or runtime test.