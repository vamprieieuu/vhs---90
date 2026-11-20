# NTSCRT — same-session port status and remaining engineering work

Date: 2026-11-20  
Upstream: https://github.com/finnmckenty/NTSCRT  
Target: https://github.com/vamprieieuu/vhs---90

## Verified source audit

The upstream recursive Git tree is complete: 359 file blobs, 59 directory entries, truncated=false. Every blob has now been read or inspected in its appropriate form and assigned a specific disposition in [46-ntscrt-359-file-disposition-ledger.md](46-ntscrt-359-file-disposition-ledger.md):

- 314 rows: content-reviewed (engine, Swift app, Rust platform app, tests, fixtures/tools, docs/scripts, dependency/license metadata and binary assets).
- 45 rows: semantic-reviewed (all JSON looks; per-preset stage classification and migration decision in [47-ntscrt-preset-semantic-migration.md](47-ntscrt-preset-semantic-migration.md)).
- 0 pending rows.

This closes the file-by-file audit/disposition task. It does not mean all upstream code is portable or all algorithms have been ported. UI, CLI/MCP, Media Foundation, Metal/WGPU, digital codec and Screen Loop code are explicitly distinguished from analog VHS engine code.

## Implemented and integrated

### 1. Adjacent track read

VHS90TrackReadEngine uses the NTSCRT physical overlap model and opposite-azimuth sinc attenuation. The attenuation uses 4.0 MHz FM and 40/63.556 MHz color-under, adjacent samples are positioned relative to the active center track, overlap shares remain absolute relative to a 58 µm SP track, source round-away-from-zero center selection is used, and neighboring raster rows include VBI-scaled track offset. This remains an image-domain read approximation; it does not yet sample a full sample-domain RF tape chain.

### 2. Control-track servo

VHS90TapeTransportEngine has deterministic render-local replay of a control-track PI loop:
- proportional pulse correction coefficient 0.377;
- integral coefficient 0.0986 with ±0.15 clamp;
- missing-pulse hold, false-pulse injection and pulse-strength/dropout controls;
- picture-search speed and relock approximation;
- arbitrary-frame deterministic replay without process-global mutable state.

The AE adapter connects Tracking Servo and physical control-track tape sampling at the upstream control-head location (79.244 mm along track, y=0.35 mm). It applies remanence-normalized amplitude and spacing loss to pulse detection; crease lift can create false pulses. Remaining deviations: simplified frame-domain replay instead of the full line-domain capstan/motor/stick-slip model, exact source 64-bit noise/value interpolation and complete phase/history snapshots.

### 3. Physical tape material map

VHS90TapeSurfaceEngine exposes sampleTapeSurfaceAtMm(xMm,yMm) in NTSCRT millimetre coordinates. The raster adapter maps field rows along the helical track, horizontal samples over the active NTSC line interval (9.4–62.056 µs), and field origin using 33.35 mm/speed field advance, selected SP/LP/EP speed and 5.97° track angle.

Important geometry correction: upstream TapeGeometry::TRACK_START is **1.0 mm**, not 1.52 mm. The C++ constant and smoke expectations have been corrected to 1.0 mm; the track rises about 10.1 mm over its length at 5.97°.

The sampler ports the upstream deterministic 32-bit hash/value-noise pattern and principal tape_at mechanisms: remanence/wear variation, pinholes, flakes/shed, lengthwise scratches, creases, edge lift, mold colonies and Vaseline film. The raster sampler averages four physical samples across the selected SP (58 µm) or LP/EP (26 µm) head width, offset sideways by configured track-pitch misalignment. Head-state and control-track paths use detail-off/flake-on sampling, matching the source deck/sync sampling mode. TapeSurfaceSample reports shed as well as magnetization, spacing lift, roughness and film.

This remains an adaptation, not yet sample-by-sample bit-identical parity: the source measured wear/dropout/scratch/edge curves, crease top-end transform and tape-seed transform are ported, but the AE UI still has one master Tape Surface control rather than independent upstream tape knobs. Sticky-binder/transport stick-slip coupling and user-selectable cassette seed remain incomplete. The image-domain renderer still approximates the RF chain.

### 4. Two-head deck recurrence with physical tape sampling

HeadConditionParams supports usePhysicalTapeSamples. When enabled, each active head field samples 12 evenly spaced points along the physical helical track, averages tape shed and film, and feeds deterministic debris pickup/lodging/clean-off and asymmetric grease transfer/decay. The AE adapter enables this when Head Condition is active and passes selected tape speed. Head efficiency affects FM and color-under; color-only compensation follows source DeckParams::color_boost.

The source Swift DeckSimulator was reviewed: it samples 12 points per active field with detail disabled and flakes enabled and retains entering states of the last 12 fields. VHS-90 ports the 12-point sampling and recurrence but still lacks the source rolling 12-entry snapshot/history cache; it replays from field zero.

### 5. VCR coefficient/helper layer

VHS90VcrSignalEngine contains helper coefficients for the source 28.636 MHz sample rate, 1820 samples/line, channel gain/CNR, 64×81 spacing bank, 95-tap analytic FM bandpass, Butterworth low-pass, emphasis/de-emphasis shelves, input trap and chroma-delay calculation. These helpers are tested but are not yet consumed by a complete record/playback image pipeline.

## Verification

- Linux host-independent core CI run 561 passed after the tape geometry helper was first added: https://github.com/vamprieieuu/vhs---90/actions/runs/44626248722
- The source track-start correction changed 1.52 mm to 1.0 mm. Runs 567/568 exposed a stale old-start assertion and then an incorrect track-rise expectation; both were corrected in commit 7c07cfe.
- Follow-up Linux core CI is run 572: https://github.com/vamprieieuu/vhs---90/actions/runs/44779518318. Treat the corrected geometry as pending verification until this run concludes successfully.
- The Linux workflow validates only host-independent C++ core and smoke tests. It does not compile the AE adapter or prove Windows/MSVC, AE SDK, PiPL, AEX loading, live AE render or MFR.

## Preset migration disposition

All 45 JSON presets were semantically reviewed. The report 47 classifies:
- physical VCR/tape/deck looks that are useful candidates for the current analog engines;
- receiver/sync animation looks requiring parameter mapping and host keyframes;
- NTSC-only looks whose effects live in ntsc.settings even when a preset name says “Glitch”;
- seven digital-camera/codec looks excluded from the analog VHS port;
- five Screen Loop feedback looks excluded as a separate effect;
- CRT shader and downscale settings that are separate stages/host behavior.

Normal look timeline keys are whole-state NTSC/shader snapshots with easing. They must be converted to AE host keyframes for the controls actually implemented. JSON parsing alone is not a native preset migration.

## Licensing

NTSCRT is MIT-licensed. Attribution and license are preserved in ThirdParty/NTSCRT. Dependency and shader notices were reviewed: ntsc-rs has permissive MIT/ISC/Apache options; librashader runtime is MPL-2.0; bundled shader licenses vary and include GPL-2.0/3.0. GPL-covered shader sources are not copied into VHS-90 by default.

## Still required before NTSCRT-derived implementation can be called complete

1. Expose independent source-equivalent tape knobs and user-selectable tape seed in the AE UI; implement sticky-binder/transport stick-slip coupling.
2. Verify tape material sample parity against upstream point outputs across positions, seeds, damage combinations and SP/LP/EP.
3. Complete the nine-stage sample-domain RF/FM/color-under record/playback chain: vcr_record, vcr_tape, vcr_magnet, vcr_play, vcr_fmband, vcr_line, vcr_comb, vcr_acc and vcr_out.
4. Complete receiver sync/color-reference parity: line/field timing, sync separator, horizontal AFC/VCO, vertical integrator, burst phase, chroma PLL, ACC/color killer and deterministic rolling history.
5. Port source 12-field rolling deck/head snapshot semantics and transport history/replay.
6. Convert selected analog presets into native VHS-90/AE presets, map supported parameters and translate timeline snapshots/easing to host keyframes; do not port digital/Screen Loop looks into the tape engine.
7. Run deterministic source-vs-C++ point/field/frame comparisons and visual reference comparisons.
8. Build the optional AEX target on Windows x64/MSVC with the exact AE SDK, validate PiPL and load/render/MFR in After Effects. No Windows or AE-host validation is currently claimed.
