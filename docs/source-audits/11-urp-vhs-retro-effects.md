# Source Audit 11 — URP VHS Retro Effects

**Folder:** github repos/URP-VHS-Retro-Effects-main  
**Status:** full Unity project inventory; effect runtime, render pass and shader reviewed.

## Project and code inspected
The Unity 2022.3 URP project includes the complete sample scene, example models/materials/textures, render-pipeline settings, volume profile, C# scripts, Retro Render Feature and its shader. The actual effect implementation is concentrated in Retro.cs, RetroRenderFeature.cs, RetroRenderPass.cs and Retro.shader; the remaining Unity assets configure the sample environment and are not VHS algorithms.

## Actual effect passes and equations
1. **Dual blur pyramid:** downsample the source through a variable number of half-resolution levels; use four bilinear taps per downsample. Upsample selected levels with alpha-controlled blending. The README documents sRGB render-target requirements and warns that an alternate Blitter copy path can bind the wrong texture in some URP versions.
2. **Directional smear:** two successive horizontal passes; each samples four pixels toward the left, weights each by exponential falloff, and normalizes accumulated energy. First pass falloff 0.3 / step 1 pixel; second falloff 1.2 / step 5 pixels.
3. **Edge sharpening:** subtract a slightly blurred sample from the original and add the high-frequency residual by an adjustable amount.
4. **Color bleed:** convert gamma-space RGB to YCbCr, blur the image, blend only Cb/Cr (shader .gb) toward the blurred chroma, then convert back to RGB. Luma remains comparatively sharp.
5. **Tape noise:** procedural, texture-free multi-scale line noise. Three vertically correlated noise functions are multiplied, then multiplied by a moving hashed 2D mask, cubed and thresholded; alpha controls the additive noise level.
6. **Interlacing offset:** a time-dependent scan-row index is folded modulo 2.75 and converted to a horizontal pixel displacement.
7. **Scanlines:** sinusoidal line pattern with speed, frequency and intensity controls.
8. **Vertical wrap event:** a seeded-by-time random event occasionally shifts the UV vertically by up to 0.3 frame height.

## Controls found
Bleed radius, bleed direction, bleed intensity, smear intensity, edge intensity/distance, tape noise amount/speed/alpha, interlacing amount, scanline speed/frequency/intensity. The original project ranges are source UI ranges only; VHS-90 will use its shared control architecture and extended ranges where needed.

## Porting and limitations
- Shader math can be translated to C++ CPU kernels: blur pyramid, weighted directional smear, high-pass edge sharpening, YCbCr chroma-only blur, procedural correlated noise, interlace displacement and scanline modulation.
- This project uses time globals and shader sampling; VHS-90 must seed noise by frame index and make frame output deterministic for preview/render/MFR.
- The shader returns alpha=1. VHS-90 must preserve input alpha and process premultiplied RGB safely.
- The bleed conversion assumes gamma-encoded RGB, as the README explicitly notes. VHS-90 must account for AE working color space and avoid applying gamma transforms twice.
- No real NTSC composite encoding, VCR tracking, head switching or tape physics is present here. Treat it as an image-space effect source, not the core signal engine.
- Do not bring Unity URP, RenderFeature, Volume UI or sample scene into the native AE plugin.

## VHS-90 integration targets
Use the multi-scale blur pyramid and directional smear as optional shared image-processing modules; use the chroma-only YCbCr blur as one Color Bleed variant; adapt the procedural line-noise mask for selected tape-noise presets; use scanline/interlace math only where it matches the supplied references. Keep these modules inside the unified VHS-90 pipeline and combine them with the physical signal engine rather than presenting a source-specific panel.
