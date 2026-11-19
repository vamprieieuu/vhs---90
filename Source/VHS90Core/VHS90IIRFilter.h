#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>

namespace vhs90 {

enum class IIRFilterDesign : std::uint8_t {
    ConstantKTriple = 0,
    Butterworth2 = 1
};

enum class IIRInitialCondition : std::uint8_t {
    Zero = 0,
    FirstSample = 1
};

struct YiqBandwidthParams {
    float sampleRateHz = 14318181.8f;
    float iCutoffHz = 1300000.0f;
    float qCutoffHz = 600000.0f;
    float lumaCutoffHz = 0.0f;
    int iDelaySamples = 2;
    int qDelaySamples = 4;
    int lumaDelaySamples = 0;
    int verticalLumaRadiusRows = 0;
    float verticalLumaSigmaRows = 1.0f;
    IIRFilterDesign design = IIRFilterDesign::ConstantKTriple;
    IIRInitialCondition initial = IIRInitialCondition::Zero;
    bool premultipliedAlpha = true;
};

// Applies independent I/Q low-pass filters in the YIQ domain, based on the
// ntsc-rs constant-K / Butterworth transfer-function implementation.
// Each scanline is filtered independently; optional vertical luma blur is separate. Alpha is preserved.
bool processYiqBandwidth(ConstImageView src, ImageView dst,
                         const YiqBandwidthParams& params) noexcept;

} // namespace vhs90
