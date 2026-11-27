#include "VHS90DirectWriteText.h"
#include <cassert>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <fstream>
int main() {
    using namespace vhs90;
    using namespace vhs90::win;
    assert(isDirectWriteAvailable());
    const auto families = enumerateSystemFontFamilies();
    assert(!families.empty());
#if defined(VHS90_TEST_BUNDLED_FONTS)
    assert(std::find(families.begin(), families.end(), L"RooyinFree") != families.end());
    assert(std::find(families.begin(), families.end(), L"YR Sponge Arabic") != families.end());
#endif
    constexpr int w=640, h=240;
    std::vector<float> pixels(static_cast<std::size_t>(w)*h*4,0.0f);
    for (std::size_t i=3;i<pixels.size();i+=4) pixels[i]=1.0f;
    DirectWriteTextOptions o;
    o.utf8Text=u8"VHS-90 اختبار عربي 123";
    o.utf8FontFamily="Segoe UI"; o.x=12; o.y=24; o.fontSize=42;
    const auto r=drawDirectWriteText({pixels.data(),w,h,w*4},o);
    assert(r.rendered);
    // Exercise representative Arabic shaping, combining marks, bidi punctuation,
    // mixed Arabic/Latin labels, and both Arabic digit families in the Windows
    // DirectWrite backend. These are smoke checks; AE-host visual parity remains
    // a separate release gate.
    const char* rtlCases[] = {
        u8"السَّلَامُ عَلَيْكُمْ ١٢٣",
        u8"VHS-90 | تسجيل REC 1994",
        u8"PAUSE - إيقاف، REW ← ۱۲۳",
        u8"تاريخ: 21.1.1994 | TIME 23:59"
    };
    for (const char* sample : rtlCases) {
        o.utf8Text = sample;
        o.direction = TextDirection::Auto;
        assert(drawDirectWriteText({pixels.data(),w,h,w*4},o).rendered);
    }
    o.utf8Text = u8"السَّلَامُ عَلَيْكُمْ ١٢٣";
    o.direction = TextDirection::RightToLeft;
    assert(drawDirectWriteText({pixels.data(),w,h,w*4},o).rendered);
#if defined(VHS90_TEST_BUNDLED_FONTS)
    // Both OFL-cleared font families are embedded in this test executable
    // and loaded privately by the current process (no machine-wide install).
    o.utf8Text = u8"تسجيل REC ١٢٣";
    o.utf8FontFamily = "RooyinFree";
    const auto rooyin = drawDirectWriteText({pixels.data(),w,h,w*4},o);
    assert(rooyin.rendered && rooyin.requestedFontFound && !rooyin.usedFallbackFont);
    o.utf8FontFamily = "YR Sponge Arabic";
    const auto sponge = drawDirectWriteText({pixels.data(),w,h,w*4},o);
    assert(sponge.rendered && sponge.requestedFontFound && !sponge.usedFallbackFont);
#endif
    o.utf8FontFamily = "Segoe UI";
    o.utf8Text = u8"VHS-90 اختبار عربي 123";
    o.direction = TextDirection::Auto;
    o.fontWeight=700;
    assert(drawDirectWriteText({pixels.data(),w,h,w*4},o).rendered);
    o.fontWeight=900;
    assert(drawDirectWriteText({pixels.data(),w,h,w*4},o).rendered);
    bool changed=false;
    for(std::size_t i=0;i<pixels.size();i+=4)
        if(pixels[i]>0.05f||pixels[i+1]>0.05f||pixels[i+2]>0.05f){changed=true;break;}
    assert(changed);
    for(std::size_t i=3;i<pixels.size();i+=4) assert(std::fabs(pixels[i]-1.0f)<1.0e-5f);
    // A bounded text region must not modify pixels outside its requested rectangle.
    std::vector<float> bounded(static_cast<std::size_t>(w)*h*4,0.0f);
    for (std::size_t i=3;i<bounded.size();i+=4) bounded[i]=1.0f;
    DirectWriteTextOptions small;
    small.utf8Text=u8"REC";
    small.utf8FontFamily="Segoe UI"; small.x=420; small.y=150;
    small.fontSize=28; small.maxWidth=180; small.maxHeight=70;
    const auto boundedResult=drawDirectWriteText({bounded.data(),w,h,w*4},small);
    assert(boundedResult.rendered);
    bool insideChanged=false;
    for(int yy=0;yy<h;++yy) for(int xx=0;xx<w;++xx) {
        const std::size_t p=(static_cast<std::size_t>(yy)*w+xx)*4;
        const bool changedPixel=bounded[p]>0.001f||bounded[p+1]>0.001f||bounded[p+2]>0.001f;
        if(xx>=420&&xx<600&&yy>=150&&yy<220) insideChanged=insideChanged||changedPixel;
        else assert(!changedPixel);
    }
    assert(insideChanged);

    // Save a readable multi-row OSD visual proof, not the overlapping
    // accumulation buffer used by the individual shaping smoke assertions.
    {
        std::vector<float> visual(static_cast<std::size_t>(w)*h*4,0.0f);
        for(std::size_t i=3;i<visual.size();i+=4)visual[i]=1.0f;
        DirectWriteTextOptions line;
        line.x=20;line.maxWidth=580;line.maxHeight=42;line.fontSize=26;line.fontWeight=600;line.direction=TextDirection::Auto;
        line.utf8FontFamily="Segoe UI";line.utf8Text=u8"VHS-90 | REC  PLAY  STOP  REW  FF  SP";
        line.y=10;assert(drawDirectWriteText({visual.data(),w,h,w*4},line).rendered);
        line.utf8Text=u8"السَّلَامُ عَلَيْكُمْ ١٢٣  |  VHS-90";
        line.direction=TextDirection::RightToLeft;line.y=62;
        assert(drawDirectWriteText({visual.data(),w,h,w*4},line).rendered);
#if defined(VHS90_TEST_BUNDLED_FONTS)
        line.utf8Text=u8"تسجيل REC ١٢٣  |  التاريخ 21.1.1994";
        line.utf8FontFamily="RooyinFree";line.y=114;
        const auto rooyinVisual=drawDirectWriteText({visual.data(),w,h,w*4},line);
        assert(rooyinVisual.rendered&&rooyinVisual.requestedFontFound&&!rooyinVisual.usedFallbackFont);
        line.utf8FontFamily="YR Sponge Arabic";line.y=166;
        const auto spongeVisual=drawDirectWriteText({visual.data(),w,h,w*4},line);
        assert(spongeVisual.rendered&&spongeVisual.requestedFontFound&&!spongeVisual.usedFallbackFont);
#else
        line.utf8Text=u8"تسجيل REC ١٢٣  |  التاريخ 21.1.1994";
        line.utf8FontFamily="Segoe UI";line.y=114;
        assert(drawDirectWriteText({visual.data(),w,h,w*4},line).rendered);
        line.utf8Text=u8"PAUSE  إيقاف  REW  ←  ۱۲۳";line.y=166;
        assert(drawDirectWriteText({visual.data(),w,h,w*4},line).rendered);
#endif
        // Write the BMP headers field-by-field to avoid compiler-specific padding.
        const std::uint32_t imageBytes=static_cast<std::uint32_t>(w*h*4);
        const std::uint16_t type=0x4D42u,zero16=0;
        const std::uint32_t fileSize=54u+imageBytes,offset=54u,infoSize=40u;
        const std::uint16_t planes=1u,bits=32u;
        const std::uint32_t compression=0u;
        const std::int32_t ppm=2835;
        const std::uint32_t zero32=0;
        std::ofstream bmp("vhs90-osd-arabic-smoke.bmp",std::ios::binary);
        assert(bmp.good());
        bmp.write(reinterpret_cast<const char*>(&type),2);
        bmp.write(reinterpret_cast<const char*>(&fileSize),4);
        bmp.write(reinterpret_cast<const char*>(&zero16),2);
        bmp.write(reinterpret_cast<const char*>(&zero16),2);
        bmp.write(reinterpret_cast<const char*>(&offset),4);
        bmp.write(reinterpret_cast<const char*>(&infoSize),4);
        bmp.write(reinterpret_cast<const char*>(&w),4);
        bmp.write(reinterpret_cast<const char*>(&h),4);
        bmp.write(reinterpret_cast<const char*>(&planes),2);
        bmp.write(reinterpret_cast<const char*>(&bits),2);
        bmp.write(reinterpret_cast<const char*>(&compression),4);
        bmp.write(reinterpret_cast<const char*>(&imageBytes),4);
        bmp.write(reinterpret_cast<const char*>(&ppm),4);
        bmp.write(reinterpret_cast<const char*>(&ppm),4);
        bmp.write(reinterpret_cast<const char*>(&zero32),4);
        bmp.write(reinterpret_cast<const char*>(&zero32),4);
        for(int yy=h-1;yy>=0;--yy) for(int xx=0;xx<w;++xx) {
            const std::size_t p=(static_cast<std::size_t>(yy)*w+xx)*4;
            const auto byte=[](float v)->std::uint8_t {
                return static_cast<std::uint8_t>(std::lround(std::max(0.0f,std::min(1.0f,v))*255.0f));
            };
            const std::uint8_t bgra[4]={byte(visual[p+2]),byte(visual[p+1]),byte(visual[p]),255u};
            bmp.write(reinterpret_cast<const char*>(bgra),4);
        }
        bmp.flush();
        assert(bmp.good());
    }
    return 0;
}
