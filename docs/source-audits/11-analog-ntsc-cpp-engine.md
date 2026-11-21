# VHS-90 — Analog NTSC C++ Engine Audit

Date: 2026-11-19
Input: `vhs/github repos/Analog-NTSC-master/` from the supplied `vhs.zip`
Scope: full 30-file inventory; detailed inspection of the public signal/parameter/effect/encoder/decoder/processor interfaces, README parameter specification, parameter table, processor pipeline, and the VHS/tracking/noise sections of the composite effect implementation. This is a core-engine audit, not a claim that every installer/platform file has been reviewed.

## Repository shape

- 30 files: C++ implementation and headers, a macOS app entry, plugin wrapper/resource files, Makefile, documentation and Windows third-party setup note.
- Core files are `src/common/ntsc_signal.cpp`, `ntsc_encoder.cpp`, `ntsc_effects.cpp`, `ntsc_decoder.cpp`, `ntsc_processor.cpp`, and the corresponding `include/rf2/*.h` interfaces.
- The repository builds Premiere/After Effects plugin formats through its own SDK-dependent Makefile, plus separate encoder/player tools. Its plugin packaging is not the VHS-90 CMake/PiPL base.

## Core pipeline

The processor implements a real signal-domain round trip:

1. Fixed 720×480 RGB input is encoded to a synthetic NTSC composite waveform.
2. Signal-domain effects are applied to the waveform in IRE float units.
3. The composite signal is decoded back to RGB.

The real-time path uses float waveform samples to avoid an intermediate int16 quantization pass. Timing defaults describe 525 lines, 910 samples per line, 67 horizontal-sync samples, 75 back-porch samples, 142 active-start samples and 754 active samples. The signal model includes NTSC sync/blank/black/white/burst levels and field mapping.

This is useful for VHS-90's optional composite/NTSC signal stage. It is not a complete VHS tape-transport model: the repository itself explicitly describes its VHS tracking, wrinkle, head-switch and dropout behaviors as approximations applied to the composite signal.

## Useful signal processing to port

- RGB-to-composite encoding with Y/C bandwidth limitation and chroma subcarrier modulation.
- Separate luma, I and Q cutoff controls; default nominal cutoffs are 4.2 MHz, 1.1 MHz and 0.45 MHz.
- Chroma modulation scale and configurable filter passes.
- Comb-filter decode and optional dot-crawl phase behavior.
- Chroma delay, tint rotation, saturation and post-decode sharpness.
- Composite-domain Gaussian noise, white-to-pink noise color, impulse noise and sync/burst noise.
- Multipath ghosting with sample delay and an ensemble of delayed reflections with alternating gains.
- Line-time jitter, AFC hunt, RF drift, AM nonlinearity, AGC pump, 60 Hz hum, chroma flutter, Y/C crosstalk and frequency-dependent group delay.
- VHS tracking, wrinkle, head-switch and dropout effects applied before decoding.

## Parameter reference

The implementation exposes 39 controls in its parameter metadata table, grouped by decoder, signal effects and encoder configuration.

### Decoder
Brightness, Contrast, Saturation, Tint, Sharpness, Comb Filter, Dot Crawl, Chroma Delay, Overscan Reveal, H Lock Instability, V Hold Instability, Burst Lock Instability and Decoder Seed.

### Signal effects
Noise, Ghost Gain, Ghost Delay, Ghost Ensemble, Line Jitter, AFC Hunt, RF Drift, AM Nonlinearity, Impulse Noise, AGC Pump, Hum, Chroma Flutter, Y/C Crosstalk, H Sync Noise, V Sync Noise, Burst Noise, VHS Tracking, VHS Wrinkle, VHS Head Switch, VHS Dropouts, Effects Seed, Noise Color and Group Delay.

### Encoder
Luma Cutoff, I Cutoff, Q Cutoff, Chroma Mod Scale and Filter Passes.

The README also defines six scenario presets: Studio Monitor, Broadcast RF, Weak RF, Good VHS, Worn VHS and Damaged VHS, plus Custom. Their descriptions are scenario-level guidance; the preset data itself should be reconstructed from the repository's actual preset code/config before transferring exact values.

## Tracking Noise / Head Switching limitations relevant to VHS-90

- Tracking accepts a signed value from -1 to +1. Positive tracking selects a bottom-anchored band; negative tracking selects a top-anchored band. Band height grows with strength, up to roughly 138 scanlines. Its implementation adds per-line noise, horizontal-sync noise, white dash bursts and line displacement.
- This top/bottom direction control is better than a fixed bottom strip, but it still cannot position the tracking band at an arbitrary Y coordinate, set an arbitrary X/width/height rectangle, or manage multiple independent regions.
- Head switching is explicitly bottom anchored: the switch line is calculated from `lines_per_frame - 8` plus a small random jitter; its height grows from a few lines to about 30 lines. It also adds a broad noisy section and white dashes.
- VHS wrinkle is a separate traveling full-frame horizontal disturbance. It moves from top to bottom (then reverses on the next epoch), with a noisy core, halo, edge smearing, line displacement and paired-field treatment.
- Dropouts create random short line segments with white streaks or smeared/repeated-signal-like behavior.

VHS-90 must keep the useful texture/signal mathematics but replace tracking and head-switch placement with the required arbitrary normalized region model. Head switching must be positionable anywhere and grow to full-frame coverage.

## Engineering caveats before integration

- The reference engine is hard-coded to 720×480 RGB24 input and a fixed NTSC raster model. VHS-90 must adapt it to arbitrary AE dimensions, pixel aspect, rowbytes, deep-color formats and render rectangles.
- The processor stores a mutable frame buffer and advances an internal frame index. VHS-90 should use immutable per-render settings and deterministic frame/time seeds so MFR renders do not share mutable temporal state.
- The effect code allocates vectors/scratch buffers inside processing passes. Rework scratch ownership for AE render performance and safe reuse.
- The waveform representation is float IRE, which is useful for a high-quality composite stage but more expensive than direct pixel-domain effects. Keep it optional and avoid forcing all VHS-90 looks through composite encoding.
- The source does not establish VHS tape mechanics; do not label its approximate tracking/wrinkle functions as physically accurate helical-scan simulation.
- Alpha is not part of the documented packed RGB24 input model. VHS-90 must keep alpha in a separate plane and composite/process only RGB with a defined premultiplication policy.

## Integration decision

Port selected C++ algorithms into the shared VHS-90 engine after normalizing dimensions and frame context:
- composite encode/decode and YIQ/Y-C bandwidth behavior;
- sync/burst noise and timing instability;
- multipath/ghost, group delay, hum, chroma flutter and Y/C crosstalk;
- tracking dash/noise textures and wrinkle/dropout event generation as reusable procedural kernels.

Do not port the fixed bottom head-switch geometry or fixed top/bottom tracking anchor. Replace them with VHS-90's arbitrary region engine. Do not import the reference plugin wrapper or installer as the VHS-90 native host architecture.

## Verification status

- Core C++ sources and interfaces were inspected statically from the archive.
- No standalone compilation or Adobe SDK build was run in this pass.
- No After Effects runtime test was performed.
