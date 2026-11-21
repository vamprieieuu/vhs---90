# VHS-90 — BarashNTSC Algorithm Audit

Date: 2026-11-19
Input: `vhs/github repos/BarashNTSC-0.1.2/` from supplied `vhs.zip`.
Inventory: 9 files (3 Python, 2 JSON presets, README, requirements, license and icon-generation helper) plus one PNG icon.

## Project shape

- Python/OpenCV/NumPy/Tkinter desktop processor, not an AE plugin or native C++ library.
- Handles still images and video, live preview, audio playback, multi-worker export and JSON preset import/export.
- README describes the look as VHS/NTSC-inspired; it is not a sample-accurate composite encoder/decoder.
- Dependencies: opencv-python, numpy, Pillow, sounddevice.
- License file grants permissive MIT-style use with notice retention.

## Processing pipeline recovered from VHSProcessor.process

1. Normalize input to BGR; grayscale becomes BGR and BGRA input is converted to BGR (alpha is discarded).
2. Convert BGR to YCrCb and split luma/chroma.
3. Reduce horizontal luma detail according to the Tape detail slider, then resize back.
4. Apply horizontal-edge enhancement to luma.
5. Tape fade reduces luma, lifts black, and reduces chroma saturation.
6. Reduce chroma resolution in both axes using Color Bleed; resize back linearly.
7. Add noise to both chroma planes.
8. Shift Cr and Cb in opposite horizontal directions for chroma separation.
9. Generate composite/rainbow-like artifacts by multiplying horizontal luma high-frequency detail with two phase-shifted sine carriers; carrier phase depends on frame index modulo four and scanline parity.
10. Apply frame-time-dependent chroma phase rotation.
11. Apply contrast, brightness and saturation.
12. Add low-resolution upscaled luma noise.
13. Tape Wear creates random short bright streaks and occasional darkened short segments.
14. Convert YCrCb back to BGR.
15. Build per-row X/Y displacement from sinusoidal jitter and random smoothed line noise.
16. Tracking creates a moving triangular vertical band, adding line displacement, vertical wobble and noise within the band.
17. Interlace alternates row/field displacement by frame parity.
18. Apply the resulting remap.
19. Add tracking-band noise.
20. Motion ghosting blends the current image with the previous processed frame.
21. Optional scanline darkening, vignette and temporal flicker.

## Parameter map

| Parameter | Range | Default | Meaning |
|---|---:|---:|---|
| Tape detail (horizontal) | 10–100 | 45 | Horizontal luma resolution |
| Video noise | 0–100 | 12 | Chroma and luma noise |
| Color bleed | 0–20 | 12 | Chroma downsample amount |
| Chroma shift | 0–20 | 0 | Opposing Cr/Cb horizontal shift |
| Color phase instability | 0–50 | 4 | Frame-varying chroma phase |
| Composite artifacts (rainbow) | 0–100 | 18 | Luma detail modulated into chroma carriers |
| Edge enhancement | 0–100 | 25 | Horizontal luma high-frequency enhancement |
| Tape fade | 0–100 | 15 | Luma lift/darken and saturation reduction |
| Motion ghost | 0–100 | 15 | Previous-frame blend |
| Interlace artifacts | 0–100 | 10 | Alternating field displacement |
| Tracking error | 0–100 | 12 | Moving band displacement/noise |
| Line jitter | 0–100 | 8 | Per-row sinusoidal/random displacement |
| Tape wear / dropouts | 0–100 | 12 | Random short bright/dark signal streaks |
| Flicker | 0–100 | 5 | Temporal brightness variation |
| CRT scanlines | 0–100 | 0 | Alternating row darkening |
| Vignette | 0–100 | 6 | Radial edge darkening |
| Saturation | 0–200 | 105 | Chroma gain |
| Contrast | 0–200 | 100 | Luma contrast |
| Brightness | −100–100 | 0 | Luma offset |

## Preset samples

Two JSON presets are included:
- `example_ANALOG`: detail 39, noise 74, color bleed 20, color phase 4, composite 18, ghosting 15, interlace 10, tracking 12, jitter 8, wear 12, flicker 5, vignette 6, saturation 105, contrast 129.
- `worn_tape_90s`: detail 35, noise 25, color bleed 15, chroma shift 3, phase 10, composite 25, ghosting 20, interlace 15, tracking 30, jitter 12, wear 35, flicker 10, vignette 8, edge enhancement 30, fade 25.

The JSON schema is `{app, type, version, name, params}`; import clamps each parameter to the declared UI range.

## VHS-90 reuse opportunities

- Opposing chroma-plane shifts.
- Chroma resolution reduction independent of luma.
- Frame-indexed two-phase carrier/rainbow artifacts.
- Horizontal detail enhancement separate from blur.
- Short streak/dropout event shapes.
- Moving band mask with smoothed edges.
- Alternating-field displacement.
- Temporal previous-frame ghosting.
- Scanline/flicker/vignette modules.

These are visual-domain approximations and should remain distinct from the real composite encode/decode engine. The carrier modulation in this project is a simplified YCrCb sine-carrier trick, not full NTSC waveform modulation.

## Limitations and porting corrections

- Alpha is explicitly discarded when BGRA input is converted to BGR. VHS-90 must preserve alpha.
- Random generation uses a mutable `np.random.default_rng()` initialized without a user seed. Despite a `frame_idx` argument, all noise/dropout patterns are not reproducible from frame index alone. VHS-90 should use deterministic hash-based events keyed by effect seed, frame, row and pixel.
- The tracking band moves vertically through the frame but its horizontal coverage is the full row; it is not a general user-positioned region system.
- Tape wear's random short streaks are image-domain approximations, not magnetic tape dropout simulation.
- Temporal ghosting depends on previous rendered frames and can be problematic for random-access rendering/MFR unless history is explicitly managed. VHS-90 should use time-indexed history or a stateless trail approximation.
- UI, playback, multiprocessing, FFmpeg export and audio code are not suitable for the native AE plugin. Port only selected pixel algorithms.
- No Arabic/text OSD engine or font support is present.

## Verification

- All Python processing functions, parameter definitions, import/export preset logic and both preset JSON files were inspected.
- No dependency installation or GUI run was performed.
- No C++ port is claimed from this audit.
