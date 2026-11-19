# VHS-90 — NTSC-CRT C89 / VHS System Audit

Date: 2026-11-19
Input: `vhs/github repos/NTSC-CRT-v.2.3.2/` from supplied `vhs.zip`.
Inventory: 28 archive files (about 9.98 MB including a bundled ZIP), primarily 12 C source files and 10 headers, plus docs, controls, CMake and a bundled compressed archive.

## Architecture

- Integer-only C89 composite NTSC encoder/decoder.
- No floating-point arithmetic, third-party runtime libraries, explicit GPU acceleration, SIMD or threading in the described core.
- Uses fixed-point sine/cosine, fixed-point exponential and integer IIR/biquad-style filters.
- Supports multiple system profiles; `CRT_SYSTEM_NTSCVHS` is profile 5.
- The VHS profile is a variant of the composite NTSC modem, not a full mechanical tape transport simulation.

## VHS profile timing / signal constants

- 227.5 chroma cycles per line (scaled as 2275).
- Four samples per chroma cycle.
- 262-line internal signal raster.
- Active video lines 21 through 260.
- NTSC sync, blanking, burst and active-video intervals are generated in the modulator.
- Chroma phase alternates by line when `CRT_CHROMA_PATTERN=1`.
- Luma, I and Q each have separate integer IIR low-pass filters.
- The VHS profile's Y/I/Q cutoffs and timing are configured in `crt_ntscvhs.h`.

## VHS-specific behavior found in the source

- The modulator resets horizontal sync state at the beginning of each frame; the source comment says this causes only the bottom part of the picture to be warped.
- The demodulator injects additional random noise in a region near the bottom of the frame, with randomized start/end line positions and a phase-shaped envelope.
- The VHS-specific noise path uses the C library global `rand()`.
- The remainder of the signal path retains composite NTSC sync detection, burst/chroma demodulation, phase alignment and color decoding.
- The core can optionally render a raw analog signal view and supports field/interlaced processing.

This is useful for authentic low-level composite signal generation and bottom-edge VHS switching/noise behavior, but it does not satisfy VHS-90's custom-position Tracking Noise or full-height Head Switching requirements.

## Reusable algorithms

1. Fixed-point sine/cosine carrier tables and integer phase arithmetic.
2. Integer-only exponential approximation for filter coefficient setup.
3. Separate Y/I/Q IIR low-pass filtering.
4. Composite signal generation with horizontal/vertical sync, blanking, burst and active video.
5. Chroma phase alternation and field phase alignment.
6. Sync recovery by integrating signal windows and searching for threshold crossings.
7. Composite demodulation with carrier phase alignment and I/Q recovery.
8. Integer RGB↔YIQ transforms.
9. Stateful scanline/field processing and optional line blending.

## Thread-safety and integration blockers

- `crt_demodulate` owns a function-static output buffer.
- Filter history objects are file-static globals (`iirY`, `iirI`, `iirQ`).
- The VHS path uses global `rand()`, and the modulator also calls `rand()` for optional aberration.
- `struct CRT` contains mutable per-instance signal buffers/state, but the static filter/noise state is shared between instances.
- The public header explicitly says the library does not use alpha.
- Input is byte-based RGB/BGR/RGBA/BGRA/ARGB/ABGR; the signal pipeline ignores alpha.
- This design is not safe for concurrent AE MFR renders without moving all mutable state into per-render/per-instance context and replacing global RNG.
- It uses a fixed internal raster and resamples source to that format; VHS-90 needs arbitrary AE dimensions and correct source/destination world handling.

## Build verification

The VHS-selected core units `crt_core.c` and `crt_ntscvhs.c` compiled successfully with GCC using strict C89 mode and `CRT_SYSTEM=5`. One warning remains: an `iccf` local array is written but not used in the VHS modulator. This is a source-level compile check only; the full command-line app and an AE plugin wrapper were not built.

## VHS-90 integration decision

Use this source as a reference for integer fixed-point composite carrier/sync math and a possible low-level compatibility implementation. Do not import its global state or direct render architecture. Refactor every filter state and RNG into an explicit render context, preserve alpha, support MFR, and implement VHS-90's general arbitrary-region model for tracking/head-switching. The full NTSC modem should remain an optional composite-signal stage, not a prerequisite for every VHS damage effect.
