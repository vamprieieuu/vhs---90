#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct TrackReadParams {
    float amount=0.0f;
    float trackingOffset=0.0f; // measured in neighboring track pitches
    int speed=0; // 0 SP, 1 LP, 2 EP
    bool premultipliedAlpha=true;
};
// Raster approximation of NTSCRT's three-track head read and azimuth loss.
// The center and adjacent recorded tracks contribute independently to FM luma and color-under chroma.
bool processTrackRead(ConstImageView src,ImageView dst,const TrackReadParams& params) noexcept;
} // namespace vhs90
