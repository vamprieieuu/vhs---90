#include "VHS90SignalKernels.h"
#include <algorithm>
#include <cmath>
#include <vector>

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
    // Exact inverse of rgbToYiq() above. Rounded coefficients rotate chroma
    // slightly on every round-trip and break hue-preserving quantization.
    return {c.y+0.95605023f*c.i+0.62075494f*c.q,
            c.y-0.27205234f*c.i-0.64720571f*c.q,
            c.y-1.10670432f*c.i+1.70442128f*c.q};
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

bool processChromaSubsample(ConstImageView src,ImageView dst,
                            const ChromaSubsampleParams&p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||
       dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||
       src.rgba==dst.rgba||!std::isfinite(p.amount)||!std::isfinite(p.blurRadius)||
       !std::isfinite(p.iOffsetPixels)||!std::isfinite(p.qOffsetPixels)||
       p.amount<0.0f||p.blurRadius<0.0f) return false;
    const float amount=std::min(1.0f,p.amount);
    const int sx=std::max(1,std::min(8,p.horizontalFactor));
    const int sy=std::max(1,std::min(8,p.verticalFactor));
    const int w=src.width,h=src.height,cw=(w+sx-1)/sx,ch=(h+sy-1)/sy;
    if(amount<=0.0f || (sx==1&&sy==1&&p.blurRadius<=0.0f&&
       p.iOffsetPixels==0.0f&&p.qOffsetPixels==0.0f)) {
        for(int y=0;y<h;++y) std::copy_n(src.rgba+y*src.rowStrideFloats,w*4,dst.rgba+y*dst.rowStrideFloats);
        return true;
    }
    try {
        std::vector<float> lowI(std::size_t(cw)*ch,0.0f),lowQ(lowI.size(),0.0f);
        for(int cy=0;cy<ch;++cy) for(int cx=0;cx<cw;++cx) {
            float si=0.0f,sq=0.0f,sa=0.0f;
            for(int yy=cy*sy;yy<std::min(h,(cy+1)*sy);++yy) {
                const float* row=src.rgba+yy*src.rowStrideFloats;
                for(int xx=cx*sx;xx<std::min(w,(cx+1)*sx);++xx) {
                    const float* px=row+xx*4; const float a=std::max(0.0f,px[3]);
                    if(a<=1.0e-6f) continue;
                    RGB rgb{px[0],px[1],px[2]};
                    if(p.premultipliedAlpha) { rgb.r/=a;rgb.g/=a;rgb.b/=a; }
                    const YIQ yiq=rgbToYiq(rgb);
                    si+=yiq.i*a; sq+=yiq.q*a; sa+=a;
                }
            }
            const std::size_t k=std::size_t(cy)*cw+cx;
            if(sa>1.0e-6f) { lowI[k]=si/sa;lowQ[k]=sq/sa; }
        }
        const int radius=std::clamp(int(std::ceil(p.blurRadius)),0,24);
        if(radius>0) {
            const float sigma=std::max(0.5f,p.blurRadius/float(sx));
            std::vector<float> weights(std::size_t(radius*2+1));
            float total=0.0f;
            for(int d=-radius;d<=radius;++d) { const float q=std::exp(-float(d*d)/(2.0f*sigma*sigma));weights[std::size_t(d+radius)]=q;total+=q; }
            for(float&v:weights)v/=total;
            std::vector<float> blurI(lowI.size()),blurQ(lowQ.size());
            for(int cy=0;cy<ch;++cy) for(int cx=0;cx<cw;++cx) {
                float si=0.0f,sq=0.0f;
                for(int d=-radius;d<=radius;++d) {
                    const int xx=std::clamp(cx+d,0,cw-1);const float q=weights[std::size_t(d+radius)];
                    si+=lowI[std::size_t(cy)*cw+xx]*q;sq+=lowQ[std::size_t(cy)*cw+xx]*q;
                }
                blurI[std::size_t(cy)*cw+cx]=si;blurQ[std::size_t(cy)*cw+cx]=sq;
            }
            lowI.swap(blurI);lowQ.swap(blurQ);
        }
        auto samplePlane=[&](const std::vector<float>&plane,float x,float y) noexcept {
            x=std::clamp(x,0.0f,float(cw-1));y=std::clamp(y,0.0f,float(ch-1));
            const int x0=int(x),y0=int(y),x1=std::min(x0+1,cw-1),y1=std::min(y0+1,ch-1);
            const float fx=x-x0,fy=y-y0;
            const float a=plane[std::size_t(y0)*cw+x0]+(plane[std::size_t(y0)*cw+x1]-plane[std::size_t(y0)*cw+x0])*fx;
            const float b=plane[std::size_t(y1)*cw+x0]+(plane[std::size_t(y1)*cw+x1]-plane[std::size_t(y1)*cw+x0])*fx;
            return a+(b-a)*fy;
        };
        for(int y=0;y<h;++y) {
            const float* sr=src.rgba+y*src.rowStrideFloats;float* dr=dst.rgba+y*dst.rowStrideFloats;
            for(int x=0;x<w;++x) {
                const float* px=sr+x*4;float* out=dr+x*4;const float alpha=px[3];
                if(alpha<=1.0e-6f) { std::copy_n(px,4,out);continue; }
                RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=alpha;rgb.g/=alpha;rgb.b/=alpha;}
                const YIQ original=rgbToYiq(rgb);
                const float ii=samplePlane(lowI,float(x)/sx-p.iOffsetPixels/float(sx),float(y)/sy);
                const float qq=samplePlane(lowQ,float(x)/sx-p.qOffsetPixels/float(sx),float(y)/sy);
                RGB result=yiqToRgb({original.y,original.i+(ii-original.i)*amount,original.q+(qq-original.q)*amount});
                if(p.premultipliedAlpha){result.r*=alpha;result.g*=alpha;result.b*=alpha;}
                out[0]=result.r;out[1]=result.g;out[2]=result.b;out[3]=alpha;
            }
        }
        return true;
    } catch(...) { return false; }
}

bool processVerticalChromaBlend(ConstImageView src,ImageView dst,
                                const VerticalChromaBlendParams& p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||
       dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||
       src.rgba==dst.rgba||!std::isfinite(p.amount)||p.amount<0.0f||
       !std::isfinite(p.delayRows)||p.delayRows<0.0f)
        return false;
    const float amount=std::min(1.0f,p.amount);
    for(int y=0;y<src.height;++y) {
        const float* srcRow=src.rgba+y*src.rowStrideFloats;
        float* dstRow=dst.rgba+y*dst.rowStrideFloats;
        const float delayedY=float(y)-p.delayRows;
        const int y0=static_cast<int>(std::floor(delayedY));
        const float fy=delayedY-float(y0);
        const bool hasDelayed=y0>=0&&y0<src.height;
        const int y1=std::min(src.height-1,y0+1);
        const float* row0=hasDelayed?src.rgba+y0*src.rowStrideFloats:nullptr;
        const float* row1=hasDelayed?src.rgba+y1*src.rowStrideFloats:nullptr;
        for(int x=0;x<src.width;++x) {
            const float* px=srcRow+x*4;float* out=dstRow+x*4;const float alpha=px[3];
            if(amount<=0.0f||!hasDelayed||alpha<=1.0e-6f) { std::copy_n(px,4,out);continue; }
            const float* p0=row0+x*4;const float* p1=row1+x*4;
            const float a0=p0[3],a1=p1[3];
            RGB current{px[0],px[1],px[2]},c0{p0[0],p0[1],p0[2]},c1{p1[0],p1[1],p1[2]};
            if(p.premultipliedAlpha) {
                current.r/=alpha;current.g/=alpha;current.b/=alpha;
                if(a0>1.0e-6f){c0.r/=a0;c0.g/=a0;c0.b/=a0;}else c0=current;
                if(a1>1.0e-6f){c1.r/=a1;c1.g/=a1;c1.b/=a1;}else c1=current;
            }
            const YIQ cur=rgbToYiq(current),q0=rgbToYiq(c0),q1=rgbToYiq(c1);
            const float di=q0.i+(q1.i-q0.i)*fy,dq=q0.q+(q1.q-q0.q)*fy;
            RGB result=yiqToRgb({cur.y,cur.i+(di-cur.i)*amount,cur.q+(dq-cur.q)*amount});
            if(p.premultipliedAlpha){result.r*=alpha;result.g*=alpha;result.b*=alpha;}
            out[0]=result.r;out[1]=result.g;out[2]=result.b;out[3]=alpha;
        }
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
