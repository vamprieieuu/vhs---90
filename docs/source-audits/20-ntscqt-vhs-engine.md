# VHS-90 — ntscQT VHS / Composite Engine Audit

Date: 2026-11-19
Input: vhs/github repos/ntscqt-1.5.0/
Scope: 23-file inventory; reviewed README, core app/ntsc.py, VHS speed constants, composite/VHS stage order and bundled preset template JSON.

## Core model

- Python/NumPy/OpenCV emulator derived from the composite-video-simulator algorithm.
- Converts BGR to planar integer YIQ, processes fields separately, then converts the selected field back to BGR.
- Supports NTSC composite modulation/demodulation, YIQ low-pass filtering, dot-crawl/cross-colour behavior, noise, phase shifts, ringing and VHS-specific bandwidth limitations.
- Uses a seeded random source; the GUI exposes a Seed so a chosen parameter/noise configuration can be reproduced.

## VHS tape speed model

The source defines three VHS profiles:
- SP: luma cutoff 2.4 MHz, chroma cutoff 320 kHz, chroma delay 9 samples.
- LP: luma cutoff 1.9 MHz, chroma cutoff 300 kHz, chroma delay 12 samples.
- EP: luma cutoff 1.4 MHz, chroma cutoff 280 kHz, chroma delay 14 samples.

The implementation low-pass filters Y, I and Q separately; applies a luma pre-emphasis/sharpen stage; applies horizontal chroma delay; and vertically blends adjacent chroma scanlines to emulate the colour-under playback delay line. If S-video output is disabled, it recombines chroma into luma and decodes it back through the composite path.

## Distinct modules

- Composite input/output chroma low-pass (full or lighter TV path).
- Frequency-domain ringing variants, including an optional frequency mask pattern.
- Luma noise and separate chroma noise.
- Per-line chroma phase noise rotating I/Q.
- Horizontal/vertical color bleed.
- VHS edge wave using seeded per-row shifts smoothed by a low-pass filter.
- VHS chroma loss that can zero I/Q on selected scanlines.
- Head switching with a phase point, speed, phase noise and line-by-line displacement/decay.
- SP/LP/EP tape speed selection.

## Head-switching geometry

Head switching is represented as a phase point on the NTSC/PAL raster and advances by a configured speed. The code maps it to scanlines and a horizontal shift that decays as the affected rows progress. It is tied to the raster's head-switch interval near vertical sync and is not a general user-positioned rectangle. VHS-90 must implement the arbitrary region engine rather than copying this raster-specific placement.

## VHS-90 reuse decisions

1. Port the SP/LP/EP bandwidth curves, chroma delay, vertical chroma blending and luma pre-emphasis as separate shared-engine modules.
2. Reuse seeded per-row edge-wave profiles and I/Q phase-noise rotation.
3. Treat chroma loss as a separate event/module from luma dropout and static.
4. Use the head-switch phase/decay texture as a visual/signal reference, but replace the fixed raster position with VHS-90's arbitrary region controls.
5. Keep composite input/output stages independently configurable so clean Y/C output and composite re-encoding can be distinguished.

## Preset data

The archive includes builtin_templates.json with community preset dictionaries and random_ntsc(seed) behavior in the core. The JSON is an algorithm/configuration reference; source usernames or labels should not be exposed in VHS-90's UI.

## Limitations and verification

- The code is Python and uses mutable random state; native VHS-90 needs per-frame deterministic streams safe for MFR.
- Processing is field-based and expects its own image/layout conventions; adapt to AE pixel formats, arbitrary dimensions and alpha.
- No Python runtime render or test suite was executed here.