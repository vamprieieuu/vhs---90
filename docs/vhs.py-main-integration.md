# vhs.py-main — VHS-90 integration audit

Source: `vhs/github repos/vhs.py-main` from the supplied `vhs.zip`.
Inventory: 9 files (2 Python, 2 JavaScript, 1 HTML, 1 CSS, 1 Markdown, 1 requirements file, 1 gitignore); source payload 138,272 bytes.

## Source audit

All nine files were inspected. The main processing implementation is `vhs.py` (class `VHS`): YCrCb split, independent horizontal luma/chroma downsample-upsample, Gaussian luma noise, independent chroma noise, per-row horizontal chroma waves, additional frame warp, blur, soft-light high-pass sharpening, and generation multiplier. The Python/Eel/Tk UI, video/audio file I/O, HTML/CSS/JS, and pinned dependency list are desktop application infrastructure, not AE render kernels.

## Reuse decisions

- Ported the distinctive independent luma/chroma horizontal compression and separate noise path into `VHS90PyVhsGenerationEngine`, including deterministic per-frame chroma row-wave displacement.
- Uses an 8-tap Lanczos-4 horizontal resampler, matching the source's OpenCV `INTER_LANCZOS4` choice; makes random noise deterministic for AE multi-frame rendering.
- Keeps alpha intact and supports premultiplied input.
- Existing VHS-90 analog geometry, image grade, tape detail, chroma subsampling, and blur/sharpen-related stages already cover the source's generic warp, blur, edge enhancement, and sharpening concepts; those were not duplicated.
- The source's right-edge green-channel zeroing and zero-width slicing quirks were treated as implementation defects, not reproduced.
- The Python GUI, Eel bridge, Tk dialogs, preview window, audio muxing, and video export are excluded from the native plugin core.

## Integration

The new engine is part of the shared C++ Composite pipeline and exposed as a dedicated **Python VHS Generation** topic in the AE effect UI:
- Luma Compression
- Chroma Compression
- Luma Noise
- Chroma Noise
- Chroma Wave
- Generation

Defaults are neutral/pass-through. Seed and frame are derived deterministically from the effect render context.

## Validation

A dedicated smoke test covers neutral pass-through, active effect, repeatable same-frame output, and alpha preservation. CMake registers the new source and smoke test. Native Windows AE SDK packaging remains separate from host-independent core CI.
