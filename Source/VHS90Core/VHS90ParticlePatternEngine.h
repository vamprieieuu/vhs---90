#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>

namespace vhs90 {

enum class ParticlePatternMode : std::uint8_t {
    DarkFieldSpeckles = 0,
    HorizontalFragments = 1,
    ColoredSpeckles = 2,
    BottomWeightedStreaks = 3,
    VerticalColumns = 4
};

enum class ParticleDistribution : std::uint8_t {
    Uniform = 0,
    TopWeighted = 1,
    BottomWeighted = 2,
    CenterWeighted = 3,
    EdgeWeighted = 4
};

struct ParticlePatternParams {
    ParticlePatternMode mode = ParticlePatternMode::DarkFieldSpeckles;
    ParticleDistribution distribution = ParticleDistribution::Uniform;
    float intensity = 1.0f;
    float density = 0.18f;
    float particleSizePx = 1.5f;
    float sizeVariation = 0.65f;
    float streakLengthPx = 7.0f;
    float aspectRatio = 2.5f;
    float orientationDegrees = 0.0f;
    float brightness = 0.85f;
    float contrast = 1.0f;
    float chromaAmount = 0.0f;
    float flicker = 0.2f;
    float animationSpeed = 1.0f;
    float sourceMix = 1.0f; // 0 = generated dark field, 1 = original footage plus particles.
    float backgroundLuma = 0.015f;
    bool animated = true;
    bool premultipliedAlpha = true;
    std::uint32_t seed = 1;
    std::int64_t frame = 0;
};

// Deterministic procedural particle/fragment generation. Density controls event count;
// particle size and streak length control geometry independently. Alpha is preserved.
bool processParticlePattern(ConstImageView src, ImageView dst,
                            const ParticlePatternParams& params) noexcept;

} // namespace vhs90
