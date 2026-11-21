# URP VHS Retro Effects — VHS-90 Integration Audit

Source: `URP-VHS-Retro-Effects-main` (Unity 2022.3 URP project)
License: MIT, copyright (c) 2024 nscTechArt. No Unity runtime, scene, material, model, texture, or project settings are required by the native VHS-90 plug-in.

## Audit scope

The supplied `vhs.zip` contains 2,919 archive entries overall. The complete `URP-VHS-Retro-Effects-main` subtree was extracted and enumerated: 291 files, including all project assets and metadata. All four C# runtime/example scripts, the Retro shader, README, package manifests, project version, and license were inspected. The example camera controller and Unity scene/material/model/texture assets are not native post-processing algorithms and were not copied into VHS-90.

## Feature-by-feature disposition

| URP feature | VHS-90 destination | Integration decision |
|---|---|---|
| Color Bleed | `processChromaSubsample`, `processVerticalChromaBlend`, composite chroma path | Existing native implementation is more configurable: independent horizontal/vertical subsampling, Gaussian chroma-plane blur, I/Q offsets, alpha-safe processing and vertical field blend. Preserve and use these together; do not create a second Color Bleed effect. |
| Smear | `processLumaSmear` | Existing native implementation uses a configurable multi-tap exponential trail and highlight contribution, beyond the source's two fixed four-tap passes. Keep the richer kernel; its parameters can reproduce shorter/tighter trails. |
| Edge Sharpening | `processImageGrade` | Strengthened in commit `464946e`: replaced the one-dimensional three-sample reference with a normalized isotropic 3x3 Gaussian reference and a small detail dead-zone. This carries the source's blurred-reference/unsharp-mask idea into the existing grade engine without adding a duplicate control. |
| Tape Noises | `processAnalogNoise` | Existing multi-scale tape-line noise uses three correlated spatial scales, animated time, deterministic masking and a thresholded line event. Retain it as the shared native tape-noise module. |
| Interlacing | Composite/temporal field processing and vertical chroma blend | Already represented by the native field-aware pipeline; no parallel Unity-style interlace stage is introduced. |
| Scanlines | Existing scanline/signal path | Already represented by native scanline controls and processing; keep one shared implementation. |
| Film Grain | Analog/composite noise engines | Already covered by luma, chroma and composite noise systems with deterministic seeds and frame control. Do not add a redundant film-grain-only pass. |

## Important source observations

- The shader performs blur-pyramid downsample/upsample for bleed, two horizontal smear passes, edge enhancement against a slightly blurred image, YCbCr conversion, animated tape-line noise, interlacing offset, and animated scanlines.
- Its README specifically notes that RGB/YCbCr conversion assumes gamma-space RGB, and that temporary blur render-target format/blit choice affects correctness. Those Unity render-target details are not copied into the CPU-native AE renderer; the signal-domain intent is retained.
- The shader's YCbCr path is not copied as an independent color model because VHS-90 already uses YIQ/composite encoding/decoding and chroma processing. Maintaining one coherent signal model avoids inconsistent double conversions.
- Unity-only C# renderer-feature registration, volume component plumbing, camera controller, URP package files, sample scenes, materials, models, textures, and Unity metadata are intentionally excluded from the After Effects plug-in.

## Validation

The edge-refinement implementation is committed to the existing `VHS90ImageGradeEngine`. The repository's GitHub Actions Core CI was triggered by that commit; record the run result before treating this audit as fully validated.

## Next source folder

After this source is signed off, continue with `vhs-phys-main` as the next folder in the supplied collection.
