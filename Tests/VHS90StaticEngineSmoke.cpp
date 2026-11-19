#include "VHS90StaticEngine.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    using namespace vhs90;
    constexpr int w=80,h=60;
    std::vector<float> src(w*h*4), a(w*h*4), b(w*h*4), c(w*h*4);
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        size_t i=(y*w+x)*4;
        src[i]=0.35f; src[i+1]=0.45f; src[i+2]=0.55f; src[i+3]=1.0f;
    }
    ConstImageView si{src.data(),w,h,w*4};
    ImageView da{a.data(),w,h,w*4}, db{b.data(),w,h,w*4}, dc{c.data(),w,h,w*4};

    StaticParams p; p.mode=StaticPatternMode::FullFrameHorizontal; p.intensity=1.0f;
    p.sourceMix=1.0f; p.density=1.0f; p.lumaAmount=0.8f; p.chromaAmount=0.25f;
    p.cellSizePx=2.0f; p.streakLengthPx=12.0f; p.seed=91; p.frame=4;
    assert(processStaticPattern(si,da,p));
    assert(processStaticPattern(si,db,p));
    assert(a==b);
    StaticParams pNext=p; pNext.frame=5;
    assert(processStaticPattern(si,dc,pNext));
    assert(a!=c);

    std::vector<float> frozenA(src.size()), frozenB(src.size());
    ImageView dfa{frozenA.data(),w,h,w*4}, dfb{frozenB.data(),w,h,w*4};
    p.animated=false; p.frame=4; assert(processStaticPattern(si,dfa,p));
    p.frame=40; assert(processStaticPattern(si,dfb,p));
    assert(frozenA==frozenB);

    std::vector<float> top(src.size()); ImageView dt{top.data(),w,h,w*4};
    StaticParams pt=p; pt.animated=true; pt.mode=StaticPatternMode::TopStrip;
    pt.topBandHeight=0.2f; pt.sourceMix=1.0f; pt.frame=8; pt.density=1.0f;
    assert(processStaticPattern(si,dt,pt));
    const size_t upper=(5*w+20)*4, lower=(50*w+20)*4;
    assert(std::fabs(top[upper]-src[upper])>1e-5f);
    assert(top[lower]==src[lower] && top[lower+1]==src[lower+1] && top[lower+2]==src[lower+2]);

    std::vector<float> black(src.size()); ImageView dblack{black.data(),w,h,w*4};
    StaticParams pb; pb.mode=StaticPatternMode::StaticToBlackout; pb.sourceMix=0.0f;
    pb.blackoutProbability=1.0f; pb.baseLuma=0.2f; pb.frame=2;
    assert(processStaticPattern(si,dblack,pb));
    for(size_t i=0;i<black.size();i+=4) {
        assert(black[i]==0.0f && black[i+1]==0.0f && black[i+2]==0.0f);
        assert(black[i+3]==1.0f);
    }

    std::vector<float> identity(src.size()); ImageView did{identity.data(),w,h,w*4};
    StaticParams pi; pi.intensity=0.0f; pi.sourceMix=1.0f;
    assert(processStaticPattern(si,did,pi));
    assert(identity==src);

    std::vector<float> semi=src, semiOut(src.size());
    for(size_t i=0;i<semi.size();i+=4) { semi[i]*=0.4f; semi[i+1]*=0.4f; semi[i+2]*=0.4f; semi[i+3]=0.4f; }
    ConstImageView ssi{semi.data(),w,h,w*4}; ImageView sso{semiOut.data(),w,h,w*4};
    StaticParams ps; ps.sourceMix=1.0f; ps.intensity=1.0f; ps.density=1.0f; ps.seed=2; ps.frame=3;
    assert(processStaticPattern(ssi,sso,ps));
    for(size_t i=3;i<semiOut.size();i+=4) assert(std::fabs(semiOut[i]-0.4f)<1e-6f);
    return 0;
}
