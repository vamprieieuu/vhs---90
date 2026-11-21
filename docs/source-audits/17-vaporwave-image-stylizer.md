# VHS-90 — Vaporwave Image Stylizer Audit

Date: 2026-11-19
Input: `vhs/github repos/vaporwave-master/`
Scope: all 11 files inventoried; reviewed README, YUV conversion, style definitions, pixel transforms, interlace, noise, snow and convolution code.

## Project purpose

A browser-based image stylizer for 1990s/digital/vaporwave aesthetics. It is not a dedicated VHS video signal emulator. Its styles combine YUV shifts, quantization, geometry, noise, interlace, resampling and image framing.

## Useful image transforms

- RGB↔YUV conversion.
- Independent U/V offsets and saturation multipliers.
- Y contrast, brightness, dark/bright clipping and quantization.
- Horizontal/vertical image shifts and zoom/fit modes.
- Interlace line displacement and brightness modulation.
- YUV 4:2:0-like chroma averaging and chroma noise.
- Independent light/dark luma noise.
- Snow mode generates noisy YUV fields.
- Optional convolution kernels for sharpen/emboss-like looks.

## VHS-90 decision

These are optional stylized post-processing ideas, not physically grounded VHS signal algorithms. Keep them out of the core analog signal path; if VHS-90 later needs a deliberately stylized/lo-fi recipe, selected YUV grading and chroma quantization concepts may be implemented as a separate finishing stage. The browser app, image assets, watermark and UI are not relevant to the native AE plugin.

## Validation

All source files were inventoried; the main pixel-processing and style definitions were inspected. No browser rendering test was run.
