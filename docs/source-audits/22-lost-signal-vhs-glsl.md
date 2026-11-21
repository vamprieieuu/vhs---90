# VHS-90 — Lost Signal VHS GLSL Signal Engine Audit

Date: 2026-11-19
Input: `vhs/github repos/lost-signal-vhs-main/` from supplied `vhs.zip`.
Scope: full README and settings menu, main composite/final fragment shaders, vertex shaders, analog YIQ helper, shader properties and localization resources. The archive also contains multiple versioned shader-pack ZIPs; they are package variants, not additional independent signal engines.

## Project shape

- Iris/OpenGL GLSL 1.20 Minecraft shader pack.
- No noise textures, image overlays or compute-shader dependency; artifacts are procedurally generated with deterministic hash/value noise and render-target history.
- Main stages: scene/depth composite stage, persistent temporal metadata/history, then final tape playback/decoder stage.
- Not an AE plugin. Its shaders depend on Minecraft/Iris uniforms, depth textures, camera matrices, field timing and framebuffer attachments.
- The shader explicitly uses OpenGL; it is not a Vulkan shader implementation.

## Presets

The README identifies 13 looks:
1. Reference VHS
2. Real VHS
3. Backrooms
4. Poolrooms
5. Liminal Night
6. Subtle
7. Found Footage
8. Damaged Tape
9. Rental Tape
10. Camcorder 1996
11. Broken Signal
12. Composite Decode
13. Analog Fog

These are source recipe references only. VHS-90 should create its own preset names and map their parameter intent into the unified engine.

## Main signal / transport controls

- Quality Level: Performance / Balanced / Cinematic.
- Signal Standard: NTSC / PAL.
- Tape Format: VHS SP / LP / SLP / VHS-C / worn rental.
- Generation Loss: 0–5.
- Tape Wear: 0–1.
- Noise Strength, Scanline Strength.
- Glitch Frequency and Glitch Strength.
- Continuous Timebase Error.
- Static Burst Frequency.
- Chroma Phase Error.
- Automatic Exposure and Exposure Pump Strength.
- Chroma Killer and Strength.
- Vertical Sync Glitch, frequency and strength.
- Chroma Persistence and Strength.
- Composite Decoder: bypass / consumer notch / NTSC two-line comb.
- Dot Crawl, Cross Color and Cross Luma strengths.
- Chroma Amount / timing split and Chroma Bleed.
- Lens Distortion, Flicker, Frame Jitter, Wobble.
- Motion Smear, Ghost Strength, History Stabilization, Motion-Aware History, Reprojection Strength.
- Spatial Echo and Strength.
- Pixel Scale.
- Tape Softness, Black Crush, Color Saturation, Head Switching, Halation, Luma Ringing.
- Tracking Control (signed bias), RF Interference, Tape Chew.
- Tape Defect Memory, Defect Density and Defect Strength.
- Frame Repeat, frequency and strength.
- Interlaced Fields and Interlace Strength.
- 4:3 playback framing, rounded overscan, vignette.
- Optional VCR OSD and camcorder HUD.

## Core algorithms recovered

### 1. YIQ record/playback path
- Explicit RGB→YIQ and YIQ→RGB matrices.
- Luma Y remains higher bandwidth; I and Q are independently filtered with different asymmetric five-tap kernels.
- Chroma timing split samples positive/negative horizontal offsets without softening the luma edge.
- Chroma phase error rotates I/Q per line and time, producing hue instability in signal space.
- VHS tape formats change effective chroma retention and luma quantization levels; generation loss compounds chroma reduction, cross-color contamination and noise.
- Chroma bleed uses broad asymmetric horizontal taps, with different I and Q weighting.

### 2. Composite carrier and decoder
- Encodes Y + I·cos(phase) + Q·sin(phase).
- Carrier phase depends on horizontal sample position, line number, frame phase and NTSC/PAL mode.
- Consumer notch decoder uses five taps with [1, 2, 2, 2, 1] weighting to separate luma/chroma.
- NTSC two-line comb compares the adjacent line; correlated detail is used to suppress cross-color and dot crawl.
- PAL keeps the notch path because its alternating V phase differs.
- Cross-color and cross-luma are separate controls.
- Quality levels reduce taps and sampling cost while retaining a scalar-like fallback.

### 3. Tracking tears and continuous time-base error
- Slow and fast smooth-noise fields produce correlated line timing errors.
- Rare tear events use an attack/release envelope, primary and companion horizontal bands, randomized width and signed displacement.
- A separate narrow tracking band rolls vertically through the image and adds local horizontal displacement.
- Tracking Control biases the rolling band's position.
- Tape Chew creates a wider, brief buckle event across several adjacent lines, with correlated ripple/noise.

### 4. Head Switching
- Event timing is locked to the analog field clock.
- Head center varies only between fields, around the lower 1.5–4.5% of the frame; width is also randomized per field.
- One shared mask drives correlated horizontal/vertical shift, chroma phase jump/loss, line noise and RF streaks.
- This is much more coherent than independent noise sliders.
- However, the head center is deliberately bottom-anchored; there is no free vertical position or region-height control. VHS-90 must replace the fixed center range with user-defined normalized Y/height and allow full-frame coverage, while retaining field-locked event timing.

### 5. Static bursts, RF and oxide defects
- Static is a short full-screen burst gated by a field/time-derived event, with a spatial hash and line mask.
- RF interference combines two diagonal sine carriers with a rolling bright band.
- Persistent oxide defects are keyed to a virtual longitudinal tape coordinate. Their horizontal start, length, band width, event probability and fray texture remain stable as the tape moves through the raster.
- Defects desaturate chroma first, then replace damaged signal with seeded light/dark RF levels.
- Tape Wear affects defect probability/visibility and generation noise separately from transport speed.
- This persistent material-coordinate approach is a strong basis for VHS-90's tape wear/dropout engine.

### 6. Temporal history / interlacing / sync loss
- Stores the previous fully processed signal frame in a history render target.
- Reprojects previous history through camera depth and previous view/projection matrices.
- Stores logarithmic depth metadata and rejects history when depth/disocclusion differs.
- Motion-aware history prevents trails from sticking to screen coordinates when objects move.
- Chroma persistence can reuse only previous I/Q while retaining current luma.
- Vertical sync glitches choose between rolling the raster and holding the previous processed frame.
- Optional frame-repeat events recursively retain the prior frame.
- Interlaced fields update one line parity at a time and weave the previous field into unrefreshed lines.

These are sequential-frame effects. AE Multi-Frame Rendering and random-access rendering cannot depend on an implicit prior GPU frame; VHS-90 needs explicit per-frame state, deterministic reconstruction/checkpointing or a stateless approximation.

### 7. Image and camera stages
- Tape softness uses horizontal taps.
- Luma ringing creates bright leading and dark trailing edge echoes.
- Spatial echo is a separate one-frame horizontal tap, independent of temporal ghosting.
- Halation uses highlight luma.
- Camera era changes color response; zoom, lens distortion, focus hunting, exposure pump, white-balance drift and flicker are separate from tape format.
- Analog fog uses depth and world-position noise before the signal enters the virtual deck.

## OSD and ordering

- Optional VCR OSD and camcorder HUD are separate overlays with different color behavior.
- The VCR OSD is composed in the final playback shader, after the tape and decoder stages; it therefore does not receive all preceding tape damage.
- VHS-90 requires its OSD to be generated before VHS damage, so noise, tracking, chroma loss and sync errors affect it. Reorder the pipeline rather than copy the shader's final overlay placement.
- The pack's final shader writes alpha 1.0. VHS-90 must preserve original alpha and premultiplication.

## VHS-90 adaptation decisions

1. Port the YIQ matrix, asymmetric I/Q filters, composite carrier/notch decoder, two-line comb, cross-color/cross-luma leakage and field phase model.
2. Port correlated line-time noise, tear attack/release envelopes, rolling tracking band, tape chew, static burst, RF herringbone, persistent oxide-defect hash and head-event correlation.
3. Keep Tape Wear, Generation Loss, Tape Format, Chroma Bleed, Luma Ringing, Motion Smear, Ghosting and Spatial Echo as distinct modules.
4. Replace fixed bottom head-switch center with arbitrary user Y/height and full-frame coverage; expose region geometry separately from field-event phase.
5. Replace Tracking Control's single rolling-line bias with VHS-90's arbitrary region model: position, width, height, direction, motion and multiple regions.
6. Port temporal history only with a render-context model that supports AE MFR, deterministic frame indexing and random-access renders.
7. Preserve alpha; place OSD before the damage engine; use CPU-first C++ implementations and optional acceleration only.

## Verification status

- Reviewed the settings macros, menu/property wiring, preset descriptions, analog color helper and both primary fragment shaders.
- No GLSL compilation under Iris, Minecraft runtime test, OpenGL driver test or shaderpack rendering was performed.
- No code from this shader pack has yet been ported into VHS-90's production C++ render path.
