# VHS-90 — Preset Migration, Font/Text and Pre-AEX Gap Audit

Date: 2026-11-21  
Scope: supplied `vhs.zip`, current `main` branch, previous VHS-90 user requirements and source-audit reports.

## 1. Factory preset migration checkpoint

The supplied archive contains seven native AEX source plugins:

1. `CratesVHSMaster.aex`
2. `HEISEI VHS.aex`
3. `signal.aex`
4. `Universe_Transitions_VHS_Transition_AE_Fx.aex`
5. `VHSCraft.aex`
6. `VHSPro.aex`
7. `VHSTape.aex`

The archive does **not** contain a plugin/file named `Quicks Master` or `Quick Master`. It does contain `CratesVHSMaster.aex`; these names are not silently treated as the same plugin. If “Quicks Master” refers to Crates VHS Master, that source is included below; otherwise its source file is absent from the supplied archive.

### 1.1 VHS Style popup

The native `VHS Style Preset` popup now contains 72 entries: 71 named recipes plus Custom.

| Source family | Recovered named looks | VHS-90 menu range | Migration state |
|---|---:|---:|---|
| VHS6RIFT | 16 | 2–17 | Existing native recipes |
| VHS Effect Add-on | 5 | 18–22 | Existing source-inspired recipes |
| Composite Video Simulator | 3 | 23–25 | Existing SP/LP/EP recipes |
| Lost Signal VHS v1.9 | 13 | 26–38 | Existing translated profiles |
| Crates VHS Master | 16 | 39–54 | Newly mapped from recovered embedded metadata |
| VHSPro | 10 | 55–64 | Newly represented as source-inspired native recipes |
| VHSCraft | 8 | 65–72 | Newly represented as source-inspired native recipes |
| **Total** | **71** | **2–72** | **71 named recipes + Custom** |

Source plugin labels were renamed in the VHS-90 menu. The menu no longer exposes the source family labels for Add-on, CVS or Lost Signal. Camera Profiles (Off + 11 profiles) and NTSC-RS-derived Looks (Custom + 10 looks) remain separate menus; they are not included in the 72-entry VHS Style count.

### 1.2 Crates VHS Master — source-value recovery

The supplied `CratesVHSMaster.aex` contains an embedded JSON metadata object. It was extracted and parsed from the actual binary, not inferred from decompiler variable names:

- 17 serialized preset entries: None + 16 named looks.
- 66 source user controls across Global/Input/Noise/Distortion/Signal/Modulation/Post FX.
- All 16 named looks have serialized per-parameter values.
- The native VHS-90 mapper now has a 16×44 source-value table and maps 44 recovered source fields to the closest matching VHS-90 DSP controls.

The 22 source fields without a direct native mapping are indices:  
`1, 24, 25, 26, 28, 31, 32, 33, 34, 82, 84, 85, 93, 101, 102, 103, 104, 105, 106, 107, 108, 109`.

These include the source master slider, RGB bias/add-color and raw-signal switches, sample count/attack/decay, subcarrier-frequency and modulation-mask controls. They must not be silently described as exact one-to-one ports. The mapped recipe is a semantic adaptation to VHS-90's existing engine; visual parity still requires AEX reference renders and tuning.

### 1.3 VHSPro and VHSCraft

- VHSPro's ten actual menu looks are recovered from its decompilation: Default, Fresh Tape, Rental Copy, Worn Out, Bad Tracking, Home Camcorder, Color Bleed, Broadcast, Dubbed Copy and Music Video. The decompilation does not provide reliable full per-preset parameter tuples. Ten renamed recipes now exist in VHS-90, but they are source-intent translations, not exact recovered settings.
- VHSCraft's eight actual menu looks are recovered: Standard, Clean, Sharp, Purple Tint, Green Tint, Damaged Tape, Camcorder and Black & White. Eight renamed recipes now exist in VHS-90. Exact per-preset value tuples have not yet been recovered.
- VHSCraft also has separate Classic VHS composite and Cinematic VHS RGB-domain render paths. The two paths must remain distinct; a complete VHSCraft renderer port and visual parity check are not claimed by adding the eight menu recipes.

### 1.4 Sources without bundled factory preset lists

- HEISEI VHS: no factory preset menu was recovered; it exposes parameter controls.
- Signal: exposes a “Destruction” button plus signal/tape controls; the button is not a named factory preset list. Its full render routine was not completely recoverable from the supplied decompilation.
- Universe VHS Transition: exposes Wipe/Slide Down/Slide Up and Color Noise/Bad Tracking/Very Bad Tracking/Degauss type menus, plus Choose Preset/Save Preset for user-saved presets. No bundled factory preset names/values were recovered. Its resource-backed transition patterns remain a separate procedural parity task, not 7 extra factory presets.
- VHS Tape: no factory preset menu was recovered. It exposes NTSC/PAL, SP/LP/EP, playback states and OSD fields. The source has no font-family selector in its recovered strings and no matching C decompilation.
- NTSCRT: all 45 JSON presets were semantically audited. They are not 45 interchangeable analog VHS looks: digital-only looks and five Screen Loop feedback looks were explicitly classified out of the analog VHS engine. Selected analog/timeline presets still need native AE recipe and keyframe conversion; the 45/45 audit is not a claim that 45 presets have been ported.

## 2. Font and Text/OSD gap audit

The supplied archive contains exactly 15 font binaries under `vhs/fonts/`; no additional TTF/OTF/TTC/WOFF font files were found elsewhere in the archive.

- 10 of 15 contain Arabic codepoints and OpenType joining/shaping features.
- All 10 Arabic-capable fonts contain Arabic-Indic digits U+0660–U+0669.
- Six also contain Persian digits U+06F0–U+06F9.
- `RealVhsFontRegular-WyV0z.ttf` is a Latin/display face with 55 glyphs and no Arabic shaping tables. It is VHS-themed by name/design, but the supplied evidence does not prove it is an exact font used by a particular Sony/JVC/Panasonic/other camcorder.
- No manufacturer-specific original camera OSD font was recovered from VHSTape; its binary strings do not expose a font family.

### Current implementation versus requested Text system

The current `VHS90OsdEngine.cpp` still uses a hard-coded 5×7 bitmap glyph switch for digits, A–Z and a few punctuation marks. It draws fixed REC/PLAY/SP/date-time/battery elements. It does not load the supplied font files and does not support arbitrary user-entered text.

Still outstanding:
1. Bundle or securely locate the permitted supplied fonts and expose their original supplied filenames in a Font selector.
2. Add a real font renderer with Arabic shaping, bidi/RTL ordering, ligatures and diacritics (DirectWrite/Uniscribe or another suitable native Windows shaping path).
3. Add arbitrary user text, text position/size/opacity and OSD element layout controls.
4. Add Arabic month names and localized date formats; support Arabic-Indic and Latin digits with fallback only to another compatible font from the supplied pack.
5. Ensure text is composited into the source before the VHS signal chain so all VHS effects damage the text as they damage the footage.
6. Calibrate the camera-inspired faces against actual camcorder OSD reference frames. Do not label a look as an exact manufacturer font without evidence.

### Font redistribution/licensing review

Font metadata was inspected. Embedding flags alone are not redistribution permission. Examples of explicit restrictions in the supplied metadata include:
- A Massir Ballpoint: WinSoft license text says workstation/personal publishing use and prohibits copying/distribution; fsType 4.
- Arial Narrow Bold: Monotype copyright notice; no redistribution license was supplied in the archive.
- Milan Titles Black: free for personal use only; modification not allowed; fsType 264.
- Nasma: metadata says do not sell or redistribute.
- ZIAN Black: metadata says all rights reserved/commercial version contact.
- Rooyin Free Bold and Sponge Arabic contain SIL OFL 1.1 license text (Sponge embeds the OFL text under its Yusuf Refaiy license).
- Several other files have no explicit redistribution license text in their font metadata.

Therefore the font binaries have been inventoried and their shaping capability verified, but they must not all be copied into a distributable AEX until the individual license/redistribution rights are confirmed. Preserve the original user-supplied files; do not substitute unrelated downloaded fonts.

## 3. Remaining implementation work from previous user requirements

### Source-specific DSP parity
- NTSC-QT: source-specific luma pre-emphasis before modulation and the 720-sample frequency-domain ringing response remain unported; the spatial VHS90RingingEngine is not equivalent.
- NTSCRT: complete 12-field rolling head-condition history and a full RF/FM/color-under record/playback signal pipeline remain incomplete. Selected analog presets and their full-state timeline snapshots/easing still need native AE keyframe migration.
- VHSCraft: distinct Classic composite and Cinematic RGB paths require a complete source-specific port/validation.
- VHSPro: the full render math is not recoverable from the supplied decompilation; current recipes are not pixel-identical claims.
- Signal: complete render reconstruction and the Destruction-button behavior need further source analysis.
- Universe Transition: the four procedural noise/tape-state sequences and Wipe/Slide transition behavior need native procedural reproduction if included in the final VHS-90 scope.

### VHS Cameras / Damage Lab acceptance
- Camera recipes FXC6 Direct Capture and Gen 7 Reference exist, but controlled native AEX before/after renders and visual calibration against supplied clips remain outstanding.
- Gen 7's described audio-noise/muffling behavior needs a separate audio-processing design; the supplied before clip has no audio, so no measured before/after curve exists.
- Damage Lab procedural engines are wired, but visual acceptance against the supplied still/video references remains outstanding.

### Host/build/release gates
- Windows x64 + MSVC 2022 build against the exact Adobe After Effects SDK.
- PiPL compilation and metadata/version/output-flag consistency.
- AEX artifact creation and binary inspection.
- After Effects 23.2.1 load/apply/parameter/render/save/reopen/repeated-use checks.
- MFR and thread-safety checks; alpha/premultiplication and ARGB32/64/128 validation.
- Controlled visual render comparisons and crash/regression sweeps.

Linux CI success validates host-independent C++ and static parameter/preset wiring only. It does not establish Windows/MSVC, PiPL, AEX load, AE rendering or MFR success.

## 4. Current code checkpoint

- `Source/VHS90Effect.cpp`: 72-entry VHS Style popup; new Crates VHS Master 16-look source-value mapper; ten VHSPro and eight VHSCraft source-inspired recipes; renamed source-family labels.
- `tools/validate_imported_presets.py`: validates all seven currently migrated source families, the 72 popup entries, the 16×44 Crates table and existing AE parameter/disk-ID order.
- Latest confirmed CI for the first 72-entry migration commit: run 44821704478 passed. The subsequent Crates source-value mapping and label-renaming commits require a fresh CI run before this checkpoint is considered validated.
- No Windows AEX build or After Effects runtime validation is claimed.

## 5. Pre-AEX decision

Do not begin final AEX production yet. First close the Text/font implementation, remaining source-specific DSP/preset migration, and visual/host-independent acceptance checks. Then run the Windows/MSVC + Adobe SDK + PiPL + AEX + AE/MFR release gates.
