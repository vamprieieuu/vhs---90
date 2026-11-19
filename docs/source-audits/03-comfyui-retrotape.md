# Source Audit 03 — ComfyUI RetroTape

**Folder:** `github repos/ComfyUI_RetroTape-main`  
**Status:** complete source review; test suite executed.

## Files and architecture reviewed
Reviewed the complete CPU core (`ntsc_core.py`), CUDA/Torch path (`ntsc_gpu.py`), device fallback, node integration, preview cache, preview server, browser panel, presets, requirements, documentation, and all seven test modules.

This is a 1,737-line CPU image-processing core plus a 958-line CUDA path, with a substantial browser control panel and tests.

## Processing algorithms identified
The CPU core uses RGB-to-YIQ decomposition and a multi-stage image/signal pipeline:
- Scale-aware one-pole horizontal low-pass filters with cached cascaded coefficients.
- Chroma horizontal/vertical bleed, chroma delay, saturation and frame-driven color drift.
- Composite carrier artifacts and NTSC-like scanline phase.
- Luma ringing/sharpening and causal directional luma smear with a separate highlight trail.
- Luma noise, chroma noise, chroma phase noise and row-based chroma loss.
- Analog/VHS bandwidth reduction and chroma vertical blending.
- Deterministic servo-style row displacement, tape warp and interpolated temporal epochs.
- Interlace/field comb and vertical roll.
- Rectangular tracking/dropout bands with separate temporal seeds and sparse speckles.
- Chroma-aware ghosting, scanlines, analog glow and head switching.
- Frame blending with original image.
- Processing is resolution-aware around a 720x480 reference scale; unlike the first C++ source, it does not force every input to 720x480.

## Temporal and reliability design worth carrying over
- Separate deterministic random streams for normal frame noise, slow tape mechanics and dropout events.
- Slow artifacts are generated over short temporal epochs and interpolated, reducing flicker while keeping movement.
- CPU frame batches can be rendered in parallel with stable frame ordering and cancellation handling.
- GPU path has bounded chunks, OOM retry, and fallback to CPU.
- Input alpha is preserved; only RGB channels are processed.
- The GPU path uses PyTorch CUDA, not Vulkan. It is not a reason to add a mandatory GPU dependency to VHS-90.

## Preset inventory
Four upstream-derived presets plus fifteen project-specific presets (19 total):
- Four base: `yobelkcip`, `Master Nama`, `RGM`, `JaneLon`.
- Fifteen custom: `Sun Bleached`, `First Gen`, `Home Movie 1987`, `Rental Copy`, `Film Transfer`, `Late Night Broadcast`, `Broadcast Dirty`, `Weather Channel`, `Security Cam`, `Kinescope`, `AE Punch`, `Chrome Dream`, `Bootleg Anime`, `Damaged Tape`, `Tracking Hell`.
These names are source inventory only; VHS-90 UI names must be original and must not expose source plugin/repository names.

## Test execution
The project tests were run from a correctly named temporary package directory so Python package-relative imports resolve.
Result: **204 passed, 89 skipped**. The skipped tests include CUDA-dependent paths because this environment has CPU-only PyTorch. The pass result covers CPU processing, preset bounds/catalog, fidelity checks, hardening, frame determinism, temporal behavior and CPU/GPU parity tests that do not require a CUDA device.

## VHS-90 implementation decisions
- Port the CPU algorithms to native C++ modules; do not embed Python, OpenCV, ComfyUI, PyTorch or the browser UI in the AE plugin.
- Preserve the scale-aware math, directional smear, epoch-interpolated tracking, separate seeded noise streams, and alpha behavior.
- Rework head-switching geometry so its region can be positioned and expanded to cover the full frame, as already required for VHS-90.
- Rework tracking noise into freely positioned/oriented regions rather than a single hard-coded moving band.
- Keep CPU as the complete baseline. Consider optional hardware acceleration only after profiling the native C++ CPU implementation and without making a powerful GPU mandatory.

## Transfer status
This audit identifies the native C++ port targets. The Python source has not yet been transferred or converted into VHS-90 C++ in this commit.
