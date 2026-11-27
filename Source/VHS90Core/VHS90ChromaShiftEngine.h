#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>
namespace vhs90 {
struct ChromaChannelShiftParams {
    float shiftPixels=0.0f;
    bool premultipliedAlpha=true;
};
// Separate Cr/Cb channel displacement with luma anchored to the original pixel.
bool processChromaChannelShift(ConstImageView src,ImageView dst,const ChromaChannelShiftParams& params) noexcept;

enum class RgbRegistrationMode : std::uint8_t { RGBSplit=0, RedCyan=1, GreenMagenta=2, BlueYellow=3 };
struct RgbChannelRegistrationParams {
    float redOffsetPixels=0.0f;
    float greenOffsetPixels=0.0f;
    float blueOffsetPixels=0.0f;
    RgbRegistrationMode mode=RgbRegistrationMode::RGBSplit;
    bool premultipliedAlpha=true;
};
// Independent RGB offsets, with selectable RGB split or single-channel/complementary color modes.
bool processRgbChannelRegistration(ConstImageView src,ImageView dst,const RgbChannelRegistrationParams& params) noexcept;
} // namespace vhs90
