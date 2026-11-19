#include "VHS90SignalKernels.h"
#include <algorithm>
#include <cmath>

namespace vhs90 {
namespace {
constexpr float kLumaR = 0.299f, kLumaG = 0.587f, kLumaB = 0.114f;
inline float clamp01(float x) noexcept { return std::max(0.0f, std::min(1.0f, x)); }
inline float smooth(float a, float b, float x) noexcept {
    if (a == b) return x < a ? 0.0f : 1.0f;
    float t = clamp01((x-a)/(b-a)); return t*t*(3.0f-2.0f*t);
}
inline RGB readRgb(ConstImageView im, int x, int y) noexcept {
    x=std::max(0,std::min(im.width-1,x)); y=std::max(0,std::min(im.height-1,y));
    const float* p=im.rgba + y*im.rowStrideFloats + x*4;
    return {p[0],p[1],p[2]};
}
inline float readAlpha(ConstImageView im,int x,int y) noexcept {
    const float* p=im.rgba + y*im.rowStrideFloats + x*4; return p[3];
}
inline void write(ImageView im,int x,int y,RGB c,float a) noexcept {
    float* p=im.rgba + y*im.rowStrideFloats + x*4;
    p[0]=c.r; p[1]=c.g; p[2]=c.b; p[3]=a;
}
inline RGB mix(RGB a,RGB b,float t) noexcept {
    return {a.r+(b.r-a.r)*t,a.g+(b.g-a.g)*t,a.b+(b.b-a.b)*t};
}
inline float luma(RGB c) noexcept { return c.r*kLumaR+c.g*kLumaG+c.b*kLumaB; }
inline float sampleChannel(ConstImageView im,float x,int y,int ch) noexcept {
    x=std::max(0.0f,std::min(float(im.width-1),x));
    int x0=int(std::floor(x)), x1=std::min(im.width-1,x0+1); float t=x-x0;
    const float* a=im.rgba+y*im.rowStrideFloats+x0*4;
    const float* b=im.rgba+y*im.rowStrideFloats+x1*4;
    return a[ch]+(b[ch]-a[ch])*t;
}
inline float rowNoise(int y,const SignalParams& p) noexcept {
    const float correlation = std::max(0.0f, p.lineWobbleCorrelationRows);
    if (correlation <= 1.0e-4f)
        return hash01(0u,std::uint32_t(y),p.seed,p.frame);
    return valueNoise1D(float(y) / correlation, p.seed, p.frame);
}
}

YIQ rgbToYiq(RGB c) noexcept {
    return {0.2990f*c.r+0.5870f*c.g+0.1140f*c.b,
            0.5959f*c.r-0.2746f*c.g-0.3213f*c.b,
            0.2115f*c.r-0.5227f*c.g+0.3112f*c.b};
}
RGB yiqToRgb(YIQ c) noexcept {
    return {c.y+0.9560f*c.i+0.6190f*c.q,
            c.y-0.2720f*c.i-0.6470f*c.q,
            c.y-1.1060f*c.i+1.7030f*c.q};
}
std::uint32_t hash32(std::uint32_t x) noexcept {
    x ^= x >> 16; x *= 0x7feb352du; x ^= x >> 15; x *= 0x846ca68bu; x ^= x >> 16; return x;
}
float hash01(std::uint32_t x,std::uint32_t y,std::uint32_t seed,std::int64_t frame) noexcept {
    std::uint32_t f=std::uint32_t(frame) ^ std::uint32_t(std::uint64_t(frame)>>32);
    std::uint32_t h=hash32(x*0x9e3779b9u ^ y*0x85ebca6bu ^ seed ^ f*0xc2b2ae35u);
    return float(h) / 4294967295.0f;
}

float valueNoise1D(float coordinate,std::uint32_t seed,std::int64_t frame) noexcept {
    if (!std::isfinite(coordinate)) return 0.5f;
    const float x = std::max(-1.0e7f, std::min(1.0e7f, coordinate));
    const int cell = static_cast<int>(std::floor(x));
    const float t = x - static_cast<float>(cell);
    const float s = t*t*(3.0f-2.0f*t);
    const float a = hash01(0u,static_cast<std::uint32_t>(cell),seed,frame);
    const float b = hash01(0u,static_cast<std::uint32_t>(cell+1),seed,frame);
    return a+(b-a)*s;
}

bool processYiqChromaBlur(ConstImageView src,ImageView dst,float shift,float bleed,float saturation) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4) return false;
    for(int y=0;y<src.height;++y) for(int x=0;x<src.width;++x) {
        RGB center=readRgb(src,x,y); float Y=luma(center), ci=0,cq=0;
        // Five horizontal taps: low-frequency chroma, full-resolution luma.
        for(int k=-2;k<=2;++k) {
            float dx=float(k)*bleed+shift;
            RGB s{sampleChannel(src,x+dx,y,0),sampleChannel(src,x+dx,y,1),sampleChannel(src,x+dx,y,2)};
            YIQ q=rgbToYiq(s); ci+=q.i*0.2f; cq+=q.q*0.2f;
        }
        RGB out=yiqToRgb({Y,ci*saturation,cq*saturation});
        write(dst,x,y,out,readAlpha(src,x,y));
    }
    return true;
}

bool processLumaSmear(ConstImageView src,ImageView dst,const LumaSmearParams& p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||
       dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||
       src.rgba==dst.rgba||!std::isfinite(p.strength)||
       !std::isfinite(p.decay)||!std::isfinite(p.offsetPx)||
       !std::isfinite(p.highlightThreshold)||!std::isfinite(p.highlightStrength)||
       p.strength<0.0f||p.decay<0.0f||p.offsetPx<0.0f||p.highlightStrength<0.0f)
        return false;
    const float amount=std::min(1.0f,p.strength);
    if(amount<=0.0f) {
        for(int y=0;y<src.height;++y)
            std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
        return true;
    }
    const int taps=std::max(2,std::min(64,p.tapCount));
    float weights[64]{};
    float weightSum=0.0f;
    for(int tap=0;tap<taps;++tap) {
        const float phase=float(tap)/float(taps-1);
        weights[tap]=std::exp(-p.decay*phase*3.2f);
        weightSum+=weights[tap];
    }
    if(weightSum<=1.0e-8f) return false;
    for(int y=0;y<src.height;++y) for(int x=0;x<src.width;++x) {
        const float alpha=sampleChannel(src,float(x),y,3);
        if(alpha<=1.0e-6f) { write(dst,x,y,{0.0f,0.0f,0.0f},alpha); continue; }
        auto straightLumaAt=[&](float sx) noexcept {
            RGB c{sampleChannel(src,sx,y,0),sampleChannel(src,sx,y,1),sampleChannel(src,sx,y,2)};
            const float a=sampleChannel(src,sx,y,3);
            if(p.premultipliedAlpha) {
                if(a>1.0e-6f) { c.r/=a; c.g/=a; c.b/=a; }
                else return 0.0f;
            }
            return luma(c);
        };
        const float center=straightLumaAt(float(x));
        float smear=0.0f,glow=0.0f;
        for(int tap=0;tap<taps;++tap) {
            const float sample=straightLumaAt(float(x)-p.offsetPx*float(tap));
            const float w=weights[tap];
            smear+=sample*w;
            glow+=std::max(0.0f,sample-p.highlightThreshold)*w;
        }
        smear/=weightSum;
        glow/=weightSum;
        const float trailMix=amount*0.18f;
        const float outY=center*(1.0f-trailMix)+smear*trailMix
                        +glow*(amount*p.highlightStrength);
        RGB centerRgb{sampleChannel(src,float(x),y,0),
                       sampleChannel(src,float(x),y,1),
                       sampleChannel(src,float(x),y,2)};
        if(p.premultipliedAlpha) {
            centerRgb.r/=alpha; centerRgb.g/=alpha; centerRgb.b/=alpha;
        }
        const YIQ centerYiq=rgbToYiq(centerRgb);
        RGB out=yiqToRgb({outY,centerYiq.i,centerYiq.q});
        if(p.premultipliedAlpha) { out.r*=alpha; out.g*=alpha; out.b*=alpha; }
        write(dst,x,y,out,alpha);
    }
    return true;
}

bool processCompositeVhs(ConstImageView src,ImageView dst,const SignalParams& p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4) return false;
    const float invH=1.0f/float(src.height);
    const float framePulse=1.0f-p.flicker*hash01(0u,1u,p.seed,p.frame);
    for(int y=0;y<src.height;++y) {
        const float row=float(y)*invH;
        const float lineHash=rowNoise(y,p);
        const float baseWobble=(lineHash-0.5f)*p.lineWobblePx;
        const float bandDist=std::fabs(row-p.trackingCenter);
        const float band=1.0f-smooth(std::max(0.0f,p.trackingHeight*0.5f),p.trackingHeight*0.5f+invH,bandDist);
        const float headDist=std::fabs(row-p.headSwitchCenterY);
        const float head=1.0f-smooth(std::max(0.0f,p.headSwitchHeight*0.5f),p.headSwitchHeight*0.5f+invH,headDist);
        const float roll=std::sin((row*p.trackingSpeed*8.0f+float(p.frame)*0.035f)*6.2831853f);
        for(int x=0;x<src.width;++x) {
            float n=hash01(std::uint32_t(x),std::uint32_t(y),p.seed,p.frame);
            float displacement=baseWobble
                + band*p.trackingAmount*(n-0.3f)*40.0f
                + head*(0.5f+n*0.5f)*30.0f
                + band*roll*p.trackingAmount*4.0f;
            float sx=float(x)+displacement;
            RGB c{sampleChannel(src,sx,y,0),sampleChannel(src,sx,y,1),sampleChannel(src,sx,y,2)};
            float Y=luma(c), ci=0.0f,cq=0.0f;
            for(int k=-2;k<=2;++k) {
                float dx=float(k)*p.chromaBleedPx+p.chromaShiftPx;
                RGB s{sampleChannel(src,sx+dx,y,0),sampleChannel(src,sx+dx,y,1),sampleChannel(src,sx+dx,y,2)};
                YIQ q=rgbToYiq(s); ci+=q.i*0.2f; cq+=q.q*0.2f;
            }
            RGB out=yiqToRgb({Y,ci*p.chromaSaturation,cq*p.chromaSaturation});
            float grain=(n-0.5f)*p.noiseAmount;
            float streak=(n>1.0f-p.staticLineAmount*0.015f && hash01(std::uint32_t(x/8),std::uint32_t(y),p.seed+17u,p.frame)>0.4f)?0.6f:0.0f;
            float local=(band*p.trackingAmount+head)*(hash01(std::uint32_t(x*3),std::uint32_t(y),p.seed+31u,p.frame)-0.4f)*0.5f;
            out.r=(out.r+grain+streak+local)*framePulse;
            out.g=(out.g+grain+streak+local)*framePulse;
            out.b=(out.b+grain+streak+local)*framePulse;
            RGB original=readRgb(src,x,y);
            write(dst,x,y,mix(original,{clamp01(out.r),clamp01(out.g),clamp01(out.b)},clamp01(p.intensity)),readAlpha(src,x,y));
        }
    }
    return true;
}
} // namespace vhs90
