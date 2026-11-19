# VHS-90 — NTSC Composite Modem Implementation

Date: 2026-11-19
Files added:
- Source/VHS90Core/VHS90CompositeEngine.h
- Source/VHS90Core/VHS90CompositeEngine.cpp
- Tests/VHS90CompositeEngineSmoke.cpp

## Algorithm port

Ported the four-phase NTSC composite modulation/demodulation structure from ntsc-rs:
- Four-sample chroma carrier with I_MULT = [1, 0, -1, 0] and Q_MULT = [0, 1, 0, -1].
- Scanline/frame phase modes: fixed 0°, quadrature 90°/270° and alternating 180°.
- Composite encode: add modulated I/Q chroma to the Y plane.
- Box demodulator: 4-sample luma box filter, chroma residual and quadrature demodulation distributed to adjacent samples.
- One-line comb demodulator: average with the adjacent line and recover chroma from the residual.
- Two-line comb demodulator: weighted current/previous/next line average and chroma residual.
- Optional input and output YIQ bandwidth stages using the new IIR filter module.
- Strength mix with exact identity bypass at zero.

## Alpha / determinism

- No global state or mutable temporal history.
- Phase depends only on explicit frame index, line and phase offset.
- Alpha is preserved; premultiplied input is unassociated before YIQ modulation and reassociated after decoding.
- Source/destination row strides are respected.

## Local verification

- CMake Release build: PASS.
- CTest: 5/5 PASS.
- CMake Debug build with AddressSanitizer + UndefinedBehaviorSanitizer: 5/5 PASS.
- Tests cover deterministic same-frame output, phase-dependent output, distinct Box/OneLine/TwoLine demodulators, strength-zero exact identity, alpha preservation and neutral grayscale away from the left filter boundary.

## Scope and limitations

This is a compact four-phase NTSC composite round-trip kernel, not the complete ntsc-rs implementation. It does not yet include the full field model, PAL timing, sync/burst waveform, AFC/PLL recovery, temporal chroma persistence, dot-crawl controls, RF noise or all comb/notch filter variants. It is a CPU module and is not yet connected to the AE host, parameter UI or preset recipes.