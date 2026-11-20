# VHS-90 — Composite Video Simulator Full Audit & Native Integration

Date: 2026-11-20  
Source archive path: `vhs/github repos/composite-video-simulator-master/`  
Inventory: **36/36 files** (20 C++ translation units, 16 support/documentation/assets)  
Integration: native VHS playback chroma sharpening + three source-derived VHS speed profiles

## 1. License and compatibility

The source folder contains the GNU General Public License, Version 2 (GPL-2.0-only). Its README describes a deliberately slow, accuracy-oriented analog composite simulator built around FFmpeg 3.x and Linux. The source is a collection of separate command-line tools, not one reusable library.

No upstream C++ file was copied verbatim into VHS-90 and no FFmpeg executable/library was bundled. The native change below is a new VHS-90 implementation of the documented three-stage chroma low-pass/unsharp behavior, with a separate AE control and tape-speed mapping. Because it intentionally reproduces behavior described in GPL-2.0 source, this audit does **not** make a legal determination that the implementation is independent of GPL obligations. Before distributing a closed-source/proprietary AEX, the project's licensing policy should be checked with qualified counsel or the component should be replaced with a demonstrably clean-room specification. This is a real release consideration, not a technical build failure.

## 2. Complete file inventory and disposition

All 36 files were inspected and assigned a disposition.

| File | Findings / disposition |
|---|---|
| `.gitignore` | Build-artifact exclusions only; no engine. |
| `AUTHORS` | Empty placeholder (1 byte). |
| `ChangeLog` | Empty placeholder (1 byte). |
| `LICENSE` | GPL version 2; distribution compatibility flagged above. |
| `Makefile.am` | Automake targets for separate FFmpeg command-line tools; not reusable in AE CMake. |
| `NEWS` | Empty placeholder (1 byte). |
| `README` | Project purpose, accuracy/performance warning, composite/VHS scope and historical notes reviewed. |
| `aclocal.m4` | Generated Autotools macros; build infrastructure only. |
| `autogen.sh` | Autotools regeneration wrapper; not part of the effect. |
| `configure.ac` | Requires legacy FFmpeg 3.x libraries and Linux-oriented configure checks; incompatible with the AE SDK build. |
| `ffmpeg_average_delay.cpp` | FFmpeg decode/encode shell plus delayed frame averaging and a time-varying pixel pattern; temporal video utility, not a VHS signal engine. |
| `ffmpeg_cassette.cpp` | Audio-only cassette filtering, attenuation and noise chain; not an image effect and requires an audio architecture to use. |
| `ffmpeg_colorkey.cpp` | Multi-input black-key/layer compositing utility; no VHS signal engine. |
| `ffmpeg_colormap.cpp` | Samples a 256-entry map from a source scanline and maps grayscale values through it; palette utility, not core VHS. |
| `ffmpeg_ntsc.cpp` | Earlier NTSC/YIQ composite and VHS implementation; reviewed for phase, low-pass, head-switching, tape speed, chroma vertical blend and sharpening. Overlaps the later `ffmpeg_to_composite.cpp` path and existing VHS-90 engines. |
| `ffmpeg_posterize.cpp` | ADC-style bit-depth truncation/quantization utility; not a VHS transport or composite engine. |
| `ffmpeg_raw28ntsc.cpp` | Final raw 28.636 MHz composite sample-stream decoder: sync detection, black/white level calibration, sample-rate timing and chroma separation. It expects raw waveform bytes, not an AE RGB frame, so it is not a direct image-effect port. |
| `ffmpeg_raw28ntsc_step1.cpp` | First development snapshot of the raw-sample decoder. |
| `ffmpeg_raw28ntsc_step2.cpp` | Adds further sync/line handling to the raw-sample decoder. |
| `ffmpeg_raw28ntsc_step3.cpp` | Intermediate raw-sample decoder refinement. |
| `ffmpeg_raw28ntsc_step4.cpp` | Adds filtered sync/DC level handling and raw sample calibration. |
| `ffmpeg_raw28ntsc_step5.cpp` | Intermediate chroma extraction refinement. |
| `ffmpeg_raw28ntsc_step6.cpp` | Adds raw-luma fallback / sync marker behavior. |
| `ffmpeg_raw28ntsc_step7.cpp` | Near-final raw-sample decoder snapshot. The seven step files are development history, not seven independent engines; the final implementation is represented by `ffmpeg_raw28ntsc.cpp`. |
| `ffmpeg_scanimate.cpp` | CRT phosphor-dot raster renderer with time-cycled trapezoid, vertical stretch, vertical rotate and diffuse raster warps. Technically interesting, but a separate CRT-display engine rather than a VHS/composite tape subsystem; retained as a future CRT-mode candidate, not silently inserted into the VHS chain. |
| `ffmpeg_to_composite.cpp` | Main composite/VHS path: YUV composite carrier encode/decode, NTSC/PAL phase handling, cascaded chroma filtering, luma/chroma noise, head switching, VHS SP/LP/EP bandwidth, chroma delay and vertical blend, independent luma/chroma playback sharpening, optional audio emulation and FFmpeg I/O. Core VHS behaviors were mapped to VHS-90's existing signal/tape modules; the chroma playback-sharpening gap was implemented below. |
| `ffmpeg_vhsled.cpp` | Per-row black-border detection, smoothed horizontal alignment and crop/shift utility; not a VHS damage engine. |
| `filmac.cpp` | FFmpeg media conversion/gamma-transfer path; not a VHS transport or composite effect. |
| `flac2rawpipe.sh` | Audio sample extraction helper; not used by the AE video effect. |
| `frameblend.cpp` | Time-weighted frame-rate conversion/interpolation with near-match squelching; overlaps temporal concepts but depends on multi-frame decode/timing and future-frame access, so it is not wired into the AE per-frame effect. |
| `git-source-pickup.pl` | Source archive packaging helper; no effect logic. |
| `git-update-all` | Destructive/interactive legacy git-clean/commit/push helper; not executed. |
| `git-update-all-wo-push` | Legacy git-clean/commit helper; not executed. |
| `normalize_ts.cpp` | Timestamp normalization utility; not an image-processing engine. |
| `papermario.jpg` | Example/test artwork, not an algorithm or preset. |
| `text2vaporwave.pl` | ASCII-to-fullwidth-Unicode text converter; unrelated to VHS rendering. |

## 3. Engine reuse and duplicate elimination

The source's main video path was compared with the current VHS-90 core. The following capabilities already exist and were reused rather than duplicated:

- Composite carrier/phase encode-decode: `VHS90CompositeEngine` and `VHS90WaveformEngine`.
- NTSC/PAL phase and line/field timing: existing composite signal configuration.
- Chroma low-pass/subsampling and vertical line blending: existing composite bandwidth, `Chroma Subsample` and `Vertical Chroma` paths.
- Tape SP/LP/EP speed selection, transport, tracking and head switching: existing VHS-90 tape/transport/region controls.
- Luma enhancement and overall grade: existing `ImageGrade` stage.
- Noise, chroma phase instability and dropouts: existing analog-noise, receiver-instability and dropout engines.
- FFmpeg demux/mux, audio processing and command-line orchestration: excluded because AE supplies frames and manages host rendering, timing, audio and MFR.

The raw-28 MHz decoder is a genuine separate subsystem, but its input contract is raw sampled composite waveform. It cannot be meaningfully attached to an ordinary AE RGB frame without a raw-capture/sample-stream source. The Scanimate CRT raster renderer is also a separate display simulation and remains a candidate for a future CRT-specific engine, not a VHS engine.

## 4. New native VHS playback chroma-sharpening integration

The source's VHS playback stage independently sharpens luma and chroma after VHS bandwidth loss. VHS-90 already had luma edge enhancement, but did not expose an independent I/Q chroma sharpening control. This missing behavior was added:

- New AE group: **VHS Playback**.
- New AE slider: **Chroma Edge Enhancement** (0–100%, default 0).
- New core setting: `ImageGradeParams::chromaEdgeEnhancement`.
- New cutoff setting: `ImageGradeParams::chromaSharpenCutoffHz`.
- Implementation: three cascaded first-order low-pass states per scanline for I and Q, followed by an independent chroma unsharp gain. Alpha is preserved; the effect does not increase luma edge contrast.
- Tape-speed cutoff mapping follows the source VHS playback values: SP 640 kHz, LP 600 kHz, EP 560 kHz for the playback sharpen reference. The filter resets to neutral chroma at each scanline.
- The existing luma `Edge Enhancement` control remains independent and is reused for the source's luma playback sharpening concept.

Files changed:
- `Source/VHS90Core/VHS90ImageGradeEngine.h`
- `Source/VHS90Core/VHS90ImageGradeEngine.cpp`
- `Source/VHS90Effect.cpp`
- `Tests/VHS90ImageGradeEngineSmoke.cpp`
- `tools/validate_imported_presets.py`

## 5. Composite Video Simulator VHS speed profiles in the preset menu

The source exposes VHS tape speeds rather than a separate named list of artistic presets. To preserve these source operating modes in the unified VHS-90 preset list, three entries were added:

| VHS-90 preset | Source mode | Settings applied |
|---|---|---|
| `CVS VHS SP` | VHS SP | Composite 100, vertical chroma blend 50, luma edge enhancement 100, chroma edge enhancement 85, tape speed SP. |
| `CVS VHS LP` | VHS LP | Same playback path, tape speed LP. |
| `CVS VHS EP` | VHS EP | Same playback path, tape speed EP. |

These are clearly labelled source-derived speed profiles, not claimed to be three original upstream preset files. They are registered in the existing unified `VHS Style Preset` popup, which now has 25 entries total (Custom + 16 VHS6RIFT styles + 5 VHS Effect Add-on looks + 3 CVS VHS speed profiles). They blend through the existing `Style Intensity` control.

## 6. Verification

- The static AE preset/parameter validator reports **153 AE parameter slots** and confirms the PF_ADD registration order matches the ParamDiskID order.
- The validator confirms 16 VHS6RIFT styles + 5 VHS Effect Add-on looks + 3 CVS VHS speed profiles, 9 camera profiles and 10 NTSC-RS-derived VHS looks.
- Linux CMake build completed and all **32/32 VHS-90 core smoke tests passed**, including `VHS90ImageGradeEngineSmoke` with independent chroma enhancement and alpha preservation.
- Passing CI run for the new chroma implementation and tests: https://github.com/vamprieieuu/vhs---90/actions/runs/44807065869
- The latest follow-up validator run for the final UI/preset state must be confirmed before the folder is marked closed: https://github.com/vamprieieuu/vhs---90/actions/runs/44807131889
- These Linux tests do not compile `VHS90Effect.cpp` against the Adobe SDK and do not establish Windows/MSVC, PiPL, AEX loading, in-host rendering or MFR success. Those remain global release gates.

## 7. Folder disposition

**File audit: complete (36/36).**  
**VHS-relevant behavior: mapped to existing native engines; independent chroma playback sharpening added.**  
**Source speed modes: represented by three unified VHS-90 presets.**  
**Unrelated utility/CRT/audio/raw-sample tools: classified and intentionally not mixed into the VHS tape engine.**  
**GPL-2.0 distribution compatibility: flagged for project licensing review.**  
**Windows/AE runtime validation: still pending globally.**
