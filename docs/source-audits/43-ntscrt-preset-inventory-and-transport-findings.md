# NTSCRT source audit — preset inventory and transport findings

Date: 2026-11-20  
Upstream: https://github.com/finnmckenty/NTSCRT  
Target: https://github.com/vamprieieuu/vhs---90

## Repository inventory verified

The upstream recursive Git tree was fetched successfully (not truncated):
- 420 total tree entries
- 359 file blobs
- 59 directory entries

This is a repository inventory, not a claim that all 359 files have been read line-by-line. Binary artwork, test fixtures, application/UI code, generated notices, Rust engine code, shaders, and presets require different review/port decisions. The file-level completion ledger remains open.

## Full preset inventory pass

All 45 JSON preset files under `presets/` were fetched and parsed. The set is not 45 independent VHS looks:
- VHS/deck/transport looks include clean, mild, medium, damaged, dirty-deck, bootleg, color-loss, tracking, head-switch, and severe glitch cases.
- Several entries are animated/timeline-driven effect sequences (2-second/5-second glitch loops, blips, static, tracking noise, vertical roll/spike, and wavy loops).
- Some are digital-only codec/downscale looks (CD-ROM, cheap camcorder, webcam, chunky webcam, streaming/video-block presets); these must not be misrepresented as analog tape presets.
- Five `Screen Loop` JSON files use the distinct `ntscrt-video-feedback` schema (copy count/delay, keying, loop, zoom, drift, turn, shake, color/hue drift, contrast/brightness). They are not ordinary `GlitchSettings` presets and require a feedback/reprojection engine if reproduced in VHS-90.

The five feedback presets were individually parsed. Their common parameter set contains auto exposure, brightness, center, color/hue drift, contrast, copy counter, delay, X/Y drift, focus, key/key invert, loop, push, roll, seed, shake, spin, tilt, turn, and zoom. The names and parameters should be preserved in a future preset-conversion map rather than flattened into unrelated VHS controls.

## Additional source modules read in this continuation

- `port/engine/src/glitch/tape.rs`: physical tape geometry, SP/LP/EP pitch/width, head-width/track-width relationships, three-track read weighting and azimuth behavior.
- `port/engine/src/glitch/receiver.rs`: NTSC timing constants, raster/field structure, deterministic receiver simulation, field plans.
- `port/engine/src/glitch/stage.rs`: render history, deterministic re-simulation, checkpointing, background reruns, and cancellation model.
- `port/engine/src/glitch/vcr.rs`: sample rate (8× subcarrier), 1820 samples/line, 629 kHz color-under, FM band 1.3–6.6 MHz, spacing/gap channel gain, filter banks, and VCR field-plan data.
- `port/engine/src/glitch/vcr.wgsl`: shader-side tape geometry, deterministic noise, FM/color-under signal structures and processing stages.
- `LICENSE`: upstream is MIT; copyright and permission notice must accompany substantial copied portions.

## Important transport findings

The original control-track servo is not generic line jitter:
- one control-track pulse is evaluated once per frame at the tape's control-track location;
- pulse amplitude depends on tape output/remanence and head lift;
- a pulse is accepted only above a threshold with deterministic variation;
- a passing crease may generate a false pulse;
- a missing pulse leaves the capstan's settled speed unchanged;
- a valid pulse applies a proportional kick of `-0.377 * error`, with integral update `integrator += 0.0986 * error`, clamped to ±0.15; servo speed is the negative integrator;
- picture search disables the servo and advances track phase at the selected search multiple;
- returning to play has to pull phase back into lock.

The existing VHS-90 `VHS90TapeTransportEngine` remains a deterministic raster warp with drum/flutter/stick-slip/jitter and a slow-profile servo-like noise term. It does not yet implement this pulse detector/PI servo, physical tape-edge sampling, missing/fake pulse semantics, or search/relock state. It must not be labelled a completed NTSCRT transport port.

## Deck/head findings

The upstream deck model has two alternating heads and separate wear, dirt, buildup and mismatch parameters. Each field:
- samples tape shed and film at 12 positions along the physical track;
- updates only the head active for that field;
- accumulates debris by pickup and decay;
- deterministically lodges occasional debris chunks;
- deterministically sheds/cleans debris events;
- clamps debris to a physical range;
- transfers greasy film quickly and removes it slowly;
- exposes history snapshots for field-addressable playback.
The current VHS-90 head-condition raster approximation is not equivalent and still needs a faithful field-domain state model.

## Porting decisions

1. Keep upstream UI, macOS app, CLI/MCP, exporters, media framework, and packaging code out of the native AE plugin unless a specific algorithm or data format is reusable.
2. Prioritize reusable signal algorithms and physical models: tape geometry/material, receiver timing/sync, transport servo, deck/head state, RF/FM/color-under, dropout/compensation, chroma recovery, and deterministic noise.
3. Convert each reused module to C++17 CPU code where practical; do not import Rust/WGPU runtime dependencies into the AE plugin.
4. Preserve MIT attribution for substantial copied code and track any third-party shader/crate license separately.
5. Design stateful simulations for AE's out-of-order and multi-frame rendering. Use immutable frame plans/checkpoints or deterministic replay; never rely on a process-global mutable simulator.

## Status

- Preset JSON inventory: 45/45 fetched and parsed.
- Key physical source modules listed above: reviewed.
- Upstream repository inventory: 359 files / 59 directories verified.
- Full 359-file source audit: **not complete**.
- Full control-track PI servo port and AE integration: **not complete**.
- Faithful two-head deck recurrence and AE integration: **not complete**.
- FM/color-under playback engine port: **not complete**.
- Windows/MSVC + AE SDK build and live AE/MFR validation: not claimed.
