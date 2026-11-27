#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>

namespace vhs90 {
enum class SyncDamageAnimationMode : std::uint8_t { Automatic = 0, ManualKeyframed = 1, Frozen = 2 };
enum class SyncDamageInputMode : std::uint8_t { OverOriginal = 0, GeneratedSignal = 1 };

struct SyncCollapseLabParams {
    bool enabled = true, lumaEnabled = true, dropoutsEnabled = true, chromaEnabled = true;
    SyncDamageAnimationMode animationMode = SyncDamageAnimationMode::Automatic;
    SyncDamageInputMode inputMode = SyncDamageInputMode::OverOriginal;
    float effectAmount = 1.0f;
    float horizontalTearAmount = 0.85f, bandIntensity = 0.85f, bandDensity = 7.0f;
    float bandHeight = 0.55f, bandIrregularity = 0.82f, horizontalDisplacementPx = 22.0f;
    float boundaryFragmentation = 0.62f, fullFrameCoverage = 1.0f;
    float lumaInstability = 0.90f, brightFlashIntensity = 0.85f, darkCollapseIntensity = 0.85f;
    float eventFrequencyHz = 1.7f, eventDurationSeconds = 0.30f, transitionSharpness = 0.90f;
    float brightEventProbability = 0.50f, darkEventProbability = 0.50f;
    float localBandLumaVariation = 0.70f, manualBrightPulse = 0.0f, manualDarkCollapse = 0.0f;
    float dropoutStreakDensity = 0.65f, streakLengthPx = 18.0f, streakBrightness = 0.75f;
    float darkDropoutAmount = 0.70f, horizontalFragmentation = 0.65f, temporalVariation = 0.85f;
    float subtleChromaTint = 0.20f, magentaTint = 0.20f, greenTint = 0.20f;
    float rgbOffsetPx = 0.35f, chromaBleed = 0.18f;
    float animationSpeed = 1.0f, blendWithInput = 0.80f;
    std::uint32_t seed = 1;
    std::int64_t frame = 0;
    double timeSeconds = 0.0;
    bool premultipliedAlpha = true;
};

// Procedural horizontal sync-collapse bands, frame-level luma events,
// fine horizontal dropouts and restrained chromatic tint. Deterministic,
// allocation-free per frame, alpha-preserving and MFR-safe.
bool processSyncCollapseLab(ConstImageView src, ImageView dst,
                            const SyncCollapseLabParams& params) noexcept;
} // namespace vhs90
