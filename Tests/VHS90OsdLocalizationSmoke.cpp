#include "VHS90OsdLocalization.h"
#include <cassert>
int main() {
    using namespace vhs90;
    VhsOsdDateTime v; v.year=1994;v.month=1;v.day=21;v.hour=13;v.minute=5;v.second=9;v.clockRuns=false;v.timeMode=VhsOsdTimeMode::Off;
    v.dateFormat=VhsOsdDateFormat::MonthDayYear;
    assert(formatVhsOsdDateTime(v,VhsOsdLanguage::English,VhsOsdDigitSet::Latin)=="JAN.21.1994");
    assert(formatVhsOsdDateTime(v,VhsOsdLanguage::Arabic,VhsOsdDigitSet::ArabicIndic)=="يناير.٢١.١٩٩٤");
    assert(formatVhsOsdDateTime(v,VhsOsdLanguage::Bilingual,VhsOsdDigitSet::ArabicIndic)=="JAN.21.1994 | يناير.٢١.١٩٩٤");
    v.monthStyle=VhsOsdMonthStyle::Full; assert(formatVhsOsdDateTime(v,VhsOsdLanguage::English,VhsOsdDigitSet::Latin)=="JANUARY.21.1994");
    v.monthStyle=VhsOsdMonthStyle::Short;v.dateFormat=VhsOsdDateFormat::NumericMonthFirst;
    assert(formatVhsOsdDateTime(v,VhsOsdLanguage::English,VhsOsdDigitSet::Latin)=="1.21.1994");
    v.dateFormat=VhsOsdDateFormat::NumericDayFirst; assert(formatVhsOsdDateTime(v,VhsOsdLanguage::English,VhsOsdDigitSet::Latin)=="21.1.1994");
    v.dateFormat=VhsOsdDateFormat::YearMonth; assert(formatVhsOsdDateTime(v,VhsOsdLanguage::English,VhsOsdDigitSet::Latin)=="1994.1.21");
    v.dateFormat=VhsOsdDateFormat::NumericDayFirst;v.timeMode=VhsOsdTimeMode::TwentyFourHour;
    assert(formatVhsOsdDateTime(v,VhsOsdLanguage::English,VhsOsdDigitSet::Latin)=="21.1.1994  13:05:09");
    v.timeMode=VhsOsdTimeMode::TwelveHour; assert(formatVhsOsdDateTime(v,VhsOsdLanguage::English,VhsOsdDigitSet::Latin)=="21.1.1994  01:05:09 PM");
    assert(formatVhsOsdDateTime(v,VhsOsdLanguage::Arabic,VhsOsdDigitSet::Persian)=="۲۱.۱.۱۹۹۴  ۰۱:۰۵:۰۹ م");
    v.timeMode=VhsOsdTimeMode::Off;v.year=2000;v.month=2;v.day=28;v.hour=23;v.minute=59;v.second=59;v.clockRuns=true;v.elapsedSeconds=2.0;v.dateFormat=VhsOsdDateFormat::NumericDayFirst;
    assert(formatVhsOsdDateTime(v,VhsOsdLanguage::English,VhsOsdDigitSet::Latin)=="29.2.2000");
    v.year=1900;v.month=2;v.day=29; assert(!isValidVhsOsdDateTime(v));
    assert(formatVhsOsdDateTime(v,VhsOsdLanguage::English,VhsOsdDigitSet::Latin).empty());
    assert(vhsOsdLabel(VhsOsdLabel::Play,VhsOsdLanguage::Arabic)=="تشغيل ▶");
    assert(vhsOsdLabel(VhsOsdLabel::Rewind,VhsOsdLanguage::English)=="REW ↶");
    assert(vhsOsdLabel(VhsOsdLabel::Error,VhsOsdLanguage::Bilingual)=="ERROR | خطأ");
    assert(localizeVhsOsdDigits("1994-01-21",VhsOsdDigitSet::ArabicIndic)=="١٩٩٤-٠١-٢١");
    return 0;
}