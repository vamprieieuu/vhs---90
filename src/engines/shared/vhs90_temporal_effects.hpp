#pragma once
#include "vhs90_region_effects.hpp"
namespace vhs90 {
struct TemporalGhostParams {
    bool enabled=false;
    float amount=0.18f;
    float lumaAmount=1.0f;
    float chromaAmount=0.8f;
    float offsetXPixels=2.0f;
    float offsetYPixels=0.0f;
    float redFringePixels=0.5f;
    float blueFringePixels=-0.5f;
    float fieldWeave=0.0f;
    int fieldParity=0;
};
// Previous frame is supplied by the host; the module stores no temporal state.
void applyTemporalGhost(FloatRgbaView current,const FloatRgbaView& previous,const TemporalGhostParams& params);
}
