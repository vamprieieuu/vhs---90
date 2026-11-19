#pragma once
#include <cstdint>
#include <cstddef>

namespace vhs90 {

struct RGB { float r, g, b; };
struct YIQ { float y, i, q; };

struct ConstImageView {
    const float* rgba = nullptr;
    int width = 0;
    int height = 0;
    std::ptrdiff_t rowStrideFloats = 0;
};

struct ImageView {
    float* rgba = nullptr;
    int width = 0;
    int height = 0;
    std::ptrdiff_t rowStrideFloats = 0;
};

struct SignalParams {
    float chromaShiftPx = 2.5f;
    float chromaBleedPx = 3.0f;
    float chromaSaturation = 0.8f;
    float lineWobblePx = 1.2f;
    float trackingAmount = 0.35f;
    float trackingSpeed = 0.6f;
    float trackingHeight = 0.08f;
    float trackingCenter = 0.5f;
    float headSwitchHeight = 0.03f;
    float headSwitchCenterY = 0.985f;
    float noiseAmount = 0.12f;
    float staticLineAmount = 0.25f;
    float flicker = 0.04f;
    float intensity = 1.0f;
    std::uint32_t seed = 1;
    std::int64_t frame = 0;
};

YIQ rgbToYiq(RGB c) noexcept;
RGB yiqToRgb(YIQ c) noexcept;
std::uint32_t hash32(std::uint32_t x) noexcept;
float hash01(std::uint32_t x, std::uint32_t y, std::uint32_t seed, std::int64_t frame) noexcept;

// CPU reference kernels. Source and destination must be distinct buffers.
// These routines preserve the input alpha channel.
bool processCompositeVhs(ConstImageView src, ImageView dst, const SignalParams& p) noexcept;
bool processYiqChromaBlur(ConstImageView src, ImageView dst, float chromaShiftPx,
                          float chromaBleedPx, float saturation) noexcept;

} // namespace vhs90
