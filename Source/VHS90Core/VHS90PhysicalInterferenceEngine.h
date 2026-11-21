#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>

namespace vhs90 {

// Independent image-domain model of environmental interference around a VHS
// deck. Physical field strengths are expressed in SI-derived units.
struct PhysicalInterferenceParams {
    float magnetMomentAm2 = 0.0f;
    float magnetDistanceCm = 6.0f;
    float headCoreRelativePermeability = 1500.0f;
    float headCoreSaturationTesla = 0.45f;
    float headCoreCoupling = 0.10f;
    float tapeCoercivityAm = 52000.0f;
    float tapeSwitchingSpread = 0.30f;

    float mainsFrequencyHz = 60.0f;
    float mainsPhaseRadians = 0.0f;
    float groundLoopMillivolts = 0.0f;
    float mainsHumAmount = 0.0f;

    float rfFieldVPerM = 0.0f;
    float rfCarrierMHz = 3.9f;
    float rfPickupAmount = 0.0f;

    float framesPerSecond = 29.97f;
    std::uint32_t seed = 1;
    std::int64_t frame = 0;
    bool premultipliedAlpha = true;
};

// Applies magnetic head/tape loss, mains-coupled hum and RF pickup. All
// components are deterministic for a given frame/seed and preserve alpha.
bool processPhysicalInterference(ConstImageView src, ImageView dst,
                                 const PhysicalInterferenceParams& params) noexcept;

// Exposed for deterministic diagnostics and tests.
float dipoleAxialFieldTesla(float momentAm2, float distanceMeters) noexcept;
float magneticHeadSignalGain(const PhysicalInterferenceParams& params) noexcept;
float magneticTapeEraseFraction(const PhysicalInterferenceParams& params) noexcept;

} // namespace vhs90
