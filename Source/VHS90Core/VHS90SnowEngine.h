#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>

namespace vhs90 {
struct SnowParams {
    bool enabled = false;
    bool colored = false;
    float intensity = 0.70f;
    float density = 0.48f;
    float sizePx = 1.6f;
    float distribution = 0.45f;
    float contrast = 0.82f;
    float brightness = 0.68f;
    float color = 0.35f;
    float temporalVariation = 0.80f;
    float randomness = 0.90f;
    float animationSpeed = 1.0f;
    float backgroundNoiseIntensity = 0.0f;
    std::uint32_t seed = 1;
    std::int64_t frame = 0;
    double timeSeconds = 0.0;
    bool premultipliedAlpha = true;
};

// Independent procedural snow/static and background signal-noise layers.
// Snow enable/intensity never controls the separate background-noise amount.
bool processSnow(ConstImageView src, ImageView dst, const SnowParams& params) noexcept;
} // namespace vhs90
