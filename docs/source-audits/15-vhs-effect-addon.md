# Source Audit 15 — VHS Effect Add-on 2.0

**Folder:** github repos/VHS_effect_add-on-1.0.0  
**Status:** all 16 project files inventoried; core effects, parameters, presets, OSD, video pipeline, task runner, GUI, CLI and self-test reviewed.

## Why this source is useful
A CPU-first, float32, per-frame VHS engine with a reusable context, cached static layers, deterministic frame-index random streams, OSD text, timestamp progression and five presets. It is not a native AE plug-in, but it is one of the clearest references for turning a broad VHS look into a manageable shared CPU engine.

## Core processing algorithms
- RGB↔YCbCr split; independent horizontal chroma low-pass, vertical chroma blur, chroma downsample/upscale and red/blue chroma registration offsets.
- TBC: slow global drift, high-frequency frame jitter, per-row random offsets and smooth time-varying row wave displacement; optional vertical roll.
- Head-switching band: bottom-anchored region with per-row increasing tear, Gaussian displacement, noise, luma kick and selected-row flicker stripes.
- Tracking band: vertically rolling band with half-cosine envelope, row-wise horizontal displacement, luma attenuation and noise.
- Dropouts: Poisson event count, short 1–2-row horizontal segments, random x/length and white/black values.
- Noise: cached 1/4-resolution luma noise pool, low-frequency coarse noise, decorrelated RGB chroma noise and static low-frequency dirt mask.
- Optical: softness through Gaussian blur blending; bright-pass threshold bloom at half resolution; unsharp high-frequency edge enhancement.
- Color: contrast around mid-grey, brightness, saturation with luma reconstruction and highlight compression, green/sepia tint.
- Scanlines: cached row brightness profile with frame-index phase roll; elliptical vignette.
- OSD: local glyph patch compositing, REC blink with smooth duty cycle, PLAY arrow, SP marker, timestamp, opacity, jitter and chromatic ghost. Text glyphs are cached and only small bounding rectangles are blended.

## Parameters and presets
VHSParams includes chroma bleed/subsample/offset; luma/chroma/edge noise; scanline period/intensity; vignette; global/line jitter and vertical roll; head switch/height; tracking band; dropout amount/count; softness/bloom/sharpen; saturation/contrast/brightness/tints; OSD master/REC/PLAY/timestamp/opacity/glitch/ghost/scale/blink/color; date-time start/offset/mode/speed; seed and max dimension.
Five preset recipes are supplied: light, classic, heavy, worn and clean_tv. VHS-90 will translate their visual parameter combinations into its own preset library; source labels must not appear in the UI.

## OSD findings versus VHS-90 requirements
- Source text uses a dynamic system-font search and Pillow raster glyphs; VHS-90 must use only the bundled font files supplied by the user, keep their original font names, and support Arabic shaping/RTL, Arabic digits and localized month names.
- Source OSD supports REC, PLAY, SP and date/time, but not the full requested PLAY/STOP/TAPE/ERROR/P.O.1/RES/REW/FF word set. VHS-90 will extend the shared OSD engine.
- This source composites OSD after noise, tracking, dropouts and scanlines. That is opposite to the explicit VHS-90 requirement. VHS-90 must render OSD first, merge it with the chosen source (original footage or procedural blue screen), then pass the combined frame through all VHS Damage modules so the text is damaged by the tape signal.
- Date progression is frame-index based and timestamps update by whole seconds; this is useful for preview/render consistency.

## Determinism and alpha limitations
- temporal_rng derives a deterministic NumPy generator from global seed and frame_index. Repeated rendering of the same frame index is identical; adjacent frame indices vary.
- Cached noise pool, row random, scanline profile and vignette reduce per-frame work.
- The API is BGR uint8 and drops alpha. VHS-90 must support AE 8/16/32-bit formats and preserve straight/premultiplied alpha.
- The CPU algorithm is implemented in NumPy/Pillow/OpenCV; port the math to C++ and do not embed Python/OpenCV/Pillow in the AE plug-in.

## Verification performed
- Environment checks passed for Python 3.13.5, NumPy, Pillow, OpenCV and FFmpeg; no hardware encoder was detected.
- Synthetic 720x480 frame tests were run for all five presets. Every preset produced a valid 480x720 uint8 output; repeated calls with the same frame index were byte-identical, and the next frame index changed the result.
- The bundled selftest stopped before media tests because the archive does not contain its expected Material directory. This is an asset-availability limitation, not a failed core-render assertion. No full video-export test is claimed.

## VHS-90 integration targets
Port the deterministic frame-seed model, cached noise pools, anisotropic Y/C bleed, chroma subsampling/registration, TBC row warp, generalized tracking band, generalized head-switching region, Poisson dropouts, multiscale bloom, softness, scanlines/vignette and patch-based OSD compositing. Preserve the user's required OSD-before-damage order and bundled-font-only Arabic-capable text renderer. Replace the source's bottom-only head switch with user-defined position/height/orientation and full-frame expansion.

Do not copy the Python GUI, CLI, directory batch processor, FFmpeg wrapper or Pillow font renderer into the native AE plug-in. No C++ conversion has yet been committed for this repository.
