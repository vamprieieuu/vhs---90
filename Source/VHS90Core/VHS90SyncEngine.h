#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>
namespace vhs90 {
struct SyncCollapseParams {
    float strength=1.0f, eventFrequency=0.24f, eventRateHz=1.70f;
    float eventAttackFraction=0.035f, eventReleaseStartFraction=0.10f, eventReleaseEndFraction=0.28f;
    float tearWidthMin=0.006f, tearWidthMax=0.030f;
    float secondaryTearOffsetMin=0.028f, secondaryTearOffsetMax=0.103f, secondaryTearStrength=0.58f;
    float displacementPx=36.0f, slowTimebasePx=2.5f, fastTimebasePx=1.0f, lineTextureAmount=0.38f;
    float trackingLineSpeed=0.071f, trackingLineWobble=0.13f, trackingLineStrength=0.22f;
    float syncEventRateHz=0.25f, syncEventWindowFraction=0.14f, syncEventProbability=0.18f, syncRollStrength=0.45f;
    float noiseAmount=0.12f;
    bool enableTrackingLine=true, enableVerticalRoll=true, premultipliedAlpha=true;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    double timeSeconds=0.0;
};
bool processSyncCollapse(ConstImageView src, ImageView dst,
                         const SyncCollapseParams& params) noexcept;
} // namespace vhs90
