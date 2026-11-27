#include "VHS90NtscQtEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main() {
    using namespace vhs90;
    for (int w : {7,8,13,16}) {
        constexpr int h=9;
        std::vector<float> src(static_cast<std::size_t>(w*h*4)),a(src.size()),b(src.size()),z(src.size());
        for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
            const std::size_t k=(static_cast<std::size_t>(y)*w+x)*4;
            const float v=((x+y)&1)?0.92f:0.08f;
            src[k]=v;src[k+1]=0.2f+0.03f*float(y);src[k+2]=1.0f-v;src[k+3]=1.0f;
        }
        ConstImageView si{src.data(),w,h,w*4};
        ImageView da{a.data(),w,h,w*4},db{b.data(),w,h,w*4},dz{z.data(),w,h,w*4};
        NtscQtParams p;p.spectralRingingEnabled=true;p.alpha=0.45f;
        p.frequencyNoiseSize=0.7f;p.frequencyNoiseAmplitude=1.1f;p.seed=1234;p.frame=27;
        assert(processNtscQtSignal(si,da,p));assert(processNtscQtSignal(si,db,p));
        assert(a==b);assert(a!=src);
        p.spectralRingingEnabled=false;p.frequencyNoiseSize=0;p.preEmphasis=3.0f;
        assert(processNtscQtSignal(si,dz,p));assert(z!=src);
        p.preEmphasis=0.0f;assert(processNtscQtSignal(si,dz,p));assert(z==src);
        p.fieldMode=3;assert(!processNtscQtSignal(si,dz,p));
    }
    // NTSC-QT's separate 720-sample RingPattern frequency response.
    p.spectralRingingEnabled=true;p.useRingPattern=true;p.ringingPower=4;p.ringingShift=0.0f;
    assert(processNtscQtSignal(si,da,p));assert(processNtscQtSignal(si,db,p));assert(a==b);
    p.ringingPower=6;assert(processNtscQtSignal(si,db,p));assert(a!=b);

    // Alpha and transparent pixels survive processing.
    constexpr int w=8,h=8;std::vector<float> src(w*h*4),dst(w*h*4);
    for(std::size_t k=0;k<src.size();k+=4){src[k]=0.7f;src[k+1]=0.2f;src[k+2]=0.4f;src[k+3]=(k%8==0)?0.0f:1.0f;}
    ConstImageView si{src.data(),w,h,w*4};ImageView di{dst.data(),w,h,w*4};
    NtscQtParams p;p.spectralRingingEnabled=true;p.alpha=0.4f;
    p.amount=0.0f;assert(processNtscQtSignal(si,di,p));assert(dst==src);
    p.amount=1.0f;assert(processNtscQtSignal(si,di,p));
    for(std::size_t k=3;k<src.size();k+=4)assert(dst[k]==src[k]);
    return 0;
}