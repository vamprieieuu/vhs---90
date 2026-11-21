# VHS-90 — Lost Signal VHS v1.9 Full Audit & Native Integration

Date: 2026-11-20  
Source folder: `vhs/github repos/lost-signal-vhs-main/`  
File disposition: **31/31 files reviewed** (19 current pack files, 10 historical release archives, root README and release script)  
Current source of truth: **Lost Signal VHS v1.9 — Signal Physics**

## 1. License, architecture and version history

The current pack is MIT-licensed. It is an OptiFine/Minecraft multi-pass GLSL shader pack, not a C++ library or a directly loadable After Effects effect. The shader stages depend on Minecraft G-buffer textures, depth buffers, camera matrices, frame history and shader-pack uniforms. Those host-specific pieces were not copied into the AE plugin. The useful image/signal behavior was reimplemented in the existing VHS-90 C++ engines, with the MIT attribution retained. No GLSL shader file was copied verbatim.

The folder contains ten historical release archives. All were opened and compared; they are version snapshots, not ten separate engines:

| Release | Archive entries | Audit disposition |
|---|---:|---|
| v1.0 | 3 | Initial shader snapshot; historical. |
| v1.1 | 4 | Incremental early revision; historical. |
| v1.2 | 6 | Composite-stage development; superseded. |
| v1.3 | 6 | Liminal-signal revision; superseded. |
| v1.6 | 6 | Intermediate framebuffer/shader revision. |
| v1.6.1 | 6 | Follow-up compatibility revision. |
| v1.7 | 9 | Composite-decode development milestone. |
| v1.8 | 11 | Analog-fog feature milestone. |
| v1.8.1 | 11 | Packaging/compatibility hotfix. |
| v1.9 | 22 | Latest authoritative source; contains the full 13-profile settings system. |

The v1.9 settings file defines **13 named profiles** and **77 unique public options**, with 75 preset-owned options checked by the source's `build-release.ps1` release validator. The source release validator and its profile inheritance/option coverage rules were inspected. The v1.9 release is the authoritative version used for the port; older archives were retained as history and were not merged as duplicate engines.

## 2. Complete 31-file disposition

All 31 files were inspected and classified.

| File | Disposition |
|---|---|
| Root `README.md` | Reviewed project scope, v1.9 feature list, 13 preset names and release history. |
| `scripts/build-release.ps1` | Reviewed release packaging and profile-completeness checks; not part of the AE render engine. |
| `Lost_Signal_VHS/LICENSE.txt` | MIT license; attribution retained. |
| `Lost_Signal_VHS/README.md` | Reviewed architecture, controls, preset descriptions, host requirements and known limitations. |
| `shaders/shaders.properties` | Parsed all 13 profiles, inheritance, public controls and profile-specific values; source for the imported preset table. |
| `shaders/settings.glsl` | Reviewed shader uniforms, defaults, ranges and option semantics. |
| `shaders/lib/analog_color.glsl` | Reviewed analog color transforms and color-grade helpers; related behavior maps to VHS-90 grade/color engines. |
| `shaders/composite.fsh` | Reviewed composite/tape signal pass: YIQ signal path, chroma/luma handling, phase and analog degradation. |
| `shaders/composite.vsh` | Reviewed vertex stage and host-specific coordinate/uniform setup. |
| `shaders/composite1.fsh` | Reviewed intermediate composite pass and signal handoff. |
| `shaders/composite1.vsh` | Reviewed intermediate vertex stage; host glue only. |
| `shaders/final.fsh` | Reviewed final pass: 4:3 crop, rounded overscan, lens distortion, digital zoom, frame repeat, focus hunt, camera HUD, temporal history, depth-aware effects and final color treatment. |
| `shaders/final.vsh` | Reviewed final vertex stage; host-specific coordinates and uniforms. |
| `shaders/gbuffers_basic.fsh` | Reviewed basic G-buffer pass; Minecraft host glue. |
| `shaders/gbuffers_basic.vsh` | Reviewed basic vertex pass; Minecraft host glue. |
| `shaders/gbuffers_textured.fsh` | Reviewed textured G-buffer pass; Minecraft host glue. |
| `shaders/gbuffers_textured.vsh` | Reviewed textured vertex pass; Minecraft host glue. |
| `shaders/gbuffers_textured_lit.fsh` | Reviewed lit textured G-buffer pass; Minecraft host glue. |
| `shaders/gbuffers_textured_lit.vsh` | Reviewed lit vertex pass; Minecraft host glue. |
| `shaders/lang/en_us.lang` | Reviewed English option labels and help text. |
| `shaders/lang/ru_ru.lang` | Reviewed Russian localization; no rendering code. |
| `Lost_Signal_VHS_v1.0_Initial.zip` | Historical release snapshot; superseded by v1.9. |
| `Lost_Signal_VHS_v1.1_MC26.2_Update.zip` | Historical compatibility snapshot; superseded. |
| `Lost_Signal_VHS_v1.2_Universal.zip` | Historical composite-stage snapshot; superseded. |
| `Lost_Signal_VHS_v1.3_Liminal_Signal.zip` | Historical liminal look snapshot; superseded. |
| `Lost_Signal_VHS_v1.6_Framebuffer_Fix.zip` | Historical framebuffer revision; superseded. |
| `Lost_Signal_VHS_v1.6.1_Framebuffer_Fix.zip` | Historical follow-up; superseded. |
| `Lost_Signal_VHS_v1.7_Composite_Decode.zip` | Historical composite decoder milestone; superseded by v1.9. |
| `Lost_Signal_VHS_v1.8_Analog_Fog.zip` | Historical analog fog milestone; superseded by v1.9. |
| `Lost_Signal_VHS_v1.8.1_Analog_Fog_Hotfix.zip` | Historical hotfix; superseded. |
| `Lost_Signal_VHS_v1.9_Signal_Physics.zip` | Latest authoritative archive; 13 profiles and complete signal-physics settings. |

## 3. Existing VHS-90 engines reused

The shader's VHS-specific behavior was compared with the canonical VHS-90 core. Existing modules were reused instead of duplicating them:

- Composite YIQ encode/decode and NTSC/PAL signal behavior: `VHS90CompositeEngine` and `VHS90WaveformEngine`.
- AGC/exposure pumping and receiver instability: `VHS90RfInstabilityEngine`.
- Persistent tape-coordinate defects, wear, scratches and dropout placement: `VHS90TapeSurfaceEngine`.
- Tracking tears, arbitrary signal regions and head-switch placement: `VHS90RegionEngine`, `VHS90HeadConditionEngine` and tape transport.
- Previous-frame history and field blending: `VHS90TemporalEngine`.
- REC/PLAY/timecode overlays: `VHS90OsdEngine`.
- Existing luma/chroma noise, phase drift, chroma loss, vertical blending, tape softness, ghosting and signal echo: existing signal, noise, detail and temporal engines.

The source's persistent tape-defect memory is not a separate engine in VHS-90: the existing tape-surface engine already anchors defects to the simulated tape coordinate. The imported profile mapping now activates the tape-surface master from the maximum of wear, defect density and defect strength, so defect memory is not accidentally disabled when a preset has low physical wear.

## 4. New native features integrated from Lost Signal

### Composite decoder selection

Added an AE **Composite Decoder** selector with:

- Consumer Notch
- One-Line Comb
- Two-Line Comb

The one-line and two-line comb paths already existed in the composite core and are now exposed to the AE UI. A separate five-tap Consumer Notch path was added and tested. All 13 Lost Signal profiles use the source's Consumer Notch mode.

### VHS playback frame and lens geometry

Added the **VHS Playback Frame** group:

- **VHS 4:3 Frame** — center-crops a widescreen source to the active 4:3 picture width and produces black side areas outside it.
- **Rounded Overscan** — final rounded-rectangle mask applied after the signal chain, so tape noise does not fill the outside corners.
- **Lens Distortion** — radial lens warp.
- **Digital Zoom** — centered horizontal and vertical zoom.

These were integrated into `VHS90AnalogGeometryEngine` and the final output stage of `VHS90CompositeEngine`, not left as detached shader code.

### Camcorder HUD battery

Added **OSD Battery** and a three-segment battery indicator to `VHS90OsdEngine`. The Camcorder 1996 profile enables the battery alongside the existing REC/PLAY/timecode OSD.

### Temporal frame repeat and field weaving

Added **Frame Repeat Frequency** and **Frame Repeat Strength** to the temporal controls. The native temporal engine uses a deterministic hash of frame index and tape seed to trigger a full-frame blend with the previous AE frame. The event is repeatable for the same frame/seed and is independent of the ordinary ghost amount.

The source's interlaced-field setting is also connected to VHS-90's existing temporal field-weave path, using frame parity and previous-frame data.

## 5. Imported 13 Lost Signal presets

All 13 v1.9 profile names are included in the unified **VHS Style Preset** list with an `LS` prefix to avoid collisions with other source packs:

1. LS Reference VHS
2. LS Real VHS
3. LS Backrooms
4. LS Poolrooms
5. LS Liminal Night
6. LS Subtle
7. LS Found Footage
8. LS Damaged Tape
9. LS Rental Tape
10. LS Camcorder 1996
11. LS Broken Signal
12. LS Composite Decode
13. LS Analog Fog

The embedded profile table carries 45 source values per profile: noise, scanlines, glitch frequency/strength, timebase error, static, chroma phase, AGC, chroma killer, sync glitches, chroma persistence/amount, flicker, frame jitter, wobble, motion smear, ghost/spatial echo, tape softness, chroma bleed, luma ringing, black crush, saturation, head switching, halation, vignette, tape format, generation loss, tape wear, interlace, tracking, RF interference, tape chew, defect density/strength, camcorder HUD, liminal/fog mode, lens distortion, 4:3 frame, rounded overscan, digital zoom and frame-repeat frequency/strength.

The unified preset popup now contains **38 entries** total:
- 16 VHS6RIFT looks
- 5 VHS Effect Add-on looks
- 3 Composite Video Simulator VHS SP/LP/EP profiles
- 13 Lost Signal VHS v1.9 profiles
- Custom

The current static validator reports **165 AE parameter slots** and confirms that UI registration order matches all ParamDiskID entries.

## 6. Source features that cannot be reproduced exactly from an ordinary AE RGB input

These were explicitly reviewed and not misrepresented as completed:

- **Depth-reprojected motion history** and **depth fog** require Minecraft depth textures and camera/world matrices. A normal AE effect receives image pixels, not the original 3D depth buffer. Reproducing this exactly would require a depth/auxiliary input or a separate depth-estimation system.
- **Liminal/Fog looks** are mapped to available color drift, vignette and analog glow controls. This is a 2D approximation, not the source's depth-aware volumetric fog.
- **Focus Hunt** and the exact source camera-era autofocus model remain separate unported behaviors. Camcorder 1996 currently includes the mapped zoom, OSD and battery behavior, not a claim of exact autofocus simulation.
- **Frame Repeat** is a native deterministic previous-frame blend adapted to AE's frame-history input. It is not the original shader's depth-reprojected history implementation.
- The 45 mapped values cover the controls that have meaningful VHS-90 equivalents. The remaining source options that depend on Minecraft world state, depth or camera matrices are not falsely mapped to unrelated controls.

## 7. Verification

- Static validator: **165 AE parameter slots**, ParamDiskID/UI order matched.
- Preset validator: **38 unified VHS Style entries**, including all 13 Lost Signal profiles.
- Linux CMake core build: passed.
- Core smoke tests: **32/32 passed**, including composite decoder, 4:3/lens/zoom, rounded overscan, battery OSD, temporal field weave and deterministic frame repeat.
- Final CI run: https://github.com/vamprieieuu/vhs---90/actions/runs/44810036059
- This does not establish Windows/MSVC Adobe SDK compilation, PiPL validation, AEX loading, After Effects rendering or MFR runtime success. Those remain global release gates.

## 8. Folder disposition

**File audit: complete (31/31).**  
**v1.9 release/profile review: complete (13/13 profiles; 77 public options identified).**  
**Useful VHS behavior: reused or integrated into the canonical VHS-90 C++ engines.**  
**Preset migration: complete for all 13 source profiles, with host-specific depth/fog/camera limitations recorded.**  
**Final Linux CI: passed (165 AE slots validated; 32/32 core tests).**  
**Next folder in the user's specified order: `ntscqt-1.5.0`.**
