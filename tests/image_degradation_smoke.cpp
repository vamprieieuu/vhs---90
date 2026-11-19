#include "../src/engines/shared/vhs90_image_degradation.hpp"
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const int w=128,h=96;
    std::vector<float>a(size_t(w)*h*4),b;
    for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
        const size_t i=(size_t(y)*w+x)*4;
        a[i]=float(x)/(w-1);a[i+1]=float(y)/(h-1);a[i+2]=0.35f;a[i+3]=0.52f;
    }
    const auto original=a;b=a;
    vhs90::FloatRgbaView va{a.data(),w,h,w*4},vb{b.data(),w,h,w*4};
    vhs90::ChromaBleedParams cp;
    cp.enabled=true;cp.horizontalSubsample=3;cp.verticalSubsample=2;
    cp.amount=0.8f;cp.blurRadius=2.4f;cp.cbOffsetPixels=-2.0f;cp.crOffsetPixels=2.0f;
    vhs90::applyChromaBleed(va,cp);vhs90::applyChromaBleed(vb,cp);
    vhs90::LumaSmearParams sp;sp.enabled=true;sp.radiusPixels=16;sp.amount=0.4f;
    vhs90::applyLumaSmear(va,sp);vhs90::applyLumaSmear(vb,sp);
    vhs90::RowWaveParams wp;wp.enabled=true;wp.amplitudePixels=3;wp.cycles=2.5f;
    wp.randomPerRow=0.7f;wp.frameIndex=20;wp.seed=10;
    vhs90::applyRowWave(va,wp);vhs90::applyRowWave(vb,wp);
    if(a!=b)return 1;
    float delta=0;
    for(size_t i=0;i<a.size();i+=4) {
        delta+=std::abs(a[i]-original[i]);
        if(std::abs(a[i+3]-0.52f)>1e-6f)return 2;
    }
    if(delta<0.1f)return 3;
    std::cout<<"PASS chroma bleed, luma smear and row wave deterministic; alpha preserved\n";
    return 0;
}
