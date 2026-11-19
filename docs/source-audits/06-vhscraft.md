# VHS-90 — VHSCraft Native Binary / Decompilation Audit

Date: 2026-11-19
Sources: `plugins/aex/VHSCraft.aex`, `plugins/c+/VHSCraft.aex.c`, `plugins/c+/VHSCraft.aex.h`.

## Binary and entry point

- Supplied AEX is a Windows PE32+ x86-64 DLL.
- Decompilation exposes the AE `EffectMain` dispatch, parameter setup routine, render routine, per-frame parameter packing and the OpenCL program initialization.
- The plugin has a CPU/host-side rendering path and dynamically loads OpenCL entry points. Its embedded compute kernels are named `vhscraft_cl_encode`, `vhscraft_cl_decode`, `vhscraft_cl_finish`, plus Premiere pixel load/store kernels.
- Two tape styles are explicit: Classic VHS and Cinematic VHS.
- Presets: Standard, Clean, Sharp, Purple Tint, Green Tint, Damaged Tape, Camcorder, Black & White.

## Parameter inventory recovered from parameter setup

The decompiled parameter setup exposes the following labels and IDs (IDs are hexadecimal where shown).

### Top-level
- 1 Preset
- 0x5A Tape Style
- 2 Effect Amount
- 3 Signal Quality
- 4 Noise
- 5 Distortion Amount

### Color & Glow
- 6 Color & Glow (group)
- 7 Input Blur
- 8 Color Bleed
- 9 Glow Size
- 10 Red Glow Size
- 0x0B Green Glow Size
- 0x0C Blue Glow Size
- 0x0D Glow Amount
- 0x0E Tint
- 0x0F Tint Amount
- 0x10 Saturation
- 0x11 Side Borders
- 0x12 Top & Bottom Borders
- 0x13 Use Raw Signal Input
- 0x14 Show Raw Signal

### VHS Details
- 0x15 VHS Details (group; group id appears in the decompiled setup)
- 0x16 Tape Noise
- 0x17 Static Noise
- 0x18 Noise Detail
- 0x19 Color Noise
- 0x1A Bottom Noise Height
- 0x1C Bottom Damage
- 0x1D Damage Amount
- 0x1E Damage Strength
- 0x1F Color Blotches
- 0x20 Blotch Detail
- 0x21 Moving Band Noise
- 0x22 Band Speed
- 0x23 Band Height
- 0x24 Color Recovery
- 0x26 Distortion
- 0x28 Waves
- 0x29 Top Edge Bend
- 0x2A Top Bend Height
- 0x2B Bottom Edge Bend
- 0x31 Scan Lines

### Signal / temporal and edge controls
- 0x32 Sample Count
- 0x33 Horizontal Blur
- 0x34 Edge Clarity
- 0x35 Trail Fade
- 0x36 Ghosting
- 0x38 Color Shift
- 0x39 Color Shift Speed
- 0x3A Left Edge Waves
- 0x3B Left Wave Width
- 0x40 Variation Amount
- 0x41 Variation Speed
- 0x42 Variation Detail
- 0x48 Affect Color Shift

### Final Adjustments
- 0x4A Final Adjustments (group)
- 0x4B Blur
- 0x4C Sharpen Radius
- 0x4D Sharpness
- 0x4E Brightness
- 0x4F Contrast
- 0x51 Color Grading (group)
- 0x52 Style
- 0x53 Amount
- 0x54 Exposure (EV)
- 0x55 Temperature
- 0x56 Tint
- 0x58 Contrast

The IDs above are those recovered from the parameter setup sequence. Some controls are hidden/conditional or have gaps in numbering; the list is not a claim that every control is always visible.

## Actual processing structure observed in embedded OpenCL source

### Classic VHS / composite path

1. Source sampling and optional input blur use bilinear sampling and multi-tap spiral blur.
2. A sharp luma sample is combined with blurred chroma from a separate sample.
3. RGB is transformed to YUV; per-channel bloom/halation uses independent radii and mix.
4. Gamma shaping and tape color noise are applied.
5. Luma and chroma are recombined into a scanline-dependent composite carrier. Carrier phase uses horizontal coordinate, scanline index, time and alternating-line phase.
6. Decode applies modulation masks, top-of-frame flagging, PLL instability, micro/large warps, vertical-sync error, head-switch tearing, tracking displacement, noise, chroma splotches and phase changes.
7. Finish applies vertical shake, blur, four-neighbour sharpening and optional color grading.

### Cinematic VHS path

The Cinematic path is a separate RGB-domain renderer rather than the same composite carrier chain with a different tint. It includes lens curvature, scanline drift, a narrow moving tracking band, box-softened RGB, chroma channel displacement, echo, block-residual damage, highlight-only scatter, grain, line noise, bottom-edge noise/dropouts, vignette and color/contrast controls. The source explicitly restores original alpha and clears RGB where source alpha is zero in this path.

## VHS-90 adaptation requirements

- Preserve the separation between the composite-signal path and the cinematic RGB-domain path; do not collapse them into one generic effect.
- Port the actual carrier encode/decode math, scanline phase, PLL instability, chroma smear, channel-specific halation, noise and edge processing into native C++ CPU kernels first.
- Keep OpenCL/Vulkan/GPU as optional acceleration only; the plugin must work without a powerful GPU.
- The current source's bottom-noise/head-switching controls are bottom-anchored. Replace them with VHS-90's arbitrary-position region engine, supporting X/Y/width/height, direction, custom placement and full-frame coverage.
- The classic composite path must be audited for alpha behavior before reuse: intermediate YUV code sets alpha to opaque in parts of the pipeline. VHS-90 must preserve source alpha and premultiplication across all paths.
- Keep original labels and preset names out of VHS-90's UI; translate the useful parameter behavior into VHS-90's unified categories and preset recipes.

## Verification status

- Parameter setup and rendering functions were inspected in the supplied decompilation.
- Embedded OpenCL source strings were recovered from the supplied AEX and inspected for stage and kernel names.
- No AEX was executed in After Effects here; no claim is made that the decompiled C output is directly compilable source.
