# Source audit: allthemalware/vhs-phys

Audit date: 2026-11-19
Source repository: https://github.com/allthemalware/vhs-phys
Inspected ref: `main`, tree SHA `1dccc14c74d4c575e7464b91a94b10c5f39f4beb`.

## Inventory

The recursive Git tree returned 225 entries: 202 files and 23 directories; GitHub reported `truncated=false`. Top-level areas: `.github/workflows`, `crates/vhsphys-cli`, `crates/vhsphys-core`, `docs`, `packaging`, `presets`, `python`, and `scripts`, plus Cargo metadata, README and LICENSE.

The core crate contains 42 Rust source files, including:
`aperture.rs`, `audio.rs`, `automation.rs`, `camera.rs`, `camera_processing.rs`, `composite.rs`, `condition.rs`, `config.rs`, `consts.rs`, `consts_pal.rs`, `convert.rs`, `crt.rs`, `decode.rs`, `dsp.rs`, `electronics.rs`, `emphasis.rs`, `format.rs`, `frame.rs`, `geometry.rs`, `head.rs`, `interference.rs`, `interference_params.rs`, `lib.rs`, `mech.rs`, `pipeline.rs`, `rack.rs`, `record.rs`, `rf_rate.rs`, `rng.rs`, `signal.rs`, `spectral.rs`, `standard.rs`, `stripe.rs`, `tape.rs`, `tracking.rs`, `tube.rs`, and `video.rs` (plus small format-specific modules).

## Files and subsystems inspected directly

Fetched and reviewed the source headers and implementation structure of:
- `crates/vhsphys-core/src/signal.rs` — exact rational programme time and typed signal/raster contracts.
- `tracking.rs` — physical head-to-track lateral weights, track offset, slope, bow and wander; neighbouring-track cross-read.
- `tape.rs` — deterministic spatial defect map in tape coordinates, longitudinal creases, moving point defects, debris spacing loss and missing-oxide attenuation.
- `head.rs` — RF readback pulses, spacing/Wallace loss, timebase warp and magnetisation controls.
- `record.rs` — FM luma and colour-under recording, hysteresis/reversal events and sub-sample transition timing.
- `decode.rs` — sync detection, AFC, clamp, chroma demodulation, burst reference and optional composite decode.
- `pipeline.rs` — field-based staged record/readback/electronics/decode pipeline and bounded lookahead/cache design.
- `interference.rs` — physical interference coupling into signal-chain nodes.
- `dsp.rs` — stateful biquad filters, filter design and resampling primitives.
- `LICENSE` and README architecture/feature description.

This is a targeted code-level inspection of the principal VHS signal path, not a claim that all 202 files were individually read line by line. The recursive tree inventory itself is complete.

## High-value findings for VHS-90

1. **Tracking should be tied to tape/head geometry, not only a screen-space noise band.** The source models lateral head offset, along-track slope, parabolic bow and random wander, then calculates the fractional overlap with adjacent tracks. This is a strong reference for future physically driven tracking presets. VHS-90's existing user-facing region engine remains necessary because the user's requirement is arbitrary screen-space placement and sizing, not only physical head tracking.
2. **Tape defects should be deterministic in tape coordinates.** A spatially hashed defect field gives repeatable output at arbitrary frame/time positions without storing a giant texture. Longitudinal creases stay at fixed picture height while point defects move with tape advance. This is a high-value design to port into a standalone C++ tape-defect engine.
3. **Dropout depth should have multiple physical causes.** Debris/head lift changes effective spacing and RF attenuation; missing oxide reduces magnetisation. This is better than one generic random dark-pixel pass.
4. **A real tape path is a staged signal pipeline.** Record modulation, tape transition map, playback head, tracking/adjacent-track crosstalk, electronics, sync/AFC and chroma decode are separate stages. VHS-90 can borrow the staged architecture selectively, but a full 43 MS/s RF physical simulation is not currently suitable as the default interactive AE CPU effect.
5. **Stateful DSP primitives are reusable in principle.** The Rust biquad implementation is a useful numerical reference. VHS-90 already has a C++ IIR/YIQ bandwidth module, so no duplicate filter code was copied in this audit.

## Transfer decision

- **Copied source files:** none in this pass.
- **New C++ files:** none in this pass.
- **Reason:** the inspected Rust modules depend on a large set of crate-specific types, RF sample grids, scheduling, caches, and physical units. Copying isolated Rust files into the C++ AE project would not compile or preserve their model. The correct next step is a deliberate C++ port of selected independent algorithms (deterministic tape defect spatial hash; track lateral-overlap model; separate spacing-loss and oxide-void controls), with dedicated tests and alpha/MFR-safe frame integration.
- Existing VHS-90 C++ IIR implementation overlaps the source DSP primitive; avoid adding a redundant filter implementation.

## License / provenance

The repository's LICENSE is PolyForm Noncommercial 1.0.0. It grants noncommercial uses under its conditions; commercial use requires separate permission from the author. The user has stated they have the required permissions for supplied source material. Retain provenance and the applicable license notice with any future copied or adapted source. This audit does not assert that the user's separate permission has been independently verified.

## Status

- Recursive file/folder inventory: complete for this repository ref.
- Main signal-path code inspection: targeted, not every file line-by-line.
- Algorithms identified for port: yes.
- Code transferred into VHS-90: no.
- Remaining work: inspect the remaining folders (including all documentation, presets, Python UI/lab, CLI, scripts and tests), then implement selected C++ ports with tests.
