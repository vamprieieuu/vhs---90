# vhs.py-main — VHS-90 integration audit

Source: `vhs/github repos/vhs.py-main` from the supplied `vhs.zip`.
Inventory: 9 files (2 Python, 2 JavaScript, 1 HTML, 1 CSS, 1 Markdown, 1 requirements file, 1 gitignore); source payload 138,272 bytes.

## Source audit

All nine files were inspected line-by-line:
- `vhs.py` (257 lines): class constructor and every processing helper, including YCrCb conversion, luma/chroma compression, Gaussian/uniform noise, Lanczos resizing, row waves, switch-noise warp, blur, two sharpening paths, frequency-domain ringing, YIQ conversion, generation multiplier, and final process order.
- `index.py` (91 lines): Eel/Tk file selection, preview-frame conversion, video capture/export, unused noise-video capture, fixed output dimensions, audio mux helper, and shutdown bridge.
- `ui/index.html` (79 lines plus embedded base64 preview): control ranges, initial image, and Eel/JS wiring.
- `ui/js/core.js` (complete): parameter updates, preset JSON export, frame save, record trigger, reset, and close handling.
- `ui/js/FileSaver.js`: third-party FileSaver implementation; its X11/MIT notice is retained in the source.
- `ui/css/water.css`: presentation-only stylesheet.
- `README.md`, `requirements.txt`, and `.gitignore`.

### Source defects / limitations found

- `applyRinging` calls `ringing` with an unsupported `clip` keyword and does not match that method's required arguments; the path is not used by `processAll`.
- `cut_black_line_border` uses `image[:, -0:]` when the computed width is zero, which blacks out the entire image. A similar zero-width slice in `processFrame` can zero the entire green channel.
- `processFrame` applies the same chroma wave twice; the wave functions use tiny integer-truncated offsets and global random state, so results are not stable for AE multi-frame/out-of-order rendering.
- `sharpen_image` performs unsigned 8-bit subtraction before normalization, allowing wraparound; the UI's reset routine also leaves the two blur sliders unchanged.
- The JS close bridge calls `on_window_close`, while Python exposes `on_windows_close`; the video writer forces 640x480 and the opened noise video is unused. Audio muxing is separate from the effect.
- `requirements.txt` lists numerous mutually conflicting OpenCV wheel variants, so it is not a reliable reproducible install manifest.
- No automated tests are included in this source folder. Python syntax compilation passes; runtime behavior of the original GUI/video stack was not treated as a validation of the AE engine.

## Reuse decisions

- Ported the distinctive independent luma/chroma horizontal compression and separate noise path into `VHS90PyVhsGenerationEngine`, including deterministic per-frame chroma row-wave displacement.
- Noise is applied on the reduced horizontal channel before upsampling, matching the source's downsample → noise → upsample ordering. Luma uses Gaussian noise; Cr/Cb use independent uniform noise.
- Uses an 8-tap Lanczos-4 horizontal resampler, matching the source's OpenCV `INTER_LANCZOS4` choice; replaces global random state with seed/frame deterministic noise for AE multi-frame rendering.
- Keeps alpha intact and supports premultiplied input.
- Existing VHS-90 analog geometry, image grade, tape detail, chroma subsampling, and blur/sharpen-related stages already cover the source's generic warp, blur, edge enhancement, and sharpening concepts; those were not duplicated.
- The source's right-edge green-channel zeroing and zero-width slicing quirks were treated as implementation defects, not reproduced.
- The Python GUI, Eel bridge, Tk dialogs, preview window, audio muxing, and video export are excluded from the native plugin core.

## Integration

The new engine is part of the shared C++ Composite pipeline and exposed as a dedicated **Python VHS Generation** topic in the AE effect UI:
- Luma Compression
- Chroma Compression
- Luma Noise
- Chroma Noise
- Chroma Wave
- Generation

Defaults are neutral/pass-through. Seed and frame are derived deterministically from the effect render context.

## Validation

- `python -m compileall -q .` passes for the original Python source.
- A dedicated C++ smoke test is committed and CMake registers both the engine and test. The test covers neutral pass-through, active effect, repeatable same-frame output, and alpha preservation.
- The engine translation unit and smoke-test logic were also compiled and executed locally with a minimal stand-in for the shared hash/image-view declarations: `LOCAL_ENGINE_SMOKE_OK`. This validates the new engine's C++ syntax and its local behavior, but is not a substitute for the full VHS-90 CMake build.
- The GitHub Actions run listing for the newest push commits is not available through the connected run-list endpoint, and the commit-status endpoint returned no status entries. Therefore this report does not claim a successful full-repository C++ CI run.
- Native Windows AE SDK packaging remains separate from host-independent core CI.
