#pragma once
#include "VHS90SignalKernels.h"
#include "rf2/ntsc_decoder.h"
#include "rf2/ntsc_effects.h"
#include "rf2/ntsc_encoder.h"
namespace vhs90 {
struct NtscWaveformParams {
    rf2::NtscSignalConfig signal{};
    rf2::NtscEffectControls effects{};
    rf2::NtscDecodeControls decoder{};
    std::uint32_t frame=0;
    float strength=1.0f;
    bool premultipliedAlpha=true;
};
// Full 525-line / 910-sample composite encode -> optional RF effects -> receiver decode.
// The upstream implementation is vendored under ThirdParty/AnalogNTSC (BSD-2-Clause).
bool processNtscWaveformRoundTrip(ConstImageView src,ImageView dst,const NtscWaveformParams& params) noexcept;
} // namespace vhs90
