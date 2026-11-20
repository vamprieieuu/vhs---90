# VHS-90 — NTSC-QT spectral ringing and pre-emphasis source-exact follow-up

Date: 2026-11-21  
Parent audit: `docs/source-audits/52-ntscqt-1.5.0-expanded-audit.md`  
Source: supplied `vhs.zip` → `vhs/github repos/ntscqt-1.5.0/ntscqt-1.5.0/app/ntsc.py` and `app/ringPattern.npy`

## Source code re-opened

The actual archived `ntsc.py` was re-extracted and the implementation of `ringing`, `ringing2`, `composite_preemphasis`, and `Ntsc.ringing` was read directly. This confirms the two ringing variants are not interchangeable with the current spatial edge/echo kernel.

### Variant A — rectangular horizontal DFT mask with optional seeded frequency noise

For each selected field plane (Y, I, Q), the source:

1. Converts the 2D field plane to float32 and runs a 2D complex DFT.
2. Shifts the zero-frequency origin to the center.
3. Creates a two-channel mask of shape rows × columns × 2, initially zero.
4. Sets a centered horizontal frequency band to 1, where `maskH = min(crow, int(1 + alpha * crow))`.
5. If `noiseSize > 0`, creates a uniform noise field with amplitude derived from `noiseValue`, zeros its central horizontal interval, and adds seeded random variation to the mask.
6. Multiplies the shifted complex spectrum by the mask, inverse-shifts and inverse-transforms with scale normalization.
7. Returns the real output channel; clipping to the input plane's min/max is optional and the VHS field call disables clipping.

Source call uses `_ringing`, `_freq_noise_size`, and `_freq_noise_amplitude` for Y, I and Q independently per selected field. The source's `numpy.random.RandomState(seed)` sequence is not equivalent to VHS-90's existing integer hash/noise helper; exact seeded image parity requires an explicit RNG compatibility implementation or a documented non-identical seed mapping.

### Variant B — bundled spectral response curve

The exact source operation is:

- `RingPattern` is loaded from `ringPattern.npy`; the archived array has 720 float32 samples, range 0..1.
- For an input field plane of width `cols`, compute `scalecols = int(cols * (1 + shift))`.
- Resize the 720-sample response to `scalecols` using linear interpolation.
- Center-crop to `cols` samples using integer half-width indices.
- Raise every response sample to `power`.
- Multiply the 2D shifted complex spectrum by this 1D response broadcast across every row and both complex channels.
- Inverse-shift and inverse-transform with scale normalization; source VHS field processing disables clipping.

This response is horizontal-frequency-only but is applied to every vertical-frequency row of the field plane. It is not a spatial edge kernel, a convolution with guessed taps, or a luma-only effect: source calls it separately for Y, I and Q.

### Field processing and order

The source method applies ringing to `Y[field::2]`, `I[field::2]`, and `Q[field::2]`, preserving alternating-field processing. In the source frame pipeline, the optional composite pre-emphasis is applied to YIQ before the composite modulation stage. It uses a first-order low-pass object to derive a high-pass component and adds `highpass * composite_preemphasis` to the samples; the control is intended for values 0..8. This is a pre-modulation luma-domain stage and cannot be represented faithfully by post-decode sharpening.

## Native port requirements — do not mark complete until all pass

1. Add a dedicated `VHS90NtscQtRingingEngine` or a clearly separated NTSC-QT spectral mode; do not replace or rename the existing spatial `VHS90RingingEngine`.
2. Preserve both source variants and their distinct parameters:
   - rectangular band mask: alpha, frequency-noise size, frequency-noise amplitude, seed;
   - bundled response: power and shift.
3. Preserve the 2D field-plane semantics and run the operation on Y, I and Q for each selected field, with unclipped output at the same stage as the source.
4. Preserve `ringPattern.npy` data and Apache-2.0 attribution. Convert the exact float32 samples to a checked-in generated C++ header or resource; validate sample count, min/max and a stable checksum during generation/tests.
5. Use a performant FFT implementation with correct arbitrary-width behavior. Do not use a naive O(N²) DFT for full-resolution AE frames, and do not assume radix-2-only dimensions. Validate the chosen FFT against a slow double-precision DFT oracle on small odd/even sizes and against source-generated fixtures.
6. Implement pre-emphasis as a distinct Y-only pre-modulation stage with the source low-pass cutoff and state/reset semantics. Do not route it through the existing post-decode Image Grade or edge-enhancement controls.
7. Add deterministic source fixtures for multiple widths, heights, alpha/power/shift/noise combinations and both fields. Compare numerical error for Y/I/Q separately, not only final RGB appearance.
8. Add parameter/UI wiring and include the controls in preset mapping only after the signal engine and tests are complete.
9. Run the existing Linux core CI, then Windows/MSVC build and actual AE 23.2.1 visual/MFR checks.

## Current truth

- The source algorithm and stage placement are now explicitly re-verified against the archived implementation.
- The existing `VHS90RingingEngine` remains a different spatial edge/echo effect and is not NTSC-QT spectral ringing.
- No spectral-ring code has been added by this addendum. No parity, performance or visual-match claim is made.
- The native port remains an open pre-AEX requirement.
