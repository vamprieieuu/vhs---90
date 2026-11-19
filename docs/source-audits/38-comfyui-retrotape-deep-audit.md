# ComfyUI RetroTape — complete repository audit and VHS-90 port record

Source: `SKBv0/ComfyUI_RetroTape`
Upstream branch: `main`
Recursive tree: 29 entries, 25 files, 4 directories (complete response).
License: Apache-2.0 (Copyright 2026 SKBv0). The project credits ntscQT and composite-video-simulator; this audit distinguishes original ComfyUI implementation from inherited algorithms/presets. VHS-90 ports are C++ adaptations; Python, PyTorch, CUDA, ComfyUI and browser UI code are not runtime dependencies.

## File-by-file inventory (25/25)

| File | Review / disposition |
|---|---|
| `.comfyignore`, `.gitignore` | Packaging/cache exclusions; not runtime code |
| `LICENSE` | Apache-2.0 terms and copyright reviewed; retained in `ThirdParty/ComfyUIRetroTape/LICENSE` |
| `README.md` | Node features, 19 presets, controls, CPU/GPU behavior, preview and attribution reviewed |
| `__init__.py` | ComfyUI extension registration and preview route hook; host-specific |
| `device.py` | CUDA selection, poisoned-context detection, GPU fallback and shared GPU lock; host/device wrapper, not portable to CPU-first AE |
| `nodes.py` | Node schema, batch dispatch, progress and preview-frame cache handoff; ComfyUI/PyTorch adapter |
| `ntsc_core.py` | Full 1,738-line CPU pipeline, settings parser, presets, YIQ kernels, temporal profiles, rendering and batch execution reviewed |
| `ntsc_gpu.py` | CUDA/PyTorch counterpart reviewed; tensor kernels and bounded chunk/fallback behavior are GPU-specific, not copied |
| `preview_cache.py` | Full LRU cache, byte budget, read-only frame storage, batch downsampling and frame-index preservation reviewed; UI infrastructure |
| `server.py` | Full 794-line preview source loader, path allow-list, image/video decode limits, request sequencing, semaphore, preview renderer, HTTP routes and cache clearing reviewed; not portable to AE |
| `pyproject.toml`, `requirements.txt` | Python package/dependency metadata; not imported |
| `docs/banner.webp`, `docs/demo.webp`, `docs/presets.jpg` | Visual documentation assets inspected as non-executable reference material; not runtime overlays |
| `tests/conftest.py` | Test import/bootstrap and fake ComfyUI path helpers |
| `tests/test_core.py` | Determinism, presets, controls, alpha, dimensions, malformed input, preview cache and source-loader behavior reviewed |
| `tests/test_device.py` | CPU/GPU selection, CUDA fallback/poisoning, device consistency and route behavior reviewed |
| `tests/test_fidelity.py` | Preview/render identity, frame offsets, frame transport, resolution scaling, 16-bit source conversion and interlace line pitch reviewed |
| `tests/test_gpu.py` | CPU/GPU deterministic-stage parity, noise statistics, frame splitting, chunk bounds, memory and error propagation reviewed; GPU-only |
| `tests/test_hardening.py` | hostile settings, path traversal, cache limits, allocation bounds, temporal epochs, smear direction and slider coverage reviewed |
| `tests/test_performance.py` | threaded determinism/order, cancellation, progress, cascaded line filters, dropout statistics and grain amplitude reviewed |
| `tests/test_presets.py` | preset catalog/backend/panel consistency, slider coverage, grouping and visual separation reviewed |
| `web/js/retrotape.js` | Full 1,622-line ComfyUI panel: dynamic settings/preset UI, graph-origin discovery, fast/full preview requests, cancellation, sequence control, A/B, clipboard, local preferences, lifecycle cleanup and cache actions reviewed; not portable to AE |

## Algorithm inventory and VHS-90 destinations

| Upstream behavior | VHS-90 destination / status |
|---|---|
| RGB↔YIQ, chroma phase and composite carrier | `VHS90CompositeEngine`, `VHS90WaveformEngine` |
| Horizontal/vertical chroma bleed and chroma delay | `VHS90ChromaShiftEngine`, chroma subsampling/vertical blend and IIR bandwidth |
| Frame-driven and row-dependent chroma colour drift | `VHS90ReceiverInstabilityEngine::chromaPhaseDrift` + `chromaColorDrift`; AE controls `Color Phase Drift` and `Chroma Color Drift` |
| Luma/chroma/composite noise | `VHS90AnalogNoiseEngine` |
| Causal exponential luma smear and highlight-only trail | `processLumaSmear` in `VHS90SignalKernels`, extended to 2–64 taps with independent highlight trail |
| Servo tracking profile with epoch crossfade | `VHS90TapeTransportEngine` slow one-pole servo profile plus `VHS90RegionEngine` row-correlated tracking profiles |
| Tracking warp / time-base jitter | `VHS90AnalogGeometryEngine`, `VHS90TapeTransportEngine`, region tracking |
| Interlace field weave and vertical roll | `VHS90AnalogGeometryEngine` |
| Dropout bands and speckles | `VHS90DropoutEngine`, plus `VHS90TapeSurfaceEngine` and `VHS90ImpulseNoiseEngine` for other defect families |
| Previous-frame ghosting | `VHS90TemporalEngine`; AE SmartFX now checks out the previous source frame with a separate checkout ID (source-level integration; live AE/MFR validation pending) |
| Scanlines / composite carrier modulation | `VHS90CompositeEngine::processScanlines` and composite pipeline |
| Analog glow / highlight bloom | `VHS90BloomEngine` plus `VHS90AnalogGlowEngine` for thresholded channel-weighted halo |
| Head switching | shared `RegionEngine` head-switch region |
| VHS luma/chroma bandwidth and vertical chroma blending | tape-speed IIR profiles, chroma subsampling and vertical chroma blend |
| Seven-tap luma ringing and asymmetric edge echoes | New `VHS90RingingEngine`, integrated before composite modulation; AE controls `Ringing` and `Ringing Power` |
| Two-stage luma ghost and I/Q chroma delay | New `VHS90SignalGhostEngine`, integrated after RF multipath; AE controls `Signal Ghost` and `Chroma Delay` |
| Sparse dropout bands and luma-only speckles | New `VHS90DropoutEngine`, deterministic per frame/seed, alpha-aware; AE control `Dropouts` |
| Thresholded channel-weighted analog glow | New `VHS90AnalogGlowEngine`, integrated post-scanline; AE control `Analog Glow` |
| Frame and row-dependent chroma rotation | `VHS90ReceiverInstabilityEngine::chromaColorDrift`; AE control `Chroma Color Drift` |
| Vertical roll | `VHS90AnalogGeometryEngine::verticalRoll`; AE control `Vertical Roll` |
| Per-scanline stochastic chroma loss | `VHS90AnalogNoiseEngine::chromaLoss`, seeded row mask with 8% chroma retention; AE control `Chroma Loss` (0–5000 source range) |
| VHS edge-wave per-row motion | `VHS90AnalogGeometryEngine` now has deterministic epoch-crossfaded per-row edge-wave profiles; AE control `VHS Edge Wave` |
| Black pedestal lift | `VHS90ImageGradeEngine::blackLift`; AE control `Black Lift`, white-level preserving pedestal curve |
| Preview server, CUDA device manager, ComfyUI widget and browser panel | Excluded as host-specific infrastructure |

## New C++ port in this pass

- `VHS90RingingEngine`, `VHS90DropoutEngine`, `VHS90SignalGhostEngine` and `VHS90AnalogGlowEngine` were added and registered in the core build with standalone smoke tests.
- `VHS90RingingEngine`: luma edge extraction, separable Gaussian baseline, seven-tap horizontal ring and asymmetric echoes; integrated before composite modulation.
- `VHS90DropoutEngine`: seeded sparse horizontal bands and luma speckles, with I/Q attenuation and alpha preservation.
- `VHS90SignalGhostEngine`: two delayed luma echoes and weighted I/Q chroma delay, separate from previous-frame temporal ghost.
- `VHS90AnalogGlowEngine`: thresholded, channel-weighted glow with alpha-safe compositing.
- `VHS90AnalogGeometryEngine`: deterministic edge-wave profile with temporal epoch crossfade, plus vertical roll; exposed as `VHS Edge Wave` and `Vertical Roll`.
- `VHS90ImageGradeEngine`: now includes black-pedestal lift that preserves white level.
- `VHS90ReceiverInstabilityEngine`: row-dependent color drift and phase rotation.
- `VHS90TapeTransportEngine`: epoch-crossfaded one-pole servo jitter profile, connected to the tracking servo control.
- All these modules are connected to the shared composite pipeline and their AE controls are wired. Existing Comfy-derived luma smear, servo epoch profile, tracking warp and fidelity-test ideas are retained as documented earlier ports.

## Verification boundary

- GitHub Actions run `44604160794` passed 24/24 CTest tests, including standalone Ringing, Dropout, Signal Ghost and Analog Glow tests.
- GitHub Actions run `44604803432` passed **25/25 CTest tests**, including standalone and composite tests for the chroma-loss mask.
- Later AE parameter wiring and the servo-only activation fix have newer runs; verify the latest run before claiming the combined adapter source state is validated.
- Static AE parameter count after the ComfyUI-derived controls: **78 real parameter slots and 78 `PF_ADD_*` calls**, with exactly one `Chroma Loss` registration. This does not substitute for a Windows/MSVC AE SDK build or live After Effects/MFR validation.
- The 25-file ComfyUI RetroTape source inventory is complete. All identified portable image/signal algorithms are now represented in VHS-90, including per-scanline chroma loss. Exact OpenCV/PyTorch statistical parity remains a separate fidelity task; the GPU path is intentionally not imported into the CPU-first AE plugin.
