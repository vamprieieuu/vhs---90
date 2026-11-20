#include "VHS90ParticlePatternEngine.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    using namespace vhs90;
    constexpr int w=96,h=72;
    std::vector<float> src(w*h*4,0.2f),a(src.size()),b(src.size()),c(src.size()),d(src.size());
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        const size_t i=(y*w+x)*4;
        src[i]=float(x)/float(w-1);src[i+1]=float(y)/float(h-1);src[i+2]=0.35f;src[i+3]=1.0f;
    }
    ConstImageView si{src.data(),w,h,w*4};
    ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4},di{d.data(),w,h,w*4};
    ParticlePatternParams p;p.seed=77;p.frame=12;p.mode=ParticlePatternMode::HorizontalFragments;
    p.density=.55f;p.particleSizePx=1.4f;p.streakLengthPx=12.0f;p.sourceMix=1.0f;
    assert(processParticlePattern(si,ai,p));assert(processParticlePattern(si,bi,p));assert(a==b);
    ParticlePatternParams sparse=p;sparse.density=.08f;assert(processParticlePattern(si,ci,sparse));assert(c!=a);
    ParticlePatternParams larger=p;larger.particleSizePx=4.5f;assert(processParticlePattern(si,di,larger));assert(d!=a);
    ParticlePatternParams frozen=p;frozen.animated=false;frozen.frame=1;
    std::vector<float> f1(src.size()),f2(src.size());ImageView f1v{f1.data(),w,h,w*4},f2v{f2.data(),w,h,w*4};
    assert(processParticlePattern(si,f1v,frozen));frozen.frame=999;assert(processParticlePattern(si,f2v,frozen));assert(f1==f2);
    std::vector<float> transparent=src,transparentOut(src.size());for(size_t i=3;i<transparent.size();i+=4)transparent[i]=0.0f;
    ConstImageView ti{transparent.data(),w,h,w*4};ImageView toi{transparentOut.data(),w,h,w*4};
    assert(processParticlePattern(ti,toi,p));for(size_t i=3;i<transparentOut.size();i+=4)assert(transparentOut[i]==0.0f);
    return 0;
}
