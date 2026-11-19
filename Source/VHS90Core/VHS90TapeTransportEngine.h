#pragma once
#include "VHS90SignalKernels.h"
#include "VHS90TapeSurfaceEngine.h"
#include <cstdint>
namespace vhs90 {
struct TapeTransportParams {
    float framesPerSecond=29.97f;
    float fieldRateHz=59.94f;
    float slowDriftPixelsPerSecond=0.0f;
    float drumFlutterPixels=0.4f;
    float flutterHz=7.0f;
    float flutterPixels=0.15f;
    float stickSlipPixels=0.25f;
    float lineJitterPixels=0.25f;
    float trackingOffsetPixels=0.0f;
    float trackingSlopePixels=0.0f;
    float servoJitterPixels=0.0f;
    float controlTrackServoAmount=0.0f;
    float controlTrackPulseStrength=1.0f;
    float controlTrackPulseDropout=0.0f;
    float controlTrackFalsePulseChance=0.0f;
    float pictureSearchSpeed=1.0f;
    float trackingTargetPixels=0.0f;
    float verticalScale=1.0f;
    bool usePhysicalControlTrackMap=false;
    TapeSurfaceParams controlTrackTape{};
    std::uint32_t seed=1;
    std::int64_t frame=0;
};
bool processTapeTransportWarp(ConstImageView src,ImageView dst,
                              const TapeTransportParams& params) noexcept;
} // namespace vhs90
