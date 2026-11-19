#include "VHS90CompositeEngine.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    using namespace vhs90;
    constexpr int w=96,h=8;
    std::vector<float> src(w*h*4), boxOut(src.size()), boxAgain(src.size()), zeroOut(src.size());
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        size_t i=(y*w+x)*4;
        const bool red=((x/2+y)&1)==0;
        src[i]=red?0.9f:0.05f;
        src[i+1]=0.15f+0.05f*float(y);
        src[i+2]=red?0.08f:0.85f;
        src[i+3]=1.0f;
    }
    ConstImageView si{src.data(),w,h,w*4};
    ImageView bo{boxOut.data(),w,h,w*4}, ba{boxAgain.data(),w,h,w*4}, zo{zeroOut.data(),w,h,w*4};

    CompositeParams p; p.strength=1.0f; p.outputBandwidthEnabled=false; p.demodulator=CompositeDemodulator::Box;
    p.phase=CompositePhase::Quadrature90; p.frame=3;
    assert(processCompositeRoundTrip(si,bo,p));
    assert(processCompositeRoundTrip(si,ba,p));
    assert(boxOut==boxAgain);
    bool changed=false;
    for(size_t i=0;i<boxOut.size();i+=4) if(std::fabs(boxOut[i]-src[i])>1e-4f) { changed=true; break; }
    assert(changed);

    CompositeParams pZero=p; pZero.strength=0.0f;
    assert(processCompositeRoundTrip(si,zo,pZero));
    assert(zeroOut==src);

    std::vector<float> phase0(src.size()), phase180(src.size());
    ImageView d0{phase0.data(),w,h,w*4}, d180{phase180.data(),w,h,w*4};
    CompositeParams pp=p; pp.phase=CompositePhase::Degrees0;
    assert(processCompositeRoundTrip(si,d0,pp));
    pp.phase=CompositePhase::Degrees180;
    assert(processCompositeRoundTrip(si,d180,pp));
    assert(phase0!=phase180);

    std::vector<float> one(src.size()), two(src.size());
    ImageView d1{one.data(),w,h,w*4}, d2{two.data(),w,h,w*4};
    pp=p; pp.demodulator=CompositeDemodulator::OneLineComb;
    assert(processCompositeRoundTrip(si,d1,pp));
    pp.demodulator=CompositeDemodulator::TwoLineComb;
    assert(processCompositeRoundTrip(si,d2,pp));
    assert(one!=two);

    std::vector<float> transportComposite(src.size()),surfaceComposite(src.size()),bothComposite(src.size());
    ImageView tcv{transportComposite.data(),w,h,w*4}, scv{surfaceComposite.data(),w,h,w*4}, bcv{bothComposite.data(),w,h,w*4};
    CompositeParams physical=p; physical.tapeTransportEnabled=true; physical.tapeTransport.frame=4;
    physical.tapeTransport.drumFlutterPixels=1.2f; physical.tapeTransport.flutterPixels=.4f;
    assert(processCompositeRoundTrip(si,tcv,physical));
    physical=p; physical.tapeSurfaceEnabled=true; physical.tapeSurface.frame=4;
    physical.tapeSurface.defectDensity=.8f; physical.tapeSurface.edgeLift=.15f;
    assert(processCompositeRoundTrip(si,scv,physical));
    physical.tapeTransportEnabled=true; physical.tapeTransport.drumFlutterPixels=.8f;
    assert(processCompositeRoundTrip(si,bcv,physical));
    assert(transportComposite!=boxOut && surfaceComposite!=boxOut && bothComposite!=boxOut);
    for(size_t i=3;i<bothComposite.size();i+=4) assert(std::fabs(bothComposite[i]-1.0f)<1e-6f);

    std::vector<float> palOutput(src.size()),palNoSwitch(src.size());
    ImageView palView{palOutput.data(),w,h,w*4},palNoView{palNoSwitch.data(),w,h,w*4};
    CompositeParams pal=p; pal.palMode=true; pal.palSwitch=true;
    assert(processCompositeRoundTrip(si,palView,pal));
    pal.palSwitch=false;
    assert(processCompositeRoundTrip(si,palNoView,pal));
    assert(palOutput!=boxOut);
    assert(palNoSwitch==boxOut);

    std::vector<float> vhsVertical(src.size()),vhsSmear(src.size()),vhsCombined(src.size());
    ImageView vv{vhsVertical.data(),w,h,w*4}, sv{vhsSmear.data(),w,h,w*4}, cv{vhsCombined.data(),w,h,w*4};
    CompositeParams vhs=p; vhs.verticalChromaBlendEnabled=true; vhs.verticalChromaBlendAmount=1.0f;
    assert(processCompositeRoundTrip(si,vv,vhs));
    vhs=p; vhs.lumaSmearEnabled=true; vhs.lumaSmear.strength=1.0f;
    vhs.lumaSmear.tapCount=16; vhs.lumaSmear.highlightStrength=0.25f;
    assert(processCompositeRoundTrip(si,sv,vhs));
    vhs.verticalChromaBlendEnabled=true; vhs.verticalChromaBlendAmount=0.75f;
    assert(processCompositeRoundTrip(si,cv,vhs));
    assert(vhsVertical!=boxOut && vhsSmear!=boxOut && vhsCombined!=boxOut);
    for(size_t i=3;i<vhsCombined.size();i+=4) assert(std::fabs(vhsCombined[i]-1.0f)<1e-6f);

    std::vector<float> semi=src, semiOut(src.size());
    for(size_t i=0;i<semi.size();i+=4) { semi[i]*=0.35f; semi[i+1]*=0.35f; semi[i+2]*=0.35f; semi[i+3]=0.35f; }
    ConstImageView ssi{semi.data(),w,h,w*4}; ImageView sso{semiOut.data(),w,h,w*4};
    pp=p; pp.premultipliedAlpha=true; pp.outputBandwidthEnabled=true;
    assert(processCompositeRoundTrip(ssi,sso,pp));
    for(size_t i=3;i<semiOut.size();i+=4) assert(std::fabs(semiOut[i]-0.35f)<1e-6f);

    std::vector<float> gray(w*h*4), grayOut(gray.size());
    for(size_t i=0;i<gray.size();i+=4) { gray[i]=gray[i+1]=gray[i+2]=0.45f; gray[i+3]=1.0f; }
    ConstImageView gi{gray.data(),w,h,w*4}; ImageView go{grayOut.data(),w,h,w*4};
    pp=p; pp.phase=CompositePhase::Degrees0;
    assert(processCompositeRoundTrip(gi,go,pp));
    for(int y=0;y<h;++y) for(int x=12;x<w;++x) {
        size_t i=(y*w+x)*4;
        assert(std::fabs(grayOut[i]-grayOut[i+1])<0.003f);
        assert(std::fabs(grayOut[i+1]-grayOut[i+2])<0.003f);
    }
    return 0;
}
