# Source Audit 06 — NTSC-CRT v2.3.2

**Folder:** github repos/NTSC-CRT-v.2.3.2  
**Status:** C89 VHS path inspected and compiled; encode/decode smoke test passed.

## Files reviewed
All 28 project files were inventoried, including the complete C89 core, NTSC/VHS/NES/SNES/PV-1000/template variants, pixel I/O, CLI/video utilities, CMake configuration, controls documentation, license and the 72-image test-output archive.

## Critical source: dedicated NTSC-VHS path
The project has a dedicated CRT_SYSTEM_NTSCVHS mode in crt_ntscvhs.c/.h. It is not just a general NTSC filter with a preset name.

### Encoder/modulator behavior
- Builds a sampled composite signal with front porch, sync, breezeway, color burst, back porch and active video.
- Generates equalizing pulses and vertical-sync pulse structure.
- Converts RGB to YIQ with fixed-point integer matrices.
- Uses fixed-point sine/cosine and phase tables for quadrature chroma modulation.
- Applies separate IIR low-pass filters to Y, I and Q before modulation.
- Supports 227.5-cycle alternating/checkered chroma phase and a 228-cycle vertical pattern.
- Uses interlaced field/frame phase alignment and hue controls.
- Supports input RGB/BGR and four-channel pixel layouts, plus source scaling and offsets.
- VHS tape speed is a compile-time setting in the supplied header, not a runtime parameter: SP Y 3.0 MHz / I-Q 627 kHz; LP Y 2.4 MHz / I-Q 400 kHz; EP Y 2.0 MHz / I-Q 370 kHz.
- The VHS mode has an optional bottom-area signal-noise/aberration behavior.

### Decoder/demodulator behavior in the shared core
- Adds signal noise to the analog sample buffer.
- Detects vertical and horizontal sync using integrated pulse thresholds and tracking windows.
- Recovers carrier phase and chroma from the burst.
- Uses separate equalizer state for Y/I/Q.
- Interpolates decoded signal samples to the output dimensions.
- Converts YIQ back to RGB and applies monitor brightness/contrast/saturation/hue.
- Supports interlaced field blending and scanline behavior.

## Important engineering findings
- The core is C89, integer/fixed-point, CPU-only, no third-party runtime dependency and no SIMD requirement.
- The CRT signal buffer is fixed to the emulated NTSC raster size; output can be arbitrary dimensions.
- It explicitly does not use alpha. VHS-90 must preserve alpha separately.
- Some filter states are static globals and noise uses global rand(); these must be moved into per-instance/per-render state and made deterministic for MFR.
- The VHS mode is compile-time selected and needs to become runtime-configurable in VHS-90.
- Existing bottom noise is hard-coded to a lower frame region. It cannot satisfy VHS-90's freely positionable Tracking Noise and expandable Head Switching requirement without modification.
- The sample code supports multiple pixel formats but the host adapter must also honor AE rowbytes, pixel format, premultiplication and arbitrary dimensions.

## Verification performed
- crt_core.c + crt_ntscvhs.c compiled successfully with GCC in strict C89 mode (one non-fatal unused-variable warning).
- A standalone harness initialized a 640x480 BGRA output, modulated a 320x240 RGBA test image and demodulated it. Result: successful run and valid opaque BGRA output.
- The supplied archive contains 72 test images across original, chroma-pattern, and interlaced/progressive output groups. These are useful signal QA patterns, not assets to composite into the effect.

## VHS-90 integration decision
This is the strongest low-level C implementation found so far for a CPU-first NTSC/VHS composite signal core. Preserve its integer/fixed-point signal model as a core reference and port/adapt its encoder, sync/burst structure, carrier modulation, IIR bandwidth shaping and decoder into the shared VHS-90 engine. Do not import its CLI/UI or other console-system emulators. Convert fixed compile-time VHS settings to runtime profile parameters, make all state per-instance and MFR-safe, add alpha preservation, and replace the fixed lower noise region with the approved configurable region system.

No claim is made that this source has already been copied into or wired into the AE plug-in.
