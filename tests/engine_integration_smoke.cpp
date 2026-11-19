#include "../src/engines/shared/vhs90_region_effects.hpp"
#include "../src/engines/shared/vhs90_signal_pipeline.hpp"
#include "../src/engines/shared/vhs90_tape_surface.hpp"
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const int w=192,h=128;
    std::vector<float> a(size_t(w)*h*4), b;
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        const size_t i=(size_t(y)*w+x)*4;
        a[i]=float(x)/(w-1); a[i+1]=float(y)/(h-1); a[i+2]=0.2f; a[i+3]=0.63f;
    }
    b=a;
    vhs90::FloatRgbaView va{a.data(),w,h,w*4}, vb{b.data(),w,h,w*4};
    vhs90::AnalogSignalParams sp;
    sp.compositeStrength=0.7f; sp.dotCrawlGain=0.4f; sp.crossColorGain=0.3f;
    sp.lumaNoise=0.2f; sp.chromaNoise=0.2f; sp.bandwidthLines=180.0f;
    sp.chromaBandwidth=65.0f; sp.frameIndex=31; sp.seed=987;
    vhs90::applyAnalogSignalPipeline(va,sp);
    vhs90::applyAnalogSignalPipeline(vb,sp);

    vhs90::TrackingNoiseParams tr;
    tr.enabled=true; tr.region.centerX=0.48f; tr.region.centerY=0.27f;
    tr.region.width=0.9f; tr.region.height=0.15f; tr.displacementPixels=8.0f;
    tr.noiseAmount=0.05f;
    vhs90::applyTrackingNoise(va,tr,sp.seed,sp.frameIndex);
    vhs90::applyTrackingNoise(vb,tr,sp.seed,sp.frameIndex);

    vhs90::HeadSwitchParams hs;
    hs.enabled=true; hs.anchorY=1.0f; hs.heightFraction=1.0f;
    hs.intensity=0.7f; hs.horizontalShiftPixels=15.0f;
    vhs90::applyHeadSwitching(va,hs,sp.seed,sp.frameIndex);
    vhs90::applyHeadSwitching(vb,hs,sp.seed,sp.frameIndex);

    vhs90::TapeSurfaceParams tp;
    tp.seed=sp.seed; tp.frameIndex=sp.frameIndex; tp.defectDensity=0.5f;
    tp.oxideLoss=0.5f; tp.scratchDensity=0.4f; tp.creaseStrength=0.3f;
    vhs90::applyTapeSurface(va,tp);
    vhs90::applyTapeSurface(vb,tp);

    if(a!=b) return 1;
    float sum=0.0f;
    for(size_t i=0;i<a.size();i+=4) {
        sum+=std::abs(a[i]-float((i/4)%w)/(w-1));
        if(std::abs(a[i+3]-0.63f)>1e-6f) return 2;
    }
    if(sum<1.0f) return 3;
    std::cout<<"PASS shared VHS chain deterministic; alpha preserved; signal/tracking/head-switch/tape stages composed\n";
    return 0;
}
