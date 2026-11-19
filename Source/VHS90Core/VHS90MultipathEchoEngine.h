#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct MultipathEchoParams {
    float gain=0.0f;
    float delayPixels=24.0f;
    float ensemble=0.0f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
// Image-domain port of Analog-NTSC's delayed RF multipath/echo taps.
bool processMultipathEcho(ConstImageView src,ImageView dst,const MultipathEchoParams& params) noexcept;
} // namespace vhs90
