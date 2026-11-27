#include "VHS90TearChromaEngine.h"
#include <cassert>
#include <vector>

int main() {
    using namespace vhs90;
    constexpr int w=80,h=60;
    std::vector<float> src(w*h*4),a(src.size()),b(src.size()),c(src.size());
    for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;src[i]=float(x)/float(w-1);src[i+1]=float(y)/float(h-1);src[i+2]=.25f;src[i+3]=1.0f;}
    ConstImageView si{src.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4};
    TearBand bands[2];bands[0].centerY=.25f;bands[0].height=.18f;bands[0].seedOffset=1;bands[1].centerY=.78f;bands[1].height=.22f;bands[1].seedOffset=2;
    TearChromaParams p;p.seed=91;p.frame=7;p.timeSeconds=.3;p.mode=TearChromaMode::SpectralWithLowerTear;p.bands=bands;p.bandCount=2;
    assert(processTearChroma(si,ai,p));assert(processTearChroma(si,bi,p));assert(a==b);assert(a!=src);
    std::vector<float> changedBand(src.size()); ImageView changedBandView{changedBand.data(),w,h,w*4};
    bands[1].centerY=.42f; bands[1].chromaShiftPx=-17.0f; bands[1].horizontalShiftPx=55.0f;
    assert(processTearChroma(si,changedBandView,p)); assert(changedBand!=a);
    bands[1].centerY=.78f; bands[1].chromaShiftPx=5.0f; bands[1].horizontalShiftPx=18.0f;
    TearChromaParams spectral=p;spectral.mode=TearChromaMode::SpectralSmear;spectral.bandCount=0;
    assert(processTearChroma(si,ci,spectral));assert(c!=src&&c!=a);

    // All five exposed modes must be deterministic and mathematically distinct.
    std::vector<float> modeOutputs[5];
    for(int mode=0;mode<5;++mode) {
        modeOutputs[mode].resize(src.size());
        TearChromaParams variant=p; variant.mode=static_cast<TearChromaMode>(mode);
        variant.bandCount=2; variant.seed=0x5511u; variant.frame=23; variant.timeSeconds=0.61;
        variant.spectralIntensity=0.9f; variant.spectralCoverage=0.8f;
        ImageView modeView{modeOutputs[mode].data(),w,h,w*4};
        assert(processTearChroma(si,modeView,variant));
        assert(modeOutputs[mode]!=src);
        std::vector<float> repeated(src.size()); ImageView repeatedView{repeated.data(),w,h,w*4};
        assert(processTearChroma(si,repeatedView,variant));
        assert(repeated==modeOutputs[mode]);
    }
    for(int left=0;left<5;++left)for(int right=left+1;right<5;++right)
        assert(modeOutputs[left]!=modeOutputs[right]);
    std::vector<float> alpha=src,alphaOut(src.size());for(size_t i=3;i<alpha.size();i+=4)alpha[i]=.0f;
    ConstImageView al{alpha.data(),w,h,w*4};ImageView alo{alphaOut.data(),w,h,w*4};assert(processTearChroma(al,alo,p));
    for(size_t i=3;i<alphaOut.size();i+=4)assert(alphaOut[i]==0.0f);
    return 0;
}
