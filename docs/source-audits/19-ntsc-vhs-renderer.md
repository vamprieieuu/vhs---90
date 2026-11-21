# VHS-90 — NTSC-VHS Renderer Application Audit

Date: 2026-11-19
Input: `vhs/github repos/NTSC-VHS-Renderer-1.3.0/`
Scope: all 23 files inventoried; reviewed README/build process, frame processor, configuration and dependency layout.

## Architecture

- Windows x64 desktop GUI/video renderer built with Python/PyQt and FFmpeg.
- It wraps two compiled NTSC-CRT command-line programs: standard NTSC and VHS-mode variants.
- VHS signal modulation/demodulation is performed by the bundled `ntsc_video.exe` / `ntsc_video_vhs.exe`; the GUI orchestrates video frame extraction, processing and reassembly.
- Additional BloomEffect source is used for a post-processing bloom stage.
- This is an application wrapper and does not contain a new native AE pixel engine.

## Useful behavior

- Supports opening images/videos, preview and exporting rendered video.
- Separates standard NTSC and VHS signal variants.
- Applies optional bloom after the NTSC/VHS pass.
- Uses FFmpeg for video extraction/reassembly and preserves timeline frame-rate handling.
- Its source demonstrates a possible offline reference-render harness around the C89 NTSC-CRT engine.

## Limitations

- The core VHS signal algorithms are in the bundled NTSC-CRT executable, not in the Python GUI source.
- The repository README itself lists unresolved issues: output resolution, performance, Linux support and additional effects.
- It depends on external FFmpeg and BloomEffect components.
- It is not a reusable AE SDK plugin architecture.

## VHS-90 decision

Use the bundled NTSC-CRT source audit as the algorithm reference; use this project only as an optional offline comparison/render harness. Do not import the PyQt UI, FFmpeg orchestration or executable binaries into VHS-90's native AE plugin.
