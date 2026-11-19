# NTSC-VHS-Renderer 1.3.0 — complete repository audit and VHS-90 port record

Source: F33RNI/NTSC-VHS-Renderer, exact tag 1.3.0.
Recursive inventory at tag 1.3.0: 41 entries, 32 files and 9 directories (truncated=false). This matches the supplied NTSC-VHS-Renderer-1.3.0 archive folder.
Main application license: Apache-2.0. Bundled BloomEffect source: MIT, Copyright (c) 2019 Yoann Berenguer. Bundled NTSC-CRT has its own permissive custom license. The three license texts are retained in ThirdParty/NTSCVHSRenderer/ and ThirdParty/NTSC-CRT/.

## File-by-file inventory (32/32)

| File | Review / disposition |
|---|---|
| .gitignore | Build/temp exclusions only |
| FramesProcessor.py | Full 810-line processing and frame scheduler reviewed. It extracts BMP frame batches with FFmpeg, launches the NTSC/NTSC-VHS CLI, streams completed frames, applies Pillow brightness/contrast/saturation/sharpness and NumPy vignette, applies BloomEffect, then writes preview/video/JPEG output. These post effects map to existing VHS-90 native engines; subprocess, buffering, file paths and UI progress are host-specific. |
| GUI.py | Full 857-line PyQt5 window, preview/timeline, file open, render controls, pause/resume/abort, drag/drop, status and config update flow reviewed; no signal-processing kernel. |
| JSONReaderWriter.py | JSON load/save helpers and multiprocessing DictProxy handling reviewed; infrastructure only. |
| LICENSE | Apache-2.0 reviewed and copied to ThirdParty/NTSCVHSRenderer/LICENSE-APACHE-2.0. |
| LoggingHandler.py | Multiprocessing queue listener, file/console handlers and status-bar relay reviewed; infrastructure only. |
| NTSC-CRT/LICENSE | Upstream NTSC-CRT permissive custom license reviewed; retained under ThirdParty/NTSC-CRT/LICENSE. |
| NTSC-CRT/build/Release/ntsc_video.exe | Bundled compiled NTSC executable; not imported into the AEX. |
| NTSC-CRT/build/Release/ntsc_video_vhs.exe | Bundled compiled VHS executable; not imported into the AEX. |
| NTSC-VHS-Renderer.spec | PyInstaller bundling manifest and included resources reviewed; host packaging only. |
| README.md | Dependencies, usage, NTSC/VHS modes, frame workflow, build instructions and TODOs reviewed. |
| bloom/LICENSE | MIT license for BloomEffect reviewed and copied to ThirdParty/NTSCVHSRenderer/bloom-LICENSE-MIT. |
| bloom/bloom.c | 1.8 MB generated Cython C output; generated derivative of bloom.pyx, not treated as the editable algorithm source. |
| bloom/bloom.cp310-win_amd64.pyd | Compiled CPython 3.10 Windows x64 extension; binary-only, not portable to AE. |
| bloom/bloom.pxd | Cython declarations for RGB/RGBA blur, bright-pass, mask, filtering, scaling and bloom entry points reviewed. |
| bloom/bloom.pyx | Full 1,843-line editable bloom implementation reviewed: 5×5 separable Gaussian, BT.601 bright-pass, optional masks, x2/x4/x8/x16 downscale, repeated smoothing, bilinear upsample, additive composition, RGB/RGBA paths and fast x16-only approximation. |
| bloom/deprecated.pyx | Full 1,312-line legacy buffer-based bloom implementation reviewed. It contains earlier blur, bright-pass, mask, filtering and scale variants superseded by bloom.pyx; no separate effect was required. |
| config.json | Default values for system, monochrome/noise, scan mode, fill gaps, bottom aberration, resolution, brightness/contrast/saturation/sharpness/vignette/bloom, frame range and output format/bitrate reviewed. |
| ffmpeg-6.0-essentials_build/LICENSE.md | FFmpeg distribution license/notices reviewed; binaries are not imported. |
| ffmpeg-6.0-essentials_build/bin/ffmpeg.exe | Bundled 80 MB Windows executable; not imported. |
| ffmpeg-6.0-essentials_build/bin/ffprobe.exe | Bundled 80 MB Windows executable; not imported. |
| fonts/VCR_OSD_MONO_1.001.ttf | Bundled OSD font asset; not needed by VHS-90 processing core and not redistributed in the plugin. |
| git_media/banner.png | README artwork; reference asset only. |
| git_media/corpse_bride.gif | README demonstration clip; visual reference only, never a runtime overlay. |
| git_media/mushrooms.gif | README demonstration clip; visual reference only, never a runtime overlay. |
| gui.ui | Full 912-line Qt Designer UI inspected; 70 widgets, including preview/timeline, NTSC/VHS, monochrome/noise, scan mode, fill-between-lines, bottom-line aberration, output size, grade controls, sharpness, vignette, bloom and render settings. UI-only. |
| icon.ico, icon.png | Application icon assets; not plugin resources. |
| main.py | CLI/config initialization, Windows multiprocessing freeze support, logging process and GUI startup reviewed; host application only. |
| requirements.txt | PyQt5, psutil, NumPy, Pillow, pygame and PyInstaller dependencies reviewed; not AEX dependencies. |
| stylesheets/dark.qss, stylesheets/light.qss | Qt theme stylesheets reviewed; UI-only. |

## Algorithm map and VHS-90 destinations

| NTSC-VHS-Renderer behavior | VHS-90 destination / status |
|---|---|
| NTSC and VHS signal conversion | VHS90WaveformEngine (full NTSC) and shared composite/raster signal path; no bundled EXE dependency |
| Brightness / contrast | VHS90ImageGradeEngine |
| Saturation | YIQ grade controls in VHS90ImageGradeEngine |
| Sharpness | VHS90ImageGradeEngine edge enhancement |
| Vignette | VHS90ImageGradeEngine normalized radial falloff |
| Bloom | VHS90BloomEngine multiscale bright-pass bloom; refined in this pass to the upstream separable 5-tap kernel and x2/x4/x8/x16 pyramid |
| Scanline mode / fill gaps | VHS90CompositeEngine::processScanlines and geometry/interlace controls |
| Bottom-line aberration | Existing independently positionable Tracking Noise, Head Switching and Sync Collapse region engines; more flexible than the upstream fixed-bottom-only flag |
| Monochrome | Existing composite/color pipeline mode |
| Noise | VHS90AnalogNoiseEngine and signal-domain noise modules |
| Preview frame batching, FFmpeg encode, GUI timeline, process pause/resume | Excluded as host-specific orchestration; AE host owns frame scheduling and output |

## BloomEffect port refinement in this pass

- VHS90BloomEngine now uses the source's exact separable 5-tap kernel [1,4,6,4,1]/16, applied horizontally and vertically for a configurable number of smoothing passes.
- Bright-pass luma now uses the upstream BT.601 coefficients 0.299/0.587/0.114 and the same ratio (luma-threshold)/luma.
- Normal mode uses the upstream x2/x4/x8/x16 multiscale pyramid (up to four levels).
- Fast mode now uses the x16-only approximation rather than the earlier x2 shortcut.
- Existing alpha-aware downsampling, premultiplied-alpha handling and deterministic C++ rendering are retained; upstream PyGame/NumPy/Cython code is not copied.
- Added a smoke test for deterministic fast-mode output as well as normal-mode output, identity bypass and alpha preservation.

## Non-portable components intentionally excluded

- The application launches precompiled NTSC-CRT executables, extracts and writes BMP/JPEG/video files, and coordinates multiple Python processes and a thread. These are not native per-frame kernels.
- Qt UI, stylesheets, PyInstaller spec, logging, JSON config, FFmpeg executables and font/media assets are not plugin runtime dependencies.
- Cython generated C and the compiled .pyd are not imported; the editable .pyx source and MIT notice are retained as algorithm references.
- NTSC-CRT is already separately audited as 39-ntsc-crt-deep-audit.md; its reusable three-band YIQ equalizer is already ported into VHS-90.

## Verification boundary

- The exact 1.3.0 source inventory is complete: 32/32 files audited.
- GitHub Actions run `44607780420` passed **25/25 CTest tests** after the 5-tap Gaussian, BT.601 bright-pass, x16 fast-path and fast-mode smoke-test updates.
- Static AE parameter count remains 79 real slots / 79 PF_ADD_* calls at the time of the previous audit; no new AE controls were added in this pass.
- No Windows/MSVC AE SDK build or live After Effects runtime/MFR test is claimed.
