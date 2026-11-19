#include "VHS90RegionEngine.h"
#include <cassert>
#include <cmath>
#include <vector>

static float pixel(const std::vector<float>& v, int w, int x, int y, int c) {
    return v[(y*w+x)*4+c];
}

int main() {
    using namespace vhs90;
    constexpr int w=64, h=48;
    std::vector<float> src(w*h*4, 0.25f), a(src.size(),0.0f), b(src.size(),0.0f), c(src.size(),0.0f);
    for (size_t i=3;i<src.size();i+=4) src[i]=1.0f;
    ConstImageView si{src.data(),w,h,w*4};
    ImageView da{a.data(),w,h,w*4}, db{b.data(),w,h,w*4}, dc{c.data(),w,h,w*4};

    DamageRegion top;
    top.kind=RegionKind::TrackingNoise; top.shape=RegionShape::HorizontalBand;
    top.centerX=0.5f; top.centerY=0.25f; top.width=1.0f; top.height=0.20f;
    top.intensity=1.0f; top.maxShiftPx=8.0f; top.noiseAmount=0.8f; top.noiseDensity=1.0f;
    top.seedOffset=10;

    DamageRegion right;
    right.kind=RegionKind::HeadSwitch; right.shape=RegionShape::VerticalBand;
    right.centerX=0.90f; right.centerY=0.5f; right.width=0.20f; right.height=1.0f;
    right.intensity=1.0f; right.maxShiftPx=12.0f; right.dropoutProbability=1.0f;
    right.dropoutAmount=0.5f; right.seedOffset=20;

    DamageRegion regions[2]{top,right};
    RegionEngineParams p; p.regions=regions; p.regionCount=2; p.seed=123; p.frame=7; p.timeSeconds=0.25;
    assert(processDamageRegions(si,da,p));
    assert(processDamageRegions(si,db,p));
    assert(a==b);
    RegionEngineParams pNext=p; pNext.frame=8;
    std::vector<float> next(src.size(),0.0f); ImageView dnext{next.data(),w,h,w*4};
    assert(processDamageRegions(si,dnext,pNext));
    assert(next!=a);

    assert(std::fabs(pixel(a,w,10,12,0)-0.25f)>1e-5f);
    assert(std::fabs(pixel(a,w,10,40,0)-0.25f)<1e-5f);
    assert(std::fabs(pixel(a,w,60,40,0)-0.25f)>1e-5f);

    DamageRegion full; full.kind=RegionKind::HeadSwitch; full.shape=RegionShape::HorizontalBand;
    full.centerX=0.5f; full.centerY=0.5f; full.width=1.0f; full.height=1.0f;
    full.intensity=1.0f; full.maxShiftPx=4.0f; full.dropoutProbability=1.0f; full.dropoutAmount=0.3f;
    RegionEngineParams pFull; pFull.regions=&full; pFull.regionCount=1; pFull.seed=4; pFull.frame=1;
    assert(processDamageRegions(si,dc,pFull));
    assert(std::fabs(pixel(c,w,32,24,0)-0.25f)>1e-5f);

    std::vector<float> alphaSrc=src, alphaDst(src.size(),0.0f);
    for (size_t i=3;i<alphaSrc.size();i+=4) alphaSrc[i]=0.0f;
    ConstImageView asi{alphaSrc.data(),w,h,w*4}; ImageView adi{alphaDst.data(),w,h,w*4};
    assert(processDamageRegions(asi,adi,pFull));
    for (size_t i=0;i<alphaDst.size();i+=4) {
        assert(alphaDst[i]==alphaSrc[i]);
        assert(alphaDst[i+1]==alphaSrc[i+1]);
        assert(alphaDst[i+2]==alphaSrc[i+2]);
        assert(alphaDst[i+3]==0.0f);
    }

    std::vector<float> zero(src.size(),0.0f); ImageView dzi{zero.data(),w,h,w*4};
    RegionEngineParams pZero=p; pZero.globalIntensity=0.0f;
    assert(processDamageRegions(si,dzi,pZero));
    assert(zero==src);
    return 0;
}
