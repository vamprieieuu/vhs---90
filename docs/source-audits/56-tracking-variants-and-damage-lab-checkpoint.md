# VHS-90 — Tracking variants and VHS Damage Lab integration checkpoint

Date: 2026-11-20  
Input archive: vhs.zip in the Library (152,561,088 bytes; 2,919 archive entries).  
Reference folder: vhs/vhs damage lap/ (archive spelling retained exactly).  
Camera folder: vhs/Vhs cams/.

## 1. User-approved architecture

Keep one VHS-90 effect, one render pipeline and one coherent UI. Share compatible low-level operations, but never collapse distinct source-specific Tracking Noise looks into one generic renderer or merely recolor one effect. Original source/plugin names are provenance labels for internal engineering documentation only; user-facing choices use independent VHS-90 names.

The architecture has three levels:

1. Tracking renderer type — chooses a genuinely different signal-generation model.
2. Tracking preset/recipe — a named parameter configuration for a renderer, editable after selection.
3. Tracking region/layer — independently positioned and shaped damage region. Multiple regions may overlap and use different renderer types.

Tracking, Head Switching, Snow/Static, general signal noise, Luma Noise, Chroma Noise, Tear and Chroma Smear remain distinct effect families. Shared noise/hash/sampling/mask functions may be reused.

## 2. Source-to-renderer evidence map

| Internal evidence source | VHS-90 renderer name | Distinct behavior to preserve | Evidence / confidence |
|---|---|---|---|
| VHS6RIFT native C++ | **Servo Sweep** | Clustered line jitter, separate normal/bad/very-bad severity, servo/alignment instability, one or two moving bands, direction and speed, localized band distortion, snow inside damaged bands. | Active C++ source inspected in src/VHS6RIFT.cpp, especially moving-band geometry and line-by-line jitter (lines 539–632), plus band snow (755–763). High confidence for recovered behavior. |
| NTSC-R / NTCR preset | **RF Wave** | Tracking height, independent tracking-noise intensity, wave intensity, tracking snow intensity and snow anisotropy; animated/timeline-varying RF pattern. | NTSCRT-0.26.0/presets/Tracking Noise up.json inspected. JSON contains explicit tracking_noise_height, tracking_noise_noise_intensity, tracking_noise_wave_intensity, tracking_noise_snow_intensity, and tracking_noise_snow_anisotropy. Medium/high confidence for parameter semantics; shader details require the actual render implementation. |
| Crates VHS Master | **Composite Ribbon** | Multi-scale FBM/value-noise tracking bands, top flagging, head-switch tear, vertical shake, chroma smear, streak/dropout fragments. | Binary metadata and decompilation audit 05. High confidence that these are distinct helpers/stages; exact kernel parameter-to-pixel parity has not yet been established. |
| VHSPro | **Tape Flutter** | Tracking Noise remains independent from Snow, Edge Wave, Head Switching and Tape Wear; preserve seed, tape speed and progressive/interlaced context. | AEX decompilation audit 10 recovered the parameter interface and preset list, but not enough source-level kernel math for an exact reconstruction. This is explicitly a source-inspired behavioral model, not a claim of exact source parity. |
| VHS Damage Lab | **Four-Edge Crawl** | Independent top/bottom/left/right and interior regions; horizontal/vertical/rotated geometry; variable size up to 100% coverage; edge-specific displacement, fragmentation, wave and noise. | Actual IMG_4636, IMG_4654 and IMG_4655 stills plus all four tracking prompt/note documents inspected. Strong visual/spec evidence. |

### Exact source behaviors that must not be conflated

- VHS6RIFT moving band is not identical to its scanline tracking jitter or bottom head-switch snow.
- NTSC-R tracking-noise wave and anisotropic snow are not the same as global Snow or background Noise.
- Crates VHS Master tracking bands and top flagging are not interchangeable with its Cinematic VHS narrow moving band.
- VHSPro exposes separate Tracking Noise, Snow, Edge Wave, Head Switching and Tape Wear controls; do not combine them into one amount slider.
- Damage Lab four-edge region geometry is not a fifth noise texture. It is a spatial placement/region model that can host different tracking renderers.

## 3. Current native implementation

Added TrackingProfile to VHS90RegionEngine with five independently coded renderer branches:

- ServoSweep
- RfWave
- CompositeRibbon
- TapeFlutter
- FourEdgeCrawl

The existing region model retains normalized center, width, height, orientation, motion, servo drift, tear probability, noise density, chroma displacement and deterministic seed/time. The new renderer type changes the displacement field and event structure rather than only changing hue or strength.

Added an AE popup named Tracking Profile with neutral VHS-90 names. Existing shared region controls remain in place. SmartFX and legacy parameter-reading paths both read the selection, and the selected profile is passed to the native region renderer.

The low-level region API already accepts any number of regions and permits overlap; the current AE panel still exposes one editable Tracking Noise region plus the independent Head Switching region. A true multi-layer AE region stack, allowing several different tracking profiles simultaneously with per-layer position/size/seed, remains a required UI/parameter expansion and is not marked complete here.

Implementation commits:
- 295d1adbe849b7857b8d29323f0d2dced1c0ff26 — TrackingProfile enum and region model.
- fb53a8c17ddc1d009ce2a213a38efd7d305d6195 — five renderer branches.
- 3969322aa8d1993256f52a0119298073b377e6e0 — deterministic/distinctness smoke assertions.
- de5d81508e7279492a8c2b9a6738e53fb866e620 — AE Tracking Profile selector and render wiring.
- ef0451d426c5e89e7f50ebe6b062191f725eada1 — static validator requires the selector and mapping.
- e84d51dee740bf190ebad5c2eb2615f85f7dd820 — corrected the profile-distinctness test fixture to use spatially varying RGB input (uniform color cannot reveal displacement differences).

CI for final commit e84d51d passed: imported AE parameter validator, CMake build and all 32/32 CTest smoke tests succeeded in run 44818220252. An earlier smoke run correctly exposed that the first distinctness fixture used a uniform-color source; the fixture was corrected, not waived. Final run: https://github.com/vamprieieuu/vhs---90/actions/runs/44818220252.

## 4. VHS Damage Lab visual inventory reconfirmed

The archive was opened from the Library and the actual stills were extracted and inspected together. The six stills are:

- Tracking noise_/IMG_4636.JPG: blue VCR screen; PLAY/date/time OSD is a separate OSD reference. For Tracking Noise, inspect the four edges only; do not recreate the source watermark or bake the OSD into an overlay.
- Tracking noise_/Spectral Chroma Smear & Lower Tear/IMG_4654.JPG: upper/mid spectral chroma smear plus a separate lower tear occupying about 35–40% of the frame.
- Tracking noise_/Upper Static Band & Cyan-Green Wash/IMG_4655.JPG: dense dark upper static region around 40–45% frame height, separate cyan-green wash and irregular dark patch.
- Tear & Chroma Burst/Dense Chroma Noise Band & Bleed/IMG_4640.JPG: saturated horizontal band with directional chroma bleed below.
- Tear & Chroma Burst/Horizontal Chroma Streaks & Interleave_/IMG_4650.JPG: horizontal chroma streaks/interleave, warm yellow-green base and narrow right-edge strip.
- Tear & Chroma Burst/Headbleed/IMG_4639.JPG: localized central tear with purple/magenta and green/yellow/orange/red chroma bursts.

All source references are analysis-only; none may be rendered as an overlay.

## 5. Damage Lab engines — implementation scope still required

### A. Tear & Chroma Process

Implement a dedicated hybrid engine with independent systems:

1. Spectral Chroma Smear — irregular multi-hue chroma regions, horizontal smear extensions, controlled vertical/diagonal spread, chroma-space processing and luminance-detail preservation.
2. Localized Headbleed — broken horizontal luma tear with localized chroma bursts; keep surrounding image recognizable.
3. Multi-Band Tear — 1–5 independently controlled horizontal bands, each with center, thickness, X displacement, fragmentation, chroma displacement, smear, dropout, flicker and temporal motion.
4. Horizontal Chroma Streaks & Interleave — fine cross-color/interleaved interference and independent narrow right-edge strip.
5. Spectral Chroma + Lower Tear — independently masked upper/mid spectral smear and lower full-width tracking interference; allow controlled boundary interaction.

Reuse RGB channel registration, chroma subsampling, chroma shifts, noise and dropout kernels where appropriate. Do not create one generic glitch renderer.

### B. Particle Pattern Engine

Create an actual procedural particle/fragment renderer, distinct from Static/Snow and from tracking bands. It must support independent density, particle size, length/aspect, orientation, brightness, contrast, monochrome/colored response, spatial distribution, temporal variation, speed, seed and source-vs-generated mix. Density changes event count; size changes footprint; neither should silently alter the other. Support dark-field and source-footage modes, deterministic frame rendering and alpha correctness.

### C. Procedural Pattern Engine

The existing Static engine already has seven modes. The complete Damage Lab reference collection contains 11 distinct targets; do not mark the remaining multi-phase tracking, sync collapse, angular ribbon and diagonal collapse behaviors complete merely because Static modes exist. Add or reuse dedicated signal/geometry controllers for the remaining identities.

## 6. VHS Cameras queue

The archive contains vhs/Vhs cams/ with six files:
- Fxc6: two processed/original split-screen comparison recordings and one note.
- Gen 7: one clean/before clip, one processed/after clip and one note.

Prior reports 26 and 29 already contain first-pass metadata, sampling and visual/audio findings. Before calling the camera folder closed, re-open and analyze all original clips against their notes, preserve Fxc6 and Gen 7 as distinct camera looks, and document any additional frame-by-frame findings. Fxc6 has no audio in the supplied comparisons; Gen 7 original/before clip also has no audio, so no clean before/after audio comparison can be claimed.

## 7. Next work order

1. Await and fix the current Tracking Profile CI result; do not proceed with a red build.
2. Add a genuine multi-layer Tracking Noise stack in AE so different renderer types can coexist, each with its own region and controls.
3. Complete the procedural Particle Pattern Engine and integrate it into the native render path.
4. Complete the Tear & Chroma hybrid engine and integrate its independently controlled masks/bands.
5. Expand the remaining Damage Lab procedural-pattern targets and run the reference-specific smoke/parameter checks.
6. Only then move to the VHS Cameras folder and perform the deeper FXC6 / Gen 7 clip analysis.

This checkpoint is not a claim that all Damage Lab effects are implemented or visually matched. It records the source analysis, the first five tracking renderer models, the remaining implementation gates and the exact camera-folder handoff.
