#include "VHS90DirectWriteText.h"
#include <cassert>
#include <cmath>
#include <vector>
int main() {
    using namespace vhs90;
    using namespace vhs90::win;
    assert(isDirectWriteAvailable());
    assert(!enumerateSystemFontFamilies().empty());
    constexpr int w=640, h=240;
    std::vector<float> pixels(static_cast<std::size_t>(w)*h*4,0.0f);
    for (std::size_t i=3;i<pixels.size();i+=4) pixels[i]=1.0f;
    DirectWriteTextOptions o;
    o.utf8Text=u8"VHS-90 اختبار عربي 123";
    o.utf8FontFamily="Segoe UI"; o.x=12; o.y=24; o.fontSize=42;
    const auto r=drawDirectWriteText({pixels.data(),w,h,w*4},o);
    assert(r.rendered);
    bool changed=false;
    for(std::size_t i=0;i<pixels.size();i+=4)
        if(pixels[i]>0.05f||pixels[i+1]>0.05f||pixels[i+2]>0.05f){changed=true;break;}
    assert(changed);
    for(std::size_t i=3;i<pixels.size();i+=4) assert(std::fabs(pixels[i]-1.0f)<1.0e-5f);
    return 0;
}
