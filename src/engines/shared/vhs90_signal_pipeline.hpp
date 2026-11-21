#pragma once
#include "vhs90_region_effects.hpp"
#include <cstdint>

namespace vhs90 {
struct AnalogSignalParams {
    bool pal = false;
    bool palSwitch = true;
    float compositeStrength = 0.0f;
    float dotCrawlGain = 0.0f;
    float crossColorGain = 0.0f;
    float compositeNoise = 0.0f;
    float lumaNoise = 0.0f;
    float chromaNoise = 0.0f;
    float noiseTintHueDegrees = 0.0f;
    float bandwidthLines = 240.0f;
    float chromaBandwidth = 90.0f;
    float bandwidthRows = 240.0f;
    float lineJitterPixels = 0.0f;
    float topSkewPixels = 0.0f;
    float topSkewDecay = 120.0f;
    float chromaOffsetR = 0.0f;
    float chromaOffsetB = 0.0f;
    float verticalChromaDelayPixels = 1.0f;
    float scanlineAmount = 0.0f;
    float scanlineFrequency = 1.0f;
    float scanlinePhase = 0.0f;
    uint64_t seed = 1;
    int64_t frameIndex = 0;
};

// Rec.709 gamma-encoded RGBA float input/output; alpha is preserved exactly.
// Deterministic for a given frameIndex and seed.
void applyAnalogSignalPipeline(FloatRgbaView image, const AnalogSignalParams& params);
}
