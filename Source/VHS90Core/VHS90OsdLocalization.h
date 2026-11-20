#pragma once
#include <cstdint>
#include <string>
namespace vhs90 {
enum class VhsOsdLanguage : std::uint8_t { English=0, Arabic=1, Bilingual=2 };
enum class VhsOsdDigitSet : std::uint8_t { Latin=0, ArabicIndic=1, Persian=2 };
enum class VhsOsdDateFormat : std::uint8_t { MonthDayYear=1, NumericMonthFirst=2, NumericDayFirst=3, YearMonth=4 };
enum class VhsOsdTimeMode : std::uint8_t { Off=0, TwelveHour=1, TwentyFourHour=2 };
enum class VhsOsdMonthStyle : std::uint8_t { Short=0, Full=1 };
enum class VhsOsdLabel : std::uint8_t {
    Play, Pause, Stop, Record, FastForward, Rewind, Forward, Tape, Error,
    Program01, Resolution, Eject, Search, Tracking, Auto, Channel, Camera,
    Deck, Timer, Stereo, Slow, Still, Memory, TapeSpeedSP, TapeSpeedLP, TapeSpeedEP
};
struct VhsOsdDateTime {
    int year=1994, month=6, day=11;
    int hour=12, minute=0, second=0;
    double elapsedSeconds=0.0;
    bool clockRuns=true;
    VhsOsdDateFormat dateFormat=VhsOsdDateFormat::MonthDayYear;
    VhsOsdTimeMode timeMode=VhsOsdTimeMode::TwentyFourHour;
    VhsOsdMonthStyle monthStyle=VhsOsdMonthStyle::Short;
};
bool isValidVhsOsdDateTime(const VhsOsdDateTime& value) noexcept;
std::string localizeVhsOsdDigits(const std::string& ascii, VhsOsdDigitSet digits);
std::string vhsOsdLabel(VhsOsdLabel label, VhsOsdLanguage language);
std::string formatVhsOsdCounter(std::int64_t frame, float framesPerSecond, double offsetSeconds,\n                                  int counterFormat, VhsOsdDigitSet digits);\nstd::string formatVhsOsdTime(const VhsOsdDateTime& value,
                              VhsOsdLanguage language,
                              VhsOsdDigitSet arabicDigits);
std::string formatVhsOsdDateTime(const VhsOsdDateTime& value,
                                 VhsOsdLanguage language,
                                 VhsOsdDigitSet arabicDigits);
} // namespace vhs90
