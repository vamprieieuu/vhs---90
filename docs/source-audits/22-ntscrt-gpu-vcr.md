# VHS-90 — NTSCRT GPU VCR / Glitch Pipeline Audit

Date: 2026-11-19
Input: vhs/github repos/NTSCRT-0.26.0/
Scope: 323-file inventory; reviewed project README, Windows Rust port architecture, GPU VCR WGSL, glitch WGSL entry points, transport model and VCR parameter/condition structures. The full Swift UI and every shader branch are not fully audited in this checkpoint.

## Pipeline architecture

NTSCRT combines a digital camera/codec stage, NTSC composite signal processing, a VCR tape/deck stage, a simulated TV glitch stage and CRT shaders. Its Windows preview port uses Rust plus WGSL GPU compute/render stages; the Mac app also includes Metal implementations and the original project architecture.

The README distinguishes the NTSC signal stage (based on ntsc-rs) from the VCR tape/deck simulation and the TV receiver glitches. This separation is useful for VHS-90: recording/tape/playback damage, receiver sync failures and CRT display artifacts should be distinct internal stages even when exposed through a unified panel.

## VCR tape model in WGSL

The VCR shader carries a large parameter block for tape/deck condition, including wear, dropouts, scratches, creases, edge damage, mold, vaseline, remanence, roughness, pinholes, flakes, burst amplitude, track pitch, head width, azimuth and filter coefficients.

Procedural tape generation is coordinate-based and deterministic:
- Seeded hash functions and interpolated one-/two-dimensional noise.
- Pinholes and flakes are spatially fixed defects on the tape surface.
- Scratches occupy narrow lanes with varying width and depth.
- Creases create slanted tape-surface lift with a localized Gaussian-like profile and edge falloff.
- Edge damage alters the tape surface near the top/bottom edges.
- Defect sampling is based on tape coordinates and field/head position, so the same physical defect can recur consistently as the tape moves.

The WGSL pipeline has separate kernels for recording, tape surface, magnetization, playback head, FM band, deck line processing, combing and final image reconstruction. It is not a simple one-pass overlay.

## Transport and tracking

The transport module models a control-track servo that reads a pulse at the bottom edge once per frame. Capstan speed is corrected by a type-2 servo loop; a lost pulse leaves the capstan running at its previous speed. Stick-slip models sticky binder/mold causing the tape to stick at guides and spring loose. Picture search drives the tape faster with the servo disabled.

Tracking is expressed in track-pitch units. The tracking knob is nonlinear: its first half moves the head most of the way off-track, while the remaining range reveals stronger noise bands. Back tension, transport flutter and search speed are independent controls.

This is an advanced source for temporal transport and tracking behavior, but its physical control-track servo is not the same as VHS-90's user-facing arbitrary region editor. VHS-90 must still allow custom X/Y/width/height, direction, behavior and multiple regions for Tracking Noise.

## Receiver / TV glitch stage

The project has a separate simulated TV receiver and glitch shader. Its documented architecture separates receiver burst gating, chroma reference recovery and per-pixel signal composition. This provides a reference for severe sync loss, vertical hold, rolling, tearing, bending and snow without merging them into tape wear.

## Presets and user interface

The repository includes a large collection of preset JSON files, including clean/worn VHS, damaged VCR, tracking noise up, V-hold glitch, vertical spikes, static loops and several screen-loop compositions. It exposes separate Tape, Deck, Transport, Receiver, NTSC, CRT and Digital Video panels. VHS-90 should convert useful recipes to its own names and UI architecture rather than importing this panel hierarchy verbatim.

## VHS-90 integration decisions

1. Port deterministic tape-coordinate defect generation (scratches, creases, pinholes, flakes, mold/vaseline-like surface contamination) into C++ CPU kernels first.
2. Use the WGSL stage separation as a design reference for optional GPU acceleration; CPU rendering remains mandatory.
3. Reuse seeded coordinate hashing so defects remain fixed to a virtual tape and do not swim randomly from frame to frame.
4. Treat tracking transport, head switching, receiver sync failures and CRT effects as separate engine modules.
5. Keep VHS-90's arbitrary-position Tracking Noise and Head Switching controls; do not inherit any bottom-only placement assumptions.
6. Keep source preset names and app branding out of VHS-90's UI.

## Verification status

- Windows-port VCR WGSL, transport module, parameter structures and project pipeline documentation were inspected.
- No WGSL compilation, GPU execution, Rust build or AE runtime test was performed.
- This audit records reusable mechanisms; it does not claim the Mac app or Windows preview port is build-ready for VHS-90.