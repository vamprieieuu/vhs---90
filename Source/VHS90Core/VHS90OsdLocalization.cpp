#include "VHS90OsdLocalization.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <limits>
namespace vhs90 { namespace {
constexpr std::array<const char*,12> kEnglishShort={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
constexpr std::array<const char*,12> kEnglishFull={"JANUARY","FEBRUARY","MARCH","APRIL","MAY","JUNE","JULY","AUGUST","SEPTEMBER","OCTOBER","NOVEMBER","DECEMBER"};
constexpr std::array<const char*,12> kArabicMonths={"يناير","فبراير","مارس","أبريل","مايو","يونيو","يوليو","أغسطس","سبتمبر","أكتوبر","نوفمبر","ديسمبر"};
bool leap(int y) noexcept { return (y%4==0 && y%100!=0) || y%400==0; }
int monthDays(int y,int m) noexcept {
    static constexpr int d[]={31,28,31,30,31,30,31,31,30,31,30,31};
    return m==2 ? d[1]+(leap(y)?1:0) : d[m-1];
}
std::int64_t daysFromCivil(int y,unsigned m,unsigned d) noexcept {
    y-=m<=2; const int era=(y>=0?y:y-399)/400;
    const unsigned yoe=static_cast<unsigned>(y-era*400), mp=m>2?m-3:m+9;
    const unsigned doy=(153*mp+2)/5+d-1, doe=yoe*365+yoe/4-yoe/100+doy;
    return std::int64_t(era)*146097+doe-719468;
}
void civilFromDays(std::int64_t z,int& y,unsigned& m,unsigned& d) noexcept {
    z+=719468; const std::int64_t era=(z>=0?z:z-146096)/146097;
    const unsigned doe=static_cast<unsigned>(z-era*146097);
    const unsigned yoe=(doe-doe/1460+doe/36524-doe/146096)/365;
    y=static_cast<int>(yoe)+static_cast<int>(era*400);
    const unsigned doy=doe-(365*yoe+yoe/4-yoe/100), mp=(5*doy+2)/153;
    d=doy-(153*mp+2)/5+1; m=mp<10?mp+3:mp-9; y+=(m<=2);
}
void appendCodepoint(std::string& out,unsigned cp) {
    if(cp<=0x7f) out.push_back(static_cast<char>(cp));
    else if(cp<=0x7ff){out.push_back(static_cast<char>(0xc0|(cp>>6)));out.push_back(static_cast<char>(0x80|(cp&0x3f)));}
    else {out.push_back(static_cast<char>(0xe0|(cp>>12)));out.push_back(static_cast<char>(0x80|((cp>>6)&0x3f)));out.push_back(static_cast<char>(0x80|(cp&0x3f)));}
}
std::string datePart(int y,int m,int d,VhsOsdDateFormat fmt,VhsOsdLanguage language,VhsOsdDigitSet digits,VhsOsdMonthStyle style) {
    char b[64]{};
    if(fmt==VhsOsdDateFormat::MonthDayYear) {
        if(language==VhsOsdLanguage::English) {
            const char* name=style==VhsOsdMonthStyle::Full?kEnglishFull[m-1]:kEnglishShort[m-1];
            std::snprintf(b,sizeof(b),"%s.%02d.%04d",name,d,y); return b;
        }
        if(language==VhsOsdLanguage::Arabic) {
            std::snprintf(b,sizeof(b),"%s.%d.%d",kArabicMonths[m-1],d,y);
            return localizeVhsOsdDigits(b,digits);
        }
        const char* name=style==VhsOsdMonthStyle::Full?kEnglishFull[m-1]:kEnglishShort[m-1];
        std::snprintf(b,sizeof(b),"%s.%02d.%04d",name,d,y);
        std::string ar=std::string(kArabicMonths[m-1])+"."+std::to_string(d)+"."+std::to_string(y);
        return std::string(b)+" | "+localizeVhsOsdDigits(ar,digits);
    }
    if(fmt==VhsOsdDateFormat::NumericMonthFirst) std::snprintf(b,sizeof(b),"%d.%d.%d",m,d,y);
    else if(fmt==VhsOsdDateFormat::NumericDayFirst) std::snprintf(b,sizeof(b),"%d.%d.%d",d,m,y);
    else std::snprintf(b,sizeof(b),"%d.%d.%d",y,m,d);
    if(language==VhsOsdLanguage::English) return b;
    const std::string localized=localizeVhsOsdDigits(b,digits);
    return language==VhsOsdLanguage::Arabic?localized:(std::string(b)+" | "+localized);
}
std::string timePart(int h,int m,int s,VhsOsdTimeMode mode,VhsOsdLanguage language,VhsOsdDigitSet digits) {
    if(mode==VhsOsdTimeMode::Off) return {};
    char b[48]{};
    if(mode==VhsOsdTimeMode::TwelveHour) {
        const int hour=(h%12)==0?12:h%12;
        if(language==VhsOsdLanguage::Arabic) {
            std::snprintf(b,sizeof(b),"%02d:%02d:%02d %s",hour,m,s,h<12?"ص":"م");
            return localizeVhsOsdDigits(b,digits);
        }
        std::snprintf(b,sizeof(b),"%02d:%02d:%02d %s",hour,m,s,h<12?"AM":"PM");
        if(language==VhsOsdLanguage::Bilingual) {
            char ar[48]{}; std::snprintf(ar,sizeof(ar),"%02d:%02d:%02d %s",hour,m,s,h<12?"ص":"م");
            return std::string(b)+" | "+localizeVhsOsdDigits(ar,digits);
        }
        return b;
    }
    std::snprintf(b,sizeof(b),"%02d:%02d:%02d",h,m,s);
    if(language==VhsOsdLanguage::English) return b;
    const std::string localized=localizeVhsOsdDigits(b,digits);
    return language==VhsOsdLanguage::Arabic?localized:(std::string(b)+" | "+localized);
}
} // namespace
bool isValidVhsOsdDateTime(const VhsOsdDateTime& v) noexcept {
    return v.year>=1900&&v.year<=2100&&v.month>=1&&v.month<=12&&v.day>=1&&
        v.day<=monthDays(v.year,v.month)&&v.hour>=0&&v.hour<=23&&
        v.minute>=0&&v.minute<=59&&v.second>=0&&v.second<=59&&std::isfinite(v.elapsedSeconds);
}
std::string localizeVhsOsdDigits(const std::string& ascii,VhsOsdDigitSet digits) {
    if(digits==VhsOsdDigitSet::Latin) return ascii;
    const unsigned base=digits==VhsOsdDigitSet::Persian?0x06f0u:0x0660u;
    std::string out; out.reserve(ascii.size()*2);
    for(unsigned char c:ascii) { if(c>='0'&&c<='9') appendCodepoint(out,base+unsigned(c-'0')); else out.push_back(static_cast<char>(c)); }
    return out;
}
std::string vhsOsdLabel(VhsOsdLabel label,VhsOsdLanguage language) {
    struct Entry { VhsOsdLabel id; const char* en; const char* ar; };
    static constexpr Entry entries[]={
        {VhsOsdLabel::Play,"PLAY ▶","تشغيل ▶"},{VhsOsdLabel::Pause,"PAUSE","إيقاف مؤقت"},
        {VhsOsdLabel::Stop,"STOP","توقف"},{VhsOsdLabel::Record,"REC","تسجيل"},
        {VhsOsdLabel::FastForward,"FF →","تقديم سريع →"},{VhsOsdLabel::Rewind,"REW ↶","إرجاع ↶"},
        {VhsOsdLabel::Forward,"FORWARD →","تقديم →"},{VhsOsdLabel::Tape,"TAPE","شريط"},
        {VhsOsdLabel::Error,"ERROR","خطأ"},{VhsOsdLabel::Program01,"P.O.1","برنامج ٠١"},
        {VhsOsdLabel::Resolution,"RES","دقة"},{VhsOsdLabel::Eject,"EJECT","إخراج"},
        {VhsOsdLabel::Search,"SEARCH","بحث"},{VhsOsdLabel::Tracking,"TRACKING","تتبّع"},
        {VhsOsdLabel::Auto,"AUTO","تلقائي"},{VhsOsdLabel::Channel,"CH","قناة"},
        {VhsOsdLabel::Camera,"CAM","كاميرا"},{VhsOsdLabel::Deck,"DECK","مسجل"},
        {VhsOsdLabel::Timer,"TIMER","مؤقت"},{VhsOsdLabel::Stereo,"STEREO","ستيريو"},
        {VhsOsdLabel::Slow,"SLOW","بطيء"},{VhsOsdLabel::Still,"STILL","ثابت"},
        {VhsOsdLabel::Memory,"MEMORY","ذاكرة"},{VhsOsdLabel::TapeSpeedSP,"SP","SP"},
        {VhsOsdLabel::TapeSpeedLP,"LP","LP"},{VhsOsdLabel::TapeSpeedEP,"EP","EP"}};
    for(const auto& e:entries) if(e.id==label) {
        if(language==VhsOsdLanguage::English) return e.en;
        if(language==VhsOsdLanguage::Arabic) return e.ar;
        return std::string(e.en)+" | "+e.ar;
    }
    return {};
}
std::string formatVhsOsdDateTime(const VhsOsdDateTime& v,VhsOsdLanguage language,VhsOsdDigitSet digits) {
    if(!isValidVhsOsdDateTime(v)) return {};
    if(static_cast<unsigned>(language)>static_cast<unsigned>(VhsOsdLanguage::Bilingual)||
       static_cast<unsigned>(digits)>static_cast<unsigned>(VhsOsdDigitSet::Persian)||
       static_cast<unsigned>(v.dateFormat)<static_cast<unsigned>(VhsOsdDateFormat::MonthDayYear)||
       static_cast<unsigned>(v.dateFormat)>static_cast<unsigned>(VhsOsdDateFormat::YearMonth)||
       static_cast<unsigned>(v.timeMode)>static_cast<unsigned>(VhsOsdTimeMode::TwentyFourHour)||
       static_cast<unsigned>(v.monthStyle)>static_cast<unsigned>(VhsOsdMonthStyle::Full)) return {};
    std::int64_t dayOffset=0; int hour=v.hour,minute=v.minute,second=v.second;
    if(v.clockRuns) {
        const double whole=std::floor(v.elapsedSeconds);
        // Keep calendar arithmetic inside the supported 1900-2100 range before converting to int64.
        if(whole>6400000000.0||whole< -6400000000.0) return {};
        const std::int64_t total=std::int64_t(hour)*3600+minute*60+second+static_cast<std::int64_t>(whole);
        dayOffset=total/86400; std::int64_t rem=total%86400;
        if(rem<0){rem+=86400;--dayOffset;}
        hour=static_cast<int>(rem/3600);minute=static_cast<int>((rem/60)%60);second=static_cast<int>(rem%60);
    }
    int year=v.year; unsigned month=static_cast<unsigned>(v.month),day=static_cast<unsigned>(v.day);
    civilFromDays(daysFromCivil(year,month,day)+dayOffset,year,month,day);
    if(year<1900||year>2100) return {};
    std::string date=datePart(year,static_cast<int>(month),static_cast<int>(day),v.dateFormat,language,digits,v.monthStyle);
    const std::string time=timePart(hour,minute,second,v.timeMode,language,digits);
    if(!time.empty()) date+="  "+time;
    return date;
}
} // namespace vhs90
