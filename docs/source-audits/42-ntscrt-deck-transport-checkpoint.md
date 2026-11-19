# NTSCRT deck/transport continuation — audit and port checkpoint

Date: 2026-11-20
Upstream: https://github.com/finnmckenty/NTSCRT (MIT; retain attribution/license notice for any substantial copied code).
Target: VHS-90 native C++17 CPU core / After Effects host.

## Upstream files reviewed in this pass

- `port/engine/src/glitch/transport.rs` — reviewed through the complete `TransportSimulator`, `field_step`, and `servo` implementation and its tests.
- `port/engine/src/glitch/deck.rs` — reviewed through `DeckParams`, `HeadState`, `DeckSimulator`, field alternation, debris pickup/decay, shedding events, grease transfer, and history snapshots.
- `port/engine/src/glitch/vcr.rs` — reviewed constants and channel-gain/spacing response used by the RF chain.
- `port/engine/src/glitch/vcr.wgsl` — rechecked tape material, spacing, FM and color-under pass structure.
- Target integration files reviewed: `VHS90TapeTransportEngine.{h,cpp}`, `VHS90TapeSurfaceEngine.{h,cpp}`, `VHS90TrackReadEngine.{h,cpp}`, `VHS90CompositeEngine.{h,cpp}`, `VHS90Effect.cpp`, root `CMakeLists.txt`, and existing smoke-test conventions.

## New VHS-90 implementation checkpoint

Added `VHS90HeadConditionEngine.{h,cpp}` and `VHS90HeadConditionEngineSmoke.cpp`.

The current raster-domain head-condition stage models:
- alternating head selection by field parity;
- wear/mismatch-dependent head efficiency and gap;
- deterministic frame-keyed dirt/debris events;
- a grease-film contribution;
- stronger high-frequency FM/luma attenuation than low-frequency color-under/chroma;
- optional chroma compensation and premultiplied-alpha preservation.

It is wired into `VHS90CompositeEngine` after track-read and before chroma/composite encoding, and is exposed in the AE panel as `Head Dirt / Wear`. CMake registers the engine and smoke test. The effect now has 82 PF_ADD declarations and the new parameter has a matching enum slot and disk ID.

## Important fidelity boundary

This is an image-domain, deterministic approximation. It is **not yet** a faithful port of NTSCRT's stateful `DeckSimulator`: it does not yet accumulate debris with the original per-field recurrence/history, sample shed/film at 12 points along the physical track, reproduce stochastic lodging/shedding/clean-off probabilities exactly, or expose separate independent controls for head wear, dirt, buildup, and mismatch.

Likewise, `VHS90TapeTransportEngine` is a row-warp approximation and does not yet implement the NTSCRT control-track pulse detector and type-2 PI capstan servo. The upstream control-track behavior still requires a dedicated stateful/random-access-safe port.

## Next work — do not mark NTSCRT complete yet

1. Implement a render-order-independent control-track servo state/query model: one control pulse per frame, amplitude from tape edge coating/lift, missing-pulse hold, crease-generated fake pulses, bounded PI integrator, capstan speed correction, search mode, and snapshot/restore or deterministic checkpointing.
2. Replace the first head-condition approximation with the full field-domain deck recurrence from `deck.rs`, including both heads, 12-point tape sampling, debris pickup/decay, rare lodged chunks, shed-off events, grease transfer asymmetry, wear spacing, gap, efficiency and color boost.
3. Connect the servo's line-level time-base error to the transport warp and track-read stages; preserve MFR/out-of-order rendering by avoiding mutable global state.
4. Add behavioral tests based on the upstream servo tests: worn tape retains lock; lost control track drifts but remains bounded; fake pulses and missing pulses behave deterministically; negative and random frame access agree.
5. Continue the remaining Rust/WGSL and Swift/Metal source audit and review the 45 presets before declaring the 359-file upstream inventory fully audited.

## Validation

- New smoke test added for deterministic output, zero-amount bypass, and alpha preservation.
- GitHub Actions Core CI run `44618422359` completed successfully: Windows-independent Linux core build succeeded and **27/27 CTest tests passed**, including `VHS90HeadConditionEngineSmoke`.
- No Windows/MSVC AE SDK build or live After Effects runtime/MFR test is claimed.
