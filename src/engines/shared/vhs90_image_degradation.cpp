#include "vhs90_image_degradation.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
inline uint64_t hash64(uint64_t x) noexcept { x^=x>>30;x*=0xbf58476d1ce4e5b9ULL;x^=x>>27;x*=0x94d049bb133111ebULL;x^=x>>31;return x; }
inline float randomSigned(uint64_t seed,int64_t frame,int x,int y) noexcept {
    const uint64_t h=hash64(seed^(uint64_t(frame)*0x9e3779b97f4a7c15ULL)^(uint64_t(uint32_t(x))*0xd1b54a32d192ed03ULL)^(uint64_t(uint32_t(y))*0x94d049bb133111ebULL));
    return float((h>>40)&0xffffffu)*(1.0f/16777216.0f)*2.0f-1.0f;
}
struct Ycc { float y,cb,cr,a; };
inline Ycc toYcc(const float*p) noexcept {
    return {.299f*p[0]+.587f*p[1]+.114f*p[2],
            -.168736f*p[0]-.331264f*p[1]+.5f*p[2],
            .5f*p[0]-.418688f*p[1]-.081312f*p[2],p[3]};
}
inline void toRgb(const Ycc&v,float*p) noexcept {
    p[0]=v.y+1.402f*v.cr;p[1]=v.y-.344136f*v.cb-.714136f*v.cr;
    p[2]=v.y+1.772f*v.cb;p[3]=v.a;
}
inline Ycc bilinear(const std::vector<Ycc>&a,int w,int h,float x,float y) noexcept {
    x=std::clamp(x,0.0f,float(w-1));y=std::clamp(y,0.0f,float(h-1));
    int x0=int(x),y0=int(y),x1=std::min(x0+1,w-1),y1=std::min(y0+1,h-1);
    float fx=x-x0,fy=y-y0;
    const Ycc&A=a[size_t(y0)*w+x0],&B=a[size_t(y0)*w+x1],&C=a[size_t(y1)*w+x0],&D=a[size_t(y1)*w+x1];
    auto m=[&](float av,float bv,float cv,float dv){float ab=av+(bv-av)*fx,cd=cv+(dv-cv)*fx;return ab+(cd-ab)*fy;};
    return {m(A.y,B.y,C.y,D.y),m(A.cb,B.cb,C.cb,D.cb),m(A.cr,B.cr,C.cr,D.cr),m(A.a,B.a,C.a,D.a)};
}
inline bool valid(FloatRgbaView im) noexcept { return im.pixels&&im.width>0&&im.height>0&&im.rowStrideFloats>=im.width*4; }
}
void applyChromaBleed(FloatRgbaView im,const ChromaBleedParams&p) {
    if(!valid(im)||!p.enabled||p.amount<=0) return;
    const int w=im.width,h=im.height;
    std::vector<Ycc> src(size_t(w)*h),out(size_t(w)*h);
    for(int y=0;y<h;++y)for(int x=0;x<w;++x)
        src[size_t(y)*w+x]=toYcc(im.pixels+size_t(y)*im.rowStrideFloats+x*4);
    const int sx=std::clamp(p.horizontalSubsample,1,8),sy=std::clamp(p.verticalSubsample,1,8);
    const int cw=(w+sx-1)/sx,ch=(h+sy-1)/sy;
    std::vector<float> cb(size_t(cw)*ch),cr(size_t(cw)*ch);
    for(int cy=0;cy<ch;++cy)for(int cx=0;cx<cw;++cx) {
        float b=0,r=0,n=0;
        for(int yy=cy*sy;yy<std::min(h,(cy+1)*sy);++yy)
            for(int xx=cx*sx;xx<std::min(w,(cx+1)*sx);++xx) {
                const auto&v=src[size_t(yy)*w+xx];b+=v.cb;r+=v.cr;n+=1;
            }
        cb[size_t(cy)*cw+cx]=b/std::max(n,1.0f);
        cr[size_t(cy)*cw+cx]=r/std::max(n,1.0f);
    }
    const int radius=std::clamp(int(std::ceil(p.blurRadius)),0,24);
    const float shiftB=p.cbOffsetPixels/sx,shiftR=p.crOffsetPixels/sx;
    for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
        const auto&v=src[size_t(y)*w+x];
        const float fx=float(x)/sx,fy=float(y)/sy;
        auto samplePlane=[&](const std::vector<float>&pl,float px) {
            px=std::clamp(px,0.0f,float(cw-1));
            const int x0=int(px),x1=std::min(x0+1,cw-1);const float f=px-x0;
            const float nearest=pl[size_t(int(fy))*cw+x0]*(1-f)+pl[size_t(int(fy))*cw+x1]*f;
            if(radius==0)return nearest;
            const float sigma=std::max(0.5f,p.blurRadius/float(sx));
            float sum=0,weights=0;
            for(int d=-radius;d<=radius;++d) {
                const int xx=std::clamp(x0+d,0,cw-1);
                const float weight=std::exp(-float(d*d)/(2.0f*sigma*sigma));
                sum+=pl[size_t(int(fy))*cw+xx]*weight;weights+=weight;
            }
            return sum/std::max(weights,1e-6f);
        };
        Ycc o=v;
        o.cb=v.cb+(samplePlane(cb,fx-shiftB)-v.cb)*std::clamp(p.amount,0.0f,1.0f);
        o.cr=v.cr+(samplePlane(cr,fx-shiftR)-v.cr)*std::clamp(p.amount,0.0f,1.0f);
        out[size_t(y)*w+x]=o;
    }
    for(int y=0;y<h;++y)for(int x=0;x<w;++x)
        toRgb(out[size_t(y)*w+x],im.pixels+size_t(y)*im.rowStrideFloats+x*4);
}
void applyLumaSmear(FloatRgbaView im,const LumaSmearParams&p) {
    if(!valid(im)||!p.enabled||p.amount<=0||p.radiusPixels<=0) return;
    const int w=im.width,h=im.height;
    std::vector<Ycc>src(size_t(w)*h),out(size_t(w)*h);
    for(int y=0;y<h;++y)for(int x=0;x<w;++x)
        src[size_t(y)*w+x]=toYcc(im.pixels+size_t(y)*im.rowStrideFloats+x*4);
    const int direction=p.direction<0?-1:1,radius=std::clamp(p.radiusPixels,1,96);
    const float amount=std::clamp(p.amount,0.0f,2.0f);
    for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
        const Ycc base=src[size_t(y)*w+x];float trail=0,weights=0,highlight=0,highlightWeights=0;
        for(int d=1;d<=radius;++d) {
            const int xx=std::clamp(x-direction*d,0,w-1);
            const float weight=std::exp(-float(d)/(radius*0.42f));
            const float lum=src[size_t(y)*w+xx].y;
            trail+=lum*weight;weights+=weight;
            highlight+=std::max(lum-p.highlightThreshold,0.0f)*weight;
            highlightWeights+=weight;
        }
        const float smear=weights>0?trail/weights:base.y;
        const float brightTrail=highlightWeights>0?highlight/highlightWeights:0.0f;
        Ycc o=base;o.y=base.y+(smear-base.y)*amount+brightTrail*p.highlightTrail*amount;
        out[size_t(y)*w+x]=o;
    }
    for(int y=0;y<h;++y)for(int x=0;x<w;++x)
        toRgb(out[size_t(y)*w+x],im.pixels+size_t(y)*im.rowStrideFloats+x*4);
}
void applyRowWave(FloatRgbaView im,const RowWaveParams&p) {
    if(!valid(im)||!p.enabled||p.amplitudePixels==0) return;
    const int w=im.width,h=im.height;std::vector<float>src(size_t(w)*h*4);
    for(int y=0;y<h;++y)
        std::copy(im.pixels+size_t(y)*im.rowStrideFloats,
                  im.pixels+size_t(y)*im.rowStrideFloats+w*4,src.data()+size_t(y)*w*4);
    for(int y=0;y<h;++y) {
        const float phase=(float(y)/std::max(1,h-1))*p.cycles*2.0f*kPi+p.phase+float(p.frameIndex)*p.speed;
        const float shift=std::sin(phase)*p.amplitudePixels+randomSigned(p.seed,p.frameIndex,0,y)*p.randomPerRow;
        float*dst=im.pixels+size_t(y)*im.rowStrideFloats;
        for(int x=0;x<w;++x) {
            const float fx=std::clamp(float(x)-shift,0.0f,float(w-1));
            const int x0=int(fx),x1=std::min(x0+1,w-1);const float t=fx-x0;
            const float*a=src.data()+(size_t(y)*w+x0)*4,*b=src.data()+(size_t(y)*w+x1)*4;
            for(int c=0;c<3;++c)dst[x*4+c]=a[c]+(b[c]-a[c])*t;
        }
    }
}
} // namespace vhs90
