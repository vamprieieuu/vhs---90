# BarashNTSC — complete repository inventory and VHS-90 port audit

Source: `LittleBlabbit763/BarashNTSC`
Audited upstream tree: `main`
Tree inventory: 10 files, 2 directories (repository root + `presets/`).
License: MIT-style permission grant; retain copyright and permission notice when redistributing copied source portions. The VHS-90 ports are independent C++ adaptations of algorithm behavior, not copied Python/OpenCV implementation.

## File-by-file inventory

| File | Review / disposition |
|---|---|
| `BarashNTSC.py` (66,208 bytes) | Core processor and application inspected. Algorithmic stages in `VHSProcessor.process` reviewed; preview/export/audio/Tkinter/FFmpeg orchestration classified as host-specific and not portable to an AE effect. |
| `License` | MIT-style terms reviewed; attribution requirement recorded. |
| `README.md` | Scope, dependencies, inspirations and user-facing feature set reviewed. |
| `requirements.txt` | Python/OpenCV/NumPy/Pillow/sounddevice dependencies identified; not imported into native plugin. |
| `hook_ffmpeg.py` | PyInstaller PATH setup only; excluded as packaging-specific. |
| `make_icon.py` | PIL icon conversion only; excluded as packaging-specific. |
| `presets/example_ANALOG.json` | Parameter schema and analog look values reviewed. |
| `presets/example_worn_tape.json` | Worn-tape parameter schema and values reviewed. |
| `icon.ico` | Binary application icon; non-algorithm asset, not migrated. |
| `icon.png` | Binary application icon; non-algorithm asset, not migrated. |

## Algorithm inventory and VHS-90 destinations

| BarashNTSC stage | VHS-90 destination / status |
|---|---|
| Horizontal luma detail reduction and reconstruction | `VHS90TapeDetailEngine`; connected before composite encoding; AE control `Tape Detail`. |
| Luma/chroma noise | Existing `VHS90AnalogNoiseEngine` and static/noise modules; independent controls already exist. The upstream NumPy random-noise implementation was not copied. |
| Chroma bandwidth reduction / bleed | Existing `processChromaSubsample`, vertical chroma blend and composite bandwidth pipeline. |
| Opposing Cr/Cb horizontal displacement | New `VHS90ChromaShiftEngine`; connected before composite encoding; AE control `Chroma Shift`. |
| Frame-driven color phase instability | Added to `VHS90ReceiverInstabilityEngine` as `chromaPhaseDrift`; AE control `Color Phase Drift`. |
| Composite carrier / rainbow artifacts | Existing shared composite encoder/decoder pipeline; no Python carrier arrays or OpenCV runtime dependency. |
| Horizontal edge enhancement, tape fade, contrast, brightness, saturation, vignette, flicker | `VHS90ImageGradeEngine`; connected in the shared pipeline and exposed as AE controls. |
| Previous-frame motion ghost | `VHS90TemporalEngine` is now wired to AE SmartFX using a second declared checkout ID for the immediately preceding source frame; previous pixels are checked out/in per render, passed through render-local buffers and never stored in global mutable history. The first frame (or unavailable previous frame) safely skips the ghost. Source-level wiring is committed; Windows AE SDK build/runtime and MFR validation remain pending. |
| Interlace field weave | `VHS90AnalogGeometryEngine` now includes deterministic alternating-field horizontal phase and subpixel vertical offset; AE control `Interlace / Field Weave`. |
| Tracking band / line jitter | Existing region/tracking and geometry/transport modules cover the portable behavior. Python random-number sequence is not reused. |
| Tape wear / dropouts | Existing tape-surface defect and impulse-noise engines cover the procedural defect families; per-frame randomness is deterministic in VHS-90. |
| Scanlines | Existing `processScanlines`. |

## New C++ files added

- `Source/VHS90Core/VHS90TapeDetailEngine.h/.cpp`
- `Source/VHS90Core/VHS90ChromaShiftEngine.h/.cpp`
- `Tests/VHS90TapeDetailEngineSmoke.cpp`
- `Tests/VHS90ChromaShiftEngineSmoke.cpp`

Additional edits connect these engines to `VHS90CompositeEngine`, add their smoke coverage to CMake, and expose their AE controls. The image-grade module is a direct C++ adaptation of Barash's edge/fade/grade operations. The receiver module is shared with Analog-NTSC-derived receiver effects; its global chroma-phase drift control was specifically ported from Barash's frame-driven phase formula. The previous-frame ghost now uses a SmartFX previous-time checkout rather than unsafe global temporal state.

## Verification boundary

- At commit `419f4adbea2638643085737416b562f6aea6ffe8`, GitHub Actions run `44600396446` built the core and passed **18/18 CTest tests**, including composite integration of Tape Detail and the standalone detail test.
- Core CI run `44602045505` passed 20/20 tests, including the tape-detail and waveform paths.
- GitHub Actions run `44602823951` at commit `fdeb1df2c5b7af7a568710e15e9530df1ab1cf6b` passed **20/20 CTest tests** after connecting previous-frame ghosting to the recursive composite pipeline.
- Static AE parameter audit: **67 real parameter slots and 67 `PF_ADD_*` calls**.
- Previous-frame ghost uses a second SmartFX checkout ID, explicit previous-time checkout and per-render pixel checkout/checkin. No global frame cache is used. The previous frame is run through the same composite pipeline before ghost mixing. Windows/MSVC AE build and live AE/MFR validation remain unverified.
