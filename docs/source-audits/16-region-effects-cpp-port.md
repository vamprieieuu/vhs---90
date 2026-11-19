# VHS-90 C++ Port 01 — Deterministic Region Effects

**Files:** src/engines/shared/vhs90_region_effects.hpp/.cpp, tests/region_effects_smoke.cpp  
**Language:** C++17, standard library only  
**Build verification:** PASS with GCC 12 in the working environment.

## Source algorithm mapping
- Tracking band envelope, row displacement, chroma/luma noise and moving tracking behavior are adapted from the inspected VHS Effect Add-on CPU processor and the NTSC-RS tracking implementation.
- Head-switching line ramp, phase-seeded noise and chroma fringe are adapted from the NTSC-RS and ntscQT head-switching models, with the VHS Effect Add-on tear/noise structure.
- Dropout segments use deterministic event generation inspired by the VHS Effect Add-on Poisson line-dropout model and the native VHS kernel's seeded dropout blocks.
- Randomness is implemented with a stateless 64-bit avalanche hash and smooth interpolated noise. It has no mutable global state and is safe to evaluate out of frame order.

## Required user behavior implemented
- Tracking Noise is a freely positionable oriented region, not fixed to the bottom. It has normalized center, width, height, band angle, displacement angle, feather, strength, noise, chroma amount, tear amount, speed and seed offset.
- Head Switching has a normalized vertical anchor and independent height. With anchorY=1 and heightFraction=1, the affected region covers the complete image; the default remains a narrow band expanding upward from the bottom.
- Dropouts are independent deterministic horizontal events.
- RGB channels are processed while alpha is left unchanged.

## API and integration notes
The current API accepts straight RGBA float pixels, width, height and row stride in floats. The caller owns the memory. Each effect snapshots the source into a local scratch vector before remapping, avoiding in-place read/write feedback. The AE adapter must unpremultiply before processing and restore the original alpha/premultiplication afterward; do not use a global scratch buffer.

## Test results
Command used:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic \
  src/engines/shared/vhs90_region_effects.cpp \
  tests/region_effects_smoke.cpp -o region_effects_smoke
./region_effects_smoke
```

Result: PASS. Same frame/seed produced byte-identical float buffers; alpha remained unchanged; head switching with heightFraction=1.0 changed rows across the full 240-row image; deterministic dropouts executed.

## Known limitations before AE integration
- This is a focused shared-engine module, not the full VHS pipeline and not an AE plug-in yet.
- Region tracking currently uses a per-pixel bilinear remap and allocates a scratch vector per call; later optimize with caller-provided per-render scratch storage.
- Head-switching uses a bottom-edge anchor model. For arbitrary placement, change anchorY and heightFraction; horizontal x-region controls can be added when needed.
- No color-space conversion is performed in this module; it operates on linear/working RGB values supplied by the caller. The full engine must place it at the appropriate signal stage.
- OSD ordering is not implemented here. VHS-90's OSD must be composited before this and every other VHS damage stage.
