#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct ThreeBandEqParams {
    float amount=0.0f;
    bool premultipliedAlpha=true;
};
// NTSC-CRT's three-band Y/I/Q equalizer: four cascaded low/high poles,
// low/mid/high gain recombination, and a three-sample high-band delay.
bool processThreeBandEq(ConstImageView src,ImageView dst,const ThreeBandEqParams& params) noexcept;
} // namespace vhs90
