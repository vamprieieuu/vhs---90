#pragma once
#include "VHS90SignalKernels.h"
#include "VHS90TapeSurfaceEngine.h"
#include <cstdint>
namespace vhs90 {
struct HeadConditionParams {
    float wear=0.0f;
    float dirt=0.0f;
    float buildup=0.0f;
    float mismatch=0.0f;
    float grease=0.0f;
    float amount=0.0f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
    // Enables the NTSCRT 12-position tape_at() sampling path for each field.
    bool usePhysicalTapeSamples=false;
    TapeSurfaceParams tapeSurface{};
};
// Deterministic alternating-head model. Physical tape sampling is optional so
// existing callers retain their established defaults.
bool processHeadCondition(ConstImageView src,ImageView dst,const HeadConditionParams& params) noexcept;
} // namespace vhs90
