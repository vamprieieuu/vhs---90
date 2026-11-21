#pragma once
#include "VHS90SignalKernels.h"
#include <cstddef>
#include <cstdint>

namespace vhs90 {

enum class RegionKind : std::uint8_t {
    TrackingNoise = 0,
    HeadSwitch = 1
};

enum class RegionShape : std::uint8_t {
    HorizontalBand = 0,
    VerticalBand = 1,
    Rectangle = 2
};

// Distinct signal-generation models. These are real processing variants, not color-only presets.
enum class TrackingProfile : std::uint8_t {
    ServoSweep = 0,
    RfWave = 1,
    CompositeRibbon = 2,
    TapeFlutter = 3,
    FourEdgeCrawl = 4
};

// All position and size values are normalized to [0,1] frame coordinates.
// HorizontalBand uses width for horizontal span and height for thickness.
// VerticalBand uses width for thickness and height for vertical span.
// Rectangle is a freely positioned region using both dimensions.
struct DamageRegion {
    bool enabled = true;
    RegionKind kind = RegionKind::TrackingNoise;
    RegionShape shape = RegionShape::HorizontalBand;
    TrackingProfile trackingProfile = TrackingProfile::ServoSweep;

    float centerX = 0.5f;
    float centerY = 0.5f;
    float angleRadians = 0.0f;
    float width = 1.0f;
    float height = 0.08f;
    float feather = 0.01f;

    float intensity = 0.5f;
    float maxShiftPx = 18.0f;
    float verticalShiftPx = 0.0f;
    float waveAmplitudePx = 2.0f;
    float waveFrequency = 3.0f;
    float servoDriftPx = 4.0f;
    float servoSmoothnessPx = 24.0f;
    int servoEpochFrames = 8;
    float tearProbability = 0.025f;
    float tearShiftPx = 28.0f;

    float noiseAmount = 0.12f;
    float noiseDensity = 0.15f;
    float dropoutProbability = 0.02f;
    float dropoutAmount = 0.25f;
    float chromaShiftPx = 1.5f;
    float cellWidthPx = 8.0f;
    float cellHeightPx = 2.0f;

    float motionXPerSecond = 0.0f;
    float motionYPerSecond = 0.0f;
    std::uint32_t seedOffset = 0;
};

struct RegionEngineParams {
    const DamageRegion* regions = nullptr;
    std::size_t regionCount = 0;
    std::uint32_t seed = 1;
    std::int64_t frame = 0;
    double timeSeconds = 0.0;
    float globalIntensity = 1.0f;
    bool premultipliedAlpha = true;
};

// Applies any number of independent Tracking Noise / Head Switching regions.
// Regions can be horizontal, vertical, or rotated rectangles and may overlap.
// Source and destination must be distinct buffers. Input alpha is preserved.
bool processDamageRegions(ConstImageView src, ImageView dst,
                          const RegionEngineParams& params) noexcept;

} // namespace vhs90
