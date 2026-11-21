# VHS-90 continuation checkpoint — UI separation and visual fidelity

Date: 2026-11-21
Base: `main` at `a6e107a0e0ef3c0e1109043c993bf4c61a28bed7`

This checkpoint records the actual repository state before further edits. It is deliberately not a completion claim.

## Verified UI structure

The current AE parameter-registration routine in `Source/VHS90Effect.cpp` already has independent topics for:
- `Tracking Noise` (`ID_TRACKING_TOPIC`)
- `Tear & Chroma` (`ID_TEAR_CHROMA_TOPIC`)
- `Tape Material` (`ID_TAPE_MATERIAL_TOPIC`)
- `Camcorder OSD`, `VHS Tape OSD Layout`, `Tape Counter`, `VHS Text Localization`, and `VHS Text Words`

The VHS6RIFT-derived Grain controls are currently grouped under `VHS-90 Signal Looks` (`ID_NTSCRS_TOPIC`), alongside the NTSCRS look/intensity/phase-noise and signal-hue-noise controls. Grain therefore is not yet a clean standalone section. Do not duplicate controls to make a second panel.

### Compatibility rules for the section split

1. Preserve every existing `ID_*` disk ID. Existing AE projects depend on those IDs.
2. Preserve the meaning, units, ranges, defaults and render mapping of existing controls.
3. Keep legacy parameter lookup and preset deserialization able to resolve old IDs.
4. Reordering the parameter-registration sequence requires a matching `ParamIndex` enum/order audit; never move only the `PF_ADD_*` call or only the enum.
5. Add a migration test that loads a fixture containing the pre-split Grain/Tracking/Tear disk IDs and proves values survive save/reopen.
6. Tracking Noise and Tear & Chroma are already independent topics. Refine their contents and labels rather than adding duplicate topics/controls.

## Visual-fidelity verification matrix

The following must be tested against extracted frames from the supplied source clips, not inferred from parameter names or smoke tests.

| Effect | Required visual signature | Current verification status |
|---|---|---|
| RF Drift | Slow receiver/tuning instability with coherent horizontal/phase displacement; should not resemble random per-pixel noise | Not visually compared |
| Mains Hum | Periodic line/field-linked interference with stable mains-related cadence; distinguish from RF pickup and generic banding | Not visually compared |
| AM Nonlinearity | Signal-level-dependent envelope distortion/compression, not a global contrast slider | Not visually compared |
| AGC Pump | Temporally coherent gain recovery/pumping after signal-level changes; not static brightness flicker | Not visually compared |
| Impulse Noise | Sparse, brief, high-amplitude impulses with controlled spatial/temporal distribution; not continuous grain | Not visually compared |
| Tape Dropouts | Localized loss/hold/white-or-dark streak behavior with plausible line/track orientation and temporal persistence | Requires retuning and source comparison |
| Tape Material | Physical defects should remain distinct (wear, pinholes/dropouts, scratches, creases, edge lift, mold, binder/Vaseline, sticky shed) and respond coherently to tape seed/speed | Requires source-frame comparison |
| Glow | Highlight-selective analog halation/bloom with controlled spread and restrained energy; avoid generic soft blur/washed frame | Requires source-frame comparison |
| Chroma Subsample | Horizontal chroma bandwidth/resolution loss with luma detail retained | Requires source-frame comparison |
| Vertical Chroma | Vertical chroma delay/averaging distinct from vertical luma blur; inspect saturated edges and fine luma detail | Requires source-frame comparison |
| Signal effects | Confirm stage order and interaction in Y/I/Q or composite path; isolate each control before combined presets | Requires source-frame comparison |
| OSD/text | Verify text is inserted pre-composite and is damaged by the same tape/signal chain; verify arbitrary text, font, RTL/Arabic, dates, deck states and alpha | Existing DirectWrite plan is in audit 62; runtime visual verification incomplete |

## Required visual test method

- Use the original `VHS/Vhs cams/Fxc6` and `VHS/Vhs cams/Gen 7` before/after clips and the relevant `VHS/vhs damage lap` reference clips/stills.
- Extract representative frames and short temporal windows. Single stills cannot validate AGC recovery, hum cadence, drift, dropout persistence, tracking motion or OSD temporal behavior.
- Render isolated effects at low/medium/high settings, then paired interactions. Keep source frame, dimensions, color management, time and render settings identical.
- Compare side-by-side and with difference/overlay views. Record the reference clip/timecode, AE render settings, expected signature, observed mismatch and code correction.
- A smoke test proves determinism/contracts only; it is not visual evidence. Linux core CI is not an AE-host or Windows/MSVC test.

## UI simplification policy

- Keep controls that map to a distinct physical or signal phenomenon and produce a reproducible visual difference.
- Merge duplicate controls only when their transfer functions and render stage are demonstrably the same.
- Hide advanced/rare controls behind concise topic groups rather than deleting serialized parameters.
- Remove a control only after confirming it has no meaningful render effect, is not referenced by presets, and is not required for old-project deserialization.
- Prefer one primary amount control plus a small number of physically meaningful shape/position/timing controls. Avoid exposing internal implementation knobs that do not create a user-visible decision.

## Immediate implementation order

1. Audit the parameter enum, disk IDs, `PF_ADD_*` registration order, UI grouping and render mapping as one unit.
2. Extract Grain into its own section while retaining old disk IDs and proving legacy-project migration.
3. Keep Tracking Noise and Tear & Chroma as independent existing sections; simplify their controls without deleting old IDs.
4. Build deterministic frame/sequence fixtures for RF Drift, Mains Hum, AM Nonlinearity, AGC Pump, Impulse Noise, Tape Dropouts, Tape Material, Glow, Chroma Subsample and Vertical Chroma.
5. Finish OSD text/font/Arabic/date/deck-state checks against the DirectWrite implementation and supplied licensed font files.
6. Run Linux core tests, Windows/MSVC AEX build, then actual AE 23.2.1 load/render/MFR tests. Do not call visual fidelity complete before step 6.

## Current boundary

The repository's current `main` head is `a6e107a`; recent commits repaired legacy render parameter wiring. The current code has separate Tracking Noise and Tear & Chroma topics, but Grain is still grouped in VHS-90 Signal Looks. Audit 62 documents OSD/DirectWrite implementation gaps. No visual comparison of the named effects is claimed by this checkpoint, and no claim is made that a reference clip was frame-inspected in this pass.

## Source-reference frames inspected (2026-11-21)

The four source clips in `VHS/Vhs cams` were decoded through the dedicated `VHS reference frame extraction` workflow (run 44915922717). Twelve evenly distributed frames per clip were extracted to a temporary CI artifact and inspected as contact sheets. This verifies source-reference appearance only; it does not compare the current plug-in render against those clips.

### FXC6 split-screen references

- `ScreenRecording_10-14-2026 18-22-43_1.mov`: the reference is vertically split. The left half is visibly softer during motion and has stronger smear/blur; the right half retains more edge detail and shows a cool blue/purple chroma/noise character with an RF Direct Capture/VHS overlay. At frame 103 (~2.69 s), the moving pedestrian is substantially more smeared on the left half while the right half retains more localized detail.
- `ScreenRecording_10-14-2026 18-23-02_1.mov`: same split-screen comparison pattern. At frame 100 (~2.70 s), the right half has stronger blue/purple chroma texture/noise while the left half is smoother and softer. The camera movement and scene geometry remain aligned across the split.

### Gen 7 reference

- `IMG_3621.MP4` is the clean green-screen source with natural skin tone and shirt colors.
- `After vhs_/ScreenRecording_10-14-2026 17-30-46_1.mov` demonstrates a wide range of signal damage, not one static overlay: early frames show a green/teal cast, yellow-biased skin/highlights and cyan/red edge separation; later samples show dense horizontal interference and RF-like snow, with the subject still partly visible; some later samples return to a cleaner saturated color image before strong interference reappears.
- Representative sampled positions: 0.90 s (initial chroma distortion), 1.79 s (stronger interference), 2.66–4.46 s (dense horizontal/noise breakdown), 5.35 s (cleaner color return), and 7.12–8.47 s (strong color/noise variation). These are screen-recording time positions, not calibrated effect-strength values.

### Implications for the named effect checks

- RF Drift must create coherent, temporally evolving receiver instability and horizontal displacement, not just a static RGB offset.
- Mains Hum must have coherent line/field cadence and remain distinguishable from ground-loop hum and broad noise.
- AM Nonlinearity should visibly alter the signal envelope/highlight compression, not merely apply global contrast.
- AGC Pump must respond to image energy/transients over time; it should not be a generic frame flicker.
- Impulse Noise should remain sparse and transient against the dense RF snow seen in the Gen 7 breakdown samples.
- The FXC6 split-screen establishes that motion smear, sharpness loss and chroma/noise texture are separate behaviors; do not collapse them into one blur/noise control.

The sampled frames were not enough to infer exact numerical settings for each individual RF effect. A true per-effect fidelity pass still requires isolated AE renders at known parameter values and a side-by-side comparison to matching source time ranges.

## OSD and text audit update (2026-11-21)

Current source review found that the earlier OSD specification is partially implemented, but not complete:

- The Windows-only DirectWrite backend is present and is used by the AE render path. The OSD is composited before the composite/tape signal stages, so its text receives the same downstream signal damage as the footage.
- Arabic/Latin mixed strings, bidi direction, combining marks, date localization, Arabic-Indic/Persian digits, clock modes, date formats, counter formats, and the Play/Pause/Stop/FF/REW labels have platform-neutral formatting and/or Windows smoke coverage.
- The two redistribution-cleared font files are embedded by the Windows build path; the runtime creates a private DirectWrite font collection. The font selector in AE is still a fixed 15-entry popup, even though `enumerateSystemFontFamilies()` exists in the Windows backend. Dynamic system-font enumeration is not wired to the effect UI.
- `OSD Text Data` is registered with `PF_PUI_NO_ECW_UI`. It is checked out and parsed, but there is no visible AE text editor for entering or changing arbitrary user text. This remains a real user-facing gap; the hidden arbitrary-data parameter is storage, not an editor.
- The selected-font result previously ignored `requestedFontFound`, allowing a missing family to fall back silently. The current source now keeps the readable DirectWrite fallback but draws a visible `FONT?` indicator when the requested family cannot be found.
- The hardcoded font popup includes families whose supplied files are not licensed for redistribution. This is acceptable only when the family is already installed on the user's system; the plugin must not bundle those restricted files. The visible fallback indicator now helps distinguish a missing family.
- The OSD still needs an AE-host visual pass for all deck states, corners, date/counter positions, custom text editing, text glow/ghost, Arabic shaping and actual bundled-font appearance. The Windows DirectWrite smoke test is not a substitute for checking the effect inside AE.

### OSD remaining work

1. Provide a genuinely editable custom-text UI for the arbitrary-data parameter; do not label the hidden `PF_PUI_NO_ECW_UI` storage field as a completed text editor.
2. Connect the DirectWrite system-font enumeration to a usable font-selection workflow, or explicitly constrain the selector to the bundled + installed families that are actually supported.
3. Keep a clear warning when a requested family is missing; never silently present an unrelated family as the requested font.
4. Test all OSD states/positions in AE 23.2.1 and confirm OSD text is damaged by downstream VHS stages as intended.
