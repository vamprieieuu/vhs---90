#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct RfInstabilityParams {
    float drift=0.0f;
    float hum=0.0f;
    float amNonlinearity=0.0f;
    float blackPedestal=0.0f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
// Image-domain adaptation of Analog-NTSC RF drift, mains hum and AM compression.
// Applies scanline-correlated luma instability while retaining chroma and alpha.
bool processRfInstability(ConstImageView src,ImageView dst,const RfInstabilityParams& params) noexcept;
} // namespace vhs90
