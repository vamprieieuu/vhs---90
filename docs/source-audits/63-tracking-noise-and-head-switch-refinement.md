# Tracking Noise and Head Switching refinement

Date: 2026-11-21

## Scope

This checkpoint records the native C++ changes to `VHS90RegionEngine`, the AE parameter mapping, regression tests, and Windows/Linux CI results. It is not a full visual acceptance report for every VHS-90 effect.

## Tracking Noise

- `Vhs6RiftBand` retains the source-derived VHS6RIFT hash pattern for moving scanline disturbance and band-limited snow.
- `NtscRTracking` retains the NTSC-R row-speckle/transient model and three-octave luma-noise field.
- `VhsMainTracking` adds Gaussian luma disturbance, four-line-correlated horizontal jitter, and chroma attenuation. Its noise field is sampled in moving spatial coordinates so adjacent frames are correlated and the texture visibly travels instead of being independently re-seeded.
- The tracking region continues to expose editable horizontal/vertical/rectangle geometry, center, width, height, angle, X/Y movement, servo drift, tear probability, snow intensity and anisotropy.
- The preset menu exposes 19 choices. The VHS6RIFT and NTSC-R paths reuse available source algorithms. Other named plugin families are source-inspired reconstructions from the supplied binary/UI/source audits; they are not represented as exact recovered source code.

## Head Switching

The menu now exposes eight profiles: Custom, VHS6RIFT, NTSC-R, VHSPro, Crates, VHSCraft, Universe Bad and Universe Very Bad. The engine keeps the same editable region geometry, so the user can place a head-switching disturbance at the top, bottom, left, right or across the full frame by adjusting shape, center and size.

VHS6RIFT and NTSC-R use the corresponding available source-derived models. VHSPro, Crates, VHSCraft and Universe variants are reconstructed from the supplied reference/audit evidence. Their values and visual behavior still require comparison against the exact supplied clips and plugin references.

## Verification

- Linux Core Tests: run `44914282265`, 36/36 smoke tests passed.
- Windows Build: run `44914282254`, SDK/font validation, AE parameter contract, MSVC build, tests, root-level AEX verification and Release publishing all passed.
- AEX output: 867,840 bytes, built from commit `27a7d36963d3c3c7602b2ef833ea5e5c4de23f14`.
- Raw AEX: https://github.com/vamprieieuu/vhs---90/releases/download/vhs90-aex-27a7d36963d3/VHS-90.aex

## Remaining limits

- The combined Grain & Tracking Noise UI group still needs a safe split into independent Grain, Tracking Noise and Tear groups. Parameter disk IDs and saved-project compatibility must be preserved.
- The current CI proves compilation and host-independent smoke tests, not that After Effects loads the AEX or that each effect matches the supplied visual references.
- All reverse-engineered plugin-family presets remain approximate until individually rendered and compared against the original plugin/reference footage.
