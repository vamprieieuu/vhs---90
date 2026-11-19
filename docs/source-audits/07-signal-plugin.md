# VHS-90 — Signal AEX Audit

Date: 2026-11-19
Sources: `plugins/aex/signal.aex`, `plugins/c+/signal.aex.c`, `plugins/c+/signal.aex.h`.

## Binary and recovered parameter interface

- Supplied AEX is Windows PE32+ x86-64.
- Plugin identity strings include `Signal` / `ADBE Signal`.
- The AE entry point dispatches parameter setup and render separately. The decompiled render dispatch reaches a function Ghidra could not fully decompile in this pass; therefore parameter presence is confirmed, but not every render formula is yet reconstructed.

### Core controls recovered from parameter setup

| ID | Label |
|---:|---|
| 1 | Random seed |
| 2 | Signal strength |
| 3 | Signal amplification |
| 4 | Cutoff filter |
| 5 | Luma noise |
| 6 | Chroma noise |
| 7 | Hue Noise |
| 8 | Chroma loss |
| 9 | Sharpen |
| 10 | Device-to-device connections |
| 11 | Emulating VHS (VHS_SP / VHS_LP / VHS_EP) |
| 12 | Destruction button |
| 13 | Luma modulation |
| 14 | Turn On/Off |
| 15 | Frequency |
| 16 | Phase |
| 17 | Lightness |
| 18 | Quantize |
| 19 | Tape Errors |
| 20 | TE start |
| 21 | TE end |
| 22 | TE probability |
| 23 | TE fade |
| 25 | TE gamma |
| 27 | TV distortion |
| 28 | Thick distortion |
| 29 | Fine distotrion (spelling as present in binary strings) |
| 30 | Distortion speed |
| 31 | Roll speed |
| 33 | Experimental parameters |
| 33 | Low-Pass filter in |
| 34 | Low-Pass filter out |
| 35 | Phase shift |
| 36 | Phase shift offset |
| 37 | S-video out |
| 38 | Chroma vertical blend |
| 39 | Sharpen On/Off |
| 41 | Scanlines blur |

Some indices are inferred from adjacent parameter-construction code and may include hidden/experimental rows; preserve source IDs as internal audit references, not as final VHS-90 IDs.

## Recovered processing concepts

- Random seed.
- Signal strength and amplification.
- Input/output low-pass filtering.
- Independent luma, chroma and hue noise.
- Chroma loss and vertical chroma blending.
- Sharpen enable/intensity.
- SP/LP/EP tape modes.
- Destruction preset/button.
- Luma modulation with enable, frequency, phase, lightness and quantization.
- Tape Errors with start/end/probability/fade/gamma controls.
- TV distortion with thick/fine components and speed.
- Roll speed.
- Phase shift and phase offset.
- S-video output option.
- Scanline blur.
- Experimental low-pass input/output controls.

## VHS-90 integration decision

These controls map naturally to the shared VHS-90 signal engine: signal gain/cutoff, luma/chroma/hue noise, chroma loss, tape-speed bandwidth, luma modulation, time-varying tape errors, vertical roll, phase shift, sharpening and scanline filtering. Keep tape errors as an independent module with deterministic seeded event placement. Do not expose a separate “Signal” plugin identity or source-brand names in the final UI.

## Reverse-engineering status and limitation

- Parameter setup routine was inspected and the control labels recovered from decompiled code.
- The decompiled `EffectMain` shows the render callback call, but Ghidra reports the core render function as unable to decompile. Other helper functions exist in the C dump; a complete formula-by-formula reconstruction is not claimed in this checkpoint.
- No execution of the supplied AEX in After Effects was performed here.
