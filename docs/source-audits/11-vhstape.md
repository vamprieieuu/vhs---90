# VHS-90 — VHSTape AEX / OSD Reference Audit

Date: 2026-11-19
Sources: `plugins/aex/VHSTape.aex` and the supplied Ghidra header `plugins/c+/VHSTape.aex.h`. No corresponding `VHSTape.aex.c` decompilation is present in the archive.

## Binary identity

- PE32+ Windows x64 module.
- Product strings: VHS Tape / dbVHS / Retro Effects.
- Description: “Simulates recording to and playing back from a VHS tape.”
- The binary includes AE entry-point and AEGP suite strings, including AEGP PF Interface, Layer and Stream suites.
- Header declares an AE `EffectMain` and 20+ internal function prototypes, including float/vector pixel processing and signal/noise helper functions. Since the matching C decompilation is absent, those routines cannot be mapped confidently to exact algorithms in this pass.

## Full exposed parameter/menu vocabulary recovered from binary strings

### Tape
- Format: NTSC (480 lines) / PAL (576 lines)
- Tape Speed: SP / LP / EP

### Picture
- Luma Blur
- Chroma Blur
- Chroma Shift
- Sharpening
- Saturation

### Tape Damage
- Noise
- Line Jitter
- Edge Wave
- Head Switching
- Dropouts
- Tracking Error

### Playback
- Deck Mode: Play / Pause / Stop / Fast Forward / Rewind
- Search Speed
- Blue Screen
- Static

### On-Screen Display
- Show Deck Mode
- Deck Mode Position: Top Left / Top Right / Bottom Left / Bottom Right
- Show Tape Speed
- Tape Speed Position
- Show Counter
- Counter Position
- Counter Offset (sec)
- Text Size
- Date Stamp
- Show Date
- Time: Off / 12-Hour / 24-Hour
- Date Position
- Date Format:
  - JAN. 21 1994
  - 1.21.1994
  - 21.1.1994
  - 1994.1.21
- Month
- Day
- Year
- Hour (0–23)
- Minute
- Clock Runs
- Random Seed
- Mix

Month menu uses English full month names from January through December in the supplied binary.

## Important VHS-90 feature matches

- The binary explicitly separates NTSC/PAL format, SP/LP/EP tape speed, picture processing, tape damage, playback state, static/blue-screen behavior and OSD.
- It includes independent Luma Blur, Chroma Blur and Chroma Shift controls.
- Tape damage controls include Noise, Line Jitter, Edge Wave, Head Switching, Dropouts and Tracking Error.
- Playback OSD exposes Play/Pause/Stop/Fast Forward/Rewind states and Search Speed.
- OSD has independently positioned Deck Mode, Tape Speed, Counter and Date groups, with four-corner positioning options for Deck Mode.
- Counter Offset (sec), Date Stamp, 12/24-hour display, manually editable month/day/year/hour/minute, and Clock Runs are all explicitly represented.
- Date formats include the four common layouts listed above. VHS-90 should preserve these formats while implementing Arabic month localization and Arabic-Indic digits as previously specified.
- Static and Blue Screen are distinct controls; do not assume Static is simply the same as Blue Screen.
- The source exposes Text Size but no explicit font-family selector in the recovered strings. VHS-90's Text section must use the user's supplied font pack and expose font selection independently.

## VHS-90 adaptation decisions

1. Preserve NTSC/PAL as a real format mode affecting line count, timing and signal behavior, not just a UI label.
2. Preserve SP/LP/EP as tape bandwidth/quality profiles.
3. Implement Picture, Tape Damage, Playback and OSD as shared engine modules in VHS-90, not as separate source-plugin panels.
4. Keep all OSD groups independently enabled and positioned; allow free X/Y placement in addition to the source's four-corner presets.
5. OSD must be composited before VHS damage so tape noise, sync errors, color loss and tracking damage can affect the generated display.
6. Keep the clock/counter deterministic from AE comp time and explicit offsets. Clock Runs off means a fixed manually entered time; on means time advances with footage.
7. Localize month names and digits through VHS-90's locale layer, rather than hard-coding the source's English month strings.
8. Preserve source alpha and premultiplied color in all picture, OSD and blue-screen paths.
9. Static and Blue Screen remain separate procedural effects. No frame-image overlays should be shipped as effect output.

## Verification limits

- All readable UI/menu strings and the Ghidra header's function declarations were inspected.
- No matching `.aex.c` decompilation was supplied, so parameter ranges/defaults, internal processing order and exact formulas remain unverified.
- No execution in After Effects was performed.
