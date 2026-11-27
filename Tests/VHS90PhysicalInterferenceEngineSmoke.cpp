#include "VHS90PhysicalInterferenceEngine.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    constexpr int w = 32, h = 12;
    std::vector<float> src(w*h*4), dst(w*h*4), copy(w*h*4);
    for (int y=0;y<h;++y) for(int x=0;x<w;++x) {
        const auto n=(y*w+x)*4;
        src[n]=0.35f; src[n+1]=0.42f; src[n+2]=0.50f; src[n+3]=1.0f;
    }
    vhs90::ConstImageView in{src.data(),w,h,w*4};
    vhs90::ImageView out{dst.data(),w,h,w*4};
    vhs90::PhysicalInterferenceParams p;
    assert(vhs90::processPhysicalInterference(in,out,p));
    assert(dst==src);

    const float nearField=vhs90::dipoleAxialFieldTesla(1.6f,0.02f);
    const float farField=vhs90::dipoleAxialFieldTesla(1.6f,0.06f);
    assert(nearField>farField*20.0f);
    p.magnetMomentAm2=1.6f; p.magnetDistanceCm=2.0f;
    assert(vhs90::magneticHeadSignalGain(p)<vhs90::magneticHeadSignalGain(vhs90::PhysicalInterferenceParams{}));
    assert(vhs90::magneticTapeEraseFraction(p)>=0.0f &&
           vhs90::magneticTapeEraseFraction(p)<=1.0f);

    p={}; p.mainsHumAmount=0.8f;
    assert(vhs90::processPhysicalInterference(in,out,p));
    assert(dst!=src); // Mains Hum works without enabling Ground Loop.
    for (float v:dst) assert(std::isfinite(v));

    const auto humOnly=dst;

    // A small nonzero Ground Loop voltage must not attenuate the independent
    // Mains Hum control (the old coupled drive almost muted it at 1 mV).
    p={}; p.mainsHumAmount=0.8f; p.groundLoopMillivolts=1.0f;
    assert(vhs90::processPhysicalInterference(in,out,p));
    double humBase=0.0,lowLoopDelta=0.0;
    for(size_t i=0;i<dst.size();i+=4)for(int c=0;c<3;++c){
        humBase+=std::fabs(double(humOnly[i+c])-double(src[i+c]));
        lowLoopDelta+=std::fabs(double(dst[i+c])-double(humOnly[i+c]));
    }
    assert(humBase>0.0&&lowLoopDelta<humBase*0.05);

    p={}; p.mainsHumAmount=0.8f; p.groundLoopMillivolts=350.0f;
    assert(vhs90::processPhysicalInterference(in,out,p));
    assert(dst!=src);
    for (float v:dst) assert(std::isfinite(v));

    // Ground Loop must not be discarded by the neutral fast path when it is
    // the only active physical-interference control.
    p={}; p.groundLoopMillivolts=500.0f;
    assert(vhs90::processPhysicalInterference(in,out,p));
    assert(dst!=src);

    p={}; p.rfPickupAmount=1.0f; p.rfFieldVPerM=25.0f;
    p.frame=17; p.seed=91;
    assert(vhs90::processPhysicalInterference(in,out,p));
    const auto first=dst;
    assert(vhs90::processPhysicalInterference(in,out,p));
    assert(dst==first);
    return 0;
}
