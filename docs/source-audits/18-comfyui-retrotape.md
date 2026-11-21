# VHS-90 — ComfyUI RetroTape Engine Audit

Date: 2026-11-19
Input: `vhs/github repos/ComfyUI_RetroTape-main/`
Scope: all 25 files inventoried; detailed review of README, CPU processing engine, CUDA implementation, device fallback, preset/settings model, stage order and selected fidelity/hardening/performance tests.

## Architecture

- Python/NumPy/OpenCV CPU engine plus a PyTorch CUDA port.
- 19 presets, 28 controls, still-image and frame-batch processing, JSON settings, live preview and frame-offset support.
- CPU and GPU paths use the same named processing stages; the CUDA path is a stage-by-stage port.
- The code explicitly documents that CPU and GPU use different random generators, so grain is not identical across devices, although event placement and deterministic stages are tested for parity.
- The engine normalizes spatial constants against 720×480 and scales behavior to arbitrary frame dimensions.

## Processing stages recovered

- RGB↔YIQ conversion.
- Saturation and black lift.
- Horizontal/vertical chroma bleed and chroma delay.
- Time-varying chroma phase/color drift.
- Composite artifacts and ringing.
- Luma/chroma/video noise.
- Ghosting and chroma delay.
- VHS luma/chroma low-pass filtering, edge-wave shifts and line-to-line chroma blending.
- Tracking warp with slow servo profile, per-row shifts, occasional tear segments and seeded epoch interpolation.
- Interlace/vertical roll.
- Causal luma smear and highlight trail.
- Dropout blocks and isolated speckles.
- Scanlines and analog glow.
- Final YIQ-to-RGB conversion and blend with original.

Temporal changes use explicit frame indices, deterministic seeds and four-frame epochs for slow mechanical motion. This is a useful model for stable frame-by-frame VHS motion without requiring mutable previous-frame history.

## Tracking and head-switch geometry

- Tracking uses seeded per-row servo noise filtered by a one-pole profile. Tape warp adds two sinusoidal profiles. Occasional tears are localized to random row spans. The tracking model is not a fixed bottom strip and is more detailed than a simple vertical shift.
- Head switching remains bottom-only: it selects a band of approximately 4.5% of image height starting at `height - band`, then displaces/noises those rows and reduces chroma.
- Dropouts are rectangular regions with randomized row, vertical band height, X position, horizontal length, luma target (bright or dark), noise and partial chroma loss.
- VHS-90 can reuse the tracking servo profile, tear construction, dropout geometry, temporal epoch logic and deterministic seeding. Head switching must be rewritten using the arbitrary-region engine.

## Presets and tests

The project includes four upstream ntscQT presets and a set of custom recipes; the README advertises 19 total. Custom presets include clean/recording, camcorder, damaged, bootleg/anime-like and other stylized looks. Exact source parameter dictionaries are in `ntsc_core.py` and should be translated as recipe data, not copied blindly into VHS-90's different parameter units.

The test suite covers:
- Deterministic output for the same seed/frame.
- Resolution independence and odd-width dimensions.
- Persistence of slow artifacts across frames.
- Alpha preservation.
- CPU/GPU deterministic-stage parity and head-switch geometry parity.
- Similar row locations for tracking tears/dropouts.
- Device fallback, poisoned CUDA context handling and progress reporting.
- Preview/render fidelity and cache/path hardening.
- Performance measurements and preset behavior.

## VHS-90 integration decision

1. Port the seeded tracking servo/epoch model and dropout geometry into C++.
2. Reuse the YIQ pipeline stage order as a candidate analog preset chain.
3. Use the test strategy as a model for VHS-90: same-seed repeatability, frame-offset continuity, resolution independence, alpha preservation, CPU/GPU geometry parity and no-op/neutral parameter checks.
4. Keep the CPU implementation authoritative and optional GPU acceleration. Avoid requiring CUDA.
5. Replace the bottom-only head-switch function with arbitrary normalized region geometry; do not copy its 4.5%-height anchor.
6. Keep source preset identities internal; create VHS-90-native recipe names.

## Validation status

- Static review of the CPU and GPU implementations and selected tests was performed.
- Python test suite and CUDA runtime were not executed in this environment.
- No code from this repository is claimed to have been integrated into VHS-90 yet.
