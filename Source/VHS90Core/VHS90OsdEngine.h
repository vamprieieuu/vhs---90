#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>
namespace vhs90 {
enum class VhsDeckMode : std::uint8_t { Play=1, Pause=2, Stop=3, FastForward=4, Rewind=5 };
enum class VhsTimestampFormat : std::uint8_t { Iso=1, MonthFirst=2, DayFirst=3, DateOnly=4, TimeOnly=5 };
enum class VhsTapeCounterFormat : std::uint8_t { Frames6=1, HoursMinutesSeconds=2, HoursMinutesSecondsFrames=3 };
struct VhsOsdParams {
    float amount=1.0f, opacity=0.82f, scale=1.0f, glitch=0.5f, ghost=0.35f;
    float recAmount=1.0f, playAmount=1.0f, timestampAmount=1.0f, batteryAmount=0.0f;
    float recBlinkHz=1.4f, timestampSpeed=1.0f, tapeCounterAmount=1.0f;
    float tapeCounterX=2.2f, tapeCounterY=96.0f;
    float recColorR=1.0f, recColorG=46.0f/255.0f, recColorB=46.0f/255.0f;
    double timeSeconds=0.0;
    float framesPerSecond=29.97f;
    std::int64_t frame=0;
    std::uint32_t seed=0x9050D5u;
    VhsDeckMode deckMode=VhsDeckMode::Play;
    VhsTimestampFormat timestampFormat=VhsTimestampFormat::Iso;
    VhsTapeCounterFormat tapeCounterFormat=VhsTapeCounterFormat::Frames6;
    int tapeSpeed=1; // SP, LP, EP
    float recX=2.2f, recY=3.0f, statusX=98.0f, statusY=3.0f, timestampX=98.0f, timestampY=96.0f;
    int startYear=1994, startMonth=6, startDay=11;
    int startHour=12, startMinute=0, startSecond=0;
    bool showRec=true, showPlay=true, showTimestamp=true, showBattery=false, showTapeCounter=false;
    bool premultipliedAlpha=true;\n    bool drawRecText=true, drawDeckText=true, drawTapeSpeedText=true, drawTimestampText=true;
};
bool applyVhsOsd(ImageView image,const VhsOsdParams& params) noexcept;
} // namespace vhs90
