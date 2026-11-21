# VHS-90 — ntsc-rs 0.9.0 Engine Audit

Date: 2026-11-19
Input: `vhs/open source native plugin for ae/ntsc-rs-0.9.0/` from the supplied `vhs.zip`
Scope: full file inventory plus detailed review of the processing crate (`crates/ntscrs`), settings model, YIQ buffer conversion, filter implementation, row-shift/noise primitives, and processing order. GUI/launcher/plugin-wrapper files were inventoried but are not yet treated as audited implementation sources.

## Inventory

- 93 files in this archive subtree: 61 Rust source files, 9 TOML files, 7 Markdown files, 6 extensionless files, 2 PNGs, 2 SVGs, and single workflow/lock/JSON/resource/header files.
- Core image engine: `crates/ntscrs/src/ntsc.rs` (1,402 lines).
- YIQ conversion, pixel formats, fielding and strided-buffer operations: `yiq_fielding.rs` (896 lines).
- IIR filtering: `filter.rs` (490 lines).
- SIMD dispatch and vector primitives: `f32x4.rs` (513 lines).
- Settings and effect parameter model: `settings/standard.rs` (1,176 lines), `settings/settings.rs` (606 lines), `settings/easy.rs` (616 lines).
- Deterministic seed/noise utilities: `random.rs` (83 lines).
- Fractional scanline shifting: `shift.rs` (202 lines).
- Workspace includes Rust AE and OpenFX adapters, a GUI/CLI, and an xtask build system. This is not a C++ drop-in library.

## Processing algorithms worth porting into VHS-90

1. **Planar YIQ processing:** Convert RGB into separate Y, I and Q float planes, process luma/chroma independently, then reconstruct RGB. The matrix is explicitly implemented and useful as a shared native color-domain layer.
2. **Frequency-shaped composite chroma filtering:** Separate I/Q low-pass cutoff frequencies (approximately 1.3 MHz for I and 0.6 MHz for Q in its full composite pass), plus a lighter 2.6 MHz path. Both constant-K cascaded filters and Butterworth filters are supported.
3. **Composite modulation/demodulation:** The engine mixes chroma into luma and recovers it using scanline phase, field parity, phase offsets and selectable demodulation filters (Box, Notch, One-line comb, Two-line comb). This is substantially richer than simple RGB channel offset.
4. **Luma bandwidth and ringing:** Input luma filtering (None/Box/Notch), IIR notch ringing, luma smear, and a pre-emphasis stage.
5. **Independent noise planes:** Composite noise, luma noise and separate I/Q chroma noise have independent settings; frequency/detail/intensity are exposed through FBM-style noise controls.
6. **Phase instability:** Chroma phase error and time/frame-seeded phase noise can alter I/Q independently.
7. **Chroma displacement:** Horizontal fractional chroma delay and vertical scanline delay are independently adjustable.
8. **Tape-speed response:** SP, LP and EP select distinct luma cutoffs, chroma cutoffs and chroma delay values; this provides a useful basis for VHS-90 tape-speed presets.
9. **VHS edge wave and chroma loss:** Edge wave exposes intensity, speed, frequency and detail; chroma loss is an independent degradation pass.
10. **Deterministic transients:** A seed mixer combines global seed, effect-specific identifiers, frame number and row index. Snow is synthesized as shaped transient bursts using geometric event spacing rather than uniform pixel noise.
11. **Fractional row displacement:** `shift_row` and `shift_row_to` perform fractional horizontal shifts with linear interpolation and explicit edge handling.
12. **Field-aware rendering:** Upper/lower/both/interleaved fields are represented explicitly, with Bob/Skip output modes and strided rowbytes-aware input/output support.

## Tracking Noise and Head Switching: important limitation

The source's own tracking-noise function contains a TODO stating that it is inaccurate and does not let the user control its position. Its implementation anchors the affected rows at the bottom and increases intensity toward the lower edge.

Head switching is also anchored to the bottom raster: it derives the affected block from `num_rows - offset`, with a height and offset control. Its optional mid-line setting controls a horizontal transition position/jitter within a row; it does not move the whole head-switch region vertically to arbitrary frame locations.

Therefore, reuse the signal/noise mathematics, but do **not** copy either region geometry into VHS-90. Implement VHS-90's shared arbitrary-region model with normalized X/Y/width/height, orientation, direction, motion, multiple independent regions, and full-frame-capable head-switch height.

## Data layout, alpha and performance observations

- The engine stores Y/I/Q and scratch in separate packed float planes.
- Input conversion accepts multiple RGB/BGR channel orders and 8-bit, 16-bit, signed 16-bit, float32 and a dedicated After Effects 16-bit normalization type.
- It accounts for rowbytes, buffer offsets, vertical flipping, partial rectangles and interlaced fields.
- Alpha is not processed as a YIQ plane. The writer can fill alpha when requested; the normal in-place path leaves existing alpha data intact. VHS-90 should preserve alpha explicitly in its own host adapter and ensure premultiplied RGB is handled correctly.
- CPU work is parallelized with Rayon and some IIR paths use architecture-specific SIMD. Porting to C++ should retain a scalar reference implementation first, then add safe SIMD/parallel acceleration behind deterministic output tests.
- `NtscEffect::apply_effect_to_yiq` constructs a new Rayon thread pool per call. VHS-90 should use a persistent host-safe worker strategy rather than create a pool per frame.
- The crate depends on external Rust crates including `glam`, `rayon`, `biquad`, `rand`, `rand_xoshiro`, and a Git-pinned SIMD noise crate. The Rust workspace is not directly suitable as a self-contained C++ AE effect.

## VHS-90 integration plan

- Port the YIQ transform, scalar IIR filter, fractional row-shift, deterministic seed mixer, transient/snow generator, chroma phase handling, independent Y/I/Q noise and VHS tape-speed parameter curves into native C++.
- Keep modulation/demodulation as an optional higher-fidelity composite pipeline stage; do not force it on effects that are specifically tracking/static/tape-damage looks.
- Rebuild the tracking and head-switch region geometry to satisfy VHS-90's any-position requirement. Source implementations provide behavior inspiration, not placement architecture.
- Use normalized float working planes across 8/16/32-bit AE formats, then convert back through one consistent pixel-format adapter.
- Retain deterministic seed + frame indexing; avoid shared mutable random state across MFR render calls.
- Use the original source's algorithm order as a reference when building a Composite NTSC/VHS preset, while allowing the VHS-90 unified pipeline to route other effects independently.

## Validation status

- Source files were inspected statically from the supplied archive.
- Rust tests/build were not run: Cargo is not installed in this execution environment.
- No claim is made that the Rust workspace or its AE/OpenFX wrappers compile here.
- No Rust code has yet been copied into the VHS-90 native target; this report records the exact algorithms and the constraints for their C++ port.
