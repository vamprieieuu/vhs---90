#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>
namespace vhs90 {
struct TemporalGhostParams {
    float amount=0.18f;
    float lumaAmount=1.0f;
    float chromaAmount=0.8f;
    float offsetXPixels=2.0f;
    float offsetYPixels=0.0f;
    float redFringePixels=0.5f;
    float blueFringePixels=-0.5f;
    float fieldWeave=0.0f;
    float frameRepeatFrequency=0.0f;
    float frameRepeatStrength=0.0f;
    int fieldParity=0;
    std::int64_t frame=0;
    std::uint32_t seed=0x9050D5u;
    bool premultipliedAlpha=true;
};
bool processTemporalGhost(ConstImageView current,ConstImageView previous,ImageView dst,
                          const TemporalGhostParams& params) noexcept;
} // namespace vhs90
