# VHS-90 — Analog-NTSC Native C++ Engine Audit

Date: 2026-11-19
Input: `vhs/github repos/Analog-NTSC-master/` from supplied `vhs.zip`.
Repository files: 30 files, 331,612 bytes. Includes 9 public headers, 9 C++ files, 4 Markdown documents, one Objective-C++ app, resource/installer files, Makefile and BSD-2-Clause license.

## Architecture

This is a genuine native C++20 composite-video engine, not a shader-only look or an image-overlay package. Its core pipeline is:

`RGB 720×480 → composite NTSC encoder → IRE-domain signal effects → composite decoder → RGB 720×480`

Signal geometry:
- 525 total lines per frame.
- 910 samples per line (4× color-subcarrier sampling).
- 477,750 composite samples per frame.
- NTSC-M subcarrier timing uses 227.5 cycles per line.
- 720×480 active RGB source/output.
- IRE levels represent sync tip, blanking, black and white.
- Separate luma and I/Q chroma bandwidth limits are applied during encoding.

The core library is split into encoder, signal effects, decoder, parameter metadata and a convenience processor. A separate NTRF container handles composite waveform plus synchronized audio; the standalone macOS player/encoder are not required by the effect engine.

## Useful source algorithms for VHS-90

### Composite encoder
- Converts RGB into a synthetic composite waveform.
- Applies separate Y, I and Q bandwidth shaping.
- Inserts horizontal sync, vertical broad/equalizing sync, back porch and color burst.
- Uses alternating scanline/field subcarrier phase.
- Builds pseudo-VITC/VITS/VBI data in the blanking region.
- Offers float IRE output to avoid an int16 quantization round-trip.

### Composite decoder
- Estimates blanking and sync levels from the waveform.
- Detects horizontal sync centers and vertical sync anchors.
- Recovers luma and chroma with a 2-line comb filter or simpler bandpass path.
- Supports dot crawl, chroma delay, overscan reveal, sharpness, tint, saturation and brightness/contrast.
- Has independent H-lock, V-hold and burst-lock instability controls.

### Signal effects
- Additive Gaussian composite noise with selectable white-to-pink spectral character.
- Multipath ghosting with sample delay, gain and ensemble reflections.
- Line-time jitter with fractional interpolation.
- AFC hunting and slow RF drift.
- AM nonlinearity and signal intermodulation.
- Impulse spikes, AGC pumping and 60 Hz hum bars.
- Chroma flutter and Y/C crosstalk.
- Independent H-sync, V-sync and colorburst noise.
- Group-delay filtering to create frequency-dependent timing smear.
- VHS tracking, tape wrinkle, head-switch noise and signal dropouts.
- Deterministic seeds.

### VHS-specific behavior and geometry limits

The source explicitly documents its VHS mechanics as approximate visual simulation, not a helical-scan tape-transport model.

- Tracking uses a signed control: positive values anchor a band at the bottom; negative values anchor it at the top. It does not provide arbitrary X/Y placement or multiple user-defined regions.
- Tracking bands include line displacement, sync-region noise, random white dashes, burst corruption and paired-field line behavior.
- Head switching is hard-coded close to the bottom: its switch line is derived from `lines_per_frame - 8`, then a band of roughly 2–30 lines is processed depending on strength. It cannot be moved to an arbitrary vertical location or expanded to the whole image through a dedicated height parameter.
- Wrinkle distortion is a traveling band that moves through the frame over a deterministic cycle.
- Dropouts are signal-loss events rather than a uniform noise overlay.

VHS-90 must not inherit the bottom-only tracking/head-switch geometry. Rebuild those two modules on the shared arbitrary-region engine already specified: custom X/Y/width/height, direction, movement, independent multiple regions and full-frame-capable head switching.

## Parameter inventory

The source defines 40 metadata entries across decoder, effects and encoder categories.

### Decoder
- Brightness: −0.5 to 0.5, default 0
- Contrast: 0.2 to 2, default 1
- Saturation: 0 to 2, default 0.9
- Tint: −45° to 45°, default 0
- Sharpness: 0 to 1, default 0
- Comb Filter: on by default
- Dot Crawl: off by default
- Chroma Delay: −6 to 6 px, default 0
- Overscan Reveal: 0 to 0.25, default 0
- H Lock Instability, V Hold Instability, Burst Lock Instability: each 0 to 1, default 0
- Decoder Random Seed

### Signal effects
- Noise: 0 to 20 IRE, default 0
- Noise Color: 0 to 1, default 0 (white to pink)
- Ghost Gain: 0 to 1; Ghost Delay: 1 to 200 samples; Ghost Ensemble: 0 to 1
- Line Jitter: 0 to 2 samples
- AFC Hunt, RF Drift, AM Nonlinearity, Impulse Noise, AGC Pump, Hum, Chroma Flutter, Y/C Crosstalk: each 0 to 1
- H Sync Noise, V Sync Noise, Burst Noise: each 0 to 16 IRE
- VHS Tracking: −1 to 1
- VHS Wrinkle, VHS Head Switch, VHS Dropouts, Group Delay: each 0 to 1
- Effects Random Seed

### Encoder
- Luma Cutoff: 0.5–6.0 MHz, default 4.2 MHz
- I Cutoff: 0.2–1.5 MHz, default 1.1 MHz
- Q Cutoff: 0.1–0.6 MHz, default 0.45 MHz
- Chroma Mod Scale: 0.5–1.5, default 0.88
- Filter Passes: 1–6, default 3

The source README, API guide and parameter registry have one documentation discrepancy: README and parameter metadata specify Noise up to 20 IRE, while the API guide's field table says 0–10 IRE. The actual plugin UI and `ParamMeta` registry both use 0–20 IRE; use the implementation value as the authoritative range.

## Presets recovered from README and native plugin adapter

The core README lists:
- Custom
- Studio Monitor
- Broadcast RF
- Weak RF
- Good VHS
- Worn VHS
- Damaged VHS

The adapter UI instead exposes:
- Custom
- Clean
- Good RF
- Average RF
- Bad RF
- Good VHS
- Dirty Head VHS
- Damaged Tape VHS

These are two different preset sets in the same repository. VHS-90 should build its own recipe data using the signal values and intent, not copy either list verbatim.

## Native host adapter: critical limitation

The supplied `src/plugin/rf2_ntsc.cpp` is not a ready-made AE renderer for VHS-90:
- It registers BGRA 8-bit pixel format only when the host is Premiere (`appl_id == 'PrMr'`).
- Its Render function immediately returns without processing when the host is not Premiere.
- It converts arbitrary input to a fixed 720×480 RGB24 buffer, processes the frame, then scales back.
- This path discards alpha and has no native 16-bit/32-bit AE world processing.
- It uses thread-local processor and buffers, which is a useful allocation-reuse pattern, but is not by itself proof of AE Multi-Frame Rendering correctness.
- The Makefile's Windows target produces a Premiere `.prm`, and requests AE SDK paths as part of a combined build setup. It should not be treated as proof of an AE-ready `.aex`.

Therefore, port/adapt the core signal engine, not the host adapter. VHS-90 requires a new native AE SDK adapter with correct SmartFX, 8/16/32-bit formats, alpha/premultiplication handling and thread-safe deterministic rendering.

## Local compilation and smoke verification

The six platform-independent core source files were compiled successfully with GCC 14.2 using C++20:
- `ntsc_signal.cpp`
- `ntsc_params.cpp`
- `ntsc_effects.cpp`
- `ntsc_encoder.cpp`
- `ntsc_decoder.cpp`
- `ntsc_processor.cpp`

A standalone 720×480 RGB test frame was processed through `NtscProcessor` with non-zero noise and VHS tracking. The program completed, returned the expected 1,036,800-byte RGB output and printed checksum `122461748`.

This confirms that the six core C++ files compile and that a basic encode/effects/decode call executes in this environment. It does not verify visual accuracy, AE integration, Windows/MSVC compatibility, alpha behavior or performance in After Effects.

## VHS-90 integration decision

This is one of the strongest native source references in the supplied bundle for the composite signal layer. Port the encoder/decoder math and useful signal-domain effects into the VHS-90 C++ core, retaining a scalar CPU reference implementation. Refactor fixed 720×480 geometry to support arbitrary AE dimensions, normalize processing across 8/16/32-bit input, preserve alpha, and replace bottom-only tracking/head-switch regions with VHS-90's general region system. Do not import the Premiere-only adapter, standalone macOS GUI/player or NTRF file/container system into the AEX runtime.
