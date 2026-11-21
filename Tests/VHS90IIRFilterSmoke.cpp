#include "VHS90IIRFilter.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

static double chromaDiff(const std::vector<float>& image,int w,int y0,int y1) {
    double e=0.0; int n=0;
    for(int y=y0;y<y1;++y) for(int x=12;x<w-1;++x) {
        size_t a=(y*w+x)*4, b=(y*w+x+1)*4;
        vhs90::YIQ p=vhs90::rgbToYiq({image[a],image[a+1],image[a+2]});
        vhs90::YIQ q=vhs90::rgbToYiq({image[b],image[b+1],image[b+2]});
        e+=std::fabs(p.i-q.i)+std::fabs(p.q-q.q); ++n;
    }
    return e/std::max(1,n);
}

int main() {
    using namespace vhs90;
    constexpr int w=256,h=4;
    std::vector<float> src(w*h*4), out(src.size()), gray(src.size()), grayOut(src.size());
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        size_t i=(y*w+x)*4;
        const bool red=(x&1)==0;
        src[i]=red?1.0f:0.0f; src[i+1]=0.0f; src[i+2]=red?0.0f:1.0f; src[i+3]=1.0f;
        const float v=(x&1)?1.0f:0.0f;
        gray[i]=gray[i+1]=gray[i+2]=v; gray[i+3]=1.0f;
    }
    ConstImageView si{src.data(),w,h,w*4}; ImageView oi{out.data(),w,h,w*4};
    YiqBandwidthParams p; p.initial=IIRInitialCondition::FirstSample;
    assert(processYiqBandwidth(si,oi,p));
    assert(chromaDiff(out,w,0,h)<chromaDiff(src,w,0,h)*0.45);

    ConstImageView gi{gray.data(),w,h,w*4}; ImageView go{grayOut.data(),w,h,w*4};
    assert(processYiqBandwidth(gi,go,p));
    for(size_t i=0;i<gray.size();i+=4) {
        assert(std::fabs(grayOut[i]-gray[i])<0.002f);
        assert(std::fabs(grayOut[i+1]-gray[i+1])<0.002f);
        assert(std::fabs(grayOut[i+2]-gray[i+2])<0.002f);
        assert(grayOut[i+3]==1.0f);
    }

    std::vector<float> flat(w*4,0.0f), flatOut(w*4,0.0f);
    for(int x=0;x<w;++x) { flat[x*4]=0.2f; flat[x*4+1]=0.4f; flat[x*4+2]=0.6f; flat[x*4+3]=1.0f; }
    ConstImageView fi{flat.data(),w,1,w*4}; ImageView fo{flatOut.data(),w,1,w*4};
    assert(processYiqBandwidth(fi,fo,p));
    for(int x=10;x<w;++x) for(int c=0;c<3;++c) assert(std::fabs(flatOut[x*4+c]-flat[x*4+c])<0.002f);

    std::vector<float> semi=src, semiOut(src.size());
    for(size_t i=0;i<semi.size();i+=4) { semi[i]*=0.4f; semi[i+1]*=0.4f; semi[i+2]*=0.4f; semi[i+3]=0.4f; }
    ConstImageView ssi{semi.data(),w,h,w*4}; ImageView sso{semiOut.data(),w,h,w*4};
    assert(processYiqBandwidth(ssi,sso,p));
    for(size_t i=3;i<semiOut.size();i+=4) assert(std::fabs(semiOut[i]-0.4f)<1e-6f);

    YiqBandwidthParams bypass; bypass.iCutoffHz=0; bypass.qCutoffHz=0; bypass.lumaCutoffHz=0;
    std::vector<float> identity(src.size()); ImageView id{identity.data(),w,h,w*4};
    assert(processYiqBandwidth(si,id,bypass)); assert(identity==src);

    // Separate vertical luma bandwidth smooths scanline transitions without changing alpha.
    constexpr int vw=4,vh=4;
    std::vector<float> verticalSrc(vw*vh*4),verticalDst(verticalSrc.size());
    for(int y=0;y<vh;++y)for(int x=0;x<vw;++x){size_t k=(y*vw+x)*4;const float v=(y&1)?0.0f:1.0f;verticalSrc[k]=verticalSrc[k+1]=verticalSrc[k+2]=v;verticalSrc[k+3]=1.0f;}
    ConstImageView vsi{verticalSrc.data(),vw,vh,vw*4};ImageView vdi{verticalDst.data(),vw,vh,vw*4};
    YiqBandwidthParams vp;vp.iCutoffHz=0;vp.qCutoffHz=0;vp.lumaCutoffHz=0;vp.verticalLumaRadiusRows=1;vp.verticalLumaSigmaRows=.8f;vp.premultipliedAlpha=false;
    assert(processYiqBandwidth(vsi,vdi,vp));assert(verticalDst!=verticalSrc);
    for(size_t k=3;k<verticalDst.size();k+=4)assert(verticalDst[k]==1.0f);

    const auto sp=makeVhsTapeBandwidth(VHS90TapeSpeed::SP);
    const auto lp=makeVhsTapeBandwidth(VHS90TapeSpeed::LP);
    const auto ep=makeVhsTapeBandwidth(VHS90TapeSpeed::EP);
    assert(sp.lumaCutoffHz==2400000.0f&&sp.iCutoffHz==320000.0f&&sp.iDelaySamples==9);
    assert(lp.lumaCutoffHz==1900000.0f&&lp.iCutoffHz==300000.0f&&lp.iDelaySamples==12);
    assert(ep.lumaCutoffHz==1400000.0f&&ep.iCutoffHz==280000.0f&&ep.iDelaySamples==14);

    YiqBandwidthParams butter=p; butter.design=IIRFilterDesign::Butterworth2;
    std::vector<float> butterOut(src.size()); ImageView bo{butterOut.data(),w,h,w*4};
    assert(processYiqBandwidth(si,bo,butter));
    assert(chromaDiff(butterOut,w,0,h)<chromaDiff(src,w,0,h));

    YiqBandwidthParams invalid=p; invalid.sampleRateHz=0.0f;
    assert(!processYiqBandwidth(si,oi,invalid));
    return 0;
}
