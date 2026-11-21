# VHS-90 — Sync Collapse / Time-Base Damage Engine

Date: 2026-11-19
Files added:
- Source/VHS90Core/VHS90SyncEngine.h
- Source/VHS90Core/VHS90SyncEngine.cpp
- Tests/VHS90SyncEngineSmoke.cpp

## Source algorithm basis

Ported the event structure from the supplied Lost Signal VHS GLSL composite shader into a deterministic CPU kernel:
- Field-rate event clock and seeded event selection.
- Smooth attack/release envelope for short tracking-loss events.
- One or two independently positioned soft horizontal tear bands.
- Variable tear width, secondary tear offset, direction and textured displacement.
- Slow and fast line-timebase noise layers.
- Narrow rolling tracking line with sinusoidal pulse.
- Optional vertical raster roll from a seeded sync-loss event window.
- Independent line noise and tear-local noise.

## Behavior and safety

- All event timing is driven by explicit timeSeconds/frame/seed; no wall-clock or mutable global state.
- Strength zero is exact identity.
- Arbitrary frame dimensions and row strides are supported.
- Alpha is preserved; premultiplied input is unassociated/reassociated to the destination alpha.
- Fully transparent pixels are copied unchanged.

## Local verification

- CMake Release build: PASS.
- CTest: 6/6 PASS.
- CMake Debug build with ASan/UBSan: 6/6 PASS.
- Tests cover deterministic output, visible tear event, exact bypass, disabled-event identity, vertical-roll event, partial alpha preservation and transparent-pixel identity.

## Scope and limitations

This is a CPU image-domain sync/tear approximation of the source shader's event geometry. It does not implement temporal frame freeze/hold, a full VBI/HSYNC waveform, receiver PLL/AFC recovery, or a full RF/no-signal state machine. It is not yet connected to AE parameters or the native plugin host.