# VHS-90 — vhs.py Basic Python VHS Processor Audit

Date: 2026-11-19
Input: `vhs/github repos/vhs.py-main/` from supplied `vhs.zip`.
Inventory: 9 files: Python frame processor and Eel desktop bridge, HTML/CSS/JavaScript UI, README and dependency list.

## Processing model

This is a lightweight OpenCV/NumPy VHS-style image processor. It separates luma and chroma through YCrCb, compresses them at different horizontal rates, adds noise, applies row displacement and blur, then recombines them. It is not a full composite RF encode/decode or tape/head physics model.

## Core pipeline

1. Convert BGR image to YCrCb.
2. Luma path: horizontal downscale using Lanczos4, add Gaussian noise, upscale to original width, apply a black right-edge border.
3. Chroma path: downscale YCrCb, add independent random per-channel noise, upscale, apply a black right-edge border.
4. Apply two row-dependent sinusoidal horizontal displacement passes to the chroma image.
5. Merge the luma channel from the luma path with Cr/Cb from the chroma path.
6. Convert back to BGR.
7. Apply a green-channel cut at the right border.
8. The outer pipeline also applies a soft-light high-pass blend, random horizontal switch noise, additional waves, box blur and unsharp mask.

## Controls recovered

- Luma Compression Rate
- Luma Noise Sigma
- Luma Noise Mean
- Chroma Compression Rate
- Chroma Noise Intensity
- Vertical Blur
- Horizontal Blur
- Border Size
- Generation (default 3; multiplies noise mean/sigma/chroma noise)

## Other functions in source

- Gaussian luma noise.
- Independent per-channel chroma noise.
- Saturation adjustment through HSV.
- DFT-based ringing helper.
- YIQ conversion helpers.
- Two sinusoidal row-displacement functions.
- A random switch-noise row displacement.
- Soft-light high-pass sharpening.
- Gaussian/unsharp sharpening.
- Border cropping/black edge.

## Code-quality findings

- Randomness uses global Python `random` and NumPy random calls without an exposed seed; output is not reproducible.
- The `applyRinging` helper calls `self.ringing` with too few positional arguments and would raise a TypeError if invoked. It is not used by the main `processAll` path.
- `cv2.medianBlur(..., 1)` is effectively a no-op.
- `yiq2bgr` is an auxiliary helper and is not used in the primary processing path.
- The main processor accepts BGR only and has no alpha preservation.
- `generation` scales noise parameters; it does not actually re-record and replay the image through multiple generations.
- No temporal history, frame-index seed, field timing or physical tape model.
- The source imports `blendmodes`, which was not installed in this environment; the full Python processing path could not be executed.

## VHS-90 reuse decision

Low priority. Selected algorithms that may be useful as lightweight alternatives:
- Independent luma/chroma horizontal resolution reduction.
- Gaussian luma noise and per-channel chroma noise.
- Row-dependent sine displacement.
- Separate horizontal/vertical blur.
- High-pass soft-light sharpening.

Do not import the Python/Eel UI or treat its generation multiplier as a true multi-generation tape simulation. VHS-90 must use deterministic frame-indexed noise, preserve alpha, and keep the physical composite/tape engine separate from these stylized image-space approximations.
