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
    float chromaIGain=1.0f;
    float chromaQGain=1.0f;
    float chromaIOffset=0.0f;
    float chromaQOffset=0.0f;
    float chromaQuantization=0.0f;
    float vignette=0.0f;
    float flicker=0.0f;
    std::uint32_t seed=1;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
// Combines the existing image-grade path with a separate VHS playback I/Q sharpening pass
// modeled on the three-stage chroma low-pass/unsharp behavior in Composite Video Simulator.
bool processImageGrade(ConstImageView src,ImageView dst,const ImageGradeParams& params) noexcept;
} // namespace vhs90
