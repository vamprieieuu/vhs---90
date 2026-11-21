# Source Audit 13 — Native C++ VHS Kernel / OFX implementation

**Folder:** github repos/vhs-main  
**Status:** all 20 project files inventoried; shared signal kernel, parameter schema, OSD renderer, preset parser, LUT handling, Metal dispatcher and OFX render path reviewed.

## Why this repository matters
This is a native C++ video-effect project with a shared per-pixel VHS kernel. Its signal algorithms are already expressed in a common C++-like kernel source and are shared between a Metal path and a planned CUDA path. It is one of the closest reusable sources for VHS-90's processing engine. However, its current plugin is OpenFX for DaVinci Resolve, not Adobe AE, and its Windows/Linux CUDA functions are stubs: the OFX wrapper reports GPU support but Windows/Linux rendering falls back to passthrough. It is not a CPU-first implementation and must not be copied as the final VHS-90 build.

## Shared processing kernel: actual pipeline
The five-pass processing path is:
1. **Signal generation:** input CST, DI→linear→Rec.709, gamma, BT.601 Y'CbCr conversion; burn OSD into the source before any VHS degradation; composite encode/decode with 3-tap horizontal carrier neighborhood, NTSC/PAL line phase, dot crawl and cross-color; deterministic composite noise; coarse-cell magnetic luma/chroma noise; deterministic dropouts; head-switch noise; tracking-band noise.
2. **Geometric warp:** per-line jitter, exponentially decaying top skew, edge pincushion, moving tracking displacement, chroma channel registration and one-line vertical chroma delay.
3. **Horizontal bandwidth:** separable Gaussian low-pass with independent luma and chroma bandwidth targets; tap radius capped at 33.
4. **Vertical luma bandwidth:** independent Gaussian luma row filtering; skipped when interlace is active to preserve comb/field structure.
5. **Final color/output:** Y'CbCr→Rec.709 gamma RGB; color fade, hue rotation, black lift, white knee, dynamic-range compression, wide multi-ring highlight bloom and veiling glare; convert back to DI/DWG and optional output LUT; preserve alpha.

## Unique algorithm details
- Deterministic hash chain uses a cell seed from x/y/frame/user offset, FNV-like mixing and a final avalanche hash; Box-Muller generates Gaussian noise.
- Composite phase uses 3.77 px/cycle NTSC and 3.05 px/cycle PAL with alternating line phase / PAL switch.
- Magnetic noise is spatially coarsened (2x2 luma cells and 4x4 chroma cells) so it survives later bandwidth filtering.
- Dropouts use block-height-shared decisions, seeded start/length, soft edges and white luma / reduced chroma.
- Tracking is a vertically moving feathered band with per-4-row horizontal displacement, luma noise and chroma attenuation.
- Head-switch noise is added before bandwidth filtering so its pixel structure is softened by the later signal pass.
- Chroma registration samples Cb and Cr at independent horizontal offsets; vertical chroma delay is applied separately from luma.
- Bloom uses a six-ring, 49-tap Gaussian-like pattern, squared highlight extraction and a separate mean-brightness veil term.
- Color transforms explicitly distinguish DaVinci Intermediate, DWG-linear, Rec.709-linear and Rec.709 gamma; the analog stage operates in display-referred Rec.709 gamma and BT.601 Y'CbCr.

## Parameter schema and preset inventory
VhsParams includes dimensions/rowbytes/render scale/frame/seed; NTSC/PAL; horizontal and vertical luma bandwidth; chroma bandwidth; composite strength/dot crawl/cross-color/noise; luma/chroma noise and hue bias; interlace mode/amount/field order; head-switch enable/height/intensity; tracking enable/amount/speed/height/opacity; dropout density/length/height/opacity/softness; line jitter/top skew/decay/pincushion; color fade/hue tint/black lift/white knee/dynamic range; OSD master/elements/positions/glyph/scale/mode/speed/clock/date; input/output CST; chroma registration; mist bloom controls.

Five bundled parameter presets were read in full: VHS Camcorder, Betacam SP, Hi8 Camcorder, Damaged VHS and Blank Slate. The recipes demonstrate meaningful differences in bandwidth, chroma bandwidth, noise, tracking, head-switch, dropout, geometry, color fade, OSD and mist. Their names are internal source references only and must not be exposed in VHS-90's UI.

## OSD implementation
The bundled OSD renderer builds a full-frame grayscale mask using a 5x7 bitmap glyph table and 7-segment digits, supports nine anchor positions, shadow, REC blinking, PLAY/PAUSE/FF/REW/STOP icons, SP/LP/EP, date formats and a free-running timeline clock. The kernel burns this mask into the input before composite/tape damage. VHS-90 will reuse the burn-in ordering and clock/layout concepts, but replace the limited ASCII bitmap glyphs with the user's bundled fonts only, with Arabic shaping/RTL, Arabic digits and Arabic month/date names.

## Host/build and reliability findings
- C++17 OFX plugin uses the OpenFX SDK, not the Adobe AE SDK.
- The real kernel backend is Metal on macOS. CUDA dispatch functions are no-op stubs, and non-Apple output is a passthrough. This is unsuitable as the final VHS-90 engine.
- Metal pipeline uses static global textures and pipeline states; do not copy that global mutable design into an AE MFR plugin.
- The shared kernel is highly parallel per pixel, but the blur/bandwidth passes sample neighboring pixels and require correct edge clamp and render-scale behavior.
- The code stores float RGBA and rowbytes, preserves alpha in the signal texture and final output; VHS-90 still needs correct AE pixel format, premultiplication and rowbytes conversion.
- OSD mask is uploaded separately; VHS-90's text generation must remain vector/font-based and procedural, not image/video overlays.
- LUT parser supports 3D .cube and tetrahedral interpolation; useful for an optional look system but should be reconciled with the user's requested gradient/palette UI.

## VHS-90 implementation decisions
Port the signal equations and pass order into native C++ CPU modules first. Preserve deterministic hashing and coarse-cell noise; expand tracking and head-switching from fixed bottom regions to arbitrary user-defined frame regions, including full-frame height. Keep the OSD burn-in before all VHS degradation. Retain the color-management boundaries only where they make sense for AE's working-space/pixel-format APIs. Build per-instance scratch buffers and deterministic frame-index processing; no static global GPU state. Keep GPU acceleration optional and never required for the core feature set.

Important: this audit does not claim the OFX plugin itself has been integrated into AE or that its GPU kernel has been ported to the VHS-90 CPU engine yet.
