#pragma once
#include "VHS90IIRFilter.h"
#include <cstdint>

namespace vhs90 {

enum class CompositePhase : std::uint8_t {
    Degrees0 = 0,
    Quadrature90 = 1,
    Degrees180 = 2
};

enum class CompositeDemodulator : std::uint8_t {
    Box = 0,
    OneLineComb = 1,
    TwoLineComb = 2
};

struct CompositeParams {
    float strength = 1.0f;
    CompositePhase phase = CompositePhase::Quadrature90;
    int phaseOffset = 0;
    bool palMode = false;
    bool palSwitch = true;
    CompositeDemodulator demodulator = CompositeDemodulator::Box;
    std::int64_t frame = 0;
    bool inputBandwidthEnabled = false;
    bool outputBandwidthEnabled = true;
    bool verticalChromaBlendEnabled = false;
    float verticalChromaBlendAmount = 0.5f;
    bool lumaSmearEnabled = false;
    LumaSmearParams lumaSmear{};
    bool premultipliedAlpha = true;
    YiqBandwidthParams inputBandwidth{};
    YiqBandwidthParams outputBandwidth{};
};

// NTSC-style four-phase chroma encode/decode round trip, based on ntsc-rs.
// Supports box, one-line comb and two-line comb demodulation.
bool processCompositeRoundTrip(ConstImageView src, ImageView dst,
                               const CompositeParams& params) noexcept;

} // namespace vhs90
