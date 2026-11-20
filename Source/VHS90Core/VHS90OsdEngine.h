#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>
namespace vhs90 {
struct VhsOsdParams {
    float amount=1.0f, opacity=0.82f, scale=1.0f, glitch=0.5f, ghost=0.35f;
    float recAmount=1.0f, playAmount=1.0f, timestampAmount=1.0f;
    float recBlinkHz=1.4f, timestampSpeed=1.0f;
    float recColorR=1.0f, recColorG=46.0f/255.0f, recColorB=46.0f/255.0f;
    double timeSeconds=0.0;
    float framesPerSecond=29.97f;
    std::int64_t frame=0;
    std::uint32_t seed=0x9050D5u;
    int startYear=1994, startMonth=6, startDay=11;
    int startHour=12, startMinute=0, startSecond=0;
    bool showRec=true, showPlay=true, showTimestamp=true;
    bool premultipliedAlpha=true;
};
bool applyVhsOsd(ImageView image,const VhsOsdParams& params) noexcept;
} // namespace vhs90
