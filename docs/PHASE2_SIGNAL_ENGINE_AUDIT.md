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

## RF Hum isolation defect found and corrected

The image-domain RF path also had small baseline Hum terms in its offset, ripple, interference and gain equations. As a result, enabling Drift/AM could introduce hum-like modulation even with RF Hum at 0%. Each term is now multiplied by the Hum control, making the Hum stage mathematically neutral at zero. Added exact-transfer regression checks on flat gray and white fields under Drift-only operation; the white-field case also guards against hidden AGC pumping at zero.

## Ground Loop-only fast-path defect

The physical-interference neutral fast path previously ignored `groundLoopMillivolts`. A Ground Loop-only configuration therefore returned the untouched frame before its own line-bias contribution could run. The fast-path guard now includes `loopDrive`, and a regression test verifies Ground Loop changes the output when it is the only active physical control.

## Physical Mains Hum / Ground Loop decoupling

A follow-up audit found that Mains Hum was technically active at zero Ground Loop, but any small nonzero Ground Loop voltage multiplied the Mains Hum amplitude by a very small loop-drive factor. Separated the dedicated Mains Hum term from the Ground Loop contribution: the former now responds directly to its own amount, while the latter contributes a smaller independent line-bias/gain term. Added a regression check showing that 1 mV Ground Loop changes the 80% Mains Hum output by less than 5% instead of nearly muting it.

## Tape Material control wiring defect found and corrected

The physical tape-surface engine already implemented pinhole, oxide-flake and surface-roughness behavior, but the After Effects parameter layer did not expose these controls and `makeTapeSurfaceParams` hardcoded all three values to zero. Added three new parameter slots at the end of the parameter list (preserving all existing parameter indices and disk IDs), exposed the controls in a dedicated Tape Surface Detail group, and wired both SmartFX and Legacy render-settings reads through `TapeMaterialControls` into the engine. Added mapping assertions for 75% pinholes, 60% flakes and 80% roughness.

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
