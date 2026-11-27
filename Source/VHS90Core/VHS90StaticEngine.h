#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>

namespace vhs90 {

enum class StaticPatternMode : std::uint8_t {
    DarkFieldSpeckles = 0,
    FullFrameHorizontal = 1,
    TopStrip = 2,
    BottomWeightedStreaks = 3,
    MonochromeStreaks = 4,
    SplitBandChromatic = 5,
    StaticToBlackout = 6,
    DarkFieldLumaStreaks = 7,
    MultiPhaseGreenLine = 8,
    TwoStageAngularRibbon = 9,
    DiagonalSyncCollapse = 10,
    ColoredSpeckles = 11,
    VerticalColumns = 12
};

struct StaticParams {
    StaticPatternMode mode = StaticPatternMode::FullFrameHorizontal;
    float intensity = 1.0f;
    float sourceMix = 0.0f; // 0 = generated signal, 1 = source footage plus static.
    float density = 0.35f;
    float cellSizePx = 2.0f;
    float streakLengthPx = 32.0f;
    float lumaAmount = 0.65f;
    float chromaAmount = 0.18f;
    float baseLuma = 0.015f;
    float topBandHeight = 0.18f;
    float splitPosition = 0.5f;
    float bottomExponent = 2.0f;
    float flicker = 0.06f;
    float animationSpeed = 1.0f;
    float blackoutProbability = 0.08f;
    float angleDegrees = 24.0f;
    float ribbonWidth = 0.12f;
    float greenLineAmount = 0.75f;
    int phaseCount = 3;
    float diagonalShiftPx = 12.0f;
    int blackoutDurationFrames = 4;
    bool animated = true;
    bool premultipliedAlpha = true;
    std::uint32_t seed = 1;
    std::int64_t frame = 0;
};

// Procedural luma/chroma static patterns. No image/video overlays are used.
// Source and destination must be distinct RGBA float buffers.
bool processStaticPattern(ConstImageView src, ImageView dst,
                          const StaticParams& params) noexcept;

} // namespace vhs90
