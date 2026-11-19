#include "vhs90_signal_pipeline.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace vhs90 {
namespace {
constexpr float kPi = 3.14159265358979323846f;

struct Ycca {
    float y, cb, cr, a;
};

inline uint32_t hash32(uint32_t x) noexcept {
    x ^= x >> 15; x *= 0x2C1B3C6Du;
    x ^= x >> 12; x *= 0x297A2D39u;
    x ^= x >> 15;
    return x;
}
inline uint32_t cellSeed(int x, int y, uint32_t offset) noexcept {
    uint32_t h = offset ^ 0x811C9DC5u;
    h = (h ^ (uint32_t(x) & 0xFFFFu)) * 0x01000193u;
    h = (h ^ (uint32_t(y) & 0xFFFFu)) * 0x01000193u;
    return hash32(h);
}
inline float uniform01(uint32_t h) noexcept {
    return float(h) * 2.3283064365e-10f;
}
inline float gaussian(uint32_t a, uint32_t b) noexcept {
    const float u1 = std::max(uniform01(a), 1e-7f);
    const float u2 = uniform01(b);
    return std::clamp(std::sqrt(-2.0f * std::log(u1)) *
                      std::cos(2.0f * kPi * u2), -3.0f, 3.0f);
}
inline Ycca rgbToYcc(const float* p) noexcept {
    const float r = p[0], g = p[1], b = p[2];
    return {0.299f*r + 0.587f*g + 0.114f*b,
            -0.168736f*r - 0.331264f*g + 0.5f*b,
            0.5f*r - 0.418688f*g - 0.081312f*b, p[3]};
}
inline void yccToRgb(const Ycca& c, float* p) noexcept {
    p[0] = c.y + 1.402f*c.cr;
    p[1] = c.y - 0.344136f*c.cb - 0.714136f*c.cr;
    p[2] = c.y + 1.772f*c.cb;
    p[3] = c.a;
}
inline Ycca sample(const std::vector<Ycca>& v, int w, int h,
                   float x, float y) noexcept {
    x = std::clamp(x, 0.0f, float(w-1));
    y = std::clamp(y, 0.0f, float(h-1));
    const int x0=int(x), y0=int(y), x1=std::min(x0+1,w-1), y1=std::min(y0+1,h-1);
    const float fx=x-x0, fy=y-y0;
    const Ycca &a=v[size_t(y0)*w+x0], &b=v[size_t(y0)*w+x1],
               &c=v[size_t(y1)*w+x0], &d=v[size_t(y1)*w+x1];
    auto mix=[&](float av,float bv,float cv,float dv) {
        const float ab=av+(bv-av)*fx, cd=cv+(dv-cv)*fx;
        return ab+(cd-ab)*fy;
    };
    return {mix(a.y,b.y,c.y,d.y), mix(a.cb,b.cb,c.cb,d.cb),
            mix(a.cr,b.cr,c.cr,d.cr), mix(a.a,b.a,c.a,d.a)};
}
inline float gaussianWeight(int d, float inv2s2) noexcept {
    return std::exp(float(d*d)*inv2s2);
}
} // namespace

void applyAnalogSignalPipeline(FloatRgbaView im, const AnalogSignalParams& p) {
    if (!im.pixels || im.width <= 0 || im.height <= 0 ||
        im.rowStrideFloats < im.width * 4) return;

    const int w=im.width, h=im.height;
    const size_t n=size_t(w)*h;
    std::vector<Ycca> src(n), encoded(n), warped(n), horizontal(n), output(n);
    for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
        const float* px=im.pixels+size_t(y)*im.rowStrideFloats+x*4;
        src[size_t(y)*w+x]=rgbToYcc(px);
    }

    const float comp=std::clamp(p.compositeStrength,0.0f,1.0f);
    const uint32_t frameSeed=uint32_t(p.frameIndex)*0x9E3779B1u+uint32_t(p.seed);

    // Three-tap composite encode/decode: carrier leakage creates dot crawl and cross-colour.
    for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
        Ycca s=src[size_t(y)*w+x];
        float Y=s.y, Cb=s.cb, Cr=s.cr;
        if (comp>0.001f) {
            const float pxPerCycle=p.pal?3.05f:3.77f;
            const float lineOffset=p.pal
                ? ((p.palSwitch && (y&1))?kPi:0.0f)
                : float(y&1)*kPi;
            const float phase=2.0f*kPi*(float(x)+0.5f)/pxPerCycle+lineOffset;
            const float phaseM=phase-2.0f*kPi/pxPerCycle;
            const float phaseP=phase+2.0f*kPi/pxPerCycle;
            const Ycca sm=src[size_t(y)*w+std::max(0,x-1)];
            const Ycca sp=src[size_t(y)*w+std::min(w-1,x+1)];
            float cm=sm.y+sm.cb*std::sin(phaseM)+sm.cr*std::cos(phaseM);
            float c0=Y+Cb*std::sin(phase)+Cr*std::cos(phase);
            float cp=sp.y+sp.cb*std::sin(phaseP)+sp.cr*std::cos(phaseP);
            if (p.compositeNoise>0.001f) {
                const uint32_t h1=cellSeed(x>>1,y,frameSeed^0xBEEF1234u);
                const uint32_t h2=cellSeed((x>>1)^0xA5,y,frameSeed^0x12345678u);
                c0+=gaussian(h1,h2)*p.compositeNoise*0.15f;
            }
            const float yd=(cm+c0+cp)/3.0f, residual=c0-yd;
            const float cbDecoded=2.0f*residual*std::sin(phase);
            const float crDecoded=2.0f*residual*std::cos(phase);
            const float highLuma=Y-yd;
            Cb+=highLuma*p.dotCrawlGain*std::sin(phase)*comp;
            Cr+=highLuma*p.dotCrawlGain*std::cos(phase)*comp;
            Y=Y*(1.0f-comp)+(yd+(Y-yd)*(1.0f-p.crossColorGain))*comp;
            Cb=Cb*(1.0f-comp*0.3f)+(Cb*0.5f+cbDecoded*0.5f)*(comp*0.3f);
            Cr=Cr*(1.0f-comp*0.3f)+(Cr*0.5f+crDecoded*0.5f)*(comp*0.3f);
        }
        // Coarse magnetic noise: 2x2 luma cells and 4x4 chroma cells.
        if (p.lumaNoise>0.001f) {
            const uint32_t a=cellSeed(x>>1,y>>1,frameSeed^0xCAFEBABEu);
            const uint32_t b=cellSeed((x>>1)+17,(y>>1)-23,frameSeed^0xDEADBEEFu);
            Y+=gaussian(a,b)*p.lumaNoise*0.10f;
        }
        if (p.chromaNoise>0.001f) {
            const uint32_t a=cellSeed(x>>2,y>>2,frameSeed^0x55667788u);
            const uint32_t b=cellSeed((x>>2)+91,(y>>2)+17,frameSeed^0x99AABBCCu);
            const float g=gaussian(a,b)*p.chromaNoise*0.10f;
            const float angle=p.noiseTintHueDegrees*kPi/180.0f;
            Cb+=g*std::cos(angle);
            Cr+=g*std::sin(angle);
        }
        encoded[size_t(y)*w+x]={Y,Cb,Cr,s.a};
    }

    // Per-line timing error and top-of-frame skew; chroma registration/delay is separate from luma.
    for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
        float fx=float(x), fy=float(y);
        if (p.lineJitterPixels>0.001f) {
            const uint32_t k=cellSeed(0,y,frameSeed);
            fx+=(uniform01(k)*2.0f-1.0f)*p.lineJitterPixels;
        }
        if (p.topSkewPixels>0.001f && p.topSkewDecay>0.001f) {
            const float decay=std::exp(-float(y)/std::max(p.topSkewDecay,1.0f));
            const uint32_t k=cellSeed(7,int(p.frameIndex),frameSeed);
            fx+=((uniform01(k)>0.5f)?1.0f:-1.0f)*p.topSkewPixels*decay;
        }
        const Ycca luma=sample(encoded,w,h,fx,fy);
        const Ycca blue=sample(encoded,w,h,fx+p.chromaOffsetB,fy+p.verticalChromaDelayPixels);
        const Ycca red=sample(encoded,w,h,fx+p.chromaOffsetR,fy+p.verticalChromaDelayPixels);
        warped[size_t(y)*w+x]={luma.y,blue.cb,red.cr,luma.a};
    }

    // Separable horizontal Gaussian: luma and chroma have independent bandwidth limits.
    const float sigmaY=std::clamp(float(w)/(kPi*std::max(p.bandwidthLines,40.0f)),0.5f,60.0f);
    const float sigmaC=std::clamp(float(w)/(kPi*std::max(p.chromaBandwidth,10.0f)),0.5f,80.0f);
    const int rY=std::min(int(std::ceil(sigmaY*3.0f)),33);
    const int rC=std::min(int(std::ceil(sigmaC*3.0f)),33);
    const int radius=std::max(rY,rC);
    const float invY=-0.5f/(sigmaY*sigmaY), invC=-0.5f/(sigmaC*sigmaC);
    for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
        float sy=0,wy=0,scb=0,scr=0,wc=0;
        for (int d=-radius;d<=radius;++d) {
            const Ycca& s=warped[size_t(y)*w+std::clamp(x+d,0,w-1)];
            if (d>=-rY && d<=rY) { const float q=gaussianWeight(d,invY); sy+=s.y*q; wy+=q; }
            if (d>=-rC && d<=rC) { const float q=gaussianWeight(d,invC); scb+=s.cb*q; scr+=s.cr*q; wc+=q; }
        }
        const Ycca& mid=warped[size_t(y)*w+x];
        horizontal[size_t(y)*w+x]={wy>0?sy/wy:mid.y,wc>0?scb/wc:mid.cb,
                                   wc>0?scr/wc:mid.cr,mid.a};
    }

    // Vertical luma-only bandwidth, preserving chroma on the current scanline.
    const float sigmaRows=std::clamp(float(h)/(kPi*std::max(p.bandwidthRows,20.0f)),0.5f,60.0f);
    const int rRows=std::min(int(std::ceil(sigmaRows*3.0f)),33);
    const float invRows=-0.5f/(sigmaRows*sigmaRows);
    for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
        float sum=0,weightSum=0;
        for (int d=-rRows;d<=rRows;++d) {
            const Ycca& s=horizontal[size_t(std::clamp(y+d,0,h-1))*w+x];
            const float q=gaussianWeight(d,invRows); sum+=s.y*q; weightSum+=q;
        }
        const Ycca& mid=horizontal[size_t(y)*w+x];
        Ycca out{weightSum>0?sum/weightSum:mid.y,mid.cb,mid.cr,mid.a};
        if (p.scanlineAmount>0.001f) {
            const float phase=(float(y)+p.scanlinePhase)*p.scanlineFrequency;
            out.y*=1.0f-p.scanlineAmount*(0.5f+0.5f*std::sin(2.0f*kPi*phase));
        }
        output[size_t(y)*w+x]=out;
    }

    for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
        float* dst=im.pixels+size_t(y)*im.rowStrideFloats+x*4;
        yccToRgb(output[size_t(y)*w+x],dst);
    }
}
} // namespace vhs90
