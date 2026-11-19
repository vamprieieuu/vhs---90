# VHS-90 — Signal AEX Audit

Date: 2026-11-19
Sources: `plugins/aex/signal.aex`, `plugins/c+/signal.aex.c`, `plugins/c+/signal.aex.h`.

## Binary and recovered parameter interface

- Supplied AEX is Windows PE32+ x86-64.
- Plugin identity strings include `Signal` / `ADBE Signal`.
- The AE entry point dispatches parameter setup and render separately. The decompiled render dispatch reaches a function Ghidra could not fully decompile in this pass; therefore parameter presence is confirmed, but not every render formula is yet reconstructed.

### Core controls recovered from parameter setup

The first ten core controls have confirmed sequential IDs 1–10. Later controls are nested/experimental groups and the Ghidra decompilation does not preserve enough of the AE parameter structure to safely assign every numeric ID here. The complete recovered label sequence is:

- Random seed
- Signal strength
- Signal amplification
- Cutoff filter
- Luma noise
- Chroma noise
- Hue Noise
- Chroma loss
- Sharpen
- Device-to-device connections
- Emulating VHS (VHS_SP / VHS_LP / VHS_EP)
- Destruction button
- Luma modulation
- Turn On/Off
- Frequency
- Phase
- Lightness
- Quantize
- Tape Errors
- TE start
- TE end
- TE probability
- TE fade
- TE gamma
- TV distortion
- Thick distortion
- Fine distotrion (spelling as present in binary strings)
- Distortion speed
- Roll speed
- Experimental parameters
- Low-Pass filter in
- Low-Pass filter out
- Off / Lite / Strong
- Phase shift
- Phase shift offset
- S-video out
- Chroma vertical blend
- Sharpen On/Off
- Scanlines blur

Numeric IDs for the remaining nested/experimental rows are intentionally omitted rather than guessed. Preserve the confirmed source IDs internally when the parameter layout is reconstructed from the original binary.

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
