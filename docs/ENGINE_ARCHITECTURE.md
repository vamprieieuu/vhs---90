# VHS-90 — Unified Engine Architecture

Status: shared CPU engines and an initial AE host adapter are present in source. The Windows/MSVC build and AE runtime have not yet been verified.

## Core principle

VHS-90 is one native AE effect with one shared render pipeline. The UI is organized by function, not by the source plugin/repository. Presets are parameter recipes that activate combinations of shared modules. Reference-specific visual identities remain separate presets; low-level kernels may be shared.

## Render pipeline

1. **AE input adapter** — read 8-bit, 16-bit or float worlds using rowbytes and render-rectangle coordinates; convert to normalized working representation without losing HDR headroom.
2. **Alpha normalization** — carry alpha separately; define straight/premultiplied handling once and preserve the original alpha plane through every stage.
3. **OSD composition** — procedurally render PLAY/STOP/SP/TAPE/ERROR/P.O.1/RES/REW/FF, date, clock and counter using the supplied font pack. Compose OSD onto the source before VHS damage so signal/tape stages affect the display text too.
4. **Base luma/chroma preparation** — optional luma bandwidth, chroma bandwidth, softness, subsampling, pre-emphasis, saturation, pedestal and channel registration.
5. **Record/composite signal stage** — optional RGB/YIQ or RGB/YUV composite carrier encode, scanline/field phase, chroma modulation and demodulation. Keep this optional for looks that are better represented by direct signal damage.
6. **Tape transport / timing** — deterministic timebase error, line jitter, vertical roll, servo drift, phase instability, edge wave and frame/field timing.
7. **Tape surface / media defects** — tape-coordinate creases, scratches, roughness, debris, voids, oxide dropouts, head wear and media noise. Expensive physical-model variants may be optional quality modes.
8. **Tracking / head-switch region engine** — arbitrary user-positioned regions, independent geometry and event behavior, supporting multiple simultaneous horizontal, vertical and rectangular regions.
9. **Signal damage events** — horizontal sync collapse, luma pulses, static bursts, dropouts, tears, RF ingress, snow and blackout. Keep event probability, intensity, duration and temporal envelope separate.
10. **Playback / receiver decode** — sync recovery, AFC hunt, chroma lock, chroma killer, comb filtering, cross-color/dot-crawl, chroma loss and decoder noise.
11. **Image-domain finish** — ghost/multipath, luma smear, chroma bleed, ringing, sharpening, bloom/halation, grade and final mix.
12. **AE output adapter** — convert back to the requested AE pixel format, preserve alpha and respect destination rowbytes.

## Shared module families

### Luma & Chroma Signal

Independent Y/I/Q or Y/C filtering, color-under bandwidth, pre-emphasis/de-emphasis, phase noise, chroma delay, vertical chroma blending, chroma bleed and Y/C crosstalk.

### Tape Transport & Defects

SP/LP/EP format curves, deterministic timebase error, servo/tracking drift, tape-coordinate defects, creases, scratches, debris, voids and wear. Physical tape modeling is an optional higher-cost path, not the default fast renderer.

### Tracking Noise & Head Switching

Use VHS90RegionEngine as the shared geometry layer. Each region stores normalized X/Y/width/height, shape, feather, intensity, direction/speed, displacement, noise density, dropout probability, chroma offset and seed offset. The current CPU implementation supports arbitrary horizontal, vertical and rectangular regions, smooth servo drift, segmented tracking tears and independent head-switch regions.

Tracking Noise must support top, bottom, left, right, arbitrary interior positions and multiple simultaneous regions. Coverage must reach the full frame. Head Switching must support arbitrary vertical position and height, including full-frame coverage. The initial AE UI exposes one Tracking Noise region and one Head Switching region; the core accepts multiple regions.

### Static / Snow / Grain / Signal Noise

Separate algorithms for monochrome luma static, colored speckles, full-frame RF snow, fine grain, chroma noise, luma noise and horizontal segmented streaks. Density, size, brightness, saturation, opacity, event rate and flicker are independent.

### Tears / Sync Collapse / Dropouts

The current sync engine implements deterministic event envelopes, primary/secondary tear bands, time-base noise, a moving tracking line and vertical roll. Full frame freeze/hold, VBI/HSYNC waveform, receiver PLL/AFC recovery, blackout and RF ingress remain separate implementation gates.

### OSD / Blue Screen

One procedural OSD renderer shared by original footage and generated blue-screen modes. Arabic shaping, bidi, RTL and Arabic-Indic digits are provided by a real Windows text-shaping API, with the supplied 15 font files as the only bundled fonts.

## Determinism and MFR

- No global mutable RNG, wall-clock time or shared frame history in render kernels.
- Every event is keyed by user seed, absolute frame/time, region/event index and spatial cell.
- Same frame/time/seed must reproduce exactly regardless of render order or worker scheduling.
- Mutable temporal history is avoided unless an explicit per-instance cache is proven safe for arbitrary-time AE rendering.
- Scratch buffers are per-render or from a thread-safe pool; no shared mutable pixel buffers.

## CPU / GPU strategy

CPU C++ is the authoritative implementation and must load/render without a GPU runtime. Optional GPU acceleration can be added after CPU output is stable, with parity tests for event placement, geometry, alpha and deterministic stages. Do not make Vulkan, CUDA, Metal or OpenCL a required dependency.

## Preset architecture

Presets are data-only recipes with independent module toggles and values. Selecting a preset configures only its intended parameters; it must not silently enable unrelated engines. Preserve user editability after selection. Keep source-plugin names and source preset branding out of the UI.

## Current implementation boundary

Implemented in repository:
- Standard source-matched RGB↔YIQ conversion.
- Row-wise Constant-K / Butterworth YIQ bandwidth filters.
- Four-phase NTSC composite encode/decode with Box, One-Line Comb and Two-Line Comb demodulators.
- Generalized region geometry, smooth tracking servo drift, segmented tears, and head-switch regions.
- Procedural static patterns and sync/time-base damage with deterministic roll events.
- Initial AE host adapter source with SmartFX pre-render/render, legacy render fallback, ARGB32/64/128 conversion, full-frame input request and output-region crop.
- PiPL resource, optional Windows/MSVC AEX target and PowerShell build script.
- Six standalone C++ smoke tests.

Not yet implemented or verified:
- Windows/MSVC compilation of the AE adapter, PiPL resource and final AEX.
- AE 23.2.1 load/apply/preview/render/MFR validation.
- Full composite carrier sync/burst/AFC/PLL, full dropout/blackout/freeze/hold and RF/no-signal state machine.
- Physical tape stage, OSD/font shaping, Blue Screen mode and preset catalog.
- Final visual comparison against all supplied reference clips/stills.

Do not call VHS-90 test-ready until the Windows build and in-host validation gates pass.
