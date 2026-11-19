#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>
namespace vhs90 {
struct TapeSurfaceParams {
    float defectDensity=0.0f, oxideLoss=0.0f, scratchDensity=0.0f;
    float creaseStrength=0.0f, debrisAmount=0.0f, edgeLift=0.0f;
    float surfaceRoughness=0.0f, tapeAdvancePerFrame=0.012f;
    float moldAmount=0.0f, vaselineAmount=0.0f, pinholeAmount=0.0f, flakeAmount=0.0f;
    float alongCellSize=0.018f, acrossCellSize=0.035f;
    std::uint32_t seed=1;
    std::uint32_t tapeSpeed=0; // 0 SP, 1 LP, 2 EP
    float trackingOffsetTracks=0.0f;
    int rasterActiveLines=480;
    int rasterVbiLines=40;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
struct TapeSurfaceSample {
    float magnetization=1.0f, spacingLift=0.0f, roughness=0.0f, creaseLoss=0.0f, film=0.0f;
    float shed=0.0f;
};
// NTSCRT physical tape coordinates: x is millimetres along the tape and y
// is millimetres across its 12.65 mm width. Deterministic across render order.
float ntscrtTapeWearAmount(float knob) noexcept;
float ntscrtTapeDropoutAmount(float knob) noexcept;
float ntscrtTapeScratchAmount(float knob) noexcept;
float ntscrtTapeEdgeAmount(float knob) noexcept;
float ntscrtTapeCreaseAmount(float knob) noexcept;
std::uint32_t ntscrtTapeSeed(std::uint32_t tapeSeed) noexcept;
TapeSurfaceSample sampleTapeSurfaceAtMm(double xMm, float yMm, const TapeSurfaceParams& params, bool detail=true, bool flakes=true) noexcept;
TapeSurfaceSample sampleTapeSurface(float u,float v,const TapeSurfaceParams& params) noexcept;
bool processTapeSurface(ConstImageView src,ImageView dst,const TapeSurfaceParams& params) noexcept;
} // namespace vhs90
