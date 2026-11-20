#pragma once
#include "VHS90SignalKernels.h"
namespace vhs90 {
struct ImageGradeParams {
    float edgeEnhancement=0.0f;
    float chromaEdgeEnhancement=0.0f; // independent VHS playback I/Q sharpening.
    float chromaSharpenCutoffHz=640000.0f; // SP 640kHz, LP 600kHz, EP 560kHz at the 4:2:2 chroma rate.
    float fade=0.0f;
    float blackLift=0.0f;
    float contrast=1.0f;
    float brightness=0.0f;
    float saturation=1.0f;
    float vignette=0.0f;
    float flicker=0.0f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
// Adapted from BarashNTSC's image-domain detail, fade, grade and display passes.
bool processImageGrade(ConstImageView src,ImageView dst,const ImageGradeParams& params) noexcept;
} // namespace vhs90
