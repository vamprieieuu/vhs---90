#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct ChromaLossParams {
    float amount=0.0f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
// Deterministic per-scanline chroma dropout: affected rows retain 8% of I/Q.
bool processChromaLoss(ConstImageView src,ImageView dst,const ChromaLossParams& params) noexcept;
} // namespace vhs90
