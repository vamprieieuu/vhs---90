# VHS-90 — Procedural Static Engine Implementation

Date: 2026-11-19
Files added:
- Source/VHS90Core/VHS90StaticEngine.h
- Source/VHS90Core/VHS90StaticEngine.cpp
- Tests/VHS90StaticEngineSmoke.cpp

## Modes implemented

- DarkFieldSpeckles: sparse luma speckles with independent I/Q chroma variation.
- FullFrameHorizontal: horizontal static fragments and correlated row noise.
- TopStrip: horizontal static restricted to an adjustable upper band.
- BottomWeightedStreaks: horizontal streak density/intensity weighted toward the bottom.
- MonochromeStreaks: luma-only horizontal streaks without chroma noise.
- SplitBandChromatic: upper horizontal static with lower vertical RGB/chroma streaks.
- StaticToBlackout: deterministic event window that can abruptly cut the generated signal to black.

## Independent controls

Intensity, source/generated mix, event density, cell size, streak length, luma amount, chroma amount, generated base luma, top-band height, split position, bottom weighting exponent, flicker, animation speed, blackout probability/duration, animation enable, seed, frame and premultiplied-alpha mode.

Density is separate from size and intensity. Luma and chroma amplitudes are independent. Animation can be frozen; when enabled, the frame index and speed define deterministic pattern ticks. The engine generates all patterns mathematically; it does not read or composite video/GIF/image overlays.

## Alpha and bypass behavior

- Source alpha is preserved exactly.
- Premultiplied RGB is unassociated before YIQ processing and re-associated to the original alpha afterward.
- Fully transparent pixels are not contaminated.
- Intensity zero with source mix enabled is a bit-exact identity copy.
- HDR values are not clamped to [0,1].

## Local C++ verification

Compiled through the root CMake CPU-core target and executed via CTest. Results: 3/3 total tests pass (signal kernels, region engine, static engine). The static smoke test additionally passed g++ C++17 with AddressSanitizer and UndefinedBehaviorSanitizer.

Test coverage includes same-frame determinism, temporal variation, frozen animation, top-band-only placement, abrupt blackout, exact identity bypass and alpha preservation.

## Scope and limitations

This is a host-independent procedural signal generator, not an AE-integrated parameter panel. It is not yet connected to EffectMain, PiPL, SmartFX, AE render formats or the VHS-90 preset system. Visual matching against each Damage Lab reference remains pending.