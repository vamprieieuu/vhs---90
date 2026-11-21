# VHS-90 — VHS Damage Lab Reference / Prompt Audit

Date: 2026-11-19
Input: vhs/vhs damage lap/
Scope: all 40 archive entries inventoried; all 20 accompanying TXT/HTML/RTF specifications extracted and reviewed for titles, visual identity, controls, processing order, deterministic timing and AE acceptance criteria; all 19 media references extracted and sampled. This audit consolidates requirements; it does not claim the effects have already been implemented.

## Folder structure and reference inventory

### 1. Procedural Pattern Engine — 11 distinct targets

| Target | Reference | Media facts | Distinct identity |
|---|---|---|---|
| Dark-Field Horizontal Luma Speckles & Streaks | ScreenRecording_11-19-2026 15-34-52_1.mov | 826×440, 60 fps, 59 frames, ~1.047 s; audio present | Monochrome white/gray short horizontal speckles and fragments on dark field, top-weighted density, variable per-frame density; no strong chroma |
| Dark-Field Horizontal Static & Top Strip | ScreenRecording_11-19-2026 15-41-20_1.mov | 826×452, 60 fps, 25 frames, ~1.003 s | Dark monochrome horizontal static with a narrow dense top strip and frame-level density pulses |
| Full-Frame Horizontal Static & Chroma Noise | -590389829563851224.mov | 848×832, 60 fps, 30 frames, ~1.015 s | Dense full-frame horizontal static segments, mostly monochrome with controlled chroma noise and a subtle near-right vertical edge line |
| Horizontal Static To Blackout Cut | ScreenRecording_11-19-2026 15-40-52_1.mov | 826×528, 60 fps, 28 frames, ~1.045 s | Abrupt cut from dense horizontal luma/chroma static to near-black signal state; subtle left-edge line |
| Bottom-Heavy Horizontal Streaks | ScreenRecording_11-19-2026 15-35-12_1.mov | 826×470, 60 fps, 43 frames, ~1.030 s | Variable horizontal streak density increasing toward bottom; upper region relatively sparse |
| Monochrome Horizontal Static & Luma Streaks | ScreenRecording_11-19-2026 15-32-03_1.mov | 626×336, 240 fps, 52 frames, ~1.007 s | Dense monochrome static evolving toward longer horizontal streaks and luma smearing |
| Multi-Phase Tracking & Green Line Static | ScreenRecording_11-19-2026 15-36-24_1.mov | 806×432, 60 fps, 134 frames, ~4.163 s | Multi-phase tracking instability and green-line static, with distinct temporal phases |
| Sync Collapse & Luma Dropout | 797845584241235147.mov | 1280×720, 30 fps, 52 frames, ~1.000 s | Severe horizontal sync collapse, alternating bright/dark bands, abrupt luma pulses and short dropouts; predominantly monochrome |
| Split-Band Chromatic Collapse | glitch cam 2.gif.MP4 | 320×240, 10 fps, 4 frames, 0.400 s | Upper multicolored horizontal static and lower dense vertical RGB streaks/columns with horizontal interruptions |
| Two-Stage Signal Glitch & Angular Ribbon | ScreenRecording_11-19-2026 15-37-05_1.mov | 826×438, 60 fps, 123 frames, ~3.917 s | Two different phases: top overdriven band/dark static, then sharp angular geometric collapse and spectral chroma ribbon |
| Diagonal Sync Collapse | glitch cam 3.gif.MP4 | 320×262, 50/3 fps, 34 frames, 2.040 s | Sudden full-frame diagonal sync collapse, diagonal shear and fragmented multicolor bands, then blue/gray signal state |

### 2. Tear & Chroma Burst — 5 distinct targets

| Target | Reference | Media facts | Distinct identity |
|---|---|---|---|
| Dense Chroma Noise Band & Bleed | IMG_4640.JPG | 808×632 still | Dense saturated horizontal interference band with directional green/yellow/orange/red bleed below it; soft degraded analog base |
| Dense Colored Speckle & Soft Bleed | glitch cam 12.gif.MP4 | 320×250, 50/3 fps, 23 frames, 1.380 s | Fine saturated RGB speckles over entire frame, subtle chroma bleed and soft low-resolution base; not a tracking tear |
| Headbleed / Localized Tear & Chroma Burst | IMG_4639.JPG | 320×180 still | Central/wide horizontal tear with localized purple/magenta and green/yellow/orange/red chroma bursts; surrounding image remains relatively intact |
| Horizontal Chroma Streaks & Interleave | IMG_4650.JPG | 828×560 still | Horizontal chroma streaks, interleaved/cross-color interference, subtle displacement, warm yellow-green cast and narrow bright right-edge strip |
| Multi-Band Tracking Tear & Chroma Smear | glitch cam 5.gif.MP4 | 320×242, 100 fps, 19 frames, 0.850 s | Approximately 1–5 independently moving horizontal tear bands with localized chroma smear, RGB separation, dropouts and flicker |

### 3. Tracking noise — 4 documents / visual references

| Target | Reference | Media facts | Distinct identity |
|---|---|---|---|
| Four-edge Tracking Noise | IMG_4636.JPG | 1000×667 still | Reference for edge-oriented tracking noise; source note requests top, bottom, left and right placement and multiple regions |
| Spectral Chroma Smear & Lower Tear | IMG_4654.JPG | 550×412 still | Broad upper/mid spectral chroma smear plus a separate full-width lower tear covering approximately 35–40% of frame height |
| Upper Static Band & Cyan-Green Wash | IMG_4655.JPG | 639×480 still | Dense upper horizontal interference region (~40–45% height), localized cyan-green wash at lower/mid right, dark irregular patch and restrained dark analog base |
| Advanced Tracking Noise / Snow research | Two research documents plus the ultimate source-integration brief | Text-only specifications | Unified but non-collapsed systems for Tracking Noise, Tracking Damage, Snow/Static, Grain, Signal Noise, Chroma Noise and Luma Noise; edge/multi-region positioning, full-frame coverage, wave intensity and source-inspired presets |

## Non-negotiable architectural conclusions

1. These references are not one generic “tracking/noise” look. Each target has a separate spatial structure, color behavior and temporal pattern. Preserve each identity as an independent preset/recipe, reusing shared low-level kernels only.
2. Static, luma speckles, colored speckles, horizontal streaks, tracking tears, sync collapse, chroma smear, chroma bursts, dropouts, tape defects and blackouts are distinct event classes. Do not merge them into one noise slider.
3. Generated-signal mode is required for references dominated by synthetic static; Apply Over Original / Blend With Original is required for footage-based looks. Keep the original contribution and generated signal intensity independent.
4. Spatial weighting must be normalized to frame coordinates. Top-heavy, bottom-heavy, split-band, upper-band, lower-band, localized burst and multi-band behavior must scale across compositions.
5. Use independent deterministic random streams for each component. The same seed and frame/time must reproduce the same result; enabling animation changes the pattern by frame/time; disabling animation freezes it.
6. Density, size/length, brightness, saturation, opacity and flicker are separate controls. Changing speckle size must not silently change density; increasing density must increase event count, not merely brightness.
7. Event timing must be parameterized or keyframeable, not hardcoded to a reference clip's midpoint or observed timestamp. Abrupt cut behavior should remain abrupt by default where the reference requires it.
8. Avoid recreating stock watermarks, app overlays, text labels or unrelated UI content visible in reference captures.
9. Preserve alpha and premultiplied-alpha correctness, support the plugin's AE pixel formats, MFR/thread safety and arbitrary dimensions. No baked media overlays.

## Engine grouping recommendation for VHS-90

- **Procedural Signal Patterns:** dark-field luma static, full-frame static, colored speckles, top/bottom density weighting, split-band static/columns, static-to-blackout, sync collapse and angular/diagonal signal failure.
- **Tracking & Tear:** four-edge/multi-region tracking, multi-band tracking tears, lower tracking tear, headbleed/central tear, green-line tracking, horizontal displacement and band fragmentation.
- **Chroma Damage:** chroma bleed, spectral smear, localized chroma bursts, interleave/cross-color, RGB displacement and chroma wash.
- **Signal Dropout & Instability:** bright/dark dropouts, luma pulses, sync loss, vertical roll/hold, flicker, signal blackout and frame-repeat events.
- **Shared Base:** softness/resolution, luma/chroma bandwidth, grain, scanline/field treatment, alpha-aware compositing and deterministic event generation.

These are internal engine families. The final UI should remain unified and should not expose a separate panel for every source folder or original plugin.

## Critical Tracking Noise / Head-switch requirements

Tracking Noise must support custom position and direction across the entire frame, including top, bottom, left, right, arbitrary interior placement and multiple simultaneous regions. Each region needs independent position, width/height, coverage, intensity, displacement, edge behavior and noise density. Height/coverage must be able to reach 100% of the image.

Head Switching must have adjustable vertical position, region height and intensity. It must be able to expand upward from the bottom and ultimately cover the full image. This user-facing geometry is mandatory even if physical source algorithms are field-locked or bottom-anchored.

## Source-document inconsistencies to preserve explicitly

- The folder name Procedural Static Generator contains a prompt whose actual engine/preset title is “Sync Collapse & Luma Dropout Engine”. Use the document's actual title, not the folder label, as the semantic identity.
- Split-Band Chromatic Collapse reference is only 4 frames at 10 fps. Its prompt defines the spatial identity clearly, but temporal behavior beyond those four frames is not strongly evidenced; keep its temporal behavior configurable rather than claiming an exact cycle.
- Spectral Chroma Smear & Lower Tear and Upper Static Band & Cyan-Green Wash are still-image references. Their actual temporal animation cannot be inferred from the stills; implement optional deterministic motion, but do not claim it is measured from the source.
- Several media captures include visible stock/watermark or UI text. The prompts explicitly say not to reproduce these; they are not effect features.

## Validation status

- All 20 prompt documents were extracted from their original TXT/HTML/RTF forms.
- All 19 media files were extracted; the video clips were probed and representative frames sampled; the six still images were dimension-checked and visually reviewed in a contact sheet.
- No VHS-90 render was compared against these references yet. This is a reference/specification audit, not a visual-matching acceptance report.