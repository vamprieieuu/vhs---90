#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>
namespace vhs90 {
struct CrtEqualizerParams {
    float amount=0.0f;
    bool premultipliedAlpha=true;
};
// EMMIR NTSC-CRT's four-pole low/high split and three-band gain mixer,
// reworked to use render-local state (thread/MFR safe).
bool processCrtThreeBandEqualizer(ConstImageView src,ImageView dst,
                                  const CrtEqualizerParams& params) noexcept;
} // namespace vhs90
