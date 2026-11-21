# VHS-90 — Codes.txt source audit

Date: 2026-11-19
Input: `vhs/codes.txt` from the supplied `vhs.zip`
Status: complete static inspection of this file; native integration is pending the plugin source/build tree.

## File facts

- 1,096 lines; 37,069 characters (UTF-8 decoded).
- 14 occurrences of Shadertoy-style `mainImage`.
- 4 Godot `canvas_item` shader declarations.
- 43 `uniform` declarations.
- The file is a concatenation of shader fragments, not one compilable translation unit. It contains repeated function names, multiple entry points, host-specific uniforms/samplers, and code with unresolved external helpers.

## Algorithm groups found

1. YIQ/RGB conversion and 15-tap directional sampling / channel-separated YIQ blur. The first shader derives Y, I, and Q from progressively offset/blurred samples and applies row-dependent displacement/noise.
2. VHS multi-pass concept documented in comments: reduced horizontal luma/chroma resolution, unsharp/detail recovery, interlacing/noise, warps/white-noise lines, final blur/vignette.
3. Luma/chroma resolution split with bicubic sampling and YIQ reconstruction. Depends on external `maxResLuminance`, `maxResChroma`, `textureBicubic`, and `yiq2rgb` declarations not included in that fragment.
4. Nine-tap horizontal convolution with per-row HSYNC jitter. The supplied kernel is signed and sums to zero; it behaves as a directional derivative/edge response, not a conventional blur.
5. A long time-layered VHS/no-signal shader with frame history-style multi-sampling, row warp, pixelation, RGB separation, quantization, tinted image fragments and static fallback. It references helper functions/resources not declared in the excerpt and therefore cannot be transplanted verbatim as a standalone unit.
6. VHS noise/tear function: tangent-shaped tear displacement, horizontal noise modulation, LOD blur, four-sample left/right color smudge and noise mixing.
7. Resolution scaling / pass-through snippets. These are host/pipeline examples, not independent VHS effects.
8. Godot VCR shader: tape crease, bottom border jitter, switching noise, crease noise, AC beat, color noise and YIQ processing. Parameters include crease smear/intensity/jitter/speed/discoloration, border thickness/jitter and noise intensity.
9. Godot VHS shader: chroma shift/bleed/saturation/tint; per-line wobble; rolling tracking band; head-switch band; grain/static streaks; band-local noise; flicker.
10. Godot wiggle/smear shader: intermittent horizontal/vertical jump, YIQ-separated blur/smear, adjustable sample count.
11. Godot composite VHS shader: wave distortion, vertical jitter, RGB aberration, blur, ghost sample, scanlines, noise, vignette and flicker.

## Conversion/integration decisions

- Preserve each source fragment separately in the source archive/catalogue; do not concatenate them into one C++ file.
- Convert shader operations into independent, stateless CPU kernels and shared sampling primitives. A future optional GPU backend can use equivalent shader kernels, but the native CPU path remains authoritative.
- Replace Shadertoy/Godot host variables (`iChannel*`, `iResolution`, `iTime`, `SCREEN_UV`, `TIME`, `COLOR`) with explicit VHS-90 frame/context/parameter inputs.
- Use deterministic hashes keyed by seed, frame index and pixel/scanline coordinates. Avoid mutable global random state so AE MFR renders remain deterministic and thread-safe.
- Keep alpha separate from RGB processing; do not force alpha to 1.0 as several source fragments do.
- Make tracking-band position/height configurable rather than hard-coded to a rolling band or bottom-only head switch. This aligns with the VHS-90 requirement that Tracking Noise may be placed anywhere and Head Switching can expand to full-frame coverage.
- The source's blur and chroma kernels must be expressed in pixel units and scaled by frame dimensions; preserve signed kernels where they are edge/derivative operators.
- Do not port the source's mouse-driven A/B UI, engine-specific syntax, texture LOD calls, or unresolved helper calls literally. Recreate their image-processing behavior through explicit CPU sampling.

## Important source limitations

- Several fragments depend on missing declarations, buffers, external helper functions or host-specific texture behavior.
- Some code has suspicious or nonportable details (e.g. `result = vec3(luminance, chroma) * yiq2rgb` where `yiq2rgb` appears as a function rather than a value; one kernel indexing expression is not a standard nine-tap indexing scheme; the 15-tap blur uses an unusual `Circle` x-coordinate formula).
- The Godot shaders explicitly output opaque alpha in places. VHS-90 must preserve source alpha instead.
- The pasted text includes snippets that are simple pass-through or resolution helpers; these are catalogued but not promoted to standalone user-facing effects.

## Completion boundary

This audit covers the entire supplied `codes.txt` file. It does not claim that all 14 Shadertoy entry points or all four Godot shader declarations are already ported, compiled, or wired into the After Effects plugin. That requires the VHS-90 native source/build tree and per-kernel equivalence tests. The archive's `VHS6RIFT` tree is the next integration reference.
