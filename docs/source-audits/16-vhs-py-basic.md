# VHS-90 — vhs.py Basic Python Emulator Audit

Date: 2026-11-19
Input: `vhs/github repos/vhs.py-main/`
Scope: all 9 files inventoried; reviewed README, Python VHS class, and web/desktop wrapper entry points.

## Algorithm

The core `VHS` class separates image luma/chroma in YCrCb, then applies independent luma and chroma compression by downsampling and upsampling. It can add Gaussian luma noise, independent per-channel chroma noise, blur, saturation changes, horizontal waves, switch-noise displacement, sharpening and a black border. The processing order includes sharpening, switch noise, Y/Cr/Cb processing, two wave passes, blur and a final sharpen.

The UI wrapper reads video frames with OpenCV and writes processed frames. The README explicitly states that output has no audio and audio must be re-added separately.

## Reuse value

- Basic independent luma/chroma resolution reduction.
- Opposing horizontal chroma wave/shift behavior.
- A simple tape border/edge mask.
- Multi-stage processing order for a deliberately rough low-fidelity look.

## Limitations

- No actual composite encode/decode or sync model.
- Several functions use global nondeterministic Python random generators.
- `processFrame` converts to YCrCb but the separate chroma compression routine resizes the entire YCrCb image before only the chroma planes are reassembled; this introduces redundant luma work.
- Border behavior is fixed to the right edge and includes an additional green-channel zeroing operation.
- Alpha is not preserved as a dedicated channel.
- No region geometry or AE host integration.

## VHS-90 decision

Use only as a low-level reference for separate luma/chroma resolution loss and simple wave texture. Do not port the Python application layer or rely on it for accurate VHS signal behavior. Replace random calls, fixed border assumptions and alpha loss in any native adaptation. No Python runtime render test was run.
