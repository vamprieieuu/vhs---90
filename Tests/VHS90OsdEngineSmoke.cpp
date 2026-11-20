#include "VHS90OsdEngine.h"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>
int main() {
    using namespace vhs90;
    constexpr int w=640,h=360;
    std::vector<float> source(static_cast<std::size_t>(w)*h*4,0.0f);
    for(std::size_t i=3;i<source.size();i+=4) source[i]=1.0f;
    auto first=source,repeat=source,second=source;
    VhsOsdParams p; p.timeSeconds=0.0; p.frame=0; p.glitch=0.0f;
    ImageView a{first.data(),w,h,w*4}, b{repeat.data(),w,h,w*4};
    assert(applyVhsOsd(a,p)); assert(applyVhsOsd(b,p));
    assert(first==repeat); assert(first!=source);
    for(std::size_t i=3;i<first.size();i+=4) assert(std::fabs(first[i]-1.0f)<1e-6f);
    bool foundRed=false;
    for(int y=0;y<50&&!foundRed;++y) for(int x=0;x<90;++x) {
        const std::size_t i=(static_cast<std::size_t>(y)*w+x)*4;
        if(first[i]>first[i+1]+0.2f&&first[i]>first[i+2]+0.2f){foundRed=true;break;}
    }
    assert(foundRed);
    VhsOsdParams later=p; later.timeSeconds=1.0;
    ImageView c{second.data(),w,h,w*4}; assert(applyVhsOsd(c,later));
    bool timestampChanged=false;
    for(int y=h-40;y<h;++y) for(int x=w-380;x<w;++x) {
        const std::size_t i=(static_cast<std::size_t>(y)*w+x)*4;
        if(std::fabs(first[i]-second[i])+std::fabs(first[i+1]-second[i+1])+std::fabs(first[i+2]-second[i+2])>0.01f){timestampChanged=true;break;}
    }
    assert(timestampChanged);
    auto untouched=source; VhsOsdParams off; off.amount=0.0f;
    ImageView d{untouched.data(),w,h,w*4}; assert(applyVhsOsd(d,off)); assert(untouched==source);
    std::vector<float> transparent(static_cast<std::size_t>(w)*h*4,0.0f);
    VhsOsdParams recOnly; recOnly.showPlay=false; recOnly.showTimestamp=false; recOnly.glitch=0.0f; recOnly.ghost=0.0f;
    ImageView e{transparent.data(),w,h,w*4}; assert(applyVhsOsd(e,recOnly));
    bool alphaAdded=false; for(std::size_t i=3;i<transparent.size();i+=4) if(transparent[i]>0.1f){alphaAdded=true;break;}
    assert(alphaAdded);
    ImageView invalid{first.data(),w,h,w*4-1}; assert(!applyVhsOsd(invalid,p));
    return 0;
}
