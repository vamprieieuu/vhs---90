#pragma once

#include <cstdint>

namespace vhs90 {

struct FloatRgbaView {
    float* pixels = nullptr;
    int width = 0;
    int height = 0;
    int rowStrideFloats = 0;
};

struct OrientedRegion {
    float centerX = 0.5f;
    float centerY = 0.5f;
    float width = 1.0f;
    float height = 0.08f;
    float angleRadians = 0.0f;
    float feather = 0.35f;
};

struct TrackingNoiseParams {
    bool enabled = false;
    OrientedRegion region{};
    float displacementPixels = 12.0f;
    float displacementAngleRadians = 0.0f;
    float noiseAmount = 0.08f;
    float chromaAmount = 0.025f;
    float tearAmount = 0.35f;
    float temporalSpeed = 1.0f;
    uint32_t seedOffset = 0;
};

struct HeadSwitchParams {
    bool enabled = false;
    float anchorY = 1.0f;
    float heightFraction = 0.035f;
    float intensity = 1.0f;
    float horizontalShiftPixels = 32.0f;
    float noiseAmount = 0.12f;
    float stripeAmount = 0.22f;
    float colorFringePixels = 1.5f;
    float rampPower = 1.35f;
    uint32_t seedOffset = 0;
};

struct DropoutParams {
    bool enabled = false;
    float densityPerFrame = 5.0f;
    float minLengthFraction = 0.006f;
    float maxLengthFraction = 0.08f;
    float heightPixels = 1.0f;
    float whiteProbability = 0.65f;
    float intensity = 1.0f;
    uint32_t seedOffset = 0;
};

// Stateless and deterministic: same seed + frame index = identical output,
// regardless of frame render order or worker-thread scheduling.
void applyTrackingNoise(FloatRgbaView image, const TrackingNoiseParams& params,
                        uint64_t globalSeed, int64_t frameIndex);
void applyHeadSwitching(FloatRgbaView image, const HeadSwitchParams& params,
                        uint64_t globalSeed, int64_t frameIndex);
void applyDropouts(FloatRgbaView image, const DropoutParams& params,
                   uint64_t globalSeed, int64_t frameIndex);

} // namespace vhs90
