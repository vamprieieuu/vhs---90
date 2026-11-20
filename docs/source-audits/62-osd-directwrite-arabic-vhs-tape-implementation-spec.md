# VHS-90 — OSD text renderer and supplied-font implementation plan

Date: 2026-11-21  
Source evidence: `Source/VHS90Core/VHS90OsdEngine.cpp`, `Source/VHS90Effect.cpp`, `Tests/VHS90OsdEngineSmoke.cpp`, `Resources/Fonts/manifest.json`, supplied `vhs.zip`.

## Existing implementation verified

- The AE adapter constructs the OSD on the working image immediately after input conversion and before composite modulation/encoding, tracking, noise, decoder and remaining VHS processing. Keep this ordering: OSD must receive the same tape/signal damage as the footage.
- The current renderer is a deterministic 5×7 bitmap table. Its supported glyphs are ASCII digits, uppercase Latin letters and a few punctuation marks; unknown glyphs return a blank bitmap.
- Current visible elements are REC lamp, PLAY/SP status, ISO-like date/time stamp and battery icon. There is no arbitrary user string, font-family choice, Arabic shaping, RTL layout, localized month, date-format selector, full deck-state selector or VHS Tape corner-position model.
- Existing smoke test verifies deterministic rendering, opacity/alpha preservation, REC color, timestamp progression, battery rendering, disabled mode and invalid row-stride rejection. It does not test Unicode, fonts, Arabic shaping, date formats or VHS Tape layout parity.

## Font redistribution decision

The supplied font manifest and actual font name-table license metadata were checked again.

Eligible supplied fonts for a bundled plugin resource, subject to preserving complete font files and their notices:
- `Rooyin_Free_Bold.ttf`: SIL Open Font License 1.1; fsType indicates no subsetting. Bundle the complete file and include the complete OFL text. Preserve the Reserved Font Name provisions if applicable.
- `Sponge_Arabic_Regular.otf`: its embedded Yusuf Refaiy license includes the SIL Open Font License 1.1 text. Bundle the complete file and preserve the full embedded license/attribution.

Do not package the other supplied fonts by default. The manifest marks them blocked or permission-unverified; in particular A Massir Ballpoint and Arial Narrow have explicit distribution restrictions, while other supplied fonts lack redistribution permission or are personal-use-only. They may be selected from the user's installed Windows fonts where legally permitted, but must not be copied into the plugin.

Important: the source ZIP contains the two eligible font binaries, but the current Git repository only contains the font manifest, not the actual font files or license bundle. Therefore no claim is made that the current AEX embeds either font.

## Required native implementation

1. Implement a Windows DirectWrite text backend. Use DirectWrite text layout and glyph-run rendering so Arabic joining, contextual forms, combining marks, bidi and mixed Arabic/Latin runs are shaped by the platform text stack. Do not manually reverse UTF-8 strings or map Arabic code points to isolated glyph bitmaps.
2. Keep a platform-neutral OSD model in VHS90Core. It should carry UTF-8 text, font-family name, font size/scale, normalized anchor/corner, direction policy, horizontal/vertical alignment, color, opacity, and per-element enable state. The Windows adapter owns DirectWrite COM objects and rasterizes to an alpha mask/overlay; no DirectWrite headers may leak into the cross-platform core.
3. Implement the complete VHS Tape deck-state set: Play, Pause, Stop, Fast Forward and Rewind; SP/LP/EP; independent Deck Mode, Tape Speed and Counter corner positions; counter offset; text size; Date Stamp and Show Date; Date Position; Off/12-hour/24-hour clock; Clock Runs; editable day/year/hour/minute and Random Seed/Mix.
4. Implement localized English/Arabic month names and independent Arabic-Indic, Persian and Latin digit selection. Keep date-format selection independent from localization. Validate month lengths/leap years and avoid locale-dependent machine defaults.
5. Add editable arbitrary user text and font-family selection. The selector must enumerate Windows installed fonts and include the exact supplied font family names when the eligible supplied fonts are staged/installed. Missing fonts must produce a visible fallback state, never a silent unrelated substitute.
6. Keep the OSD render call before all VHS signal stages. Preserve alpha/premultiplication and the existing cropped/tiled/row-stride safety contract.
7. Add tests for Arabic joining forms, lam-alef ligatures, diacritics, mixed Arabic/English/numbers, both digit systems, RTL/LTR/auto layout, fallback font, all VHS Tape modes and corner positions, date formats/leap years, deterministic animation and non-zero/cropped render origins.
8. Windows/MSVC link requirements and any DirectWrite imports must be isolated to the AE target. Linux core CI should continue to build and test without DirectWrite.
9. Visual parity must be compared with the supplied VHSTape AEX/reference captures at identical dimensions, font size and positions. Do not claim the original device font is reproduced unless its provenance is actually established.

## Current disposition

- Correct pre-VHS compositing order: verified in the current adapter.
- Bitmap OSD baseline: implemented and smoke-tested.
- Full Unicode/Arabic/font-backed renderer: not implemented.
- VHS Tape full controls/layout parity: not implemented.
- Eligible font binaries/license files staged in the Git repository: not implemented.
- Windows DirectWrite integration, AE UI parameter wiring and source-reference visual tests: not implemented.
- This report is a scoped implementation contract, not a completion claim.
