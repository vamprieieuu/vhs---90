#include "VHS90SyncEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main() {
    using namespace vhs90;
    constexpr int w=128,h=96;
    std::vector<float> src(w*h*4),a(src.size()),b(src.size()),c(src.size());
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        size_t i=(y*w+x)*4; src[i]=float(x)/float(w-1); src[i+1]=float(y)/float(h-1);
        src[i+2]=0.25f+0.5f*float((x+y)%17)/16.0f; src[i+3]=1.0f;
    }
    ConstImageView si{src.data(),w,h,w*4}; ImageView da{a.data(),w,h,w*4},db{b.data(),w,h,w*4},dc{c.data(),w,h,w*4};
    SyncCollapseParams p; p.seed=17; p.frame=9; p.timeSeconds=0.1; p.eventFrequency=1.0f;
    p.noiseAmount=0.08f; p.enableVerticalRoll=false; p.syncEventProbability=0.0f;
    assert(processSyncCollapse(si,da,p)); assert(processSyncCollapse(si,db,p)); assert(a==b); assert(a!=src);
    SyncCollapseParams zero=p; zero.strength=0.0f; assert(processSyncCollapse(si,dc,zero)); assert(c==src);
    SyncCollapseParams clean; clean.eventFrequency=0.0f; clean.slowTimebasePx=0.0f; clean.fastTimebasePx=0.0f;
    clean.noiseAmount=0.0f; clean.enableTrackingLine=false; clean.syncEventProbability=0.0f;
    std::vector<float> identity(src.size()); ImageView di{identity.data(),w,h,w*4};
    assert(processSyncCollapse(si,di,clean)); assert(identity==src);
    std::uint32_t seed=1; while(hash01(0u,92u,seed+0x94d049bbu,0)<=0.52f) ++seed;
    SyncCollapseParams roll; roll.seed=seed; roll.timeSeconds=0.1; roll.eventFrequency=0.0f;
    roll.slowTimebasePx=0.0f; roll.fastTimebasePx=0.0f; roll.noiseAmount=0.0f;
    roll.enableTrackingLine=false; roll.syncEventProbability=1.0f; roll.syncRollStrength=0.4f;
    std::vector<float> rolled(src.size()); ImageView dr{rolled.data(),w,h,w*4};
    assert(processSyncCollapse(si,dr,roll)); assert(rolled!=src);
    std::vector<float> semi=src,semiOut(src.size());
    for(size_t i=0;i<semi.size();i+=4) { semi[i]*=0.3f; semi[i+1]*=0.3f; semi[i+2]*=0.3f; semi[i+3]=0.3f; }
    ConstImageView ssi{semi.data(),w,h,w*4}; ImageView sso{semiOut.data(),w,h,w*4};
    p.premultipliedAlpha=true; assert(processSyncCollapse(ssi,sso,p));
    for(size_t i=3;i<semiOut.size();i+=4) assert(std::fabs(semiOut[i]-0.3f)<1e-6f);
    std::vector<float> transparent=src,transparentOut(src.size());
    for(size_t i=0;i<transparent.size();i+=4) { transparent[i]=0.2f; transparent[i+1]=0.3f; transparent[i+2]=0.4f; transparent[i+3]=0.0f; }
    ConstImageView ti{transparent.data(),w,h,w*4}; ImageView to{transparentOut.data(),w,h,w*4};
    assert(processSyncCollapse(ti,to,p)); assert(transparentOut==transparent);
    return 0;
}
