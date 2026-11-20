# VHS-90 — VHS Tape OSD localization and word-control integration

Date: 2026-11-21  
Branch: `work/osd-directwrite-integration-map`

## Implemented in this increment

- Added portable `VHS90OsdLocalization` core module and smoke tests.
- Added English, Arabic and bilingual output modes.
- Added Latin, Arabic-Indic and Persian digit sets.
- Added four date layouts: `JAN.21.1994`, `1.21.1994`, `21.1.1994`, and `1994.1.21`.
- Added short/full English month names; Arabic month names are localized.
- Added independent clock mode: Off, 12-hour, 24-hour. Clock can be shown without the date.
- Added Clock Runs behavior, with elapsed render time applied only when enabled.
- Added localized tape counter formatting: six-digit frames, HH:MM:SS and HH:MM:SS:FF; added a seconds offset and an independent Show Counter toggle.
- Added VHS labels/translations for PLAY, PAUSE, STOP, REC, FF, REW, SP/LP/EP, TAPE, ERROR, P.O.1/P01, RES, FORWARD, EJECT, SEARCH, TRACKING, AUTO, CH, CAM, DECK, TIMER, STEREO, SLOW, STILL and MEMORY.
- Added AE controls for master Show Text, per-word visibility, Show Date, Show Counter, extra-label selection and group position. Text Size is independent of icon scale; Text Glow has an enable switch and intensity slider. Existing OSD Glitch/Ghost now also affect DirectWrite words with deterministic frame/seed jitter and chromatic ghost copies. OSD Amount is labelled OSD Mix / Amount.
- Added a shared font-family selector containing all 15 supplied font families. The selected family is used by DirectWrite text for REC, deck status, speed, date/time, counter, auxiliary labels and optional OSD text.
- The DirectWrite overlay remains in the existing OSD position: after bitmap/icon OSD generation and before the VHS composite/signal processing. Bitmap icons remain available while their English bitmap words are suppressed to prevent duplicate text. Text, date/time and counter share the selected font family, size, opacity/mix, jitter, ghost and glow controls.

## Parameter / host wiring

- Added the new AE controls after existing OSD parameters; original parameter disk IDs for existing controls remain unchanged.
- Corrected the VHS Tape Layout and Tape Counter topic boundaries to use distinct topic IDs rather than reusing the first child parameter ID. Added explicit topic start/end parameter slots so the parameter index table matches the actual AE registration sequence.
- Updated the static parameter validator to count `PF_ADD_ARBITRARY2` IDs (the arbitrary-data macro places its persistent ID before a trailing refcon argument).
- Manually checked all 332 active parameter slots, persistent IDs and registration IDs: counts match, IDs are unique, and registration order matches the parameter disk-ID table.
- Confirmed every new setting is read by both SmartFX `ReadSettings` and the Legacy render path.

## Tests and current validation boundary

- Linux core CMake build and smoke tests passed after the battery OSD safe-margin correction (run `44834430499`).
- Windows and Linux OSD Core / DirectWrite smoke workflow passed for the OSD engine and DirectWrite renderer (run `44834430462`). This validates those portable/core and Windows DirectWrite test targets, not the AE host adapter.
- The localization smoke suite covers bilingual dates, month styles, digit conversion, 12/24-hour and clock-only output, leap-day rollover, counter formats/offsets, and key VHS labels. Linux Core CI run `44834780888` passed 36/36 tests; Windows + Linux OSD Core/DirectWrite run `44834781058` passed 37/37 tests, including `VHS90DirectWriteSmoke` on Windows. The latest AE static/core check on commit `c07234be26fe28f8ce17a4be07fd50e31aa04fa7` passed, and its Windows + Linux OSD Core/DirectWrite workflow `44835316039` also passed (37/37 tests, including Windows `VHS90DirectWriteSmoke`).
- No MSVC build of `VHS90Effect.cpp`, PiPL generation, AEX artifact, After Effects 23.2.1 load/render, save/reopen, or MFR host test is claimed.

## Supplied fonts and redistribution

The font selector includes all 15 supplied families using their actual DirectWrite family names, but the font binaries are not yet bundled into the public repository. The supplied manifest identifies `Rooyin_Free_Bold.ttf` and `Sponge_Arabic_Regular.otf` as OFL-eligible for full-file redistribution with their license notices. Several other fonts explicitly prohibit redistribution or have no verifiable redistribution license. Do not place restricted/unverified binaries in this public repository without permission. Font-backed Arabic rendering still needs packaging and actual Windows/AE visual validation.

## Remaining OSD work

1. Package the two OFL-cleared fonts with their notices and integrate a private DirectWrite font collection / font loading path; verify Arabic fallback without silently substituting unrelated fonts.
2. Confirm the exact 15-family selector labels and preferred defaults against the user's selected font references.
3. Compare all OSD positions, text sizes, glow, dates, counter and words against the supplied VHSTape/Crates VHS Master references, frame-by-frame.
4. Validate mixed Arabic/English strings, RTL, diacritics, bidi punctuation, all digit sets, save/reopen, duplicate effects, undo/redo and MFR in AE 23.2.1.
5. Build the complete AEX with Windows/MSVC and the real Adobe SDK extracted from the VHS6RIFT library source; then validate PiPL, parameter indices, output flags and AE host behavior.

This increment advances the OSD implementation; it does not close the overall VHS-90 pre-AEX or final-release gates.
