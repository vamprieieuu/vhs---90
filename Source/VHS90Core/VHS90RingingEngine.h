#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct RingingParams {
    float strength=0.0f;
    float power=1.0f;
    bool premultipliedAlpha=true;
};
// ComfyUI RetroTape ringing: luma edge extraction, seven-tap horizontal ring,
// plus asymmetric forward/back edge echoes.
bool processRinging(ConstImageView src,ImageView dst,const RingingParams& params) noexcept;
} // namespace vhs90
