#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct AnalogGlowParams {
    float amount=0.0f;
    float horizontalScale=1.0f;
    bool premultipliedAlpha=true;
};
// Thresholded RGB halo with the channel-weighted glow of ComfyUI RetroTape.
bool processAnalogGlow(ConstImageView src,ImageView dst,const AnalogGlowParams& params) noexcept;
} // namespace vhs90
