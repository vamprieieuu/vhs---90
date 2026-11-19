#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct ImpulseNoiseParams {
    float amount=0.0f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
bool processImpulseNoise(ConstImageView src,ImageView dst,const ImpulseNoiseParams& params) noexcept;
} // namespace vhs90
