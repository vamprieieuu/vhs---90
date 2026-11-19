#include "../src/engines/shared/vhs90_signal_pipeline.hpp"
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const int w=160,h=120;
    std::vector<float> a(size_t(w)*h*4), b;
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        const size_t i=(size_t(y)*w+x)*4;
        a[i]=float(x)/(w-1); a[i+1]=float(y)/(h-1); a[i+2]=0.3f; a[i+3]=0.4f;
    }
    b=a;
    vhs90::AnalogSignalParams p;
    p.compositeStrength=0.8f; p.dotCrawlGain=0.6f; p.crossColorGain=0.4f;
    p.compositeNoise=0.2f; p.lumaNoise=0.3f; p.chromaNoise=0.25f;
    p.bandwidthLines=180.0f; p.chromaBandwidth=55.0f; p.bandwidthRows=100.0f;
    p.lineJitterPixels=1.4f; p.topSkewPixels=2.0f; p.topSkewDecay=80.0f;
    p.chromaOffsetR=1.5f; p.chromaOffsetB=-1.5f; p.scanlineAmount=0.1f;
    p.seed=99; p.frameIndex=5;
    vhs90::FloatRgbaView va{a.data(),w,h,w*4}, vb{b.data(),w,h,w*4};
    vhs90::applyAnalogSignalPipeline(va,p);
    vhs90::applyAnalogSignalPipeline(vb,p);
    if(a!=b) return 1;
    float diff=0.0f;
    for(size_t i=0;i<a.size();i+=4) {
        if(std::abs(a[i+3]-0.4f)>1e-6f) return 2;
        diff+=std::abs(a[i]-float((i/4)%w)/(w-1));
    }
    if(diff<1.0f) return 3;
    std::cout<<"PASS analog signal pipeline deterministic; alpha preserved; RGB transformed\n";
    return 0;
}
