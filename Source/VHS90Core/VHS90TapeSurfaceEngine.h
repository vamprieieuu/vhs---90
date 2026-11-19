#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>
namespace vhs90 {
struct TapeSurfaceParams {
    float defectDensity=0.12f, oxideLoss=0.45f, scratchDensity=0.08f;
    float creaseStrength=0.25f, debrisAmount=0.12f, edgeLift=0.08f;
    float surfaceRoughness=0.025f, tapeAdvancePerFrame=0.012f;
    float alongCellSize=0.018f, acrossCellSize=0.035f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
struct TapeSurfaceSample { float magnetization=1.0f, spacingLift=0.0f, roughness=0.0f, creaseLoss=0.0f; };
TapeSurfaceSample sampleTapeSurface(float u,float v,const TapeSurfaceParams& params) noexcept;
bool processTapeSurface(ConstImageView src,ImageView dst,const TapeSurfaceParams& params) noexcept;
} // namespace vhs90
