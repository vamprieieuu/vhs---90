#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct BloomParams {
    float threshold=0.72f;
    float intensity=0.25f;
    float spread=1.0f;
    float veil=0.0f;
    int levels=4;
    bool fast=false;
    bool premultipliedAlpha=true;
};
bool processBloom(ConstImageView src,ImageView dst,const BloomParams& params) noexcept;
} // namespace vhs90
