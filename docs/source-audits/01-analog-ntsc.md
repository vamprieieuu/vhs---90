# Source Audit 01 — Analog-NTSC

**Repository folder:** `github repos/Analog-NTSC-master`  
**Audit state:** complete source review and standalone core smoke test; repository code import is pending.

## Files inspected
- All 7 public headers under `include/rf2/`.
- All 7 common implementation files under `src/common/`.
- Plugin host files `src/plugin/rf2_ntsc.cpp`, `rf2_ntsc.h`, `rf2_ntsc.r`, installer and Info.plist.
- README, API documentation, NTSC notes, NTRF format documentation, Makefile and Windows third-party notes.

## What the source actually implements
The processing model is a signal-domain pipeline, not an overlay stack:

1. Encode a 720x480 RGB24 frame into a synthetic NTSC composite waveform.
2. Apply RF/composite and VHS-related signal impairments to the waveform.
3. Decode the waveform back to RGB.

The core is divided into `NtscEncoder`, `NtscEffects`, `NtscDecoder`, `NtscProcessor`, parameter metadata, and signal timing/level configuration.

### Signal encoder
- 525-line frame timing, 910 samples/line, 4*fsc sampling.
- NTSC sync, blanking, burst, active-picture and field line mapping.
- Luma, I and Q channel bandwidth shaping.
- Chroma subcarrier modulation and configurable chroma modulation scale.
- Filter-pass control.
- Float IRE path avoids the int16 quantization round-trip.

### Signal-domain effects
- Gaussian/pink composite noise and color control.
- Multipath ghosting, delay and per-line ensemble.
- Line-time jitter, AFC hunt and RF drift.
- AM nonlinearity, impulse noise and AGC pump.
- 60 Hz hum, chroma flutter and Y/C crosstalk.
- Horizontal/vertical sync noise and burst noise.
- Frequency-dependent group delay.
- VHS tracking with signed direction, tape wrinkle, head-switch noise and dropouts.
- Deterministic random seed.

### Decoder
- Sync/blanking/burst estimation and recovery.
- Luma/chroma separation, comb-filter and dot-crawl behavior.
- Chroma delay, tint, saturation, brightness, contrast and sharpness.
- H-lock, V-hold and burst-lock instability.
- Overscan reveal and 4:3 mapping in the host wrapper.

## Verified constraints and integration requirements
- Core compilation was verified with GCC C++20 on the six frame-processing implementation files.
- A 720x480 RGB24 smoke harness processed a generated frame successfully and returned exactly 1,036,800 output bytes.
- The public core API is fixed to packed 720x480 RGB24. VHS-90 needs an adapter for arbitrary After Effects dimensions/strides and pixel formats.
- The imported core does not carry AE alpha. The VHS-90 adapter must preserve alpha separately and correctly handle straight/premultiplied alpha.
- The core's deterministic seed is useful for repeatable rendering; frame/time seeding must be explicitly designed for MFR-safe deterministic output.
- The upstream AE/Premiere wrapper is not the VHS-90 host implementation. VHS-90 host integration must use the supplied VHS6RIFT project architecture.
- The upstream Makefile points at Adobe SDK 25.6; that build setup is not adopted as the VHS-90 target build without reconciling the selected AE SDK.
- `ntrf_container.cpp` is a separate recording/container subsystem using Apple Compression APIs and is not part of the per-frame signal engine, so it is not a direct render-core dependency.

## Integration decision
Retain the signal encoder/effects/decoder as an internal composite-signal module in the unified VHS-90 pipeline. Its algorithms can support the shared signal-processing, chroma, sync, tracking, head-switching, dropout, noise and ghosting systems. Do not expose source project names in the VHS-90 UI. Existing shared VHS-90 modules should own common controls and avoid duplicate processing.

## Not yet claimed
This audit does not claim that the source files have been copied into VHS-90 or wired into the AE render callback. That code transfer/adapter step must be completed and verified before it is marked integrated.
