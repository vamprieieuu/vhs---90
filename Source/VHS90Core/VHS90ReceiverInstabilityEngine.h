#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct ReceiverInstabilityParams {
    float afcHunt=0.0f;
    float chromaFlutter=0.0f;
    float ycCrosstalk=0.0f;
    float horizontalSyncNoise=0.0f;
    float verticalSyncNoise=0.0f;
    float burstNoise=0.0f;
    float chromaPhaseDrift=0.0f;
    float chromaColorDrift=0.0f;
    float verticalScale=1.0f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
bool processReceiverInstability(ConstImageView src,ImageView dst,const ReceiverInstabilityParams& params) noexcept;
} // namespace vhs90
