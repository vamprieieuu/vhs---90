# VHS-90 — BarashNTSC Python Processor Audit

Date: 2026-11-19
Input: `vhs/github repos/BarashNTSC-0.1.2/`
Scope: all 9 files inventoried; reviewed README, parameter dataclass/ranges, cache and worker model, full `VHSProcessor.process` pipeline, and preset JSON structures.

## Architecture

Python desktop processor using OpenCV, NumPy, Pillow and Tkinter. It supports still images and video, live preview, audio playback and multi-process export. It is a reference application, not a native AE library.

## Parameters

The `VHSParams` model exposes 19 controls: detail, noise, color bleed, chroma shift, color phase, composite artifacts, ghosting, interlace, tracking, jitter, tape wear, flicker, scanlines, vignette, edge enhancement, fade, saturation, contrast and brightness. Ranges are authored in the UI slider table, with detail 10–100, color bleed 0–20, chroma shift 0–20, color phase 0–50, most damage values 0–100, saturation/contrast 0–200 and brightness -100–100.

## Processing order and useful algorithms

1. Convert BGR to YCrCb and split luma/chroma.
2. Horizontally reduce luma resolution based on Detail, then resize back; optional edge enhancement adds a horizontal high-frequency residual.
3. Fade lowers luma, lifts the black/gray level and reduces chroma saturation.
4. Cr/Cb are downsampled independently in both horizontal and vertical directions according to Color Bleed, optionally receive noise, then are upsampled.
5. Cr and Cb can be shifted in opposite horizontal directions.
6. Composite-artifact mode injects horizontal luma high-frequency detail into chroma using alternating scanline carriers.
7. Color phase instability rotates Cr/Cb over time.
8. Contrast, brightness and saturation are applied.
9. Luma noise is added separately.
10. Tape Wear creates short random bright streaks or darkened line segments.
11. Jitter and tracking create per-row horizontal offsets; tracking adds a moving vertical band with additional displacement and noise.
12. Interlace applies alternating field displacement.
13. Optional previous-frame blending creates temporal ghosting.
14. Scanline darkening, vignette and flicker are applied last.

## VHS-90 reuse decisions

- Reuse the separate luma/chroma resolution degradation concept, asymmetric Cr/Cb displacement, carrier-based luma-to-chroma leakage, time-varying chroma phase, localized tape-wear streaks and tracking-band texture.
- The current tracking band moves vertically as a whole and is not freely positioned; implement VHS-90's arbitrary region controls instead.
- Temporal ghosting depends on a mutable previous-frame buffer. VHS-90 should use explicit frame history only when host temporal behavior is well-defined, otherwise implement deterministic delayed sampling.
- Random generators are mutable and not keyed solely to frame/pixel; the same frame is not guaranteed to reproduce identically across render order or worker scheduling. Replace with deterministic hash-based random values.
- Input alpha is discarded when BGRA is converted to BGR. VHS-90 must preserve alpha separately.
- The DFT ringing helper is computationally expensive and should not be ported as a per-pixel operation without profiling.

## Presets and validation

The archive includes `example_ANALOG.json` and `example_worn_tape.json`. These are parameter configurations for the Python processor, not native VHS-90 presets. Python/OpenCV runtime was not executed in this pass; no rendered output is claimed.
