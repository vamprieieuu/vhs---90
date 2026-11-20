# VHS-90 — OSD DirectWrite integration map

Date: 2026-11-21
Base checkpoint: `35ba2b467cf48601ec8752eebb2b2b0f9734e3eb`
Branch: `work/osd-directwrite-integration-map`

## Code inspection findings

This pass inspected the current `main` sources, not only report 62.

- `Source/VHS90Core/VHS90OsdEngine.h` exposes `VhsOsdParams` and `applyVhsOsd`. The parameter model currently contains REC/PLAY/timestamp/battery controls, start date/time, animation and alpha mode, but no free text, font family, text color/anchor, date layout or deck-state model.
- `Source/VHS90Core/VHS90OsdEngine.cpp` contains a private 5×7 glyph switch and ASCII-only `textDraw`. Unsupported characters resolve to a blank glyph. Timestamp formatting is hard-coded to numeric `YYYY-MM-DD HH:MM:SS`.
- `Tests/VHS90OsdEngineSmoke.cpp` covers deterministic bitmap output, alpha preservation, REC color, timestamp progression, battery, disabled mode and invalid row stride. It contains no Unicode/font/layout tests.
- `CMakeLists.txt` builds `VHS90Core` independently of Windows and adds `Source/VHS90Effect.cpp` only to the Windows AE module. This is the correct isolation boundary for DirectWrite: do not add Windows headers or COM dependencies to `VHS90Core`.
- `Source/VHS90Effect.cpp`: OSD controls are declared in the Camcorder OSD topic around lines 593–611; OSD values are copied into `RenderSettings` around lines 910–918; OSD is applied around lines 1012–1027; composite signal processing begins immediately after at line 1029. Preserve this ordering exactly.
- The current AE parameter enum and disk-ID enum are positional sequences. Adding new entries must be done in matching order, and every parameter addition must be reflected in setup, checkout, smart-render parameter access and disk IDs. A partial UI-only addition is unsafe.

## Implementation contract for the next code pass

### 1. Platform-neutral OSD model

Extend the core model only with portable data:
- UTF-8 arbitrary text and a separate enable flag.
- Font family as UTF-8; font size in frame-relative units.
- Anchor/corner, horizontal alignment and vertical alignment.
- Text color and opacity independent of REC lamp color.
- Direction policy: Auto, LTR, RTL.
- Date format, month language and digit system as explicit enums.
- Deck mode (Play, Pause, Stop, Fast Forward, Rewind), tape speed (SP/LP/EP), and independent visibility/position for deck state, speed, counter, date and clock.

Do not store DirectWrite interfaces, HWNDs, COM pointers or Windows string types in the core structs.

### 2. Windows renderer

Add a Windows-only adapter under a dedicated Windows source directory. It should:
- Convert UTF-8 to UTF-16 with strict invalid-sequence handling.
- Use DirectWrite text layouts and glyph-run shaping for Arabic joining, ligatures, diacritics, bidi and mixed-script text. Never reverse UTF-8 bytes/code points manually.
- Render through a premultiplied-alpha surface and composite into the existing float image using the same row-stride and cropped-origin guarantees as the bitmap path.
- Enumerate installed system font families and report a clear fallback when the requested family is unavailable.
- Keep DirectWrite initialization, factory lifetime and COM cleanup thread-safe for AE render workers.
- Keep font file installation separate from font enumeration. Bundle only the two fonts approved by report 62, with full license notices and complete binaries; do not copy restricted/unverified fonts into the plugin.

### 3. AE integration

- Add controls only after confirming the exact AE SDK parameter API used by this repository's target SDK. Do not guess a string-parameter macro or disk-ID scheme.
- Keep the existing REC/PLAY indicators backward-compatible. The new free-text layer must be independently enabled and must not silently replace the existing HUD.
- Copy all new controls through the same legacy/smart-render settings paths so both render entry points behave identically.
- Invoke the Windows text overlay in the existing OSD block before the composite-stage block. If the DirectWrite backend fails, retain the existing bitmap OSD and surface a diagnosable fallback rather than failing the entire frame.
- Keep the platform-neutral core tests runnable on Linux; add Windows-only renderer tests behind a CMake option.

### 4. Acceptance tests

Required before calling this feature complete:
- Arabic joining in initial/medial/final/isolated positions; lam-alef; combining marks; mixed Arabic + Latin + punctuation + numbers.
- LTR, RTL and Auto paragraph direction; Latin, Arabic-Indic and Persian digits.
- Missing-font fallback, installed-font enumeration, and bundled Rooyin/Sponge Arabic licensing/notice checks.
- Every VHS Tape deck state and SP/LP/EP; independent element positions; date/clock visibility and format; leap-day rollover.
- Premultiplied and straight alpha; non-zero row stride; cropped/tiled render origins; deterministic frame output.
- A/B visual comparison with the supplied VHS Tape reference at identical frame size, scale, color and anchors.
- Regression proof that OSD is still damaged by all downstream VHS signal stages because it is composited before those stages.

## Status after this pass

Completed in this pass: source-level integration map and acceptance contract, committed separately from the previous source audit.

Not implemented in this pass: DirectWrite renderer, arbitrary text AE control, font enumeration, Arabic rendering, VHS Tape control parity, or Windows visual tests. No AEX build or After Effects validation is claimed.

## Immediate next implementation sequence

1. Confirm the exact target AE SDK string parameter API and current project version/disk-ID conventions.
2. Implement the Windows-only DirectWrite renderer and a small standalone Windows smoke test.
3. Extend the core OSD model and integrate the renderer behind the current OSD call without moving its pipeline position.
4. Wire AE parameters into both render paths and add Unicode/layout regression coverage.
5. Stage only licensed font assets/notices, then run Linux core CI and Windows/MSVC AE build.
