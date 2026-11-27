# VHS-90 Phase 2 — Signal Engine Audit

Date: 2026-11-27
Branch: `phase2-signal-engine-audit-2026-11-27`
Base: Phase 1 branch `phase1-ui-signal-rebuild-2026-11-27`

## Scope

Audit the parameter-to-engine-to-pixel path for RF Drift, RF Hum, AM Nonlinearity, AGC Pump, Impulse Noise, Tape Dropouts, Tape Material, Analog Glow, Chroma Subsample and Vertical Chroma. This is an implementation checkpoint, not a visual-fidelity completion claim.

## Confirmed render wiring

- The OSD is composited into the working image before the composite/VHS signal stages.
- When the raw NTSC waveform path is selected, RF drift, RF hum, AM nonlinearity, AGC pump and impulse noise are passed into the waveform effects structure.
- In the image-domain composite path, the same RF controls are routed through `RfInstabilityParams`; impulse noise and dropouts are separate processing stages.
- Mains Hum is routed through `PhysicalInterferenceParams`, independently from the RF Hum control. The physical engine explicitly permits Mains Hum with zero Ground Loop input.
- Tape Material is created from the unified `TapeMaterialControls` and processed before composite decoding; the material engine models wear, dropouts, scratches, creases, edge damage, mold, vaseline and sticky/debris behavior.
- Analog Glow and Bloom are separate post-decode stages. Glow is thresholded and spatially blurred; Bloom remains a separate multi-level path.
- Chroma Subsample and Vertical Chroma Blend are separate stages with independent controls.

## Defect found and corrected

The image-domain RF engine applied a nonzero baseline AM compression curve even when `amNonlinearity == 0`, and a baseline AGC pump contribution even when `agcPump == 0`. Enabling RF Drift or RF Hum could therefore also compress highlights or pump bright areas while those independent controls were disabled.

Correction:
- AM compression gain is now proportional to the AM control, and the compression curve is skipped completely at zero AM.
- AGC pump amplitude is now multiplied by its own control, making it exactly neutral at zero AGC.
- Added a regression test using two flat luma levels under Drift-only processing to ensure the transfer remains affine and does not inherit AM compression.

## Validation still required

- Run the full Linux core suite and Windows x64/MSVC build on this branch.
- Verify each RF control in isolation and inspect the waveform-path implementation separately from the image-domain path.
- Audit impulse-noise statistics, dropout density/shape and alpha behavior at multiple resolutions.
- Trace each Tape Material UI control through `makeTapeSurfaceParams` and the final pixel stage; check that the control response is monotonic and useful.
- Inspect Analog Glow's horizontal/vertical spread behavior and test against the supplied VHS camera frames.
- Compare Chroma Subsample and Vertical Chroma at SD, HD and non-4:3 frame sizes.
- Actual After Effects GUI playback and reference-frame visual matching remain a separate manual validation gate; CI success alone does not establish visual fidelity.
