# VHS-90 continuation checkpoint — source verification and release blockers

Date: 2026-11-21
Repository: `vamprieieuu/vhs---90`
Base commit inspected: `d774ae9461f09022ca8d91b0443e7f5891aeefa0`

This checkpoint is intentionally an engineering gap register, not a completion claim. The user confirmed that “Create VHS Master” means the supplied `CratesVHSMaster.aex`.

## 1. Source archive verification

The supplied `vhs.zip` is available in the working session as `/mnt/data/files/vhs_3.zip`. It contains the NTSCRT 0.26.0 source tree, NTSCQT 1.5.0, the AEX source samples, and the supplied font files.

A fresh archive listing found **39 JSON files beneath NTSCRT-0.26.0/presets/** in this exact ZIP snapshot: 34 top-level look/timeline files and 5 Screen Loop feedback presets. The existing `47-ntscrt-preset-semantic-migration.md` title says 45/45 and its table includes entries not present under that path in this ZIP snapshot. Therefore the 45/45 claim must not be treated as verified against the currently supplied archive until the six-item discrepancy is reconciled against the other source copies or prior extracted inventory. This does not invalidate the semantic classifications already documented; it means the source-count claim needs provenance.

The supplied NTSCQT 1.5.0 tree includes `app/ntsc.py`, `app/Renderer.py`, `app/ringPattern.npy`, `builtin_templates.json`, and the UI/renderer sources. The native VHS-90 `VHS90RingingEngine` is a spatial luma-edge/echo effect and is **not** a faithful substitute for NTSCQT's frequency-domain ringing response. The NTSCQT port remains incomplete until its waveform/frequency response, pre-emphasis order, sample/rate assumptions, and template mapping are implemented and tested against source-derived fixtures.

## 2. Preset migration state

The current VHS Style selector has 72 choices (71 named recipes plus Custom):
- 16 VHS6RIFT-derived recipes
- 5 VHS Effect Add-on recipes
- 3 Composite Video Simulator recipes
- 13 Lost Signal VHS recipes
- 16 Crates VHS Master recipes
- 10 VHSPro recipes
- 8 VHSCraft recipes

Crates VHS Master has a recovered 16-preset source table with 44 mapped source fields out of 66 user controls. The remaining 22 controls have no direct VHS-90 parameter equivalent and must remain explicitly classified as unsupported/host-specific rather than silently discarded. VHSPro and VHSCraft names are recovered, but their current recipes are source-inspired mappings, not verified exact source tuples or pixel-matched ports.

NTSCRT migration is **not closed**. Static analog look values can be mapped where a corresponding native engine exists. Timeline presets require AE keyframe conversion preserving source timestamps and easing. Digital-only codec/camera presets and Screen Loop camera-feedback presets remain separate effect families and must not be forced into the analog VHS engine. The existing NTSCRT audit is the semantic source map; this checkpoint corrects the archive-count verification caveat above.

NTSCQT built-in templates are not yet all represented as native VHS-90 recipes. They require per-template source-field mapping and native render comparisons; copying template names alone is not completion.

## 3. Text / VHS Tape OSD state

The current `VHS90OsdEngine` is still a fixed 5x7 bitmap glyph renderer. It currently draws REC, PLAY, SP, timestamp, and battery indicators. It does not yet provide arbitrary user-entered text, font selection, full VHS Tape deck-mode parity, or Arabic shaping.

The source-derived VHS Tape control inventory includes:
- PLAY, PAUSE, STOP, FF, REW and SP/LP/EP
- independent Deck Mode, Tape Speed, and Counter positions (top-left, top-right, bottom-left, bottom-right)
- counter offset, text size, date stamp, show date, and date position
- date formats `JAN. 21 1994`, `1.21.1994`, `21.1.1994`, `1994.1.21`
- Off/12-hour/24-hour clock modes, clock-running behavior, day/year/hour/minute, and random seed/mix controls

The 15 supplied font files are inventoried in `Resources/Fonts/manifest.json`. Font binaries must not be bundled unless the individual license permits redistribution. A Windows-native DirectWrite/Uniscribe shaping path is still required for Arabic joining, RTL layout, diacritics, Arabic-Indic digits and mixed Arabic/Latin strings. Font selection must use the supplied font inventory and preserve each font's real family name; the manifest alone is not a renderer.

OSD/text must remain in the image pipeline before composite/VHS processing so the text itself is affected by noise, chroma loss, tracking, head switching, blur and signal damage. A clean post-effect overlay would violate the requested behavior.

## 4. Source-engine and test closure

Still-open high-priority source parity items:
1. NTSCQT frequency-domain ringing and luma pre-emphasis/modulation ordering.
2. NTSCRT full receiver/VCR/tape/track read and record/playback chain, including per-head history and source-equivalent temporal behavior; current modules are selective ports, not full parity.
3. NTSCRT timeline preset conversion to AE keyframes with exact time/easing semantics.
4. VHSCraft's distinct Classic composite and Cinematic RGB paths.
5. VHSPro source-value/render behavior; existing menu recipes are not exact tuples.
6. Signal AEX render behavior and the Destruction action; controls/partial kernels do not prove full parity.
7. Universe VHS transition/noise behavior where in scope.
8. OSD/text renderer and VHS Tape control parity.
9. Source-derived image/video comparisons for every migrated family and engine.

Existing host-independent CI success is useful but insufficient for release. The latest previously checked run `44822831371` completed successfully on commit `d774ae9461f09022ca8d91b0443e7f5891aeefa0`. It validates the Linux C++ core workflow only. It does not prove Windows/MSVC compilation, PiPL generation, AEX load, AE 23.2.1 rendering, SmartFX/MFR safety, or visual parity.

## 5. Release gate — do not produce AEX yet

AEX production remains blocked until:
- preset migrations are complete or every source preset has a documented, intentional exclusion;
- Text/OSD has supplied-font selection and tested Arabic/Latin shaping, with VHS Tape controls implemented;
- NTSCQT/NTSCRT and other required source engines have native implementations plus deterministic regression fixtures;
- visual comparisons pass for the source references and presets;
- Windows x64/MSVC 2022 builds against the exact AE SDK, PiPL generation, AE 23.2.1 load/apply/preview/render/save/reopen, ARGB32/64/128, alpha/premultiplication, non-zero/cropped/tiled renders and repeated MFR tests pass.

Do not describe Linux CI or smoke tests as a successful AEX build.
