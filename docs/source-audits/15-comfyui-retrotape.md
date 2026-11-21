# VHS-90 — ComfyUI RetroTape Core / GPU Audit

Date: 2026-11-19
Input: `vhs/github repos/ComfyUI_RetroTape-main/` from supplied `vhs.zip`.
Inventory: 25 files, approximately 3.15 MB, including CPU core, CUDA/PyTorch implementation, device fallback, ComfyUI node/server/preview cache, 7 test modules and browser UI.

## Processing engine

This project has a full-resolution, frame-indexed CPU implementation and a parallel PyTorch CUDA implementation. It is a much richer source for VHS-90 than a simple shader overlay.

Core processing is performed in YIQ planes, with spatial dimensions scaled relative to a 720×480 reference frame. Its pipeline includes:

1. YIQ conversion and saturation / black-level lift.
2. Horizontal and vertical chroma bleed.
3. Frame-indexed chroma phase/color drift.
4. Composite carrier artifacts.
5. Luma ringing.
6. Independent luma/chroma noise.
7. Chroma delay and luma ghosting.
8. VHS bandwidth filtering, edge-wave distortion and chroma vertical blend.
9. Tracking warp and tape jitter.
10. Interlace/field mixing and vertical roll.
11. Causal luma smear and highlight trail.
12. Localized dropouts with bright/dark target regions and noise.
13. Scanline modulation.
14. Head-switching damage.
15. YIQ reconstruction and analog glow.
16. Blend back to original image.

## 28 exposed controls

### Float controls
- Blend: 0–1
- Saturation: 0–2
- Black Lift: 0–1
- Ringing: 0–1
- Tracking Jitter: 0–12
- Tape Warp: 0–18
- Dropouts: 0–1
- Ghosting: 0–1
- Scanlines: 0–1
- Color Drift: 0–1
- Luma Smear: 0–1
- Interlace: 0–1
- Vertical Roll: 0–1
- Analog Glow: 0–1

### Integer controls
- Seed: 0–2,147,483,647
- Video Noise: 0–9,000
- Chroma Noise: 0–32,768
- Chroma Phase Noise: 0–120
- Chroma Loss: 0–5,000
- Horizontal Color Bleed: 0–24
- Vertical Color Bleed: 0–12
- Ringing Power: 1–12
- VHS Edge Wave: 0–12
- Scanline Phase: 0–270
- Scanline Offset: 0–3
- Chroma Delay: 0–24

### Boolean controls
- VHS Enabled
- Head Switching

## Presets

The core defines 4 source-derived presets and 15 additional custom presets (19 total):
- yobelkcip
- Master Nama
- RGM
- JaneLon
- Sun Bleached
- First Gen
- Home Movie 1987
- Rental Copy
- Film Transfer
- Late Night Broadcast
- Broadcast Dirty
- Weather Channel
- Security Cam
- Kinescope
- AE Punch
- Chrome Dream
- Bootleg Anime
- Damaged Tape
- Tracking Hell

The first four are documented in the source as copied verbatim from ntscQT's community `builtin_templates.json`; the project also supplies its own custom preset recipes and overrides. VHS-90 should translate useful parameter combinations into newly authored VHS-90 preset names rather than exposing the source handles or names.

## Tracking, edge wave and head-switching geometry

- Tracking shifts are computed as a frame-dependent row displacement profile, combining sinusoidal tape warp with seeded low-frequency servo motion and occasional localized line segments.
- Tracking affects the full frame as row-wise horizontal displacement; the core does not expose independent user-positioned regions or multiple simultaneous tracking zones.
- Edge Wave is a separately seeded per-row displacement profile, interpolated across four-frame temporal epochs to avoid flicker.
- Dropouts are explicit rectangular bands with randomized row/height/X/length/strength and stripe noise. They can occur across the frame, unlike the bottom-only head switch.
- Head switching is hard-coded to the bottom 4.5% of the image. The function computes `start = height - band`, shifts the affected rows, adds noise and attenuates chroma.
- VHS-90 must replace this bottom-only head-switch geometry with its custom vertical position and adjustable height, including full-frame coverage. Tracking Noise must likewise use the shared arbitrary region engine, not only full-width row displacement.

## Temporal determinism and frame processing

- CPU noise, tracking, dropouts and edge-wave behavior are seeded from the user seed and absolute frame index.
- Slow mechanical artifacts use four-frame epochs with interpolation to reduce frame-to-frame flicker.
- Dropouts use a separate half-epoch seed stream so dropout changes are decoupled from tracking/warp changes.
- Frame batches can be processed concurrently; each frame is rendered from its absolute frame index rather than shared mutable previous-frame history.
- The node's frame_offset exists to preserve temporal continuity when a long video is rendered in separate batches.
- This stateless/frame-indexed strategy is useful for AE Multi-Frame Rendering and random-access timeline renders.

## CPU / GPU architecture

- CPU path: NumPy, OpenCV and SciPy.
- GPU path: PyTorch CUDA, not a native Vulkan compute implementation.
- Device choice: auto / CPU / GPU. Auto chooses CUDA when available and falls back to CPU.
- A lock prevents the node and preview worker from overlapping CUDA operations.
- GPU failures and out-of-memory errors are handled with CPU fallback; poisoned CUDA contexts are marked unusable for later work.
- The README reports performance numbers, but they are source-project benchmarks and should not be treated as VHS-90 performance predictions.
- CPU and GPU use different random generators, so the grain patterns are not pixel-identical across devices even with the same seed. The source explicitly warns to keep one device for an entire video render.

## Alpha and image dimensions

- RGB processing is resolution-independent and uses reference-scaled spatial parameters.
- The ComfyUI tensor-batch path allocates three processed color channels and then appends the original fourth alpha channel unchanged when input is RGBA.
- The core CPU function itself accepts/returns RGB only; alpha preservation is handled by the batch wrapper.
- VHS-90 should adopt the explicit alpha-preservation concept, but integrate it at the AE pixel adapter level for 8/16/32-bit formats and premultiplied alpha.

## Tests and local verification

The supplied test suite initially failed collection because the archive folder name differs from the package name used by the tests. After providing a temporary package-name alias (without editing the source), the tests ran:

- 204 passed
- 89 skipped
- Total: 293 tests
- Runtime: 11.20 seconds

The skipped tests are primarily GPU/CUDA-dependent because this environment has a CPU-only PyTorch build. The passing suite covers core processing, preset parsing, CPU/GPU fidelity scaffolding, preview/cache behavior, input hardening, device selection and performance expectations. No CUDA execution or ComfyUI host runtime was tested.

## VHS-90 integration decision

Port selected CPU algorithms into the shared C++17 engine: YIQ transforms, spatially scaled low-pass filters, ringing, chroma bleed/delay, phase drift, composite artifacts, deterministic noise, row-wise tracking warp, edge-wave profiles, dropout event geometry, causal luma smear, scanlines and glow. Use the PyTorch implementation as a secondary mathematical reference only; do not make PyTorch a plugin dependency. Keep the CPU path authoritative.

Do not import ComfyUI node/server/web preview code, Python UI, ComfyUI-specific tensor types or CUDA runtime into the native AEX. Preserve the source's absolute-frame determinism and alpha pass-through ideas. Replace bottom-only head switching with VHS-90's arbitrary region geometry.
