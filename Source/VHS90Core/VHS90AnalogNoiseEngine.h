#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct AnalogNoiseParams {
    float lumaAmount=0.0f;
    // Unified Signal Noise controls; Grain remains independently adjustable.
    float signalNoiseIntensity=1.0f;
    float signalNoiseFrequency=1.0f;
    float noiseSize=1.0f;
    float noiseDensity=1.0f;
    float noiseDistribution=0.0f;
    float horizontalLineIntensity=0.0f;
    float animationSpeed=1.0f;
    // VHS6RIFT procedural-noise controls (normalized UI values).
    float grainAmount=0.0f;
    float noiseDetail=0.5f;
    float shadowBias=0.65f;
    int grainSize=2;
    bool proceduralNoiseEnabled=true;
    float chromaAmount=0.0f;
    float hueNoiseAmount=0.0f;
    float compositeAmount=0.0f;
    float chromaLoss=0.0f;
    float chromaPhaseNoise=0.0f;
    float hueBiasDegrees=0.0f;
    float tapeLineAmount=0.0f;
    float tapeLineSpeed=1.0f;
    float framesPerSecond=29.97f;
    int lumaCellWidth=2,lumaCellHeight=2;
    int chromaCellWidth=4,chromaCellHeight=4;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
bool processAnalogNoise(ConstImageView src,ImageView dst,const AnalogNoiseParams& params) noexcept;
} // namespace vhs90