# VHS-90 — Vaporwave Image Stylizer Audit

Date: 2026-11-19
Input: `vhs/github repos/vaporwave-master/` from supplied `vhs.zip`.
Inventory: 11 files: a browser-based HTML/CSS/JavaScript image stylizer with four JS modules, two logo PNGs, style/preset data and manifest.

## Project purpose

- A web-canvas image stylizer for 1990s / vaporwave / virtual-signal aesthetics.
- Accepts still images (paste, drag/drop or file input) and exports JPEG.
- It is not a video processor, NTSC/VHS waveform simulator, shader pack or native plugin.
- The README describes parameterized visual exploration and user-shared style presets.

## Pixel algorithms in vaporwave.js

1. BGR/RGB-like canvas pixels are converted to YUV using a simple BT.601-style matrix.
2. Luma is adjusted by contrast, brightness/light gain, black fade and white fade.
3. U and V are independently scaled, offset and quantized into configurable color levels.
4. U and V are shifted independently in X/Y, producing chroma/luma misregistration.
5. Optional 4:2:0-style chroma reduction averages a 2×2 block of U/V samples.
6. Optional interlacing displaces pixels horizontally according to a repeating row phase and adds row-dependent brightness variation.
7. Optional vertical transpose bends horizontal rows with a power curve and random per-row displacement.
8. Optional light/dark luma noise is added with brightness-dependent weighting.
9. Optional sharpening uses one of four 3×3 convolution kernels (right-lean, left-lean, box blur, emboss).
10. Optional snow replaces the frame with generated noise and chroma values.
11. Optional watermark and text labels are drawn before the pixel processing.

## Default controls recovered

- Zoom / output aspect: 4:3, 16:9, 21:9 or 1:1.
- Light, contrast, dark fade, bright fade.
- X/Y channel shift and U/V offsets.
- Color level quantization (1–255; default 4).
- Interlacing strength/period/brightness.
- Vertical transpose, power and noise.
- Replace/shift pixel offset.
- U/V vividness.
- Sharpen and kernel selection.
- Luma inversion.
- YUV420 chroma subsampling and noise.
- Snow.
- Light noise / dark noise.
- Crop/contain/cover, border, fit ratio.
- Watermark and text label layout.
- Output JPEG quality.

## Limitations

- Randomness uses global `Math.random()`; there is no user seed or frame-time model.
- The source is single-image canvas processing; no motion, temporal persistence or field sequencing.
- The code manipulates pixel positions using array-index arithmetic; out-of-range positions may wrap or write undefined values in some paths.
- Snow replaces the image with a generated texture rather than simulating RF/no-signal circuitry.
- The output is JPEG, so it introduces an additional lossy compression stage.
- No alpha-preserving image pipeline, no OSD system, no fonts, no Arabic shaping and no VHS tape/head/transport simulation.

## VHS-90 reuse decision

This source is low-priority for VHS-90. Reuse only selected visual math if needed:
- Independent U/V offsets and quantization.
- 4:2:0 chroma subsampling.
- Row-phase interlacing displacement.
- Power-shaped horizontal row bending.
- Luma-only noise weighting and compact convolution kernels.

Do not import the browser UI, canvas, watermark, image hosting, user preset backend or JPEG export. These are generic image-stylization operations rather than VHS signal algorithms.
