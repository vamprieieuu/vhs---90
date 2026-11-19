# VHS-90 — Lost Signal VHS Shader Pipeline Audit

Date: 2026-11-19
Input: vhs/github repos/lost-signal-vhs-main/
Scope: shader-pack inventory; reviewed settings.glsl, analog_color.glsl and the composite/final fragment stages, including tracking tears, sync events, head-switch field mask, composite decode, tape defects, dropouts and static.

## Architecture

GLSL shader pack for Minecraft Iris/OptiFine-style pipelines. It runs a multi-pass camera/tape/decoder/display simulation with history buffers, depth-aware motion reprojection and compile-time configuration. It is not an AE plugin, but the fragment shader contains several useful procedural signal kernels.

## Signal and damage modules

- YIQ signal path with selectable composite decoder: bypass, consumer notch, or NTSC two-line comb.
- Dot crawl, cross-color and cross-luma leakage from imperfect composite separation.
- Chroma phase error, chroma killer when tracking/sync lock weakens, chroma persistence and chroma timing misalignment.
- Tape softness, chroma bleed, black crush, saturation, halation and luma ringing.
- Timebase error and row-locked horizontal displacement.
- Intermittent horizontal tracking tears: deterministic event ticks, one or two correlated tear bands, smooth attack/release envelope, variable band center/width, directional displacement and line-textured tear mask.
- A separate rolling tracking band travels vertically and modulates horizontal displacement.
- Vertical sync events can either roll the raster or retain/freeze the previous frame.
- Tape-chew events create broad correlated buckling and displacement across neighboring lines.
- Head-switch mask is field-clock locked; the same field event drives horizontal/vertical displacement, chroma phase jump, chroma attenuation and RF-like noise.
- Tape defects persist in virtual tape coordinates; defect density/strength are independent of transient static.
- Dropouts are short horizontal signal-loss bands with a noisy tail and deterministic time-cell placement.
- Static is a separate intermittent full-frame event with high-frequency grain and line components.
- RF ingress creates diagonal herringbone carriers and rolling bright interference bands.

## Important placement limitation

The shader's head-switch center is generated close to a fixed raster edge and changes only between fields; it is not a user-positionable region. Its tracking tear event can appear at a range of vertical positions, while the continuous tracking band wraps through the frame and responds to a signed tracking control. VHS-90 must retain its stronger requirement: arbitrary region position, width/height, direction, behavior and multiple independent regions for Tracking Noise; arbitrary vertical position and full-frame height for Head Switching.

## Valuable design details

1. Keep event probability/frequency separate from event intensity and event duration.
2. Use deterministic time cells so rare events persist for their full envelope and reproduce at a given time.
3. Keep tracking tears, slow rolling bands, vertical sync failures, tape-chew, dropouts, static and RF ingress as distinct algorithms.
4. Use a shared field-locked mask to correlate head-switch displacement, chroma loss and noise rather than applying unrelated random effects.
5. Keep chroma persistence and luma temporal history independent.
6. Apply atmospheric/source-stage effects before tape recording if they are meant to be recorded; display-stage effects belong after decoding.

## VHS-90 integration decision

Port the GLSL math into C++ CPU kernels where useful, especially event envelopes, correlated tear masks, composite decoder leakage, tape-coordinate defect memory, field-locked head-switch behavior and separate static/dropout/RF modules. Do not port Minecraft depth/fog/lighting integration or shader-pack configuration. Keep alpha and MFR-safe deterministic frame-time handling explicit.

## Verification

Shader source was inspected statically. No Iris shader compilation or in-game render test was run.