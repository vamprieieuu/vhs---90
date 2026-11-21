#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct ChromaChannelShiftParams {
    float shiftPixels=0.0f;
    bool premultipliedAlpha=true;
};
// Separate Cr/Cb channel displacement with luma anchored to the original pixel.
bool processChromaChannelShift(ConstImageView src,ImageView dst,const ChromaChannelShiftParams& params) noexcept;

struct RgbChannelRegistrationParams {
    float redOffsetPixels=0.0f;
    float greenOffsetPixels=0.0f;
    float blueOffsetPixels=0.0f;
    bool premultipliedAlpha=true;
};
// Independent horizontal red, green and blue registration offsets.
bool processRgbChannelRegistration(ConstImageView src,ImageView dst,const RgbChannelRegistrationParams& params) noexcept;
} // namespace vhs90
