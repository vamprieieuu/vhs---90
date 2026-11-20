# VHS-90 — VHS Damage Lab engine implementation and camera handoff

Date: 2026-11-20  
Related source analysis: docs/source-audits/27-vhs-damage-lab-reference-specs.md, 28-vhs-damage-lab-reference-pack.md and 56-tracking-variants-and-damage-lab-checkpoint.md.

## 1. Tracking Noise: source-specific types, editable presets and simultaneous layers

The native Tracking system now has five distinct renderer types, all named independently of the source plugins:

1. Servo Sweep — source-grounded clustered scanline jitter, servo drift and moving-band behavior.
2. RF Wave — tracking wave and anisotropic in-band noise/snow model.
3. Composite Ribbon — multi-scale row field, fragmented ribbon displacement and chroma damage.
4. Tape Flutter — slower tape slip/flutter behavior; VHSPro-derived but explicitly not claimed as exact shader parity.
5. Four-Edge Crawl — arbitrary horizontal/vertical/rotated edge and interior-region behavior.

The UI now exposes:
- One primary tracking region plus three additional independently enabled Tracking Layers (four simultaneously selectable tracking regions).
- Per-layer Tracking Type and Region Shape.
- Per-layer Amount, Center X/Y, Width, Height, Servo Drift, Displacement, Wave Intensity and Tear Probability.
- A separate Tracking Presets topic with independent preset selection for the base region and each additional layer.
- Eight editable recipes: Custom, Fine Sweep, Fragmented Sweep, RF Snowline, Composite Flagging, Tape Flutter, Four-Edge Crawl and Full-Frame Breakdown.
- Existing independent Head Switching remains a separate fifth region in the same native region stack.

The low-level RegionEngine receives up to five regions per render (four tracking layers plus Head Switching). Each tracking region has its own deterministic seed offset. No source brand/plugin names are used in the new Tracking Type or Tracking Preset choices.

## 2. Tear & Chroma Process

Added VHS90TearChromaEngine as a separate native C++ module and connected it to the AE render path.

Five selectable modes:
- Spectral Chroma Smear
- Localized Headbleed
- Chroma Streaks & Interleave
- Multi-Band Tear
- Spectral + Lower Tear

Controls cover master amount, spectral smear, hue spread, chroma saturation, spectral center/coverage, band start/height/count, horizontal displacement, fragmentation, chroma shift, smear, dropout and seed.

The native API accepts up to five independent TearBand structures. The AE UI now exposes a primary band plus four additional independently adjustable Tear Bands (2–5), each with its own amount, vertical position, height, horizontal shift, fragmentation, chroma shift, smear and dropout. The band-count control determines how many are active. Spectral Chroma Smear mode does not silently apply tear bands; the combined mode explicitly enables both independent systems.

The renderer is procedural, deterministic for a fixed seed/frame, preserves source alpha and does not use reference stills or video overlays.

## 3. Particle Pattern Engine

Added VHS90ParticlePatternEngine and integrated it into the AE render path.

Five pattern modes:
- Dark-Field Speckles
- Horizontal Fragments
- Colored Speckles
- Bottom-Weighted Streaks
- Vertical Columns

Independent controls include amount, density, particle size, streak length, distribution (uniform/top/bottom/center/edges), chroma, animation speed, seed, original-footage mix, brightness and contrast.

Density controls particle event count; size controls particle footprint and streak length controls geometry. The generated field can be used on a dark base or mixed with the source image. The engine preserves alpha/premultiplied-alpha behavior and is deterministic at a fixed seed/frame.

## 4. Procedural Static / Pattern Engine

Extended the existing seven-mode Static engine to eleven selectable procedural pattern identities, matching the 11 Damage Lab Procedural Pattern Engine targets:

1. Dark-Field Speckles
2. Full-Frame Horizontal
3. Top Strip
4. Bottom-Weighted Streaks
5. Monochrome Streaks
6. Split-Band Chromatic
7. Static to Blackout
8. Dark-Field Luma Streaks
9. Multi-Phase Green Line
10. Two-Stage Angular Ribbon
11. Diagonal Sync Collapse

Added a Pattern Type popup plus angle, ribbon/diagonal width, green-line intensity and phase-count controls. The new modes are generated mathematically; they are not static overlays.

## 5. Implementation and validation

Key implementation commits:
- Tracking profile types: 295d1ad, fb53a8c, 3969322.
- AE profile selector: de5d815.
- Multi-layer Tracking UI/render stack: 23420d6, 8acebd6, 57a37d0.
- Tracking recipe presets: a886dbe, 1471f22, 13ec312.
- Particle Pattern Engine and AE integration: 7055cb8, 3518358, d1dc803, fe199b2, 1380340, 50dcf39, a52e06d.
- Tear & Chroma engine and AE integration: 3316703, 1f57ae5, b233cce, 263d650, dded9d3, 8a9dbe9, c2bafce.
- Independent Tear Bands 2–5: 9b3389e, 8462857.
- Four advanced Static pattern modes and UI: 7998c73, caf6d49, bf99fd2, 9396dd4, 7480475, 7188252.
- Added deterministic tests for five tracking renderers, simultaneous tracking layers, independent tear bands, particle density/size/animation/alpha and the eleven static patterns.

CI run for the latest multi-layer tracking stack test: 44819741465. Check its final result before recording the final test count. Earlier milestones passed:
- Tracking profile, AE selector and initial core suite: 44818220252 (32/32).
- Tracking multi-layer and recipe parameter wiring: 44819073465 (32/32).
- Particle and Tear core modules were separately built and tested in the successful CMake runs 44818676422 and 44818766761.
- Static pattern UI and 11-mode tests: 44819301125.

## 6. Verification boundary

- Imported parameter validator checks ParamIndex, ParamDiskID and PF_ADD registration order, and verifies SmartFX reads and render mappings.
- Linux CPU core CI and deterministic smoke tests are the current automated verification.
- Windows/MSVC Adobe SDK compilation, PiPL, AEX loading, After Effects rendering and MFR are not yet verified.
- Visual calibration against the six Damage Lab stills requires running the native AEX in After Effects; passing core tests does not establish pixel-perfect visual parity.
- No reference still/video has been embedded as an effect overlay.

## 7. VHS Cameras handoff

After the latest CI is green, the next folder is vhs/Vhs cams/ in the same vhs.zip archive. It contains six files:
- FXC6: two processed/original split-screen recordings and one note.
- Gen 7: one before/clean clip, one after/processed clip and one note.

Prior first-pass work is recorded in audit reports 26 and 29. The next pass must reopen the original clips and notes, sample/compare frames in sequence, and keep FXC6 and Gen 7 as separate camera looks. The supplied FXC6 comparisons and Gen 7 before clip have no usable audio for clean before/after audio comparison.

**Disposition:** Damage Lab core engine and AE control implementation is now in place. The final automated CI gate is pending; after it passes, move to the deeper VHS Cameras clip analysis. The final Windows AEX/AE visual test remains a separate release gate.
