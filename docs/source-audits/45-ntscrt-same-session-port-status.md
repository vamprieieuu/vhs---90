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

The AE Tracking Servo control is wired into the engine. An optional physical control-track path now samples the NTSCRT control-head location (79.244 mm along track, y=0.35 mm) using the same tape material map, then applies remanence-normalized amplitude and spacing loss to pulse detection; crease lift can create false pulses. The AE adapter enables this path and passes the selected tape speed. Remaining deviations: pulse servo still uses a simplified frame-domain replay rather than the upstream line-domain capstan/motor/stick-slip model, exact source 64-bit noise/value interpolation and full phase/history snapshots. Therefore this is a first integrated PI-servo adaptation, not a bit-for-bit transport port.

### 3. Physical tape material map

`VHS90TapeSurfaceEngine` now exposes `sampleTapeSurfaceAtMm(xMm, yMm, params)` in the NTSCRT coordinate convention (millimetres along tape and across the 12.65 mm tape width). The raster adapter maps field rows along the helical track, horizontal samples over the active NTSC line interval (9.4–62.056 µs), and field origin using the 33.35 mm/speed field advance, selected SP/LP/EP speed and 5.97° track angle. The sampler ports the upstream deterministic 32-bit hash/value-noise pattern and the principal `tape_at` mechanisms: remanence/wear variation, pinholes, flakes and shed, lengthwise scratches, creases, edge lift, mold colonies and Vaseline film. `TapeSurfaceSample` now reports shed as well as magnetization, spacing lift, roughness and film.

This is a C++ adaptation, not bit-identical parity yet: the VHS-90 controls are mapped onto NTSCRT material parameters rather than using the upstream `TapeParams::new` curves/top-end transforms, and sticky-binder/transport stick-slip coupling is not yet ported. The raster sampling path now uses physical track coordinates, but the image-domain renderer still approximates the RF chain.

### 4. Two-head deck recurrence with physical tape sampling

`HeadConditionParams` can enable `usePhysicalTapeSamples`. In this mode each active head field samples 12 evenly spaced points along the physical helical track, averages tape shed and film, and feeds those values into deterministic debris pickup/lodging/clean-off and asymmetric grease transfer/decay. The default remains the earlier proxy path unless the flag is enabled. Tests explicitly exercise the physical sampling path.

The physical head-sampling path is now enabled by the AE parameter adapter when Head Condition is active; the selected SP/LP/EP format is passed to the tape sampler. The deck is still not a complete source `DeckSimulator` port: it replays from field zero rather than retaining NTSCRT's rolling 12-entry history/snapshot cache.

## Verification

Latest verified core CI at that checkpoint: [run 466](https://github.com/vamprieieuu/vhs---90/actions/runs/44620865591) — success, 27/27. Subsequent physical control-track sampling and AE parameter wiring passed [run 471](https://github.com/vamprieieuu/vhs---90/actions/runs/44621261865), 27/27. SP/LP/EP field-advance and active-line coordinate refinements are being revalidated in the latest workflow; the AE SDK adapter itself is not compiled by this Linux core workflow. This does not validate Windows/MSVC, AE SDK compilation, PiPL, AEX loading, live AE rendering or MFR.

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

1. Finish parity of the physical tape map: source `TapeParams` curves/top-end transforms, exact parameter mapping, sticky binder and transport stick-slip coupling; verify sample-by-sample against upstream reference outputs.
2. Complete deck parity: source-compatible field indexing, physical 12-point sample aggregation and bounded rolling history/snapshot semantics; wire the physical path into the actual AE render adapter.
3. Port the RF/FM/color-under chain from `vcr.wgsl`: record/pre-emphasis, tape/material channel, magnetization/spacing convolution, playback/FM bandpass and demodulation, de-emphasis, dropout detection/compensation, color-under recovery, ACC/APC/color-killer and comb cancellation.
4. Port receiver sync and color-reference loops from `receiver.rs` and `glitch.wgsl`; current VHS90 sync-collapse engine is a separate procedural approximation.
5. Build a file-by-file disposition ledger for all 359 upstream files, including a reason for every non-port.
6. Complete source parity tests and visual frame comparisons, then Windows/MSVC + AE SDK build and live After Effects/MFR validation.

## Licensing

NTSCRT is MIT-licensed. The license and attribution are now preserved in `ThirdParty/NTSCRT/LICENSE` and `ThirdParty/NTSCRT/README.md`. Do not remove these notices when redistributing substantial copied or adapted portions.
