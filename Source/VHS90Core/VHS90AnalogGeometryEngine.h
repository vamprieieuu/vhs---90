#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct AnalogGeometryParams {
    float lineJitterPixels=0.0f;
    float lineJitterCorrelationRows=12.0f;
    float topSkewPixels=0.0f;
    float topSkewDecayRows=120.0f;
    float edgePincushionPixels=0.0f;
    float edgePincushionPower=2.0f;
    float interlaceAmount=0.0f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
};
struct ScanlineParams {
    float amount=0.0f;
    float frequency=1.0f;
    float phase=0.0f;
    float speed=0.0f;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
bool processAnalogGeometry(ConstImageView src,ImageView dst,
                           const AnalogGeometryParams& params) noexcept;
bool processScanlines(ConstImageView src,ImageView dst,
                      const ScanlineParams& params) noexcept;
} // namespace vhs90
