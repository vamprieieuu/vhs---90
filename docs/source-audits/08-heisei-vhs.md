# VHS-90 — HEISEI VHS AEX Audit

Date: 2026-11-19
Sources: `plugins/aex/HEISEI VHS.aex`, `plugins/c+/HEISEI VHS.aex.c`, `plugins/c+/HEISEI VHS.aex.h`.

## Binary / host behavior

- Supplied AEX is a Windows PE32+ x86-64 DLL.
- Plugin metadata identifies it as a demo effect.
- Decompiled entry point includes About, Global Setup, Params Setup, Smart Pre-render and Smart Render command paths.
- The effect's parameter setup creates multiple sliders, checkboxes and color controls; it is not a parameterless effect.
- User's observed behavior remains important: applying the effect visibly changes the image, while its UI controls have not behaved reliably in testing. Do not infer that controls are effective just because they are declared.

## Parameter labels recovered from the binary

- Apply Color Cast
- Apply Scanlines
- Chroma Bandwidth (I)
- Chroma Bandwidth (Q)
- Chroma Crosstalk
- Chroma Noise Intensity
- Chroma Shift
- Dropout Count
- Dropout Intensity
- Dropout Max Length
- Head Switching Distortion
- Head Switching Height
- Head Switching Noise
- Jitter Amplitude
- Jitter Frequency
- Luma Bandwidth (Y)
- Luma Noise Intensity
- Scanline Brightness
- Sharpen Intensity
- Sharpen Width

## Signal-engine relevance

- Separate luma and I/Q chroma bandwidth controls.
- Chroma crosstalk and chroma shift.
- Independent luma/chroma noise intensities.
- Dropout count, intensity and maximum length.
- Head-switching distortion, height and noise.
- Jitter amplitude and frequency.
- Scanline brightness and enable.
- Sharpen width and intensity.
- Optional color-cast stage.

These controls map well to VHS-90's unified Signal, Tape Damage, Color & Chroma, and Image Processing modules. Keep each module independently controllable and combine them through presets rather than reproducing the original UI grouping.

## VHS-90 adaptation notes

- Reuse the separate I/Q bandwidth model and independent dropout parameters.
- Treat dropout count/length as event geometry rather than uniform static noise.
- The source exposes head-switching height, but VHS-90 still requires arbitrary vertical position and full-frame expansion, not only a bottom strip.
- Use the effect's visible result as a reference for its overall processing order, but validate individual controls with parameter sweeps before accepting any mapping.
- Preserve source alpha and premultiplied RGB in all native implementations.

## Validation status

- Decompiled parameter setup and entry-point command routing inspected.
- Parameter labels extracted from the supplied binary.
- No runtime test in After Effects was performed here; control responsiveness is not claimed.
