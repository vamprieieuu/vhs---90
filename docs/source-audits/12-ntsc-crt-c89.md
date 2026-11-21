# VHS-90 — NTSC-CRT C89 Signal Engine Audit

Date: 2026-11-19
Input: `vhs/github repos/NTSC-CRT-v.2.3.2/` from supplied `vhs.zip`
Scope: full 28-file inventory; detailed review of README/build instructions, VHS timing header, CRT core interface, VHS modulation source, and demodulator noise section.

## Repository and design

- 28 files, primarily C89 source and headers, with CMake, test-image archive, command-line/video conversion helpers and documentation.
- Designed as integer-only, fixed-point C, software-rendered, no third-party libraries, no SIMD and single-threaded.
- Supports standard NTSC, a VHS timing variant, NES, SNES, PV-1000 and template modes.
- Core API is a modulate/demodulate pair: `crt_modulate` converts RGB into a sampled analog waveform; `crt_demodulate` performs sync recovery, noise injection, decoding and output reconstruction.

## VHS-specific signal model

The VHS header defines:
- Checkered 227.5-cycle chroma pattern (alternating line phase), with optional 228-cycle vertical chroma mode.
- Four samples per chroma period.
- 262-line internal raster, active lines 21–260, active video window and NTSC blanking/sync/burst intervals.
- Fixed-point Y/I/Q low-pass filters.
- VHS SP/LP/EP timing profiles selected at compile time.

The VHS speed presets change the signal bandwidth constants:
- SP: Y 3.0 MHz; I/Q 627 kHz.
- LP: Y 2.4 MHz; I/Q 400 kHz.
- EP: Y 2.0 MHz; I/Q 370 kHz.

The modulation path converts RGB to fixed-point YIQ, low-pass filters each component independently, modulates I/Q onto the subcarrier, and writes the analog waveform. The demodulator searches for sync, recovers field phase, injects noise, and reconstructs the image.

## VHS noise / damage behavior

- The VHS build can add a noise region near the bottom of the analog frame. The location and height are hard-coded in the demodulator, not exposed as parameters.
- The noise pattern is synthesized from pseudo-random values and a line-dependent trigonometric term.
- The VHS modulator has an optional aberration offset applied to the bottom of the active raster.
- The command-line/video tool exposes a bottom-frame damage option, again not a general arbitrary-region control.
- The core has no full-featured tracking/head-switch region model comparable to VHS-90's requirements. This source is primarily an NTSC composite encoder/decoder with a small bottom-edge VHS treatment.

## Useful techniques for VHS-90

1. Fixed-point IIR low-pass filtering for predictable, low-cost Y/I/Q bandwidth shaping.
2. Alternating subcarrier phase for the 227.5-cycle NTSC line pattern.
3. Explicit sync, blanking, back porch and colorburst waveform generation.
4. Integer YIQ conversion and phase-based chroma modulation/demodulation.
5. Signal-domain noise injection and sync recovery to produce authentic lock instability.
6. Compile-time SP/LP/EP bandwidth presets as a reference for VHS speed curves.

## Important incompatibilities / cautions

- The code is single-threaded and contains static IIR filter state. It cannot be called concurrently from AE MFR render threads without redesign.
- It uses global `rand()` in some VHS aberration/noise paths, so output is not safe/deterministic under concurrent rendering.
- The CRT core explicitly does not use the alpha channel. VHS-90 must preserve alpha separately and define premultiplied-alpha behavior.
- It uses fixed internal raster geometry and compile-time VHS mode selection. VHS-90 needs run-time settings and arbitrary input dimensions.
- The VHS implementation's bottom-edge noise and aberration are not suitable for the user's arbitrary position/size requirements.
- The C89 source is an algorithm reference, not a complete AE effect or a replacement for VHS-90's host architecture.

## Integration decision

Port the fixed-point filter and composite timing concepts selectively where they offer performance or signal fidelity. Prefer the more flexible float composite pipeline for quality-sensitive processing, while retaining a scalar fixed-point option for inexpensive CPU paths. Do not port static mutable filter state or global RNG; make all state per-render/per-frame and deterministic. Replace bottom-only noise/aberration with the shared VHS-90 arbitrary-region engine.

## Verification status

- VHS-specific modulation and demodulation code was inspected statically.
- No C89 compile or output-image regression suite was run in this pass.
- No After Effects runtime test was performed.
