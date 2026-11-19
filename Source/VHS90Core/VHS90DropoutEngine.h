#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct DropoutParams {
    float amount=0.0f;
    float horizontalScale=1.0f;
    float verticalScale=1.0f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
// Sparse horizontal tape-dropout bands plus luma-only impulse speckles.
bool processDropouts(ConstImageView src,ImageView dst,const DropoutParams& params) noexcept;
} // namespace vhs90
