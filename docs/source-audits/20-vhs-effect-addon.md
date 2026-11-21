# VHS-90 — VHS Effect Add-on Python Core Audit

Date: 2026-11-19
Input: `vhs/github repos/VHS_effect_add-on-1.0.0/` from supplied `vhs.zip`.
Inventory: 18 files (about 253 KB), including the main processing core, task/video orchestration, GUI, self-test, environment/setup scripts, JSON config and two language README files.

## Core architecture

- Python 3 with NumPy, Pillow and OpenCV; FFmpeg handles video read/write and audio muxing.
- The effect core is organized around `VHSParams`, `VHSContext` and `process(frame, frame_index)`.
- Float32 processing in normalized 0–1 range reduces repeated uint8 clipping.
- Static per-resolution masks and noise pools are cached in the context.
- Temporal random generation is seeded from the global seed and absolute frame index.
- Five built-in looks: light, classic, heavy, worn and clean_tv.
- CLI and GUI support batch images/videos, parameter overrides, date/time progression, OSD and CPU/GPU video encoder selection.

## Recovered pixel pipeline

1. BGR uint8 input → RGB float32.
2. RGB → YCbCr.
3. Chroma low-pass with strong horizontal and weaker vertical blur.
4. Chroma downsampling/upscaling for broad color blocks.
5. Opposing Cb/Cr horizontal offsets for color-edge separation.
6. YCbCr → RGB.
7. Global horizontal jitter, per-row displacement and vertical roll.
8. Softness / bloom / sharpening / color adjustments.
9. Bottom head-switch band.
10. Vertically moving tracking band.
11. Random short horizontal bright/dark dropouts.
12. Luma noise, chroma noise and coarse edge/dirt noise.
13. Scanlines and vignette.
14. OSD glyph composition.
15. Clamp to 0–1, convert to uint8 and return BGR.

## Main effect controls

### Chroma and image quality
- Chroma Bleed
- Chroma Bleed Offset
- Chroma Subsample
- Softness
- Bloom
- Sharpen
- Saturation
- Contrast
- Brightness
- Green Tint
- Sepia Tint

### Signal and time-base
- Luma Noise
- Chroma Noise
- Edge Noise
- Jitter
- Line Jitter
- Vertical Roll
- Scanlines
- Scanline Period
- Vignette

### Tape damage
- Head Switch
- Head Switch Height
- Tracking Band
- Dropout
- Dropout Lines

### OSD
- Master OSD
- REC indicator
- PLAY indicator
- Timestamp
- REC blink frequency
- REC dot color
- OSD opacity
- OSD glitch
- OSD ghost
- OSD scale
- Timestamp start, offset, mode and speed
- Optional literal timestamp text

## Specific mechanics worth reusing

- Chroma is blurred and downsampled independently from luma, with opposite Cb/Cr offsets.
- TBC error combines slow multi-frequency drift, frame jitter and per-row wave/random offsets.
- Tracking is a soft-envelope horizontal displacement band that travels vertically through the image over time.
- Dropouts are discrete short horizontal bright or dark streak events, with Poisson-distributed count.
- Noise is generated at quarter resolution and nearest-neighbor enlarged for larger analog-looking grain.
- OSD uses cached glyph masks and blends only each glyph's local rectangle, avoiding repeated full-frame text compositing.
- REC blink is frame-rate aware and uses a 65% duty cycle with a smooth fade.
- Timestamp updates once per second and can be fixed or time-advancing.

## Critical mismatches with VHS-90 requirements

1. **Head Switching is bottom-only.** Height is configurable as a percentage, but vertical position is not; the band always occupies the bottom of the image. VHS-90 must provide position and height, including full-frame height.
2. **Tracking Band is not a general region system.** It moves vertically through the frame but spans the full image width and has no independent X/width/direction/custom-region controls. VHS-90 requires arbitrary placement and multiple independent regions.
3. **Alpha is discarded.** The API explicitly accepts BGR (H,W,3); the pipeline has no alpha channel. VHS-90 must preserve AE alpha and premultiplication.
4. **OSD is composited last.** The source adds OSD after all damage/noise/scanline processing, so the text stays clean. VHS-90 requires OSD to be composed before VHS damage so it can be distorted with the picture.
5. **OSD positions are fixed.** REC/PLAY are top corners and timestamp is bottom-right. There are no independent free X/Y controls for every OSD group.
6. **Font handling is system-dependent.** The source searches system font directories; it does not use the user's supplied VHS-90 font pack. VHS-90 must use only the 15 bundled fonts and Arabic shaping/localization.
7. **Not a physical composite signal model.** The chroma carrier effects are YCbCr-domain image approximations, not a full NTSC/VHS record/play waveform.

## Validation performed

The supplied self-test could not complete because its default `Material/` test-assets directory is not present in the archive extraction; it stopped after environment checks and reported the missing directory.

A separate synthetic 240×320 BGR frame test was run through all five presets:
- light — passed, deterministic for the same seed/frame.
- classic — passed, deterministic.
- heavy — passed, deterministic.
- worn — passed, deterministic.
- clean_tv — passed, deterministic.

Each returned the expected H×W×3 uint8 image. This validates the Python processing path on synthetic input, not visual fidelity or AE integration.

## VHS-90 integration decision

Port selected CPU algorithms into C++: anisotropic chroma blur/downsample, opposing chroma offsets, global and per-row time-base jitter, moving tracking-band envelope, Poisson-like dropout event placement, quarter-resolution noise pools, bloom/softness/sharpen stages and cached glyph-mask compositing. Keep the effects as separate shared modules. Replace bottom-only head switching and full-width tracking with VHS-90's arbitrary region engine; preserve alpha; move OSD before the damage stages; and use the supplied font pack with native Arabic shaping.
