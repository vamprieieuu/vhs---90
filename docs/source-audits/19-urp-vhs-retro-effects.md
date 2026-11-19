# VHS-90 — URP VHS Retro Effects Shader Audit

Date: 2026-11-19
Input: `vhs/github repos/URP-VHS-Retro-Effects-main/` from supplied `vhs.zip`.
Inventory: 291 files, mainly a Unity 2022.3 URP sample project (144 metadata files, 45 TIFFs, 30 materials, 26 assets, 13 FBX models, scenes/presets and one main full-screen shader).

## Project purpose

A Unity URP renderer feature that applies a stylized VHS/retro post-process. It is not a VHS signal simulator and contains no C++ AE integration. The main useful source is:
- `Assets/_Project/RetroRenderFeature/Runtime/Retro.cs`
- `RetroRenderFeature.cs`
- `RetroRenderPass.cs`
- `Shaders/Retro.shader`

## Exposed controls and ranges

| Control | Range | Default |
|---|---:|---:|
| Enable | on/off | off |
| Color Bleed Radius | 0–1 | 0 |
| Color Bleed Direction | −1–1 | 0 |
| Color Bleed Intensity | 0–2 | 0 |
| Smear Intensity | 0–1 | 0 |
| Edge Intensity | 0–2 | 0 |
| Edge Distance | 0–0.005 | 0 |
| Tape Noise Amount | 0–0.999 | 0 |
| Tape Noise Speed | 0–1 | 0 |
| Tape Noise Alpha | 0–1 | 0 |
| Interlacing Amount | 0–10 | 0 |
| Scanline Speed | 0–0.5 | 0 |
| Scanline Frequency | 20–70 | 20 |
| Scanline Intensity | 0–1 | 0 |

The shader also contains an internal `_BleedAmount` uniform, but the reviewed C# pass does not set it and the visible composite uses `_BleedIntensity`; treat the unused uniform as dead/legacy code.

## Processing stages

### 1. Color bleed / dual blur pyramid
- A multi-level downsample pyramid is created, with 3–8 iterations based on screen width and Bleed Radius.
- Downsampling samples four diagonal positions and averages them.
- The blur pyramid is then partially upsampled with alpha blending.
- The final composite converts source and blurred color to YCbCr and blends the Cb/Cr channels toward the blurred image, leaving luma comparatively sharp.
- Bleed Direction offsets the blur sampling left/right.

This is a useful chroma-only blur strategy. It is not a full NTSC composite encode/decode.

### 2. Luma smear
- Two successive horizontal causal-smear passes are applied to a reduced-resolution texture.
- Each pass samples the current pixel and four previous horizontal samples.
- Exponential falloff differs between the two passes: first pass has a slower falloff, second a sharper falloff and wider sample spacing.
- The final result blends the smear texture back using Smear Intensity.

### 3. Edge sharpening
- The shader subtracts a slightly blurred image from the current image to extract high-frequency edge detail.
- It adds that edge response scaled by Edge Intensity.
- Edge Distance shifts the blur sample horizontally before edge extraction.

### 4. Tape noise
- Three smooth one-dimensional noise functions at different vertical scales create line-shaped variation.
- A time-shifted 2D hash creates a granular noise mask.
- The line noise is multiplied by the mask, thresholded by Tape Noise Amount, then scaled by Tape Noise Alpha.
- This is a stylized line/static pattern, not a magnetic-tape surface or RF snow model.

### 5. Interlacing and scanlines
- Interlacing offsets horizontal UV position by a row/time-dependent amount; it uses a modulo-2.75 pattern rather than a physical NTSC field model.
- Scanlines are a moving sine wave based on vertical UV, speed and frequency.
- Scanline intensity blends the image toward the sine-modulated result.

### 6. Additional temporal UV wrapping
- The final composite applies a periodic vertical UV wrap/jump with a random gate.
- This is a stylized frame displacement, not a modeled vertical-sync oscillator.

## Important limitations

- No actual VHS FM recording/playback, tape geometry, track overlap, head wear, servo, head clogging, or magnetic dropouts.
- No user-positioned Tracking Noise or Head Switching region.
- The final composite returns alpha 1, so source alpha is discarded.
- Effects are rendered through Unity URP ScriptableRendererFeature / ScriptableRenderPass and temporary RenderTextures, not portable to AE as-is.
- The sample project's README warns about color-space dependence: RGB↔YCbCr conversion is intended for gamma-space RGB. VHS-90 must define its own linear/gamma conversion policy and test both.
- The C# pass's blur pyramid uses command-buffer temporary RTs; its notes identify a format mismatch risk with descriptor-based allocation. VHS-90 should use explicit, consistent intermediate formats.
- No CPU implementation or standalone shader package is present beyond this Unity project.

## VHS-90 reuse decision

Port the algorithms, not the Unity framework:
- Chroma-only blur pyramid with independent luma preservation.
- Two-stage causal horizontal smear with separate falloff profiles.
- Edge extraction/sharpening based on a slightly blurred sample.
- Multi-scale line noise multiplied by a time-varying grain mask.
- Adjustable scanline modulation.
- Horizontal displacement as an interlace-style artifact, but replace the 2.75-row UV trick with a real field model in VHS-90.

Keep these as stylized Image Processing / Color & Chroma / Tape Noise modules. Do not present this source as a physically accurate VHS signal engine, and do not reuse its alpha=1 behavior.
