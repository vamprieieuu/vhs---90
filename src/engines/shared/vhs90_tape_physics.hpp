#pragma once
#include "vhs90_region_effects.hpp"
#include <cstddef>
#include <cstdint>

namespace vhs90 {
struct MagneticTransition {
    float positionSamples=0.0f;
    float polarity=1.0f;
    float strength=1.0f;
};
struct HeadReadbackParams {
    float tapeSpeedMetersPerSecond=5.8f;
    float headGapMicrons=0.30f;
    float headToTapeSpacingMicrons=0.05f;
    float trackWidthMicrons=58.0f;
    float azimuthMismatchDegrees=0.5f;
    float headWear=0.0f;
    float headDirt=0.0f;
};
struct TapeTransportParams {
    float framesPerSecond=29.97f;
    float fieldRateHz=59.94f;
    float slowDriftPixelsPerSecond=0.0f;
    float drumFlutterPixels=0.4f;
    float flutterHz=7.0f;
    float flutterPixels=0.15f;
    float stickSlipPixels=0.25f;
    float lineJitterPixels=0.25f;
    float trackingOffsetPixels=0.0f;
    float trackingSlopePixels=0.0f;
    uint64_t seed=1;
    int64_t frameIndex=0;
};

float headReadbackGain(float frequencyHz,const HeadReadbackParams& params) noexcept;
void renderLorentzianHeadReadback(const MagneticTransition* transitions,std::size_t transitionCount,
                                  float sampleRateHz,float sampleStart,int sampleCount,
                                  const HeadReadbackParams& params,float* output) noexcept;
void applyTapeTransportWarp(FloatRgbaView image,const TapeTransportParams& params);
}
