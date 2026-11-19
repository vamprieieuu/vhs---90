#include "VHS90SignalKernels.h"
#include <cassert>
#include <cmath>
#include <vector>
int main() {
    using namespace vhs90;
    RGB c{0.25f,0.5f,0.75f};
    RGB r=yiqToRgb(rgbToYiq(c));
    assert(std::fabs(r.r-c.r)<0.002f && std::fabs(r.g-c.g)<0.002f && std::fabs(r.b-c.b)<0.002f);
    YIQ neutral=rgbToYiq({0.5f,0.5f,0.5f});
    assert(std::fabs(neutral.y-0.5f)<0.001f && std::fabs(neutral.i)<0.001f && std::fabs(neutral.q)<0.001f);
    std::vector<float> src(4*4*4,0.25f), dst(src.size(),0.0f);
    for(size_t i=3;i<src.size();i+=4) src[i]=0.37f;
    ConstImageView si{src.data(),4,4,16}; ImageView di{dst.data(),4,4,16};
    SignalParams p; p.frame=12; p.seed=7;
    assert(processCompositeVhs(si,di,p));
    for(size_t i=3;i<dst.size();i+=4) assert(std::fabs(dst[i]-0.37f)<1e-6f);
    assert(hash01(2,3,7,12)==hash01(2,3,7,12));
    const float vn0=valueNoise1D(4.0f,7,12);
    const float vn1=valueNoise1D(4.001f,7,12);
    assert(std::fabs(vn1-vn0)<0.01f);
    assert(vn0==valueNoise1D(4.0f,7,12));
    std::vector<float> smearSrc(5*4,0.0f),smearDst(5*4,0.0f);
    for(int x=0;x<5;++x) smearSrc[x*4+3]=1.0f;
    smearSrc[1*4+0]=smearSrc[1*4+1]=smearSrc[1*4+2]=1.0f;
    ConstImageView smi{smearSrc.data(),5,1,20}; ImageView smo{smearDst.data(),5,1,20};
    LumaSmearParams smp; smp.strength=1.0f; smp.decay=0.3f; smp.offsetPx=1.0f; smp.premultipliedAlpha=false;
    assert(processLumaSmear(smi,smo,smp));
    assert(smearDst[2*4+0]>0.0f && smearDst[2*4+3]==1.0f);
    return 0;
}
