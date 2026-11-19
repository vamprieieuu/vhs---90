# Source Audit 12 — Vaporwave Image Signal Stylizer

**Folder:** github repos/vaporwave-master  
**Status:** complete review of all 9 project files; browser processing and preset catalog inspected.

## What the project is
A browser-based still-image stylizer implemented in JavaScript/Canvas. It is not a video processor or a real-time analog signal simulator. The UI, image picker, public/user preset APIs, localStorage, watermark and JPEG export are not suitable for the native AE plug-in, but the pixel algorithms and preset combinations are useful references.

## Pixel algorithms found
- RGB↔YUV conversion using fixed coefficients, with integer floor/round behavior and 8-bit clipping.
- Independent luma and chroma adjustments: luma contrast/light, black/white fade clamps, U/V saturation and offset, plus chroma quantization/posterization.
- Independent spatial displacement of U and V planes through pixel-buffer index offsets, allowing colored edge separation without moving luma.
- YUV 4:2:0-style chroma subsampling: average Cb/Cr across a 2x2 block and replicate to the four pixels.
- Scanline/interlace-like horizontal displacement with periodic row profile, adjustable displacement and row brightness modulation.
- Vertical-gradient horizontal shear/transpose with a power curve and random row noise.
- Luma-only inversion and independent bright-region/dark-region noise.
- Synthetic snow generation with random luma/chroma values and alternating-line structure.
- Small convolution kernels: directional right/left edge emphasis, box blur and emboss.
- Image crop/fit/contain/cover, zoom and border sizing.
- Final JPEG compression and optional resizing to simulate lossy delivery.

## Preset catalog
The styles.js file contains 18 built-in style configurations (plus user and server-shared styles). They combine YUV offsets/saturation, posterization, luma fade, chroma displacement, interlace, shear, inversion, noise, sharpening, crop/zoom and JPEG quality. The source's Chinese preset names are not to be exposed in VHS-90; use the visual parameter recipes as internal references and assign VHS-90 names based on the actual look.

## Important limitations and porting decisions
- The code is for still images only; there is no frame-time model or video temporal behavior.
- Most random effects use Math.random() and are not reproducible. VHS-90 must use deterministic frame-index/module seeds.
- Some effects use direct linear pixel-buffer index offsets that can wrap/overwrite out-of-bounds positions. Reimplement with explicit bounds handling rather than copying these unsafe indexing patterns.
- The pipeline uses 8-bit Canvas ImageData, floors intermediate YUV values, and writes opaque alpha in some paths. VHS-90 must use its native float/16-bit/8-bit AE formats and preserve source alpha.
- JPEG quality is an output/export simulation, not an analog tape effect. Only add compression artifacts if they match a supplied VHS reference.
- The U/V displacement, 4:2:0 chroma averaging, luma/chroma split controls, line shear, interlace pattern and luminance-dependent noise are useful modules for VHS-90's shared Color/Chroma, Signal Distortion and Noise engines.

## VHS-90 integration targets
Port the independent Y/U/V displacement and 2x2 chroma averaging into a shared chroma-resolution/bleed module; port the luma/chroma color math and posterization as optional color degradation; adapt row shear/interlace to deterministic frame-seeded controls; and reuse the convolution kernels only as optional detail/edge variants. Do not import the browser app, server preset API, watermarks or JPEG export pipeline.
