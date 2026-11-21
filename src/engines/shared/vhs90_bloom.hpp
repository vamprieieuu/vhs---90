#pragma once
#include "vhs90_region_effects.hpp"
namespace vhs90 {
struct BloomParams {
    bool enabled=false;
    float threshold=0.72f;
    float intensity=0.25f;
    float spread=1.0f;
    float veil=0.0f;
    int levels=4;
    bool fast=false;
};
void applyBloom(FloatRgbaView image,const BloomParams& params);
}
