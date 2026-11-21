# URP-VHS-Retro-Effects source disposition

Source archive: user Library `vhs.zip`, path `vhs/github repos/URP-VHS-Retro-Effects-main/`.
Upstream project: `nscTechArt/URP-VHS-Retro-Effects` (Unity 2022.3 / URP 14).
License: MIT; copyright (c) 2024 nscTechArt. Preserve the upstream LICENSE notice when redistributing substantial portions.

## Inspection inventory

The archived folder contains 291 entries (144 Unity .meta sidecars and 147 substantive files). The substantive inventory includes 4 C# files, 1 HLSL shader, 1 Shader Graph, Unity render-pipeline/project configuration, 7 presets, scenes, models, materials, and texture assets. All four C# scripts and the complete 343-line Retro.shader were inspected. The Unity scene, model, material, texture, and editor metadata assets are sample-project content rather than reusable VHS algorithms.

## Useful techniques and integration disposition

| Source feature | VHS-90 disposition |
|---|---|
| Multi-scale animated horizontal TapeNoise | Ported into `VHS90AnalogNoiseEngine`; independently exposed as `Tape Noise (Multi-Scale)` in AE Signal controls |
| Smooth 1D noise at three spatial scales, time-shifted by playback time | Ported with deterministic seeded interpolation; avoids the upstream shader's repeated hash input in one interpolation branch |
| Spatial noise mask | Ported as a deterministic per-pixel mask |
| YCbCr color bleed using a dual-blur pyramid | Not copied as a second blur stack: VHS-90 already has chroma subsampling, vertical chroma blending, YIQ bandwidth, bloom, and waveform processing. The source's Gamma-space RGB/YCbCr note is retained as a calibration warning |
| Two-stage directional smear | Not duplicated: VHS-90 already has a dedicated LumaSmear engine and parameter |
| Edge sharpening | Not duplicated: VHS-90 already has image-grade edge enhancement and ringing controls |
| Interlacing offset | Not duplicated: VHS-90 already has field weave/interlace and geometry engines |
| Scanlines | Already implemented by VHS-90 AnalogGeometry/Scanline engine |
| Unity renderer feature, volume component, render-pass and scene assets | Unity-specific host integration; not portable to the native After Effects C++ SDK |

## Ported implementation

The tape-noise port is integrated through the existing analog-noise stage. It uses the effect's frame index and frame rate, a user-controlled amount, and fixed calibrated speed. It preserves source alpha and premultiplication behavior. The control is appended after the existing Tape Material parameters to keep AE parameter indices aligned with registration order.

## Validation

The new behavior is covered by the existing `VHS90AnalogNoiseEngineSmoke` test: deterministic same-frame output, temporal change on the next frame, visible difference from the neutral image, and exact neutral bypass when the new amount is zero. Full Linux core CI is required before calling the port verified.

## Not copied

The sample workshop scene, FBX models, TIFF textures, materials, prefabs, generic camera controller, Unity editor metadata, and unused Unity presets were inventoried but do not contribute a portable VHS signal-processing engine.
