# Source Audit 10 — NTSCRT 0.26.0 and its ntsc-rs submodule

**Folder:** github repos/NTSCRT-0.26.0  
**Status:** supplied project architecture, Swift/Metal simulation, Windows Rust port, tests and submodule dependency reviewed. The bundled Vendor/ntsc-rs directory is empty in the ZIP; the declared submodule URL is https://github.com/ntsc-rs/ntsc-rs. Its current upstream source was inspected separately to fill that missing dependency, but its exact commit cannot be proven from the ZIP.

## NTSCRT project architecture
- SwiftUI/Metal desktop application with a CPU NTSC stage and GPU signal-receiver/VCR stages.
- Optional NTSC CPU stage is bridged through a small C ABI dylib. The app copies clean BGRA8 pixels, processes in-place, and uploads the result; clean source pixels are cached to avoid repeated GPU readback.
- Metal stage chain has explicit input/output textures and compute passes. The Windows port uses Rust/wgpu and WGSL translations, but is a separate in-progress implementation.
- Source also includes a CRT shader chain, crop/downscale, timeline, keyframe/export, preview queue, CLI/MCP and UI. Those app/UI components are not part of VHS-90's native effect engine.

## ntSC-rs core actually inspected (upstream submodule)
The source is a no_std Rust CPU image-processing library, edition 2024, with optional standard library/Rayon and SIMD dispatch. Core modules inspected include settings, filter transfer functions, NTSC processing, noise, SplitMix64 random generation, row shifting, YIQ fielding and pixel-format normalization.

### Signal algorithms
- RGB↔YIQ transforms and field selection (alternating, upper/lower, both, interleaved fields).
- Constant-K and second-order Butterworth IIR filters; filter cascade coefficient multiplication; filter initial-condition handling; SIMD row processing.
- Input luma box/notch filters, I/Q bandwidth limiting, quadrature composite carrier modulation, pre-emphasis and selectable chroma demodulators: box, notch, one-line comb, two-line comb, 2D FIR and adaptive 2D.
- Deterministic SplitMix64 with separate mixed seeds per processing pass and frame; simplex/FBM noise with octave count, gain and lacunarity.
- Composite noise, independent luma and I/Q noise, snow transients, chroma phase error/noise, horizontal/vertical chroma delay, luma smear, ringing via a notch transfer function, VHS bandwidth and chroma loss.
- VHS SP/LP/EP settings in this core: SP 2.4 MHz luma / 320 kHz chroma / 4-sample delay; LP 1.9 MHz / 300 kHz / 5; EP 1.4 MHz / 280 kHz / 6.
- VHS edge wave is 2D FBM sampled by row and frame, then shifts Y/I/Q together with edge extension.
- Chroma vertical blending averages each chroma row with the previous row.

### Tracking Noise — important source behavior
The upstream function explicitly says its implementation is inaccurate and does not allow the user to control its position. It always affects rows at the bottom of the Y plane, applies a per-row simplex displacement that grows toward the bottom, adds FBM line noise and anisotropic transient speckles. This is not sufficient for VHS-90 without the required arbitrary region controls.

### Head Switching — important source behavior
- Settings: height, offset, horizontal shift, optional mid-line position and jitter.
- Defaults: height 8 rows, offset 3, horizontal shift 72, mid-line position 0.95 and jitter 0.03.
- It is anchored at the bottom of the image, shifts each affected line by a power-shaped amount plus deterministic per-row noise, optionally copies only the right portion of a mid-line and adds a decaying transient.
- VHS-90 must generalize the start position and region height to any point in the frame and permit expansion up to the full frame; the algorithm's line-shift and transient behavior can be reused.

## Native After Effects plug-in reference found in the upstream repository
The submodule repository also contains a native AE/Premiere Rust plug-in crate and an OpenFX plug-in crate. The AE build uses an AE/Premiere SDK wrapper and generates PiPL metadata; its flags include NonParamVary, DeepColorAware, SendUpdateParamsUI, SupportsSmartRender, FloatColorAware, RevealsZeroAlpha, SupportsThreadedRendering and SupportsGetFlattenedSequenceData. Its SmartFX pre-render requests the full input region, sets result/max-result bounds, and SmartRender handles pixel checkout. Pixel conversion paths include 8-bit, AE 16-bit normalized to 0..32768, and 32-bit float. The plug-in also contains preset load/save controls and dynamic parameter registration from a settings schema.

VHS-90's host/build architecture remains VHS6RIFT as requested. These AE details are a secondary reference for validating PiPL, flags, pixel formats, SmartFX, alpha and MFR behavior; do not replace the selected C++ AE SDK architecture with Rust.

## NTSCRT's physical VCR simulation
The supplied Swift/Metal source is the most detailed physical tape model in this folder:
- Tape geometry models helical tracks, 58 µm SP track width, 5.97° angle, field advance and 5.8 m/s writing speed.
- TapeSurface generates a seeded procedural material map in tape coordinates: oxide pinholes/flakes, longitudinal scratches, folds/creases with track shortening, crinkled edges, mold colonies, Vaseline/film, lift, magnetization, output and shedding.
- DeckSimulator models two heads, independent wear/mismatch, gap and spacing loss, baked-on dirt, debris pickup/cleaning, clogs lodging/knocking loose, and grease pickup/wipe-off. State can be snapshotted and replayed by field.
- TransportSimulator models capstan servo, control-track pulse detection, servo drift, stick-slip guides, tension, flutter, tracking offset, search/fast-forward and recovery when play resumes.
- ReceiverSimulator models sync/burst acquisition, horizontal/vertical hold, AFC response, signal strength, ghost/multipath, hum, time-base error, drum flutter, tape folds, dub-generation errors, signal loss and chroma recovery.
- VCR GPU chain models recording pre-emphasis, FM luma, color-under chroma, tape magnetic recording, head/tape gap and spacing loss, azimuth loss, FM bandpass/demodulation, dropout detection, ACC/APC/color killer, combing and dropout compensation.
- Tape/deck conditions are explicit physical causes, not arbitrary overlays. The same tape surface equations are mirrored between Swift CPU probe and Metal GPU shader and tested for CPU/GPU agreement.

## Test and portability findings
- Swift tests include receiver simulation, VCR CPU/GPU parity, field lines, glitch stage, export, crop, timeline and digital video.
- The supplied Windows Rust port includes parity tests and fixtures for healthy, weak signal, horizontal/vertical hold, transport, slack, search, VCR, tape and dubbing states.
- The NTSCRT app's optional CPU stage is explicitly not thread-safe per instance and expects single-thread use. VHS-90 must instead use per-render/per-instance state, deterministic frame-index seeding and MFR-safe buffers.
- Metal kernels and Rust SIMD cannot be copied directly into the C++ CPU plugin; translate the equations and keep GPU acceleration optional.
- The main project archive's ntsc-rs submodule is empty, so the exact pinned Rust source was not available locally. Upstream main was inspected to understand the dependency; before final code port, pin a commit and keep the actual converted C++ implementation inside VHS-90.

## VHS-90 implementation priorities
1. Reuse and port the tape-surface, track geometry, head read/gap/azimuth loss and dropout physics as a shared Tape Physics module.
2. Port the transport servo, flutter, stick-slip and timing error as a shared Transport/TBE module.
3. Port receiver sync/burst/hold/AGC/ghost/hum behavior as a shared Analog Receiver module.
4. Port NTSC YIQ, filters, composite modulation/demodulation, combs, chroma phase and deterministic noise as the shared Analog Signal engine.
5. Generalize Tracking Noise and Head Switching to arbitrary user-defined regions; Head Switching can reach full-frame coverage.
6. Preserve alpha and premultiplication in the host adapter; deterministic frame-addressable simulation is required for MFR and render-order independence.

The app's CRT shader presets, UI, timeline and file I/O are not being copied into the VHS-90 signal engine. No claim is made that the Swift/Metal/Rust modules have already been converted to C++ in this commit.
