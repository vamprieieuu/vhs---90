# NTSCRT — same-session port status and remaining audit

Date: 2026-11-20  
Upstream: https://github.com/finnmckenty/NTSCRT  
Target: https://github.com/vamprieieuu/vhs---90

## Verified repository inventory

The upstream recursive tree was fetched with `truncated=false`: 359 file blobs and 59 directory entries. All 45 JSON files in `presets/` were fetched and parsed. The inventory and preset parse are complete; the 359-file content-level audit is not complete. Many files are UI, platform glue, test fixtures, build scripts, binary images, dependency locks or generated license data and should be dispositioned separately rather than blindly copied into a native AE effect.

## Implemented and integrated

### 1. Adjacent track read

`VHS90TrackReadEngine.cpp` now uses the NTSCRT physical overlap model and opposite-azimuth sinc attenuation. The attenuation uses 4.0 MHz FM and the color-under carrier `40 / 63.556 MHz`, and adjacent samples are positioned relative to the active center track. It remains an image-domain read approximation: it does not yet sample the physical tape material map or create sample-domain RF.

CI:
- Run 439 — passed.
- Run 440 — passed.

### 2. Control-track servo

`VHS90TapeTransportEngine` now includes a deterministic render-local replay of the control-track PI loop:
- proportional pulse correction coefficient 0.377;
- integral coefficient 0.0986 with ±0.15 clamp;
- missing-pulse hold behavior;
- false-pulse injection;
- pulse-strength/dropout controls;
- picture-search speed and relock path;
- arbitrary-frame deterministic replay, with no process-global mutable state.

The AE Tracking Servo control is wired into the engine. Pulse quality and false-pulse probability currently use proxy values from existing controls; the exact upstream `tape_at` control-edge material sample, full field phase history and source value-noise interpolation are not yet ported. Therefore this is a first integrated PI-servo adaptation, not a bit-for-bit transport port.

### 3. Two-head deck recurrence

`VHS90HeadConditionEngine` now simulates two independently alternating heads and accumulates debris/grease across fields using deterministic pickup, lodging, shedding/clean-off events, bounded debris and asymmetric grease transfer/decay. Each head has its own wear/mismatch/gap/read efficiency. The recurrence is replayed from frame zero for deterministic arbitrary-frame access.

The implementation uses proxy shed/film inputs derived from existing controls. It does not yet perform NTSCRT's 12-position physical `tape_at` sampling per track, so it is not the complete `DeckSimulator`/TapeParams port.

## Verification

Latest CI: [run 449](https://github.com/vamprieieuu/vhs---90/actions/runs/44619561972) — success, CMake core build passed and CTest 27/27 passed. This validates the Linux host-independent C++ core only. It does not validate Windows/MSVC, AE SDK compilation, PiPL, AEX loading, live AE rendering or MFR.

## Preset disposition

45/45 preset JSON files were parsed. They divide into:
- VHS/deck/tape conditions and look presets;
- frame/timeline-driven glitch sequences;
- digital codec/downscale looks that do not represent analog tape;
- five `Screen Loop` feedback/reprojection presets with a separate schema.

The five Screen Loop presets require a dedicated feedback/reprojection stage; their parameters should not be flattened into the current tape or sync controls. Preset names and parameter mappings remain to be migrated into VHS-90 after the relevant engines exist.

## Source audit coverage — honest status

Content-level review in this work pass includes:
- `port/engine/src/glitch/transport.rs`
- `port/engine/src/glitch/deck.rs`
- `port/engine/src/glitch/tape.rs`
- `port/engine/src/glitch/receiver.rs`
- `port/engine/src/glitch/vcr.rs`
- `port/engine/src/glitch/vcr.wgsl`
- `port/engine/src/glitch/params.rs`
- `port/engine/src/glitch/mod.rs`
- `port/engine/src/glitch/glitch.wgsl`
- relevant VHS-90 transport, track-read, head-condition, composite, effect adapter, CMake and smoke-test files.

The remaining upstream Swift app, Rust UI/CLI/MCP, codec, media/export, shader, build, dependency, fixture and binary-asset files have not all received a line-by-line content review. They must be classified as reusable engine, platform-only, test/reference, generated/vendor, or irrelevant to native AE before the source audit can be marked complete.

## Still required before NTSCRT can be called finished

1. Port the tape material map `tape_at` (including pinholes, flakes, scratches, creases, edge lift, mold, sticky shed and film) into deterministic C++ physical tape coordinates.
2. Replace proxy deck shed/film values with 12-point physical track sampling and reproduce the source deck's per-field history/snapshot behavior.
3. Port the RF/FM/color-under chain from `vcr.wgsl`: record/pre-emphasis, tape/material channel, magnetization/spacing convolution, playback/FM bandpass and demodulation, de-emphasis, dropout detection/compensation, color-under recovery, ACC/APC/color-killer and comb cancellation.
4. Port receiver sync and color-reference loops from `receiver.rs` and `glitch.wgsl`; current VHS90 sync-collapse engine is a separate procedural approximation.
5. Build a file-by-file disposition ledger for all 359 upstream files, including a reason for every non-port.
6. Add source parity tests and visual frame comparisons, then Windows/MSVC + AE SDK build and live After Effects/MFR validation.

## Licensing

NTSCRT is MIT-licensed. The license and attribution are now preserved in `ThirdParty/NTSCRT/LICENSE` and `ThirdParty/NTSCRT/README.md`. Do not remove these notices when redistributing substantial copied or adapted portions.
