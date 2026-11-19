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
| Frame-driven chroma colour drift | `VHS90ReceiverInstabilityEngine::chromaPhaseDrift` |
| Luma/chroma/composite noise | `VHS90AnalogNoiseEngine` |
| Causal exponential luma smear and highlight-only trail | `processLumaSmear` in `VHS90SignalKernels`, extended to 2–64 taps with independent highlight trail |
| Servo tracking profile with epoch crossfade | `VHS90RegionEngine` deterministic row-correlated tracking profiles |
| Tracking warp / time-base jitter | `VHS90AnalogGeometryEngine`, `VHS90TapeTransportEngine`, region tracking |
| Interlace field weave and vertical roll | `VHS90AnalogGeometryEngine` |
| Dropout bands and speckles | `VHS90TapeSurfaceEngine`, `VHS90ImpulseNoiseEngine`, region engine |
| Previous-frame ghosting | `VHS90TemporalEngine`; AE SmartFX now checks out the previous source frame with a separate checkout ID (source-level integration; live AE/MFR validation pending) |
| Scanlines / composite carrier modulation | `VHS90CompositeEngine::processScanlines` and composite pipeline |
| Analog glow / highlight bloom | `VHS90BloomEngine` |
| Head switching | shared `RegionEngine` head-switch region |
| VHS luma/chroma bandwidth and vertical chroma blending | tape-speed IIR profiles, chroma subsampling and vertical chroma blend |
| Seven-tap luma ringing and asymmetric edge echoes | New `VHS90RingingEngine`, integrated before composite modulation; AE controls `Ringing` and `Ringing Power` |
| VHS edge-wave per-row motion | Not yet a dedicated C++ port; candidate for next migration step |
| Black pedestal lift | Not yet exposed as a separate control; candidate for next migration step |
| Preview server, CUDA device manager, ComfyUI widget and browser panel | Excluded as host-specific infrastructure |

## New C++ port in this pass

- `Source/VHS90Core/VHS90RingingEngine.h/.cpp`: extracts luma, applies a separable Gaussian edge baseline, seven-tap horizontal ringing kernel, and asymmetric forward/back edge echoes. Uses width-scaled radii, deterministic math, premultiplied-alpha handling and exact zero-strength bypass.
- `Tests/VHS90RingingEngineSmoke.cpp`: verifies identity bypass, deterministic output, changed edge response and alpha preservation.
- `VHS90CompositeEngine`: ringing is inserted before composite modulation, so it behaves as a signal-stage effect rather than a post-render overlay.
- `Source/VHS90Effect.cpp`: adds `Ringing` and `Ringing Power` controls and wires them to the shared pipeline.
- Existing Comfy-derived luma smear, servo epoch profile and fidelity-test ideas are retained as documented earlier ports.

## Verification boundary

- The latest core CI run before this ringing addition passed 20/20 CTest tests at run `44602045505`, including waveform, temporal and tape-detail tests.
- Ringing's standalone smoke test passed in its own build/test workflow; the combined CMake/composite/AE-control change is awaiting the latest CI result.
- Static parameter count must be rechecked after UI additions; it does not substitute for a Windows/MSVC AE SDK build or live After Effects/MFR validation.
- The ComfyUI RetroTape source inventory is complete. Algorithm migration is not complete until the edge-wave profile and black-lift pedestal are ported or explicitly rejected after visual comparison.
