# VHS-90 — Composite Video Simulator C++ Source Audit

Date: 2026-11-19
Input: `vhs/github repos/composite-video-simulator-master/` from supplied `vhs.zip`
Scope: full 36-file inventory; review of project README, the FFmpeg video-processing scaffold, the VHS-led processing source, and the cassette-audio source to distinguish video algorithms from audio-only material.

## Project purpose and architecture

- This is an FFmpeg-based offline media-processing toolkit for analog composite-video simulation, written in C++.
- The README explicitly prioritizes signal accuracy over speed and warns that the implementation is slow.
- It aims to encode/decode composite video rather than add a generic blur/noise/static overlay.
- The repository is a collection of separate command-line processing programs, not a reusable AE plugin SDK or one consolidated effect library.

## VHS-relevant findings

- The project includes a dedicated VHS-led video processing program and several composite-video/NTSC processing utilities.
- The processing model works on decoded video frames and FFmpeg conversion/scaling structures, with timing and frame-rate handling in the video pipeline.
- It provides an additional reference for building an offline signal-processing test harness around real video clips, but its FFmpeg command-line integration should not be placed inside the AE plugin.
- The README's stated visual goal is restrained analog behavior rather than exaggerated continuous snow/static. This is a useful calibration reference when defining realistic default VHS-90 recipes.

## Cassette audio source — separate domain

The `ffmpeg_cassette.cpp` source is explicitly an audio-cassette emulator, not a VHS video effect. It includes:
- Stateful low-pass/high-pass IIR filtering.
- Multi-pass band-limiting.
- Pre-emphasis/de-emphasis filters.
- VHS-like audio bandwidth/noise notes and SP/LP/EP audio-quality assumptions.

These audio algorithms are not direct video kernels. VHS-90's current scope is the video effect; do not mix audio cassette emulation into the image-processing engine. They may be useful only if an independent audio module is ever requested.

## Integration decision

- Do not import the FFmpeg application layer into the native AE effect.
- Use the project as a reference for external frame-sequence regression tests and for conservative analog-look calibration.
- Prioritize the dedicated NTSC/VHS signal engines for actual C++ pixel/signal algorithm ports.
- Treat cassette hiss/warble code as audio-only and exclude it from the VHS-90 video engine.

## Verification status

- The 36-file archive inventory was enumerated.
- The README and relevant video/audio processing sources were reviewed.
- This checkpoint does not claim that every FFmpeg utility was fully reverse-engineered or that any of this project's code has been integrated into VHS-90.
