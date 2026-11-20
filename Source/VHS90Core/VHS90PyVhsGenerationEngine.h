#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>

namespace vhs90 {

// Adapted from vhs.py's independent Y/Cr/Cb horizontal generation pipeline.
// Compression factors are >= 1; all noise and line displacement are frame deterministic.
struct PyVhsGenerationParams {
    float lumaCompression = 1.0f;
    float chromaCompression = 1.0f;
    float lumaNoiseSigma = 0.0f;       // 8-bit code values
    float lumaNoiseMean = 0.0f;        // 8-bit code values
    float chromaNoise = 0.0f;          // 8-bit code values
    float chromaWave = 0.0f;           // pixels, 0..2
    float waveFrequency = 0.22f;       // cycles per image height
    float generation = 1.0f;
    std::uint32_t seed = 0x56534850u;
    std::int64_t frame = 0;
    bool premultipliedAlpha = true;
};

bool processPyVhsGeneration(ConstImageView src, ImageView dst,
                            const PyVhsGenerationParams& params) noexcept;

} // namespace vhs90
