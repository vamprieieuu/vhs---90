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

`VHS90TapeSurfaceEngine` now exposes `sampleTapeSurfaceAtMm(xMm, yMm, params)` in the NTSCRT coordinate convention (millimetres along tape and across the 12.65 mm tape width). The raster adapter maps field rows along the helical track, horizontal samples over the active NTSC line interval (9.4–62.056 µs), and field origin using the 33.35 mm/speed field advance, selected SP/LP/EP speed and 5.97° track angle. The sampler ports the upstream deterministic 32-bit hash/value-noise pattern and the principal `tape_at` mechanisms: remanence/wear variation, pinholes, flakes and shed, lengthwise scratches, creases, edge lift, mold colonies and Vaseline film. The raster sampler averages four physical samples across the selected SP (58 µm) or LP/EP (26 µm) head width; it includes active-picture start timing and the raster's VBI line offset. The head-state and control-track paths call the same point sampler with `detail=false, flakes=true`, matching the source's sync/deck sampling mode. `TapeSurfaceSample` now reports shed as well as magnetization, spacing lift, roughness and film.

This is a C++ adaptation, not bit-identical parity yet: the VHS-90 controls are mapped onto NTSCRT material parameters rather than using the upstream `TapeParams::new` curves/top-end transforms, and sticky-binder/transport stick-slip coupling is not yet ported. The raster sampling path now uses physical track coordinates, but the image-domain renderer still approximates the RF chain.

### 4. Two-head deck recurrence with physical tape sampling

`HeadConditionParams` can enable `usePhysicalTapeSamples`. In this mode each active head field samples 12 evenly spaced points along the physical helical track, averages tape shed and film, and feeds those values into deterministic debris pickup/lodging/clean-off and asymmetric grease transfer/decay. The default remains the earlier proxy path unless the flag is enabled. Tests explicitly exercise the physical sampling path.

The physical head-sampling path is now enabled by the AE parameter adapter when Head Condition is active; the selected SP/LP/EP format is passed to the tape sampler. The source Swift `DeckSimulator` was reviewed: it samples 12 points per active field with detail disabled and flakes enabled, accumulates per-head debris/grease, and retains the entering states of the last 12 fields. VHS-90 ports the 12-point sampling and recurrence but still lacks the source's 12-entry rolling snapshot/history behavior. The deck is still not a complete source `DeckSimulator` port: it replays from field zero rather than retaining NTSCRT's rolling 12-entry history/snapshot cache.

## Verification

Latest verified core CI: [run 499](https://github.com/vamprieieuu/vhs---90/actions/runs/44622438195) — success, CMake core build and CTest 27/27, including physical tape coordinate/detail/flake behavior, SP/LP/EP mapping, measured tape response curves, seed transform, 12-point deck sampling and physical control-track path. The AE SDK adapter itself is not compiled by this Linux core workflow. This does not validate Windows/MSVC, AE SDK compilation, PiPL, AEX loading, live AE rendering or MFR.

## Preset disposition

45/45 preset JSON files were parsed. They divide into:
- VHS/deck/tape conditions and look presets;
- frame/timeline-driven glitch sequences;
- digital codec/downscale looks that do not represent analog tape;
- five `Screen Loop` feedback/reprojection presets with a separate schema.

The five Screen Loop presets require a dedicated feedback/reprojection stage; their parameters should not be flattened into the current tape or sync controls. Preset names and parameter mappings remain to be migrated into VHS-90 after the relevant engines exist.

## Source audit coverage — honest status

Content-level review in this work pass includes the Rust engine files below and the corresponding Swift reference implementations `Sources/CrtCore/{Tape,Deck,Transport,Receiver,VcrStage}.swift`, plus `Presets.swift`:
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

The 359-file ledger currently records 20 content-reviewed files, 45 parsed preset JSON files, 56 engine-review-pending files, 73 platform-review-pending files, 58 test/reference-pending files, 49 inventory-only files, 45 asset-inventory files and 13 metadata-inventory files. The remaining upstream Swift app, Rust UI/CLI/MCP, codec, media/export, shader, build, dependency, fixture and binary-asset files have not all received a line-by-line content review. They must be classified as reusable engine, platform-only, test/reference, generated/vendor, or irrelevant to native AE before the source audit can be marked complete.

## Still required before NTSCRT can be called finished

1. Finish physical tape map parity: expose source-equivalent independent tape knobs and user-controlled tape seed (the measured response curves and seed transform are now ported); implement sticky-binder/transport stick-slip coupling; verify sample-by-sample against upstream reference outputs.
2. Complete deck parity: verify source-compatible field indexing and physical 12-point sample aggregation; port the bounded rolling 12-field history/snapshot semantics. The physical sample path is now enabled from the AE parameter adapter when Head Condition is active, but Windows/AE execution is not yet validated.
3. Port the RF/FM/color-under chain from `VcrStage.swift` / `vcr.wgsl`. The source is an 8× color-subcarrier signal path (28.6 MHz, 1820 samples per line) with nine kernels: `vcr_record`, `vcr_tape`, `vcr_magnet`, `vcr_play`, `vcr_fmband`, `vcr_line`, `vcr_comb`, `vcr_acc`, `vcr_out`. It includes composite-copy trap/chroma separation, FM pre-emphasis/clipping, color-under heterodyning, four material samples across the head width per segment, three-track azimuth-weighted magnetization, a 64-level spacing/gap filter bank, analytic FM bandpass, instantaneous-frequency demodulation, zero-phase luma/chroma filters, de-emphasis, dropout hold/compensation, one-line chroma comb, ACC/APC and color killer. No C++ VHS-90 engine currently reproduces this sample-domain chain.
4. Port receiver sync and color-reference loops from `Receiver.swift` / `receiver.rs` and `glitch.wgsl`. The source receiver is a stateful line-by-line television model: 2 s warmup; sync separator probability from signal/noise and hum; horizontal AFC PI/VCO with cycle slipping; vertical integrator and injection-locked oscillator; burst-key phase follower; per-field chroma PLL, ACC and color-killer; 12-field/line history snapshots and deterministic resimulation. Current VHS-90 `SyncCollapseEngine` and `ReceiverInstabilityEngine` remain pixel-domain/procedural effects, not this circuit model.
5. Build a file-by-file disposition ledger for all 359 upstream files, including a reason for every non-port.
6. Complete source parity tests and visual frame comparisons, then Windows/MSVC + AE SDK build and live After Effects/MFR validation.

## Licensing

NTSCRT is MIT-licensed. The license and attribution are now preserved in `ThirdParty/NTSCRT/LICENSE` and `ThirdParty/NTSCRT/README.md`. Do not remove these notices when redistributing substantial copied or adapted portions.
