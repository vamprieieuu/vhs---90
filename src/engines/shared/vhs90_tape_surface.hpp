#pragma once
#include "vhs90_region_effects.hpp"
#include <cstdint>

namespace vhs90 {
struct TapeSurfaceParams {
    float defectDensity = 0.12f;
    float oxideLoss = 0.45f;
    float scratchDensity = 0.08f;
    float creaseStrength = 0.25f;
    float debrisAmount = 0.12f;
    float edgeLift = 0.08f;
    float surfaceRoughness = 0.025f;
    float tapeAdvancePerFrame = 0.012f;
    float alongCellSize = 0.018f;
    float acrossCellSize = 0.035f;
    uint64_t seed = 1;
    int64_t frameIndex = 0;
};
struct TapeSurfaceSample {
    float magnetization = 1.0f;
    float spacingLift = 0.0f;
    float roughness = 0.0f;
    float creaseLoss = 0.0f;
};

// Deterministic physical defect map evaluated in persistent tape coordinates [u,v].
TapeSurfaceSample sampleTapeSurface(float u, float v, const TapeSurfaceParams& params) noexcept;
// Applies magnetization loss, head-spacing loss and surface roughness; alpha is preserved.
void applyTapeSurface(FloatRgbaView image, const TapeSurfaceParams& params);
}
