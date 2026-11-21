#pragma once
#include "vhs90_region_effects.hpp"
#include <cstdint>

namespace vhs90 {
struct ChromaBleedParams {
    bool enabled=false;
    int horizontalSubsample=2;
    int verticalSubsample=1;
    float amount=0.5f;
    float blurRadius=2.0f;
    float cbOffsetPixels=0.0f;
    float crOffsetPixels=0.0f;
};
struct LumaSmearParams {
    bool enabled=false;
    int radiusPixels=12;
    float amount=0.35f;
    float highlightThreshold=0.62f;
    float highlightTrail=0.25f;
    int direction=1;
};
struct RowWaveParams {
    bool enabled=false;
    float amplitudePixels=2.0f;
    float cycles=3.0f;
    float phase=0.0f;
    float speed=0.0f;
    float randomPerRow=0.0f;
    uint64_t seed=1;
    int64_t frameIndex=0;
};
void applyChromaBleed(FloatRgbaView image,const ChromaBleedParams& params);
void applyLumaSmear(FloatRgbaView image,const LumaSmearParams& params);
void applyRowWave(FloatRgbaView image,const RowWaveParams& params);
}
