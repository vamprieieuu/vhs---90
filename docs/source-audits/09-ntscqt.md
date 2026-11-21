# Source Audit 09 — ntscQT 1.5.0

**Folder:** github repos/ntscqt-1.5.0/ntscqt-1.5.0  
**Status:** complete source review; Python compile and 720x480 processing smoke test passed.

## Files reviewed
The application code, renderer, Qt UI, configuration/template files, random generators, numerical processing, bundled ringPattern.npy resource, requirements, build script, license and demo media were inventoried. The core implementation is app/ntsc.py.

## Processing implementation
This is a readable Python/NumPy/SciPy/OpenCV implementation of the composite NTSC/VHS processing path. It includes:
- BGR-to-planar-YIQ and YIQ-to-BGR transforms.
- Stateful one-pole low/high-pass filters and three cascaded low-pass stages.
- Composite chroma low-pass (full and lighter TV variants) and configurable pre-emphasis.
- Composite chroma modulation into luma and demodulation back out, with phase shift/field timing.
- Correlated luma noise, separate correlated U/V chroma noise, and row-based chroma phase rotation.
- VHS SP/LP/EP profiles with distinct luma cutoff, chroma cutoff and chroma delay.
- VHS luma low-pass and post-filter high-frequency emphasis/sharpening.
- VHS chroma low-pass, delay, previous-line chroma blend and optional composite re-encode/decode.
- Horizontal/vertical chroma bleed with a configurable processing order (before or after other signal degradation).
- VHS edge wave generated from random per-line offsets passed through a low-pass filter.
- Chroma loss on selected scanlines.
- Two distinct ringing implementations: FFT-domain band mask with optional frequency-shaped noise; or a frequency response raised to a configurable power using the bundled ringPattern.npy curve.
- Head switching based on tape phase, head phase, field parity and phase noise. The first affected row receives the main displacement, subsequent rows decay by 7/8, and the affected lines shift across a wider virtual tape line.
- Deterministic XorWow and NumPy random generator implementations for most signal stages.

## Important implementation observations
- This repository duplicates and extends parts of the composite-video-simulator and NTSC-CRT processing logic; do not import duplicate copies of the same signal chain.
- Its tape-speed profiles differ from the NTSC-CRT v2.3.2 header and the Composite Video Simulator source: this code uses SP 2.4 MHz / 320 kHz / delay 9, LP 1.9 MHz / 300 kHz / delay 12, EP 1.4 MHz / 280 kHz / delay 14. Preserve these as a separate source profile until visual/technical reconciliation; do not silently merge conflicting constants.
- The head-switching routine's coordinates are tied to 262.5/312.5-line field geometry and a near-bottom region. It is a useful physical timing model, but must be generalized for VHS-90's arbitrary user-selected position and expandable height.
- Head-switching phase noise calls Python's module-global random.randint rather than the configured random generator, so full deterministic repeatability is not guaranteed for that branch. VHS-90 must route every random source through an explicit per-frame seed/state.
- The renderer uses mutable state and per-frame field processing. VHS-90 must expose a frame-index-addressable deterministic path and avoid global mutable filters for MFR.
- The core has no alpha handling and returns BGR; the AE adapter must preserve alpha separately.

## Verification
- app/ntsc.py passed Python bytecode compilation.
- A generated 720x480 BGR frame was processed with VHS SP, luma/chroma noise and both head-switching disabled/enabled. Both paths returned a 480x720x3 uint8-compatible image (NumPy reported float64 due the final output allocation) with valid 0..255 values.
- The XorWow implementation emitted expected signed-integer overflow warnings during the smoke test; the sequence still completed. A C++ port must define wraparound explicitly using unsigned 32-bit arithmetic.

## VHS-90 integration targets
Port the clear C++-compatible filter and signal equations where they improve the shared engine; use the two ringing algorithms as candidates for a dedicated ringing/edge-echo module; adapt the head-switching phase/decay model to arbitrary regions; retain the separate speed-profile variants for later reference matching. The Qt application, preview renderer, and Python UI are not used in the native AE plug-in.
