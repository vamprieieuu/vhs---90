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
    return 0;
}
