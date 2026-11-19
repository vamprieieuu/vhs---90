#include "vhs90_temporal_effects.hpp"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace vhs90 {
namespace {
inline bool valid(FloatRgbaView v) {
    return v.pixels&&v.width>0&&v.height>0&&v.rowStrideFloats>=v.width*4;
}
inline void sample(const std::vector<float>&src,int w,int h,float x,float y,float out[4]) {
    x=std::clamp(x,0.0f,float(w-1));y=std::clamp(y,0.0f,float(h-1));
    const int x0=int(x),y0=int(y),x1=std::min(x0+1,w-1),y1=std::min(y0+1,h-1);
    const float fx=x-x0,fy=y-y0;
    const size_t a=(size_t(y0)*w+x0)*4,b=(size_t(y0)*w+x1)*4;
    const size_t c=(size_t(y1)*w+x0)*4,d=(size_t(y1)*w+x1)*4;
    for(int k=0;k<4;++k) {
        const float ab=src[a+k]+(src[b+k]-src[a+k])*fx;
        const float cd=src[c+k]+(src[d+k]-src[c+k])*fx;
        out[k]=ab+(cd-ab)*fy;
    }
}
inline void rgbToYcc(float r,float g,float b,float&y,float&cb,float&cr) {
    y=.299f*r+.587f*g+.114f*b;
    cb=-.168736f*r-.331264f*g+.5f*b;
    cr=.5f*r-.418688f*g-.081312f*b;
}
inline void yccToRgb(float y,float cb,float cr,float&r,float&g,float&b) {
    r=y+1.402f*cr;g=y-.344136f*cb-.714136f*cr;b=y+1.772f*cb;
}
}
void applyTemporalGhost(FloatRgbaView cur,const FloatRgbaView&prev,const TemporalGhostParams&p) {
    if(!valid(cur)||!valid(prev)||!p.enabled||p.amount<=0||
       cur.width!=prev.width||cur.height!=prev.height)return;
    const int w=cur.width,h=cur.height;
    std::vector<float>src(size_t(w)*h*4),old(size_t(w)*h*4);
    for(int y=0;y<h;++y) {
        std::copy(cur.pixels+size_t(y)*cur.rowStrideFloats,
                  cur.pixels+size_t(y)*cur.rowStrideFloats+w*4,src.data()+size_t(y)*w*4);
        std::copy(prev.pixels+size_t(y)*prev.rowStrideFloats,
                  prev.pixels+size_t(y)*prev.rowStrideFloats+w*4,old.data()+size_t(y)*w*4);
    }
    const float amount=std::clamp(p.amount,0.0f,1.0f);
    const float luma=std::clamp(p.lumaAmount,0.0f,2.0f);
    const float chroma=std::clamp(p.chromaAmount,0.0f,2.0f);
    const float weave=std::clamp(p.fieldWeave,0.0f,1.0f);
    for(int y=0;y<h;++y) {
        float*dst=cur.pixels+size_t(y)*cur.rowStrideFloats;
        for(int x=0;x<w;++x) {
            const size_t i=(size_t(y)*w+x)*4;
            const float fieldMix=((y&1)==(p.fieldParity&1))?weave*amount:0.0f;
            float ghost[4],red[4],blue[4];
            sample(old,w,h,float(x)-p.offsetXPixels,float(y)-p.offsetYPixels,ghost);
            sample(old,w,h,float(x)-p.offsetXPixels-p.redFringePixels,float(y)-p.offsetYPixels,red);
            sample(old,w,h,float(x)-p.offsetXPixels-p.blueFringePixels,float(y)-p.offsetYPixels,blue);
            float gy,gcb,gcr,cy,ccb,ccr;
            rgbToYcc(ghost[0],ghost[1],ghost[2],gy,gcb,gcr);
            rgbToYcc(src[i],src[i+1],src[i+2],cy,ccb,ccr);
            const float yOut=cy+(gy-cy)*amount*luma;
            const float cbOut=ccb+(gcb-ccb)*amount*chroma;
            const float crOut=ccr+(gcr-ccr)*amount*chroma;
            float r,g,b;yccToRgb(yOut,cbOut,crOut,r,g,b);
            dst[x*4]=r+(red[0]-r)*fieldMix;
            dst[x*4+1]=g+(ghost[1]-g)*fieldMix;
            dst[x*4+2]=b+(blue[2]-b)*fieldMix;
            // Current-frame alpha is preserved.
        }
    }
}
} // namespace vhs90
