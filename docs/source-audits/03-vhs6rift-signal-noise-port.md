# VHS6RIFT Signal / Procedural Noise — focused port

## Source truth

This port follows the active native implementation in the supplied VHS6RIFT source:
- include/VHS6RIFT.h: Topic 08 declares Enable Procedural Noise, Tape Grain Amount, Grain Particle Size, Luminance Noise Amount, Chroma Noise Amount, Noise Detail / High-Frequency, and Shadow Bias.
- src/VHS6RIFT.cpp: PopulateContext reads those controls and the render loop applies the procedural grain/luma/chroma model using VHS_Hash2D.
- The grain/luma/chroma equations were ported to the VHS-90 CPU core in Source/VHS90Core/VHS90AnalogNoiseEngine.cpp. The implementation is deterministic and uses the per-instance noise seed and frame index.

Some VHS6RIFT documents describe an NTSC-RS composite-noise bridge and other features that do not match the active native C++ renderer. The CMake/source audit explicitly found that the optional bridge is not called by that renderer. This focused port therefore does not claim those stale document-only features were copied as part of the procedural-noise transfer. VHS-90's existing composite-signal noise path remains a separate shared signal-engine function.

## VHS-90 integration

The shared VHS-90 Analog Noise Engine now includes:
- VHS6RIFT-style deterministic particle grain and Grain Particle Size (1–5).
- Independent luminance noise with shadow-weighted response.
- Decorrelated red/blue chroma noise, with the existing VHS-90 hue-bias control retained.
- Noise Detail / High Frequency control.
- Shadow Bias control.
- Enable Procedural Noise toggle and a deterministic Noise Random Seed (1–32767).
- Existing composite noise, chroma loss, scanline chroma phase noise, and tape-line noise remain separate controls in the same engine; they are not duplicated.

All processing preserves source alpha and supports the engine's existing premultiplied-alpha handling. The new work is in the CPU core and the existing SmartFX/legacy parameter-reading route; it does not create a second plugin or expose VHS6RIFT branding in the UI.

## Validation

- Static parameter-order/wiring validator covers all new controls and their render assignments.
- VHS90AnalogNoiseEngineSmoke covers deterministic repeatability, seed changes, enable/disable bypass, noise-detail changes, grain-size changes, and shadow-bias changes.
- GitHub Actions run for the integrated code: https://github.com/vamprieieuu/vhs---90/actions/runs/44803231391
- No Windows/MSVC After Effects SDK build or in-host visual comparison is claimed by this report.
