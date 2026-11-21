# VHS-90 — VHS Effect Add-on Python Engine Audit

Date: 2026-11-19
Input: vhs/github repos/VHS_effect_add-on-1.0.0/
Scope: archive inventory; reviewed VHSParams, preset definitions, VHSContext caching, head-switch/tracking/dropout kernels, OSD rendering and main frame pipeline.

## Parameters and presets

The core uses float32 image processing in [0,1]. Controls include chroma bleed/offset/subsampling, luma/chroma/edge noise, scanlines, vignette, global jitter, line jitter, vertical roll, head switch amount/height, tracking band, dropouts/dropout line count, softness, bloom, sharpen, saturation, contrast, brightness, green/sepia tint and OSD controls. Five presets are included: light, classic, heavy, worn and clean_tv.

## Processing details

- Chroma is processed in YCbCr with horizontal smear/offset and subsampling.
- Timebase error includes global jitter, row jitter and vertical roll.
- Optical processing combines softness, bloom, sharpening and color adjustments.
- Head switching is a bottom-anchored band whose height is a fraction of image height; within the band it applies row-dependent horizontal tear, noise, luma kick and intermittent high-frequency stripes.
- Tracking is a separate horizontal band that moves upward through the frame over time, with a sinusoidal envelope, horizontal displacement, local luma attenuation and noise.
- Dropouts are short, randomly placed horizontal white or dark line segments.
- Noise is generated at reduced resolution and enlarged for coarse tape grain.
- OSD elements include REC, PLAY/SP and date/time, with independent opacity, glitch, ghost, scale and REC blink settings.

## OSD ordering difference

The source applies OSD after noise, scanlines and vignette. This is not the VHS-90 requirement: VHS-90's OSD must be composed before the VHS damage pipeline so the text is degraded by the same tape/signal stages as the image. Reuse its OSD element ideas and timing controls, but move the stage to the required pre-damage position.

## VHS-90 reuse decision

Port selected row-jitter, moving tracking-band envelope, dropout event and OSD parameter concepts. Replace bottom-only head switching with arbitrary region geometry. The tracking band in this source moves vertically but does not expose custom X/Y/width/height or multiple simultaneous regions. Replace mutable random state with deterministic frame/seed streams. Input is converted to RGB float and output back to uint8; alpha is not maintained as a separate preserved channel in the documented frame API.

## Verification

Source functions and preset dataclasses were inspected statically. No Python runtime render or self-test was executed.