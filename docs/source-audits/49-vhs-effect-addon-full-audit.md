# VHS-90 — VHS Effect Add-on 1.0.0 Full Source Audit & Integration

Date: 2026-11-20  
Input archive: `vhs.zip`  
Exact source path: `vhs/github repos/VHS_effect_add-on-1.0.0/`  
Source inventory: 20 entries (17 files + 3 directories)  
VHS-90 integration commit: `a452637e54e34ac7e83627587b548114224cd494`  
Validator follow-up commit: `f79b45763c7a53a3407fa740b5efa116aca7e434`

## 1. Inventory and file disposition

Every entry in this folder was inventoried and assigned a disposition. The three directories are structural containers; all 17 file entries were inspected/classified.

| Source path | Disposition |
|---|---|
| `README.md` | Chinese user/technical documentation reviewed: controls, presets, timestamp semantics, processing order, performance, library API and licensing notes. |
| `README.en.md` | English documentation reviewed as a parallel description; no separate native engine implementation. |
| `check_env.py` | Environment/dependency/layout/FFmpeg checks. Python-app bootstrap only; not part of AE rendering. |
| `gui.py` | Tkinter application UI and controls reviewed. GUI state/settings, preset selection and dispatch are application-only; do not port Tkinter widgets into AE. |
| `requirements.txt` | NumPy, Pillow, OpenCV, SciPy and imageio-ffmpeg dependencies classified. None is required by VHS90Core. |
| `selftest.py` | Test harness reviewed and executed against generated image/video fixtures; result recorded below. |
| `setup_deps.py` | Python dependency downloader/installer classified as application setup; excluded from plugin. |
| `vhs_config.json` | Default configuration reviewed; classic preset, strength 1.0, empty overrides. |
| `vhs_lib/__init__.py` | Public Python library exports reviewed; wrapper/API only. |
| `vhs_lib/vhs_effects.py` | Main processing core reviewed: VHSParams, five preset definitions, color conversion, anisotropic chroma bleed/subsampling/offset, time-base error, head switching, tracking band, dropouts, noise, optical processing, scanlines/vignette, OSD and frame pipeline. |
| `vhs_lib/vhs_task.py` | File discovery, batch mapping, image/video orchestration, interruption and cleanup reviewed; host-app orchestration, not render-kernel code. |
| `vhs_lib/vhs_video.py` | FFmpeg/ffprobe discovery, encoding, audio extraction/muxing, frame reader/writer and resizing reviewed; external media pipeline, not portable to the AE pixel kernel. |
| `vhs_processor.py` | CLI argument/configuration assembly and batch/image/video/preset commands reviewed; excluded as application shell. |
| `_gui_settings.json` | Persisted GUI settings schema reviewed; not imported into AE. |
| `安装依赖.bat` | Windows dependency-install launcher classified; excluded. |
| `运行-命令行.bat` | CLI launcher classified; excluded. |
| `运行-图形界面.bat` | GUI launcher classified; excluded. |
| `运行-自检.bat` | Self-test launcher classified; excluded. |
| `运行-命令行.bat` / `运行-图形界面.bat` / `运行-自检.bat` | All three launchers were inspected as wrappers only; no unique image-processing algorithm. |

No separate project `LICENSE` file was present in the folder. README documents licenses for bundled third-party components, including a GPLv3 FFmpeg build, but that is not a license grant for copying the project's own Python source. Therefore no Python implementation was copied verbatim into VHS-90. The port below is a native parameter/behavior adaptation using VHS-90's independently maintained C++ engines. Preserve upstream attribution in this audit; do not bundle the source app's FFmpeg binary or Python dependencies.

## 2. Source algorithms and mapping into the existing engine

The source has ten ordered stages. The following distinguishes existing VHS-90 functionality from the new work in this pass.

| Add-on behavior | VHS-90 destination / decision |
|---|---|
| YCbCr chroma low-pass with stronger horizontal than vertical blur | Covered by the existing composite/chroma bandwidth pipeline; do not add a duplicate blur implementation. |
| Chroma downsample/upsample and red/blue horizontal separation | Connected to existing `Chroma Subsample / Bleed` and `Chroma Shift` controls through the five imported preset mappings. VHS-90's signal path remains authoritative. |
| Whole-frame jitter, row jitter and vertical roll | Existing `VHS90AnalogGeometryEngine` / tape transport and `Vertical Roll` paths are reused. |
| Bottom head-switch band with tear/noise/luma kick | Existing `VHS90RegionEngine` Head Switching path is reused. The native arbitrary-region controls are retained; the source's fixed bottom-only limitation is not reintroduced. |
| Moving tracking band with tear, attenuation and noise | Existing Tracking Noise region engine is reused. Its arbitrary placement/size/angle and deterministic frame behavior supersede the source's single fixed moving band. |
| Short bright/dark dropout segments | Existing `VHS90DropoutEngine` and tape-dropout controls are reused; no second dropout generator added. |
| Coarse/fine luma and chroma noise, dirty-edge modulation | Existing `VHS90AnalogNoiseEngine`, static and tape-noise paths are reused. |
| Softness, bloom and edge enhancement | Existing Tape Detail, Bloom/Glow and Edge Enhancement paths are reused. |
| Contrast, brightness, saturation and tint | Existing `VHS90ImageGradeEngine` controls are reused. The source's green/sepia tint is not an exact one-to-one match to the current native grading controls and is not claimed as pixel-identical. |
| REC/PLAY/SP/date-time OSD, blink, ghost, scale and jitter | Existing `VHS90OsdEngine` and AE OSD controls are reused. VHS-90's required pre-damage OSD composition order remains authoritative; the Add-on's post-damage OSD order is deliberately not copied. |

The Add-on's Python `VHSContext` caches, NumPy array operations, Pillow text rasterization, Tkinter GUI, batch runner, FFmpeg writer and audio muxing are not copied. They are either application-level wrappers or incompatible with the native AE render/MFR model.

## 3. Newly connected preset entries

Five source presets have been added to the existing AE `VHS Style Preset` popup. They remain selectable with the existing `Style Intensity` blend control.

| New AE entry | Source preset | Key translated controls |
|---|---|---|
| Add-on Light | `light` | Mild chroma subsample/shift, luma/chroma/tape noise, scanlines, vignette, geometry, head switch, tracking, dropout, softness, bloom and grade. |
| Add-on Classic | `classic` | Source default balance: stronger chroma bleed, medium noise, scanlines, head switch/tracking, softness and bloom. |
| Add-on Heavy | `heavy` | High chroma reduction/shift, stronger noise, geometry, tracking/head switching, dropouts, softness and bloom. |
| Add-on Worn | `worn` | Maximum chroma degradation/noise/dropout, increased head-switch height, tracking, geometry, softness, bloom and darker grade. |
| Add-on Clean TV | `clean_tv` | Low noise, moderate scanlines/vignette, mild chroma degradation, little tracking/head-switching, no dropout. |

Implementation: `Source/VHS90Effect.cpp` extends `kVhsStyleChoices` from 17 to 22 entries, extends the AE popup count to 22, and adds cases 18–22 in `ApplyVhs6RiftStylePreset`. Values are blended against the user's current controls using the existing style-intensity mechanism. The imported settings are mapped to the native VHS-90 control model; they are not represented as a pixel-exact clone of the Python pipeline. Source controls without an exact VHS-90 equivalent (including independent Gaussian-noise mean, sepia tint, custom OSD text and Python-specific image-size behavior) are not falsely claimed as exact ports.

The static preset validator now preserves the existing 16 VHS6RIFT style checks and additionally asserts that Add-on cases 18–22 exist and that the popup exposes 22 entries.

## 4. Test results

### Source application's own self-test

Executed `selftest.py` using generated 320×240 image and 0.6-second video fixtures, with a 0.4-second video processing limit:

- Environment: Python 3.13.5, NumPy 2.3.5, Pillow 12.3.0, OpenCV available, FFmpeg available.
- Date/time parsing, fixed/advancing timestamps, REC blink/color, parameter overrides, strength scaling, preset differentiation, invalid timestamp rejection, folder mirroring, image processing and video processing all passed.
- Result: **30/30 checks passed**.
- The archive did not contain its documented `Material/` examples, so the first default self-test attempt could not run; the second run used generated fixtures and passed. The generated fixtures are not upstream reference media.

### VHS-90 CI

- The first push triggered validator failure because the existing static validator intentionally expected exactly 16 style cases. The failure was isolated to that outdated assertion; no C++ core compile/test step ran on that attempt.
- The validator was updated to require the original 16 VHS6RIFT cases plus five Add-on cases and 22 popup entries.
- Follow-up workflow run is associated with commit `f79b45763c7a53a3407fa740b5efa116aca7e434`; final conclusion must be read from the Actions run before this folder can be marked closed.
- Linux core CI does not compile `VHS90Effect.cpp` against the Adobe SDK and does not establish Windows/MSVC, PiPL, AEX load, AE render or MFR success.

## 5. Remaining verification boundary

- The five presets are connected to the AE popup and native signal/tape controls, but the new AE adapter code has not been compiled against the exact local AE SDK in this environment.
- Source-vs-native visual parity has not been performed against the upstream author's reference media. The source archive supplied no `Material/` folder.
- Windows/MSVC and live After Effects validation remain part of the global project gate, not a completed claim of this source audit.
- No upstream Python source or GPL FFmpeg binary was copied into the native plugin.

## 6. Folder disposition

**Source inventory and code review: complete at file-disposition level (20/20 entries).**  
**Portable behavior: mapped to existing VHS-90 engines; five Add-on presets newly wired.**  
**Native AE build/runtime parity: pending the global Windows/AE validation gate.**

Do not advance the next repository based only on the source audit. First confirm the follow-up CI run and retain the limitations above in the project status.
