#pragma once
#include "VHS90SignalKernels.h"
#include <cstddef>
#include <cstdint>

namespace vhs90 {

enum class TearChromaMode : std::uint8_t {
    SpectralSmear = 0,
    LocalizedHeadbleed = 1,
    ChromaStreakInterleave = 2,
    MultiBandTear = 3,
    SpectralWithLowerTear = 4
};

struct TearBand {
    bool enabled = true;
    float centerX = 0.5f;
    float centerY = 0.80f;
    float width = 1.0f;
    float height = 0.18f;
    float intensity = 0.75f;
    float horizontalShiftPx = 18.0f;
    float waveAmplitudePx = 5.0f;
    float waveFrequency = 4.0f;
    float fragmentation = 0.55f;
    float chromaShiftPx = 5.0f;
    float smearPx = 6.0f;
    float dropoutProbability = 0.12f;
    float flicker = 0.12f;
    float motionXPerSecond = 0.0f;
    float motionYPerSecond = 0.0f;
    std::uint32_t seedOffset = 0;
};

struct TearChromaParams {
    TearChromaMode mode = TearChromaMode::SpectralWithLowerTear;
    float intensity = 1.0f;
    float spectralIntensity = 0.75f;
    float spectralCenterY = 0.38f;
    float spectralCoverage = 0.72f;
    float spectralHorizontalBias = 0.5f;
    float spectralIrregularity = 0.65f;
    float spectralHueSpread = 0.72f;
    float chromaSaturation = 1.4f;
    float spectralSmearX = 14.0f;
    float spectralSmearY = 2.0f;
    float baseMix = 1.0f;
    const TearBand* bands = nullptr;
    std::size_t bandCount = 0;
    std::uint32_t seed = 1;
    std::int64_t frame = 0;
    double timeSeconds = 0.0;
    bool animated = true;
    bool premultipliedAlpha = true;
};

// Procedural chroma smear and independently masked horizontal tear bands.
// Supports up to five caller-supplied bands; no reference media or overlays are used.
bool processTearChroma(ConstImageView src, ImageView dst,
                       const TearChromaParams& params) noexcept;

} // namespace vhs90
