# VHS_effect_add-on-1.0.0 — source audit and VHS-90 integration

## Source inventory
Audited all 18 files (253,203 bytes): vhs_lib/vhs_effects.py (1,411 lines), vhs_lib/vhs_video.py (617), vhs_lib/vhs_task.py (296), vhs_processor.py (770), gui.py (1,005), selftest.py (461), setup_deps.py (201), check_env.py (143), vhs_lib/__init__.py, both READMEs, requirements.txt, vhs_config.json, _gui_settings.json, and four Windows batch launchers.

## Reuse decision
The image/video file handling, Tk GUI, CLI, FFmpeg/audio pipeline, folder mirroring, and Python dependency installer are desktop-app infrastructure, not After Effects render kernels. They are not copied into the native plug-in.

The source's chroma bleed/subsampling, line/time-base jitter, vertical roll, head switching, tracking bands, dropouts, multi-scale noise, softness/sharpening, bloom, grade/tint, scanlines and vignette already have equivalents in VHS-90's shared C++ signal/tape pipeline. They were not duplicated as separate stages.

The source's camcorder OSD is distinct and was transferred: blinking REC lamp with a user-selectable lamp color, PLAY/SP status, date/time progression, size/opacity, frame-varying jitter and chromatic ghost.

## Native implementation
Added VHS90OsdEngine.h/.cpp and VHS90OsdEngineSmoke.cpp. The engine uses a built-in 5x7 glyph set for Latin capitals, digits and punctuation; it has no Python, Pillow, external-font, GDI or runtime font-discovery dependency. It supports configurable REC/PLAY/time elements, a native REC-lamp color picker, opacity, amount, scale, deterministic frame jitter, ghosting, REC blink frequency, timestamp speed and start date/time. It composites correctly over straight or premultiplied RGBA, including transparent pixels.

The AE controls are grouped under Camcorder OSD. OSD Amount defaults to zero to preserve existing renders. The default start time is 1994-06-11 12:00:00 and can be adjusted in the controls; the REC lamp color is also editable.

## Source issues noted
- _apply_chroma_bleed accepts unbounded chroma_subsample; values above 1 can create negative blend weights.
- README.en.md understates dependencies: SciPy and imageio-ffmpeg are also declared; video/audio capability depends on FFmpeg availability.
- _gui_settings.json contains machine-specific absolute Windows paths and is not suitable for an AE effect.
- The Python GUI/video stack must not execute from an AE render callback.

## Validation
- All 18 files were inventoried; Python sources parsed with ast and compiled with python -m compileall.
- Local dependencies were detected and the source classic preset rendered a synthetic 320x180 frame.
- Native OSD smoke test compiled with g++ -std=c++17 -Wall -Wextra -Wpedantic and passed (LOCAL_OSD_SMOKE_OK): deterministic frame output, alpha preservation, red and user-customized REC lamp colors, advancing timestamp, disabled pass-through, transparent-pixel compositing and invalid-stride rejection.
- GitHub Actions run #651 completed successfully for the integration commit; the Linux core build and all smoke tests passed, including VHS90OsdEngineSmoke. This is native core CI, not a Windows After Effects SDK/AEX build.
