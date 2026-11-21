#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct TapeDetailParams {
    float detail=1.0f; // 1 = full horizontal luma resolution, 0.06 = minimum tape detail.
    int minimumWidth=48;
    bool premultipliedAlpha=true;
};
// BarashNTSC's horizontal luma downsample/upsample pass. Chroma and alpha remain intact.
bool processTapeDetail(ConstImageView src,ImageView dst,const TapeDetailParams& params) noexcept;
} // namespace vhs90
