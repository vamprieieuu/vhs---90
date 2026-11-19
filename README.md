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
- `Tests/` — standalone C++ smoke tests for the host-independent kernels.

## Build status

No root CMake project, PiPL resource, AE entry point or Windows build workflow exists yet. Do not treat the standalone C++ smoke tests as an AE plugin build or runtime test.