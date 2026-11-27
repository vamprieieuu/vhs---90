#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>

namespace vhs90 {
struct NtscQtParams {
    // Source NtscQT controls: ringing alpha 0..1, seeded frequency-mask noise,
    // and Y-only pre-emphasis amount 0..8 at a 1 MHz default cutoff.
    bool spectralRingingEnabled = false;
    float amount = 1.0f;
    float alpha = 0.65f;
    float frequencyNoiseSize = 0.0f;
    float frequencyNoiseAmplitude = 2.0f;
    float preEmphasis = 0.0f;
    float preEmphasisCutoffMHz = 1.0f;
    std::uint32_t seed = 1;
    std::int64_t frame = 0;
    // 0 = both fields, 1 = even rows, 2 = odd rows.
    int fieldMode = 0;
    bool premultipliedAlpha = true;
};

// Horizontal-frequency spectral mask (NTSC-QT ringing variant A) and the
// source's separate pre-modulation luma pre-emphasis. RingPattern/variant B
// is intentionally not substituted with the spatial VHS ringing engine.
bool processNtscQtSignal(ConstImageView src, ImageView dst,
                         const NtscQtParams& params) noexcept;
} // namespace vhs90
