# VHS-90 — VHSPro AEX / Rust-WGPU Audit

Date: 2026-11-19
Sources: `plugins/aex/VHSPro.aex` and `plugins/c+/VHSPro.aex.c` from supplied `vhs.zip`. The archive does not contain a separate `VHSPro.aex.h`; the decompilation includes the referenced header name but not that file.

## Binary and implementation identity

- Supplied AEX is a Windows x64 native module.
- The decompiled code contains Rust runtime symbols and the wgpu graphics stack (including D3D12, Vulkan and OpenGL backend code), rather than a small standalone handwritten Vulkan renderer.
- The AE command table includes Global Setup/Setdown, Params Setup, Smart Pre-render, Smart Render, a separate Smart Render GPU command, external dependency reporting and audio render.
- GPU initialization dynamically enumerates adapters/devices and creates render/compute pipelines. The embedded wgpu runtime has backend error strings for Vulkan, D3D12 and OpenGL; this does not prove every backend is usable on every host.
- The effect therefore has a GPU-capable architecture, but the decompilation alone does not establish whether its complete pixel pipeline falls back to CPU when no suitable GPU is available. VHS-90 must keep a fully working CPU path regardless.

## Parameter labels recovered from Params Setup

The recovered parameter setup creates the following controls:

- Mix
- Seed
- Scale With Video Size
- Field: Progressive / Interlaced
- Sharpen
- Saturation
- Hue Shift
- Color Bleed
- Luma Smear
- Ringing
- Edge Wave
- Head Switching
- Tracking Noise
- Snow
- Tape Wear
- Tape Speed: SP / LP / EP
- Preset
- Open Help in Browser

Preset menu entries:
1. Choose Preset...
2. Default
3. Fresh Tape
4. Rental Copy
5. Worn Out
6. Bad Tracking
7. Home Camcorder
8. Color Bleed
9. Broadcast
10. Dubbed Copy
11. Music Video

The parameter setup encodes slider defaults and ranges as binary floating-point constants. The decompilation does not retain enough named type information to safely reconstruct every slider's exact min/max/default tuple from this function alone; those values are intentionally not guessed in this audit.

## Engine concepts confirmed by this binary

- Distinct controls for luma smear and ringing.
- Separate edge-wave, head-switching, tracking-noise, snow and tape-wear controls.
- Tape-speed selection with SP/LP/EP.
- Progressive/interlaced field selection.
- Scale-with-video-size behavior.
- Sharpen, saturation, hue shift and color bleed controls.
- A seed control and global Mix.
- A preset system containing ten named looks plus the chooser item.

## Technical implications for VHS-90

1. Preserve independent processing modules for luma smear, ringing, edge wave, head switching, tracking noise, snow and tape wear. Do not merge them into a single noise/distortion slider.
2. Use Seed as a deterministic per-frame/per-pixel seed source, while avoiding shared mutable RNG state under AE Multi-Frame Rendering.
3. Implement SP/LP/EP as meaningful bandwidth/chroma-response profiles, not just labels.
4. Progressive/interlaced must affect field timing and scanline processing, with stable output for both modes.
5. Scale With Video Size should scale spatial dimensions and sampling footprints consistently with comp dimensions.
6. The original UI's Head Switching and Tracking Noise labels do not prove arbitrary region placement. VHS-90's requirement remains stronger: both effects need user-controlled location and dimensions anywhere in the image; head switching must expand to full-frame height.
7. Use the GPU path only as an optional acceleration backend. Keep a CPU implementation that works on machines with no discrete GPU, and compare GPU output against the CPU reference.
8. Do not expose the source preset labels verbatim in VHS-90. Convert their parameter combinations into newly named VHS-90 recipes and retain the source-to-recipe mapping internally.

## Source inspection and validation status

- The complete parameter setup function was reviewed, including all control-construction calls and the preset menu string.
- The AEX decompilation was reviewed for AE command dispatch and GPU backend/runtime strings.
- The decompilation is 919,615 lines and includes large amounts of compiler/runtime and graphics-backend code; this pass isolates the effect's exposed interface and runtime architecture, not a full reconstruction of every wgpu runtime function.
- No build of the original AEX, no GPU benchmark, and no After Effects runtime test was performed here.
- No VHSPro code has yet been ported into VHS-90's production C++ render path by this audit.
