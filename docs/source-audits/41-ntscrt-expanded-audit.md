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
- `port/engine/src/glitch/transport.rs`, `stage.rs`, `vcr.rs`, `glitch.wgsl`, `vcr.wgsl`: identified as the next source-review set; detailed function-by-function review remains open.
- `Sources/CrtCore/Tape.swift`: physical tape geometry and SP/LP/EP read-overlap equations.
- `Sources/CrtCore/VcrStage.swift`: 28.6 MHz / 1820 samples-per-line VCR signal path, FM luminance, color-under, emphasis/de-emphasis, band-pass, spacing-loss filter banks, and line planning.
- `Sources/CrtCore/Receiver.swift`: receiver circuit model and deterministic replay architecture (previous audit; re-check against current upstream).
- `Sources/CrtCore/GlitchRenderer.swift`: deterministic frame/field replay and checkpoint model (previous audit).

## New VHS-90 port in this pass

- Added `VHS90TrackReadEngine.{h,cpp}`: a CPU C++17 raster approximation of NTSCRT's three-track head read. It models center/neighbor track overlap, SP/LP/EP-dependent track geometry, azimuth-related suppression of the FM/luma contribution versus the lower-frequency color-under/chroma contribution, tracking misalignment, premultiplied alpha and an exact zero-amount bypass.
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

- The new track-read unit smoke test passed its first hosted run.
- The initial pipeline integration CI failed because `VHS90TrackReadEngine.cpp` was not actually registered in the CMake source list; this was corrected in a follow-up CMake commit. The corrected CI run is pending at the time of this report.
- Static AE parameter audit after adding the two controls: 81 real parameter slots and 81 `PF_ADD_*` calls.
- No Windows/MSVC AE SDK build or live After Effects runtime/MFR test is claimed.
- NTSCRT's current 359-file tree is **not fully audited**, and this repository is **not complete**.
