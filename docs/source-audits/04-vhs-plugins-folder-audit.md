# VHS Plugins archive audit — 2026-11-20

## Inventory

Archive folder: `vhs/plugins/` — 19 files total.

- Seven compiled Windows `.aex` plugin binaries: Crates VHS Master, HEISEI VHS (Demo), Signal, Universe VHS Transition, VHSCraft, VHS Pro, and VHSTape.
- Twelve Ghidra-generated C decompilation/header files for six of those binaries. These are reverse-engineering output, not original maintainable source projects. VHSTape has only the decompiler header in the supplied folder.
- The AEX binaries are useful for feature/parameter discovery, but their proprietary host wrappers, GPU backends, and stripped/decompiled function bodies are not suitable for direct transplantation into the native VHS-90 CPU engine.

## Per-plugin findings and VHS-90 mapping

| Plugin | Source-visible features | VHS-90 decision |
|---|---|---|
| Signal | Random seed, signal strength/amplification, cutoff, separate luma/chroma/hue noise, chroma loss, sharpen, device-to-device connections, SP/LP/EP, luma modulation, tape errors, TV/thick distortion, roll speed, vertical chroma blend, scanline blur | Existing VHS-90 signal/noise/tape/scanline modules already cover most signal-path functions. The Device-to-device connections feature and a distinct stochastic Hue Noise control were not present as standalone renderers, so native CPU implementations were added. |
| HEISEI VHS Demo | Independent luma bandwidth and I/Q chroma bandwidth, chroma crosstalk/shift, jitter frequency/amplitude, head-switching distortion/noise, dropout count/maximum length/intensity, scanlines | Mapped to existing composite bandwidth, receiver instability/crosstalk, geometry, head-switching, dropout, and scanline modules; no duplicate engine added. |
| VHSCraft | Eight style presets; Classic/Cinematic tape styles; signal quality; noise/detail; color bleed; raw signal input/output; tape/static/color noise; bottom-noise region; damage strength; moving band and speed/height; distortion; horizontal jitter; ghosting; color-shift speed; random variation and modulation routing | Existing VHS-90 modules cover tape noise, static, moving bands, tracking regions, geometry/jitter, chroma shift/bleed, ghosting, and tape damage. Its OpenCL/GPU host path is not transplanted; CPU-first VHS-90 modules remain authoritative. The embedded color-grade helper uses a 33³ LUT interface but the archive does not include a standalone maintainable LUT asset/source package to transplant. |
| VHS Pro | Presets: Default, Fresh Tape, Rental Copy, Worn Out, Bad Tracking, Home Camcorder, Color Bleed, Broadcast, Dubbed Copy, Music Video; tracking noise, tape wear, SP/LP/EP; embedded Rust symbols for NTSC, noise, shift, YIQ fielding | Signal/noise/shift/YIQ features substantially overlap the already-audited NTSC-RS core and VHS-90. Preset concepts map to existing style/look presets; no second Rust/GPU engine imported. |
| Universe VHS Transition | Tape-noise type, Color Noise, Bad Tracking, Very Bad Tracking, Blank Tape; tape noise/distortion, random noise, wrinkle distortion/size, chroma blur and X/Y offsets, frame jitter, presets | Mapped to VHS-90 tape surface/crease, chroma, geometry, noise, and tracking engines. Its Red Giant/Universe transition and GPU host integration are not portable to the native AE CPU effect. |
| VHSTape | Tape speed and speed position, luma/chroma blur, chroma shift, tape damage, noise, line jitter, dropouts, tracking error, static, random seed | Existing VHS-90 tape transport, bandwidth, chroma shift, tape surface, dropout, geometry, static, and tracking modules cover the functions. The binary has no accompanying original `.c` implementation in this archive. |
| Crates VHS Master | Three embedded SPIR-V compute passes, global effect/noise/distortion controls, base/chroma blur, bloom radius, and other Premiere Pro GPU effect parameters | The supplied binary is a Premiere Pro GPU plugin and shader binaries are embedded; no source GLSL/SPIR-V project is included. VHS-90's own CPU engines already implement the transferable blur/bloom/noise/distortion functions. No binary shader was copied into the AE CPU plugin. |

## New VHS-90 integration: Signal device network

Added `VHS90DeviceConnectionEngine` as a CPU renderer for animated device-to-device signal links:
- Deterministic seeded node placement with a controllable regular-to-random distribution.
- Animated node drift and signal pulses, based on frame and frame rate (no global mutable state; safe for MFR/out-of-order frames).
- Nearest-neighbour connections with bounded link distance, node size, density, speed, and amount controls.
- Alpha-aware compositing; the original source alpha is retained, including partially transparent pixels.
- Independent per-pixel YIQ Hue Noise from Signal, separate from the existing static Noise Hue phase-bias control and independently usable when procedural grain is disabled.
- UI controls are grouped under Signal Network: Device Connections, Network Density, Node Size, Animation Speed, Distribution, and Network Random Seed.
- Wired through the existing SmartFX and legacy render paths; the module is part of VHS90Core and has its own smoke test.

This is a VHS-90-native adaptation of the Signal plugin's exposed Device-to-device connections feature. The supplied Signal C file is Ghidra decompilation and does not expose a clean, directly reusable original source implementation of that renderer; therefore this port does not claim pixel-identical reproduction of Signal's proprietary connection animation.

## Validation

- New `VHS90DeviceConnectionEngineSmoke` covers deterministic output, frame animation, seed changes, density/distribution changes, zero-strength bypass, and opaque/translucent alpha preservation. AnalogNoiseEngineSmoke also covers independent Signal Hue Noise and its seed determinism.
- GitHub Actions core build and smoke test run: https://github.com/vamprieieuu/vhs---90/actions/runs/44804046170 — passed for the initial network integration. Subsequent alpha/hue-noise changes are being revalidated by the latest Actions run.
- Parameter wiring validator on the latest integration: https://github.com/vamprieieuu/vhs---90/actions/runs/44804069786 — validator passed; core build was still running when this report was written.
- No Windows/MSVC After Effects SDK build or in-host visual comparison is claimed.
