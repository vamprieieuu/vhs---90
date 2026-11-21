# VHS-90 — Universe VHS Transition AEX Audit

Date: 2026-11-19
Sources: `plugins/aex/Universe_Transitions_VHS_Transition_AE_Fx.aex` and `plugins/c+/Universe_Transitions_VHS_Transition_AE_Fx.aex.c`.

## Binary and effect identity

- Supplied AEX is a Windows PE32+ x86-64 DLL.
- The plugin identifies as a VHS Transition in Universe Transitions.
- Its description says it mimics recording over pre-existing footage.
- The UI is a transition effect with transition type controls, tape-noise type controls, color controls and presets. It should be used as a behavior/reference source for VHS-90's damage engines, not copied as a separate UI section.

## UI menus recovered

### Transition Type
- Wipe
- Slide Down
- Slide Up

### Tape Noise Type
- Color Noise
- Bad Tracking
- Very Bad Tracking
- Degauss

The source loads separate frame resources for Color Noise, Bad Tracking, Very Bad Tracking and Blank Tape. The UI's fourth label is Degauss, while the corresponding resource path is named BlankTape; preserve this observed distinction instead of assuming the two labels are semantically identical.

## Parameters and recovered defaults/ranges

| Control | Default | Range |
|---|---:|---:|
| Tape Noise | 100 | 0–200 |
| Tape Distortion | 100 | -500–500 |
| Random Noise | 25 | 0–100 |
| Wrinkle Distortion | 5 | 0–20 |
| Wrinkle Size | 5 | 0–20 |
| Red Offset | -50 | -100–100 |
| Green Offset | 0 | -100–100 |
| Blue Offset | 25 | 0–300 |
| Red Blur | 25 | 0–300 |
| Green Blur | 55 | 0–300 |
| Blue Blur | 45 | 0–300 |
| Black Level | 125 | -200–200 |
| Saturation | 125 | -200–200 |
| Luminance Blur | 4 | 0–20 |
| Chroma Blur | 0 | 0–5 |
| Chroma Offset X | 0 | -200–200 |
| Chroma Offset Y | 0 | -200–200 |
| Frame Jitter | 2 | 0–20 |

Additional controls/groups:
- Stroke
- Enable Stroke
- Width
- Color
- Color Controls
- Monochrome
- Frame Style
- Choose a Preset...
- Save Preset...

## Resource-backed damage types

The binary references image-sequence resource folders:
- `$rsc/Color Noise/Color Noise_`
- `$rsc/BadTracking/Bad tracking_`
- `$rsc/VeryBadTracking/Very bad tracking_`
- `$rsc/BlankTape/blankTape_`

These resources are source-plugin implementation assets. VHS-90 must reproduce their looks procedurally in native code, not copy frame images into the final effect as overlays. The references are valuable for reverse-engineering timing, pattern structure and appearance.

## VHS-90 integration decision

- Rebuild Color Noise, Bad Tracking, Very Bad Tracking and Blank Tape as four distinct procedural signal/damage algorithms.
- Do not treat Very Bad Tracking as a single horizontal offset. It needs unstable sync, torn bands, variable noise, horizontal displacement and temporal changes.
- Keep Degauss/Blank Tape as a distinct no-signal/tape-erase mode, not just a black frame.
- Preserve signed Tape Distortion and color offsets; do not clamp the controls to only positive values.
- Keep Luminance Blur, Chroma Blur, X/Y Chroma Offset, channel-specific blur/offset, frame jitter, wrinkle distortion and monochrome as shared modules/parameters in VHS-90's unified engine.
- The source has a transition-specific Wipe/Slide UI. VHS-90's core effect should remain a regular footage effect; transition behavior is not to be imported unless explicitly required.

## Validation

- UI parameter labels, menu values, slider defaults/ranges and resource folder paths were inspected in the decompiled source.
- No runtime test or exact frame-by-frame visual comparison was run here.
