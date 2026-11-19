#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct SignalGhostParams {
    float amount=0.0f;
    float chromaDelayPixels=0.0f;
    float horizontalScale=1.0f;
    bool premultipliedAlpha=true;
};
// ComfyUI RetroTape's two delayed luma echoes plus blended I/Q chroma delay.
bool processSignalGhost(ConstImageView src,ImageView dst,const SignalGhostParams& params) noexcept;
} // namespace vhs90
