# VHS-90 — URP VHS Retro Effects Shader Audit

Date: 2026-11-19
Input: vhs/github repos/URP-VHS-Retro-Effects-main/
Scope: Unity project inventory; reviewed the Retro volume component, render pass parameter bindings and multi-pass HLSL shader.

## Architecture

Unity URP post-processing feature using a renderer feature/pass and a multi-pass HLSL shader. It is a screen-space post effect, not a signal-accurate VHS emulator or a native AE plugin.

## Controls and shader behavior

- Bleed radius, direction and intensity.
- Smear intensity, using two directional multi-tap horizontal passes with different exponential falloff.
- Edge enhancement from a slightly offset blurred image.
- Tape noise amount, speed and alpha. Noise combines three correlated line-noise scales with a per-pixel hash mask and thresholding.
- Interlacing offset based on row/time.
- Scanline speed, frequency and intensity.
- Color bleed operates in YCbCr by replacing/blending the chroma channels from a blurred texture while preserving luma.
- A downsample/upsample blur chain provides softer color/detail loss.

## VHS-90 reuse decision

Use only the simple multi-tap smear, correlated line-noise construction and chroma-only blur concepts as optional finishing kernels. The shader's noise is a screen-space animated overlay and its scanline/interlace model is display-oriented; neither should be mistaken for physical tape tracking or head switching. Do not import Unity URP runtime code or its project assets into VHS-90.

## Verification

Unity project files and the core shader/component were inspected statically. No Unity build or rendered comparison was run.