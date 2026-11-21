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
