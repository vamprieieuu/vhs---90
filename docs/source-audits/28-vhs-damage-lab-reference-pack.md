# VHS-90 — VHS Damage Lab Reference Pack Audit

Date: 2026-11-19
Input: `vhs/vhs damage lap/` from the supplied `vhs.zip`.
Inventory: 40 files: 21 written implementation prompts, 13 video references and 6 still-image references. The three subfolders are `Procedural Pattern Engine/`, `Tear & Chroma Burst/` and `Tracking noise_/`.

This audit records the actual reference media metadata and the written requirements. The media are visual references only; VHS-90 effects must be generated procedurally and must not use the reference videos/images as runtime overlays.

## 1. Procedural Pattern Engine — 11 prompts / 11 videos

| Reference / requested look | Media | Technical identity |
|---|---|---|
| Dark-Field Horizontal Luma Speckles & Streaks | 826×440, 59 frames, 59 fps, 1.047 s | Predominantly black field; small monochrome horizontal particles and short streaks; higher density toward the top; no large continuous bands or strong chroma. |
| Dark-Field Horizontal Static & Top Strip | 826×452, 25 frames, ~24.92 fps, 1.003 s | Dark field with white/gray fragments; narrow dense top strip; full-frame static distribution with upper density bias; irregular temporal density pulse. |
| Full-Frame Horizontal Static & Chroma Noise | 848×832, 30 frames, ~29.56 fps, 1.015 s | Entire frame covered with thin 1–2 px horizontal segments; grayscale dominant with scattered low/moderate chroma; no broad smears or strong RGB split; subtle right-edge vertical line. |
| Horizontal Static To Blackout Cut | 826×528, 28 frames, ~26.79 fps, 1.045 s | Two-state sequence: dense colored full-frame horizontal static, then abrupt near-black state with a very narrow left-edge line. Measured luma drops from roughly 86–141 in the first frames to ~22 from frame 10 onward (~0.37 s); do not hardcode this exact timestamp. |
| Bottom-Heavy Horizontal Streaks | 826×470, 43 frames, ~41.75 fps, 1.030 s | Dark field; horizontally elongated bright fragments are concentrated toward the bottom; very low upper-frame density. |
| Monochrome Horizontal Static & Luma Streaks | 626×336, 52 frames, ~51.66 fps, 1.007 s | Dense full-frame monochrome static; initially fine short segments, evolving toward longer smoother horizontal streaks and luma smear. Color saturation is effectively zero. The Adobe Stock watermark is not part of the effect. |
| Multi-Phase Tracking & Green Line Static | 806×432, 134 frames, ~32.19 fps, 4.163 s | Multi-phase signal behavior: clean/dim image, tracking/static transitions, green-line interference and changing horizontal damage states. |
| Sync Collapse & Luma Dropout Engine (prompt title: Procedural Static Generator) | 1280×720, 52 frames, ~29.0 fps, 1.794 s | Rapidly changing full-frame horizontal bands, severe sync collapse, unstable luminance, dark/bright pulses and fine luma dropouts; subtle chroma only. |
| Split-Band Chromatic Collapse | 320×240, 4 frames, 10 fps, 0.400 s | Two independent zones: upper multicolored horizontal static; lower vertical RGB columns/streaks; thin horizontal dropouts and flicker. Split position starts near 40% of frame height. |
| Two-Stage Signal Glitch & Angular Ribbon | 826×438, 123 frames, ~31.40 fps, 3.917 s | Stage 1: bright/overdriven upper band and dark lower chromatic static. Stage 2: sharp angular/sawtooth geometry and a spectral chroma ribbon. Requires a temporal stage controller, not a single static preset. |
| Diagonal Sync Collapse | 320×262, 34 frames, ~16.67 fps, 2.040 s | Full-frame diagonal shear/collapse, slanted fragmented chromatic bands, horizontal chroma smear and intermittent neutral-gray/saturated-blue states. |

### Measured temporal/spatial observations

These measurements are computed from the supplied encoded clips, not from the written prompt's estimated values:

- Dark-Field Luma Speckles: mean luma ~13.2; top-quarter mean ~20.4 versus bottom-quarter ~7.8, confirming a strong top density bias.
- Dark-Field Top Strip: top-quarter mean ~56.8 versus bottom-quarter ~23.3; frame-to-frame mean absolute RGB difference ~31, consistent with visible density pulses.
- Full-Frame Static: mean luma ~127 and frame-to-frame mean absolute RGB difference ~34.8; the noise covers the full image rather than a moving band.
- Blackout Cut: the sharp transition to the dark state occurs around frame 10 (~0.37 s) and remains dark for the rest of the clip.
- Bottom-Heavy Streaks: top-quarter mean ~1.3 versus bottom-quarter ~36.1, confirming the inverse density weighting.
- Monochrome Static: measured mean chroma saturation is 0; temporal frame difference ~14.8, with the prompt describing longer streaks near the final frames.
- Multi-Phase Tracking: mean luma varies widely (~47.2 standard deviation across frame means); frame-to-frame RGB difference ~31.8, consistent with multiple changing phases.
- Split-Band Collapse: very high mean saturation (~76.6) and large temporal difference (~47.3) over its four-frame reference.
- Two-Stage Glitch: upper-quarter mean luma ~143.9 versus bottom-quarter ~47.8; frame-to-frame difference ~43.9, supporting the bright-top/dark-bottom stage distinction.
- Diagonal Sync: high saturation (~131.9) with a stable blue/gray signal state and substantial frame changes.
- The iStock/videohive/Adobe Stock marks visible in some supplied clips are source-media watermarks, not signal effects; never recreate them.

## 2. Tear & Chroma Burst — 5 prompts / 5 references

### A. Dense Chroma Noise Band & Bleed — still IMG_4640.JPG
- Image size: 808×632.
- Main band spans the full width in the upper-middle/middle area, approximately 20–30% of image height.
- Dense multicolored horizontal static fragments, luma interruptions and irregular RGB displacement.
- Strong directional chroma bleed directly below the band, approximately 15–25% of image height, dominated by green/yellow/orange/red.
- Global low-resolution softness and restrained noise remain secondary.
- Main band and bleed use independent masks, positions, heights, strengths and falloff.

### B. Headbleed / Localized Tear & Chroma Burst — still IMG_4639.JPG
- Image size: 320×180.
- A wide irregular horizontal tear with broken bright/dark lines, partial luma loss and short fragments.
- Localized purple/magenta and green/yellow/orange/red chroma bursts; not uniform full-frame color bleed.
- Written prompt suggests initial band position ~45–52%, height ~16–22%, tear intensity ~65–85%, chroma intensity ~55–80%, base softness ~35–55%. These are estimates, not measured original controls.
- The prompt explicitly says temporal motion is not verified; do not invent a moving band as a fact.

### C. Horizontal Chroma Streaks & Interleave — still IMG_4650.JPG
- Image size: 828×560.
- Horizontal chroma streaks and smearing; fine interleaved/cross-color interference; subtle horizontal signal displacement; soft warm yellow-green base; narrow bright vertical strip near the right edge.
- The bright edge strip is a small independent detail, not a thick border.
- Still image only: temporal behavior is unverified.

### D. Dense Colored Speckle & Soft Bleed — video glitch cam 12
- 320×250, 23 frames, ~16.67 fps, 1.380 s.
- Dense small saturated RGB speckles flicker across the whole frame.
- Subtle chroma bleed and a soft low-resolution analog base.
- The written prompt explicitly distinguishes this from tracking tears and horizontal static-band effects. It must remain its own noise distribution/style.

### E. Multi-Band Tracking Tear & Chroma Smear — video glitch cam 5
- 320×242, 19 frames, ~22.35 fps, 0.850 s.
- Approximately 1–5 independently moving horizontal bands, with 2–3 as the default.
- Each band has independent center, thickness, horizontal displacement, tear intensity, fragmentation, chroma displacement, smear, dropout probability, flicker and temporal motion.
- Bands can overlap, approach and separate; avoid identical bands copied to different Y positions.
- Keep damage primarily horizontal; do not add unrelated global vertical roll, rotation or diagonal shear.
- Suggested starting values in the prompt: band count 2–3, tear ~75%, thickness ~15%, chroma smear ~65%, dropout/flicker ~60%, base softness ~40%, grain ~35%.

## 3. Tracking noise_ — 5 prompts / 3 still references

### A. Tracking Noise Research, Reconstruction & Preset Collection
- Requires separate source-inspired looks rather than superficial color variants.
- Analyze the four image edges independently: top tear/static, bottom interference/scan band, right-edge displacement/noise, left-edge displacement/tear.
- Build a shared engine supporting horizontal displacement, variable band position/height, irregular scanlines, signal interruptions, white/colored static, RGB displacement, chroma noise/bleed, edge-specific distortion, multiple regions and deterministic animation.
- Keep each preset editable and preserve actual algorithmic differences.

### B. Advanced Tracking Noise & Snow Engine
- Tracking Noise coverage must grow from a thin band to 100% of the frame.
- Support top, bottom, left, right and multiple simultaneous regions.
- Each region can have independent position, width/height, coverage, intensity, displacement, edge behavior and noise density.
- Add a genuine Wave Intensity control; it must change deformation, not just grain.
- Snow is a separate engine with intensity, density, size, distribution, contrast, brightness, color/monochrome, temporal variation, animation speed and seed.
- Keep background Noise Intensity separate from Snow and Tracking Noise.
- Investigate the source-specific control called “Snow Inspiratory” rather than guessing its purpose.

### C. Ultimate Tracking Noise / Tracking Damage / Signal Noise Engine
- Preserve the supplied VHS6RIFT implementation as a dedicated behavior/preset and preserve the original source folder.
- Improve its Tracking Noise to reach full-frame coverage without destroying the original clustered style.
- Preserve the existing Tracking Damage Size behavior, which already expands to full-frame coverage.
- Remove the integrated UI controls Event Per Minute, Event Duration Second and Recovery Second because the prompt identifies them as nonfunctional; keep the original source untouched.
- Research Grain, Signal Noise Intensity, Signal Noise Frequency and Detail and combine their actual behavior into a separate unified Signal Noise engine.
- Keep Tracking Noise, Tracking Damage, Snow/Static, Grain, Signal Noise, Chroma Noise and Luma Noise independently adjustable.

### D. Spectral Chroma Smear & Lower Tear — still IMG_4654.JPG
- Image size: 550×412.
- Upper/mid-frame highly saturated spectral chroma smear and color displacement.
- Lower full-width tracking-interference band occupies approximately 35–40% of frame height.
- Lower band contains horizontal tearing, scanline fragmentation, RGB displacement, colored noise and localized dropouts.
- Two independently configurable systems: spectral chroma region and lower tracking band.

### E. Upper Static Band & Cyan-Green Wash — still IMG_4655.JPG
- Image size: 639×480.
- Upper dark dense static band occupies approximately 40–45% of frame height.
- Band combines low-frequency dark horizontal structures, medium fragmented interference, fine static, sparse colored streaks, dropouts and row displacement.
- Separate lower-right/middle-right cyan-green chroma wash, plus a separate irregular dark patch.
- Add sparse lower horizontal lines and a dark analog base without crushing all shadow detail.
- Temporal behavior cannot be established from this still; expose deterministic optional animation rather than claim verified motion.

### Additional still IMG_4636.JPG
- Image size: 1000×667.
- Blue-screen VCR display with visible PLAY indicator and date/time text. The written edge-reference prompt instructs analysis of edge artifacts only, not the central scene.
- Preserve the observed OSD as a separate reference for VHS-90's text/date/time system; do not bake the text into a static overlay.

## 4. Shared architectural conclusions

The 21 prompts describe many visual presets but only a smaller number of reusable processing systems. Avoid building 21 independent renderers.

Recommended shared engines:

1. **Procedural Static / Snow:** monochrome horizontal fragments, colored RF static, dark-field density weighting, full-frame static, static bursts, snow distributions and temporal density pulses.
2. **Tracking Region Engine:** multiple independently positioned regions with X/Y/width/height, orientation, coverage, displacement, wave intensity, edge falloff, noise density and deterministic movement. Must support all four edges and full-frame coverage.
3. **Sync Collapse / Luma Instability:** full-frame or regional band geometry, sync displacement, bright/dark luma pulses, collapse events and fine horizontal dropouts.
4. **Tear & Chroma Engine:** central/multi-band tears, localized chroma bursts, directional chroma smear, RGB displacement and segmented signal loss.
5. **Split-Band / Angular Geometry:** upper/lower independent zones, vertical RGB columns, diagonal shear, angular ribbons and temporal stage controllers.
6. **Signal Color State:** blue/gray transitions and other event-driven signal states, independently triggerable.
7. **Shared Analog Base:** low-resolution softness, luma/chroma noise, grain, scanlines, flicker and output blend.

A preset should be a data recipe over these systems, except where its geometry/algorithm is genuinely unique (for example diagonal shear or the two-stage angular ribbon).

## 5. Mandatory rendering and validation constraints from the prompts

- Native C++ After Effects SDK.
- Procedural generation only; never render supplied reference media as overlays.
- Preserve alpha and premultiplied-alpha correctness.
- Support different resolutions and aspect ratios.
- Multi-Frame Rendering safety and thread safety.
- Deterministic output for identical seed/time/frame inputs.
- Frozen mode must freeze patterns exactly.
- Automatic, manual/keyframed and seeded temporal event modes where relevant.
- Generated-signal mode and blend-with-input mode where specified.
- Every UI control must affect actual rendered pixels; no decorative/unwired controls.
- Validate at reference dimensions and synchronized timestamps; do not hardcode observed clip timestamps as universal event times.
- Preserve existing effects and preset independence.

## 6. Media analysis status

- All 13 video files were probed for codec, resolution, frame rate, frame count and duration.
- Contact sheets were created from evenly spaced frames for all 13 clips and visually inspected.
- All six still images were opened and inspected as a contact sheet.
- Frame-level luminance, saturation, dark/bright-pixel fractions, top/bottom luminance profiles and frame-to-frame RGB differences were measured for all 13 videos.
- The measurements above support the written descriptions; they are not a substitute for a final rendered VHS-90 side-by-side validation.
- No VHS-90 implementation has yet been rendered against these references in After Effects.
