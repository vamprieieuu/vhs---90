# VHS-90 — YIQ IIR Bandwidth Filter Implementation

Date: 2026-11-19
Files added:
- Source/VHS90Core/VHS90IIRFilter.h
- Source/VHS90Core/VHS90IIRFilter.cpp
- Tests/VHS90IIRFilterSmoke.cpp

## Source algorithm port

Ported the ntsc-rs filter design concepts into host-independent C++:
- Constant-K first-order low-pass coefficient design, cascaded to a third-order transfer function.
- Second-order Butterworth low-pass alternative.
- Direct-form IIR state update and row-wise filtering.
- Configurable zero or first-sample steady-state initialization.
- Independent I and Q cutoff frequencies and sample delays.
- Optional independent luma low-pass.

Default NTSC-style parameters: sample rate 14.3181818 MHz, I cutoff 1.3 MHz, Q cutoff 0.6 MHz, I delay 2 samples and Q delay 4 samples.

## YIQ matrix correction

During filter tests, the earlier YIQ conversion was found to be the transpose of the standard YIQ transform. Round-trip-only testing had not exposed this because the two transposed matrices were mutual inverses. Corrected rgbToYiq/yiqToRgb to the standard matrix and added a neutral-gray assertion (Y=gray, I=0, Q=0).

## Local C++ verification

- CMake configure/build: PASS.
- CTest: 4/4 PASS (signal kernels, region engine, static engine, YIQ IIR filter).
- YIQ IIR test under AddressSanitizer and UndefinedBehaviorSanitizer: PASS.
- Alternating red/blue high-frequency chroma is attenuated by the I/Q low-pass filters.
- Grayscale remains neutral; constant input remains stable with FirstSample initialization.
- Alpha is preserved, premultiplied RGB is unassociated/re-associated, zero-cutoff mode is exact identity, Butterworth alternative attenuates chroma, invalid sample rate is rejected.

## Scope and limitations

This is a row-wise YIQ bandwidth stage, not the complete composite encode/decode pipeline. It is not yet wired to AE SmartFX, parameter controls, tape speed presets or AEX packaging. Per-row filtering is stateless across frames and therefore deterministic/MFR-safe; full composite carrier phase and field processing remain future work.