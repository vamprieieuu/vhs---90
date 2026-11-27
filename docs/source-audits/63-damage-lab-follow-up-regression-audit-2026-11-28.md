# VHS-90 — Damage Lab follow-up regression audit

Date: 2026-11-28  
Branch: `phase3-preset-camera-migration-2026-11-27`

## Scope

This follow-up pass re-opened the native Damage Lab implementation and smoke tests after the Windows x64 build was confirmed successful on commit `e98cd22f3b749520525ba823f039caf6693b5184`. The work below strengthens behavioral coverage and corrects one confirmed duplicate mode behavior. It does not claim After Effects visual validation.

## Changes

### Tracking Noise region controls

- Added independent angle and X/Y motion parameters for Tracking Layers 2, 3 and 4.
- The controls are appended after existing AE parameters so previously serialized parameter indices remain unchanged.
- The values are transferred into each corresponding `DamageRegion` in both the common render-settings path and the active legacy render path.
- Expanded `VHS90RegionEngineSmoke` to assert that rotation, X-only movement and Y-only movement independently change the region mask output.

### Tear & Chroma mode separation

- Confirmed that `LocalizedHeadbleed` and `MultiBandTear` previously traversed the same loop over all supplied bands, producing duplicate behavior when the same multi-band input was supplied.
- `LocalizedHeadbleed` now processes the first enabled, nonzero-intensity band only.
- `MultiBandTear` and `SpectralWithLowerTear` continue to process all enabled bands.
- Expanded `VHS90TearChromaEngineSmoke` to render all five menu modes, verify deterministic repeat output, and assert pairwise-distinct results.

### Procedural Static coverage

- The native static enum and Effect Controls menu both expose 13 modes.
- Expanded `VHS90StaticEngineSmoke` to exercise all 13 indices, verify deterministic output, and confirm each mode changes the source under a deliberately active test configuration.
- Extended the AE parameter-contract validator to keep the 5 Tear & Chroma choices and 13 Static choices synchronized with their visible popups.

### Tape Dropouts coverage

- Expanded `VHS90DropoutEngineSmoke` to verify that horizontal and vertical scale independently change dropout geometry.
- Added an invalid zero-horizontal-scale rejection check.

## Validation

- Linux CTest on commit `cbcb19e756f510c58fee562d5f5bf4ea5951e706`: 33/33 passed, including the new Tracking, Static, Dropout and Tear & Chroma assertions.
- Windows x64/MSVC build and single-file AEX publication on commit `cbcb19e756f510c58fee562d5f5bf4ea5951e706`: successful.
- The subsequent commit `601186f14521a91dd156fc3d55aa15dcd76410d7` adds validator-only checks for the 5/13 menu counts; its CI is the final gate for this follow-up pass.

## Verification boundary / remaining work

- The AEX has not been loaded or rendered inside After Effects in this environment.
- Actual visual calibration of Tracking Noise, Head Switching, Tear Chroma, Static, tape-surface defects, Dropouts, RF instability and combined effects remains open until an AE render/capture is available.
- No pixel-perfect claim is made from C++ smoke tests.
- Stage 1 remains open until the AE visual pass is completed; Stage 2 must not be treated as started/closed by this report.


## Additional source-reference implementation: Sync Collapse & Luma Dropout

The source folder `VHS/vhs damage lap/Procedural Pattern Engine/Procedural Static Generator` contains a separate specification for a dedicated Sync Collapse & Luma Dropout engine. The previous generic Sync engine did not implement the full requested three-part system (horizontal sync-collapse bands, luma event pulses, and fine horizontal dropout streaks). A dedicated engine was therefore added rather than overloading the older Sync Collapse effect.

Implemented files:
- `Source/VHS90Core/VHS90SyncCollapseLabEngine.h`
- `Source/VHS90Core/VHS90SyncCollapseLabEngine.cpp`
- `Tests/VHS90SyncCollapseLabEngineSmoke.cpp`

Integration:
- Registered in CMake and called from the main render pipeline.
- Added a visible `Sync Collapse & Luma Dropout` Effect Controls topic with controls for band geometry, luma events, bright/dark event probabilities, keyframed manual pulses, fine dropouts, restrained chromatic tint, animation/input modes, seed, blend-with-input and effect amount.
- Added parameter reads to both SmartFX `ReadSettings` and the active LegacyRender path.
- All new AE parameter IDs are appended after existing parameters; previous serialized parameter indices are not renumbered.
- Generated Signal mode no longer depends on source RGB; source blending is explicit. Original alpha is preserved and premultiplied sampling is handled per shifted channel.
- Horizontal displacement is constrained to the damaged band region, leaving the outer frame geometry stable.

Additional smoke coverage:
- Deterministic output and temporal variation.
- Independent manual bright and dark keyframed pulses.
- Independent automatic bright/dark event probabilities.
- Frozen mode stability across different composition times.
- Generated signal independence from source image values.
- Exact full-input blend identity and disabled/zero-strength bypasses.
- Transparent-pixel and premultiplied-alpha preservation.

Validation:
- Linux CTest on commit `ca98630dd8a8d8552d3681ae6a4f3bf3eee49f6b`: 34/34 passed.
- Windows x64/MSVC build for that commit is still in progress at the time of this note.
- Actual After Effects visual comparison remains pending; compilation and smoke tests do not close the visual-validation gate.
