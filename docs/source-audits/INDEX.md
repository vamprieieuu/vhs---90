# VHS-90 Source Audit Index

Date: 2026-11-20

These reports are based on the supplied vhs.zip archive and the linked VHS-90 repository. Each report states its inspection scope and verification limits. An audit is not a claim that the source has already been integrated or visually validated.

## Original archive / project structure

- 00-codes-file.md — shader/code bundle inventory and conversion findings.
- 01-vhs6rift-native-base.md — VHS6RIFT core architecture and initial issues.
- 01b-vhs6rift-abi-build-harness.md — exact AE SDK ABI, PiPL flags and native harness compile failure.
- 01c-vhs6rift-revision-drift.md — source/documentation drift, workbench and SDK snapshot.
- 02-ntsc-rs-engine.md — ntsc-rs signal-processing engine.
- 03-ntsc-rs-adapters-and-workspace.md — AE/OpenFX adapters, GUI and Rust workspace.
- 04-supplied-font-pack.md — 15 supplied font files, Arabic glyphs and shaping metadata.
- 05-crates-vhs-master.md — Crates VHS Master metadata, presets and embedded shader pipeline.
- 06-vhscraft.md — VHSCraft controls and Classic/Cinematic shader stages.
- 07-signal-plugin.md — Signal AEX parameter model.
- 08-heisei-vhs.md — HEISEI VHS parameter labels and host behavior notes.
- 09-universe-vhs-transition.md — Universe transition/noise types and parameter ranges.
- 10-vhspro-and-vhstape.md — VHSPro and VHSTape binary/source findings.
- 11-analog-ntsc-cpp-engine.md — Analog NTSC C++ composite encoder/effects/decoder.
- 12-ntsc-crt-c89.md — NTSC-CRT C89 fixed-point VHS timing and signal engine.
- 13-composite-video-simulator.md — FFmpeg composite simulator and audio-only cassette code.
- 14-vhs-main-ofx-metal.md — OFX/Metal kernel, OSD and presets.
- 15-barash-ntsc-python.md — BarashNTSC YCrCb-based processor.
- 16-vhs-py-basic.md — basic Python VHS image processor.
- 17-vaporwave-image-stylizer.md — YUV stylizer reference.
- 18-comfyui-retrotape.md — RetroTape CPU/CUDA stages and tests.
- 19-ntsc-vhs-renderer.md — NTSC-VHS Renderer application wrapper.
- 20-ntscqt-vhs-engine.md — ntscQT YIQ/composite VHS pipeline.
- 21-vhs-phys-physical-model.md — physical tape/deck model and tracking geometry.
- 22-ntscrt-gpu-vcr.md — NTSCRT VCR/transport/receiver GPU stages.
- 23-urp-vhs-retro-shader.md — Unity URP post-processing shader.
- 24-vhs-effect-addon.md — Python VHS effect, tracking/head-switch and OSD order.
- 25-lost-signal-vhs-shaders.md — GLSL tracking, sync, tape defects and composite decoder.
- 26-vhs-camera-reference-clips.md — Fxc6 and Gen 7 before/after camera clips.
- 27-vhs-damage-lab-reference-specs.md — 20 prompt documents and 19 visual references.
- 28-vhs90-region-engine-implementation.md — new arbitrary-region C++ module and sanitizer tests.
- 29-vhs90-static-engine-implementation.md — procedural static modes and deterministic/alpha tests.
- 30-vhs90-yiq-iir-filter.md — standard YIQ transform correction and constant-K/Butterworth bandwidth filters.
- 31-vhs90-composite-modem.md — four-phase NTSC composite encode/decode and comb demodulator.
- 32-vhs90-sync-collapse-engine.md — procedural sync tears, time-base instability and vertical roll.
- 33-vhs90-ae-host-adapter.md — initial SmartFX/legacy host adapter, PiPL flags, parameter mapping and Windows build gate.

- 41-ntscrt-expanded-audit.md — initial 359-file NTSCRT upstream inventory checkpoint.
- 42-ntscrt-deck-transport-checkpoint.md — deck/head-condition integration checkpoint and fidelity limits.
- 43-ntscrt-preset-inventory-and-transport-findings.md — all 45 upstream JSON presets parsed; source physics findings for transport, deck, tape, receiver and VCR stages.
- 44-ntscrt-track-read-physics-port.md — adjacent-track overlap and azimuth-loss C++ port.
- 45-ntscrt-same-session-port-status.md — current transport/head recurrence implementation, CI result, attribution and remaining unported engine stages.
- 46-ntscrt-359-file-disposition-ledger.md — closed 359-file content disposition audit: 314 content-reviewed and 45 semantic-reviewed rows, zero pending.
- 47-ntscrt-preset-semantic-migration.md
- 48-urp-vhs-retro-effects.md — fast audit of the URP VHS shader and port of multi-scale tape noise.
- 49-vhs-effect-addon-full-audit.md
- 50-composite-video-simulator-full-audit.md — 36-file GPL-2.0 audit, native VHS playback chroma sharpening, and CVS VHS SP/LP/EP profiles. — 18-file VHS Effect Add-on inventory, algorithm disposition, five preset mappings, source self-test and AE/CI limitations. — per-preset semantic review of all 45 NTSCRT looks, stage boundaries, keyframe migration and explicit exclusions.

## Current project status

See docs/PROJECT_STATUS.md for the current build/integration boundary. The repository contains an initial AE host adapter and optional AEX build target. Windows/MSVC compilation and AE runtime validation remain pending; do not describe it as a test-ready AEX.