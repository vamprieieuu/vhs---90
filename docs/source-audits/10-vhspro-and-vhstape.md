# VHS-90 — VHS Pro and VHS Tape Source Audit

Date: 2026-11-19
Inputs:
- `vhs/plugins/aex/VHSPro.aex`
- `vhs/plugins/c+/VHSPro.aex.c`
- `vhs/plugins/aex/VHSTape.aex`
- `vhs/plugins/c+/VHSTape.aex.h`

## VHSPro — binary and implementation architecture

- The supplied AEX is a Windows x64 PE binary. Its decompiled C output is very large (approximately 919,615 lines / 31.2 MB) and represents a Rust-built plugin rather than original hand-written C++.
- Embedded Rust source-path strings identify a `vhsp-core` crate and a separate noise module.
- GPU/runtime strings show a wgpu-based graphics stack with Vulkan and Direct3D 12 backends, plus DXGI/WGL/GLES-related backend code and adapter/device/queue/pipeline management.
- This confirms the binary contains a modern cross-backend GPU stack. It does **not** prove that every backend is used for VHS processing on every machine, nor that a dedicated Vulkan-only path is mandatory. VHS-90 should remain CPU-first with optional GPU acceleration and a reliable CPU fallback.

## VHSPro — recovered UI controls

The parameter setup function exposes these controls:
- Preset
- Tape Speed: SP / LP / EP
- Tape Wear
- Snow
- Tracking Noise
- Head Switching
- Edge Wave
- Ringing
- Luma Smear

The decompiled setup calls show a 0–100-style slider configuration for these six damage/processing sliders, with a default around 50 in the visible parameter construction. The exact user-facing range and defaults for every slider should be verified against the original plugin UI/runtime before treating the decompiler's constructor arguments as definitive.

## VHSPro — preset list

The preset menu contains:
1. Default
2. Fresh Tape
3. Rental Copy
4. Worn Out
5. Bad Tracking
6. Home Camcorder
7. Color Bleed
8. Broadcast
9. Dubbed Copy
10. Music Video

The menu also includes a non-look placeholder, “Choose Preset...”. VHS-90 should translate the ten actual looks into its own recipe data and use its own UI labels.

## VHSPro — implementation relevance

- Tape speed modes SP/LP/EP are explicitly selectable.
- Tracking Noise and Head Switching are separate controls, not one combined glitch.
- Edge Wave and Ringing are separate controls.
- Snow, Tape Wear and Luma Smear are distinct controls.
- The GPU stack is broad and includes Vulkan and D3D12; this is a useful reference for optional compute acceleration, but should not be imported as a hard runtime dependency.
- The recovered source paths indicate a Rust core and GPU abstraction; the supplied decompiled output is not a reusable C++ source library. Port algorithms selectively after recovering their actual render kernels rather than copying decompiler output wholesale.

## VHSTape — binary metadata and UI strings

The supplied VHSTape AEX is a Windows x64 AE effect. Version metadata reports:
- Product: VHS Tape
- Internal name: VHSTape
- File version: 1.1.0
- Description: simulates recording to and playing back from a VHS tape
- Category: Retro Effects

The binary's recoverable UI strings include:
- Tape
- Tape Speed
- Luma Blur
- Chroma Blur
- Tape Damage
- Noise
- Head Switching
- Tracking Error
- Playback
- Playback choices: Play / Pause / Stop / Fast Forward / Rewind
- On-Screen Display
- Show Tape Speed
- Tape Speed Position
- OSD strings: PLAY and STOP

The archive contains a Ghidra header dump for VHSTape but no matching `.c` decompilation file. The header is principally PE/Windows type declarations and does not reconstruct the effect's parameter callbacks or pixel-processing functions. Therefore the labels above are confirmed from the binary's strings, but exact ranges, defaults, rendering math and playback timing are not yet recovered.

## VHS-90 adaptation decisions

1. Include the VHSPro preset families as distinct VHS-90 recipe references; do not expose original plugin or preset branding.
2. Keep Tape Speed, Tape Wear, Snow, Tracking Noise, Head Switching, Edge Wave, Ringing and Luma Smear as independently controllable shared-engine modules.
3. Add VHS Tape's Playback and OSD concepts to VHS-90's unified Text/OSD system: Play, Pause, Stop, Fast Forward and Rewind states; tape-speed display and adjustable OSD placement. These should be generated procedurally from the bundled font resources, not image overlays.
4. Keep tracking and head-switch regions location-independent in VHS-90. Neither source's control names justify restricting the new engine to the bottom edge.
5. Keep CPU rendering fully supported. GPU acceleration is optional and must not be required for the plugin to load or render.
6. Exact preset parameter values and VHSTape processing behavior remain open reverse-engineering tasks; do not claim those details have been extracted from the available decompilation.

## Verification status

- VHSPro: inspected the parameter-setup region, preset menu, Rust core source-path strings, and GPU backend/runtime strings in the supplied decompilation and binary.
- VHSTape: inspected the binary's version metadata and user-facing strings; the supplied `.h` dump was inventoried, but no `.c` render decompilation was included in the archive.
- Neither plugin was executed in After Effects in this environment.
