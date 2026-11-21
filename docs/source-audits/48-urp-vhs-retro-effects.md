# Source audit 48 — URP-VHS-Retro-Effects-main

## Source location and scope

- Archive: user Library `vhs.zip`
- Folder: `vhs/github repos/URP-VHS-Retro-Effects-main/`
- Upstream: `nscTechArt/URP-VHS-Retro-Effects`
- Target: `vamprieieuu/vhs---90`
- License: MIT (2024 nscTechArt), retained under `ThirdParty/URP-VHS-Retro-Effects/LICENSE`
- Archive inventory: 291 entries total; 144 Unity .meta files; 147 substantive files; archive integrity test passed.

## Fast content audit

Inspected all 4 C# scripts and the full 343-line HLSL shader. The project is Unity 2022.3 / URP 14. The remaining files are Unity sample-scene content: textures, FBX models, materials, prefabs, scenes, pipeline settings, presets, and editor metadata. They were inventoried by path/type and classified as host/sample assets, not portable VHS engines.

### Source code findings

1. `Retro.cs`: volume controls for bleed radius/direction/intensity, smear, edge enhancement, tape noise amount/speed/alpha, interlacing, scanline speed/frequency/intensity.
2. `RetroRenderFeature.cs`: Unity ScriptableRendererFeature registration and camera/volume gating. Host-specific; not portable.
3. `RetroRenderPass.cs`: dual-blur pyramid, two directional smear passes, compositing, and temporary render-target lifetime. The Unity command-buffer/RTHandle implementation is not portable to AE.
4. `Retro.shader`: four-tap dual blur downsample, alpha-weighted upsample, two fixed-kernel horizontal smear passes, YCbCr chroma bleed, edge sharpening, multi-scale animated line noise, interlace offset, vertical wrap jitter, and scanlines.

## Integration decisions

### Integrated

The multi-scale tape-noise signal is materially distinct from VHS-90's existing Gaussian luma/chroma/composite noise and random dropout engines. Its three smooth one-dimensional noise bands are time-shifted and multiplied by a deterministic spatial mask to form intermittent horizontal noise. It is ported into the existing `VHS90AnalogNoiseEngine` rather than creating a duplicate standalone engine. New `AnalogNoiseParams` fields carry amount, speed, and frame rate. A new AE Signal control, `Tape Noise (Multi-Scale)`, supplies amount and is appended after the Tape Material topic so parameter index order matches registration order.

### Already covered; no duplicate engine added

- YCbCr bleed: VHS-90 already provides chroma subsampling, vertical chroma blending, YIQ bandwidth filtering, and full waveform/composite paths.
- Directional smear: existing LumaSmear engine.
- Edge sharpening: existing ImageGrade edge enhancement and Ringing engines.
- Interlace: existing AnalogGeometry field weave/interlace implementation.
- Scanlines: existing AnalogGeometry Scanline engine.

### Excluded as host/sample-specific

Unity renderer feature, volume component, ScriptableRenderPass, RTHandle and CommandBuffer orchestration; sample scenes; FBX assets; TIFF textures; materials/prefabs; generic camera controller; editor metadata and default Unity presets. These do not provide portable VHS signal algorithms for the native AE plugin.

## Test coverage

Extended `VHS90AnalogNoiseEngineSmoke` to verify:
- deterministic same-frame tape-noise output;
- temporal change between adjacent frames;
- non-neutral output when enabled;
- exact source passthrough when the new amount is zero.

Linux core CI run 613 passed all 28/28 smoke tests after the integration and project-status update: https://github.com/vamprieieuu/vhs---90/actions/runs/44783681639. Linux core CI does not validate Windows/MSVC or loading the AEX in After Effects.

## Disposition

The source folder is fast-audited and its portable novel signal component is integrated. No source engines were copied as disconnected duplicates. Upstream code is MIT-licensed; attribution and license are preserved.
