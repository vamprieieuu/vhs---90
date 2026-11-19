# VHS-90 — Arbitrary Damage Region Engine Implementation

Date: 2026-11-19
Files added:
- Source/VHS90Core/VHS90RegionEngine.h
- Source/VHS90Core/VHS90RegionEngine.cpp
- Tests/VHS90RegionEngineSmoke.cpp

## Implemented behavior

- Normalized region centers and sizes, independent of composition resolution.
- Horizontal band, vertical band and free-rectangle region definitions.
- Any number of simultaneous regions through a region array.
- Independent TrackingNoise and HeadSwitch behavior.
- Arbitrary X/Y placement, width/height, feather, intensity, motion vector and speed.
- Row-correlated horizontal tracking jitter, wave displacement and head-switch tearing.
- Independent noise density, noise amplitude, dropout probability/amount and chroma shift.
- Deterministic per-region random streams using seed, region index, frame and cell coordinates.
- Full-frame HeadSwitch coverage by setting region width/height to 1.0 and feather as desired.
- Preserved alpha with a premultiplied-alpha path that re-associates displaced RGB to the destination pixel's alpha.

## Local C++ verification

Compiled the new region engine together with the VHS90SignalKernels source using g++ C++17, -Wall, -Wextra, -Wpedantic, AddressSanitizer and UndefinedBehaviorSanitizer.

Results:
- VHS90SignalKernelsSmoke: PASS.
- VHS90RegionEngineSmoke: PASS.
- Same seed/frame repeatability: PASS.
- New-frame temporal variation: PASS.
- Interior horizontal tracking region: PASS.
- Right-side vertical head-switch region: PASS.
- Full-frame head-switch coverage: PASS.
- Transparent-pixel non-contamination and alpha preservation: PASS.
- Zero global intensity identity copy: PASS.
- No compiler warnings in the final two test builds.

## Scope and limitations

This is a host-independent CPU kernel, not an AE-integrated feature yet. It is not connected to parameter registration, PiPL, SmartFX or an AEX target because the current VHS-90 repository does not yet contain a complete AE plugin project. The region engine is a foundation for the user's arbitrary-position/multi-region requirements; visual tuning and host-level pixel-format/MFR validation remain pending.