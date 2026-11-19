# NTSCRT expanded source audit — current upstream tree

Date: 2026-11-19  
Upstream: `finnmckenty/NTSCRT`  
License: MIT (retain copyright and notice for any copied substantial code).

## Inventory checkpoint

The current recursive upstream tree contains **359 files and 59 directories**:
- `Sources/`: 89 files (Swift app/core/codec/Metal bridge).
- `port/`: 162 files (Rust application, engine, WGSL shaders, codecs, fixtures, parity tools and support scripts).
- `Tests/`: 17 files.
- `presets/`: 45 JSON presets.
- `Vendor/`: 3 files.
- Remaining files: docs, licenses, scripts, workflows, assets, icons and test images.

This is an inventory checkpoint, **not a complete 359-file source review**. The earlier `18-ntscrt-physical-vcr-engine.md` covers the initial 323-file archive snapshot and its physical tape/deck/receiver architecture. The current upstream has since grown with a Windows Rust/wgpu port, additional codecs, UI/MCP infrastructure, presets and tests. The expanded tree must be audited against the current 359-file snapshot rather than assuming the old audit covers new files.

## Files inspected in this pass

- `LICENSE`: MIT.
- `README.md`: full processing order and design summary; digital codec → NTSC signal → downscale → tape/deck → TV glitches → CRT shader.
- `port/engine/src/glitch/receiver.rs`: Rust receiver simulation, NTSC timing, deterministic purpose-keyed noise, sync/burst/AGC/AFC and raster field plans.
- `port/engine/src/glitch/tape.rs`: slanted tape-track geometry, SP/LP/EP track pitch/head width, neighboring-track overlap and frequency-dependent azimuth loss.
- `port/engine/src/glitch/transport.rs`: servo/control-track recovery, capstan speed loop, stick-slip and search transport functions inventoried.
- `port/engine/src/glitch/stage.rs`: checkpointed deterministic rerun/resimulation and GPU pass orchestration inventoried.
- `port/engine/src/glitch/deck.rs`: per-head wear, dirt, buildup, mismatch and field-keyed shedding/clog events inventoried.
- `port/engine/src/glitch/params.rs`: VCR/tape/deck parameter metadata and preset migration functions inventoried.
- `port/engine/src/glitch/vcr.rs`: Cauchy spacing filter bank, analytic FM band-pass taps, field plans and GPU pipeline orchestration inventoried.
- `port/engine/src/glitch/vcr.wgsl`: full shader pass order inspected: tape surface, composite recording, slanted-track read/azimuth, magnetization, head spacing/gap filtering, FM demodulation/dropout detection, color-under recovery, comb filtering, ACC/APC/color killer and dropout compensation.
- `port/engine/src/glitch/glitch.wgsl`: receiver gate, chroma recovery and per-pixel composition pass map inspected; `transport.rs`, `stage.rs`, `deck.rs`, `params.rs` and WGSL function-level review remains in progress.
- `Sources/CrtCore/Tape.swift`: physical tape geometry and SP/LP/EP read-overlap equations.
- `Sources/CrtCore/VcrStage.swift`: 28.6 MHz / 1820 samples-per-line VCR signal path, FM luminance, color-under, emphasis/de-emphasis, band-pass, spacing-loss filter banks, and line planning.
- `Sources/CrtCore/Receiver.swift`: receiver circuit model and deterministic replay architecture (previous audit; re-check against current upstream).
- `Sources/CrtCore/GlitchRenderer.swift`: deterministic frame/field replay and checkpoint model (previous audit).

## New VHS-90 port in this pass

- Added `VHS90TrackReadEngine.{h,cpp}`: a CPU C++17 raster approximation of NTSCRT's three-track head read. It models center/neighbor track overlap, SP/LP/EP-dependent track geometry, azimuth-related suppression of the FM/luma contribution versus the lower-frequency color-under/chroma contribution, tracking misalignment, premultiplied alpha and an exact zero-amount bypass.
- Enhanced `VHS90TapeSurfaceEngine` using the NTSCRT `tape_at` material model: deterministic pinholes, flakes, mold colonies and vaseline film, in addition to the existing oxide voids, scratches, creases, debris and edge lift.
- Replaced the earlier uniform spacing attenuation with a frequency-dependent head-spacing/gap response adapted from `VcrStage.channelGain`: high-frequency FM luma detail is softened more strongly than the low-frequency color-under carrier. This remains a spatial/raster approximation; the upstream Cauchy filter bank and sampled FM chain are not ported.
- Integrated the stage before composite encoding in `VHS90CompositeEngine`.
- Added AE controls: `Adjacent Track Crosstalk` and `Track Misalignment`; the existing `Tape Speed` popup selects SP/LP/EP geometry.
- Added standalone and composite-pipeline smoke coverage.
- This is deliberately an **image-domain approximation**, not a port of NTSCRT's full sampled 28.6 MHz FM/color-under recorder and playback chain. It must not be described as waveform-identical.

## Remaining high-value source work

1. Audit all Rust/WGSL VCR and transport functions against the Swift originals, including tape surface sampling, head spacing, grease, wear, dropouts, FM modulation/demodulation, color-under, comb cancellation and deck state.
2. Review every current `Sources/CrtCore` file not covered by the prior audit, plus the 162-file `port/` tree; classify host/UI/CLI/codec/shader infrastructure separately from reusable signal kernels.
3. Review all 45 presets and map their settings to VHS-90 unified preset recipes where meaningful.
4. Inspect parity and unit tests; use them as behavioral specifications. The upstream's Rust/WGSL and Swift/Metal implementations must not be imported as runtime dependencies into the C++ AE plugin.
5. Consider a deeper optional CPU FM-tape engine only after profiling and after establishing a testable line/sample-domain interface. Preserve AE alpha and render-local deterministic state.

## Validation boundary

- The new track-read unit smoke test passed; GitHub Actions run `44608850831` passed the integrated core build and **26/26 CTest tests**.
- The frequency-dependent head-spacing refinement passed run `44609157611`, **26/26 CTest tests**, including a high-frequency luma-softening assertion.
- The pinhole/flake/mold/vaseline material extension and contamination test are in the current CI run; do not claim that newest change passes until its run completes.
- Initial pipeline integration CI exposed a missing CMake source registration; the corrected CMake list now includes both the engine and smoke target. GitHub Actions run `44608850831` passed the core build and **26/26 CTest tests**, including standalone track-read and composite-pipeline coverage.
- Static AE parameter audit after adding the two controls: 81 real parameter slots and 81 `PF_ADD_*` calls.
- No Windows/MSVC AE SDK build or live After Effects runtime/MFR test is claimed.
- NTSCRT's current 359-file tree is **not fully audited**, and this repository is **not complete**.
