#include "VHS90TearChromaEngine.h"
#include <algorithm>
#include <cmath>

namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
inline float clamp01(float x) noexcept { return std::max(0.0f,std::min(1.0f,x)); }
inline float clampf(float x,float a,float b) noexcept { return std::max(a,std::min(b,x)); }
inline float hashCell(int x,int y,std::uint32_t seed,std::int64_t tick) noexcept {
    return hash01(static_cast<std::uint32_t>(x),static_cast<std::uint32_t>(y),seed,tick);
}
inline float smooth(float a,float b,float x) noexcept {
    if(b<=a)return x>=b?1.0f:0.0f;
    const float t=clamp01((x-a)/(b-a));return t*t*(3.0f-2.0f*t);
}
inline float sample(ConstImageView im,float x,float y,int ch) noexcept {
    x=clampf(x,0.0f,float(im.width-1));y=clampf(y,0.0f,float(im.height-1));
    const int x0=int(std::floor(x)),y0=int(std::floor(y));
    const int x1=std::min(im.width-1,x0+1),y1=std::min(im.height-1,y0+1);
    const float tx=x-x0,ty=y-y0;
    const float a=im.rgba[y0*im.rowStrideFloats+x0*4+ch]*(1-tx)+im.rgba[y0*im.rowStrideFloats+x1*4+ch]*tx;
    const float b=im.rgba[y1*im.rowStrideFloats+x0*4+ch]*(1-tx)+im.rgba[y1*im.rowStrideFloats+x1*4+ch]*tx;
    return a+(b-a)*ty;
}
inline float regionMask(float v,float center,float height,float feather) noexcept {
    const float half=std::max(0.0f,height)*0.5f;
    const float d=std::fabs(v-center);
    if(d>=half)return 0.0f;
    if(feather<=0.0f||d<=half-feather)return 1.0f;
    return 1.0f-smooth(half-feather,half,d);
}
inline void toYiq(float r,float g,float b,float& y,float& i,float& q) noexcept {
    y=.299f*r+.587f*g+.114f*b;
    i=.596f*r-.274f*g-.322f*b;
    q=.211f*r-.523f*g+.312f*b;
}
inline void fromYiq(float y,float i,float q,float& r,float& g,float& b) noexcept {
    r=y+.956f*i+.621f*q;g=y-.272f*i-.647f*q;b=y-1.106f*i+1.703f*q;
}
}

bool processTearChroma(ConstImageView src,ImageView dst,const TearChromaParams&p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||
       dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||
       src.rgba==dst.rgba||(p.bandCount>0&&!p.bands)||p.bandCount>5) return false;
    const float master=clamp01(p.intensity),baseMix=clamp01(p.baseMix);
    const std::int64_t tick=p.animated?p.frame:0;
    for(int y=0;y<src.height;++y) {
        const float v=(float(y)+.5f)/float(src.height);
        float* dr=dst.rgba+y*dst.rowStrideFloats;
        for(int x=0;x<src.width;++x) {
            const float* sp=src.rgba+y*src.rowStrideFloats+x*4;
            const float alpha=sp[3];
            if(alpha<=0.0f||master<=0.0f) { std::copy_n(sp,4,dr+x*4); continue; }
            float rr=sp[0],gg=sp[1],bb=sp[2];
            const float u=(float(x)+.5f)/float(src.width);
            const float t=float(p.timeSeconds);
            const bool spectral=p.mode==TearChromaMode::SpectralSmear||
                                p.mode==TearChromaMode::SpectralWithLowerTear||
                                p.mode==TearChromaMode::ChromaStreakInterleave;
            if(spectral) {
                const float broad=hashCell(x/24,y/11,p.seed+0x51ed270bu,tick);
                const float fine=hashCell(x/5,y/3,p.seed+0x94d049bbu,tick);
                const float wave=std::sin(v*7.0f+t*1.3f+u*2.1f)*.5f+.5f;
                const float irregular=clamp01((broad*.55f+fine*.25f+wave*.20f-.5f)*p.spectralIrregularity+0.5f);
                const float vertical=std::fabs(v-p.spectralCenterY);
                float mask=1.0f-smooth(p.spectralCoverage*.5f,p.spectralCoverage*.5f+.16f,vertical*2.0f);
                const float horizontalBias=1.0f-std::fabs(u-p.spectralHorizontalBias)*.55f;
                mask*=clamp01(horizontalBias*(.55f+.45f*irregular));
                if(p.mode==TearChromaMode::ChromaStreakInterleave) {
                    const float line=std::fmod(float(y)+float(x)*.18f,4.0f);
                    mask*=line<2.0f?1.0f:.38f;
                }
                const float spectralAmount=clamp01(mask*p.spectralIntensity*master);
                if(spectralAmount>0.0f) {
                    const float sx=float(x)+(fine-.5f)*2.0f*p.spectralSmearX;
                    const float sy=float(y)+(broad-.5f)*2.0f*p.spectralSmearY;
                    float sr=sample(src,sx-p.spectralSmearX*.35f,sy,0);
                    float sg=sample(src,sx,sy,1);
                    float sb=sample(src,sx+p.spectralSmearX*.35f,sy,2);
                    float ly,ii,qq;toYiq(sr,sg,sb,ly,ii,qq);
                    const float hue=(irregular-.5f)*p.spectralHueSpread*2.0f*kPi;
                    const float hc=std::cos(hue),hs=std::sin(hue);
                    const float ri=(ii*hc-qq*hs)*p.chromaSaturation;
                    const float rq=(ii*hs+qq*hc)*p.chromaSaturation;
                    float cr,cg,cb;fromYiq(ly,ri,rq,cr,cg,cb);
                    rr+=(cr-rr)*spectralAmount;gg+=(cg-gg)*spectralAmount;bb+=(cb-bb)*spectralAmount;
                }
            }
            const bool bandsEnabled=p.mode!=TearChromaMode::SpectralSmear;
            for(std::size_t bi=0;bandsEnabled&&bi<p.bandCount;++bi) {
                const TearBand& band=p.bands[bi];if(!band.enabled||band.intensity<=0.0f)continue;
                const float cy=band.centerY+band.motionYPerSecond*t;
                const float cx=band.centerX+band.motionXPerSecond*t;
                const float halfH=std::max(0.0f,band.height)*.5f;
                const float dy=std::fabs(v-cy);
                float mask=regionMask(v,cy,band.height,std::max(.001f,band.height*.12f));
                if(mask<=0.0f||std::fabs(u-cx)>band.width*.5f)continue;
                const std::uint32_t seed=p.seed^band.seedOffset^std::uint32_t(bi*0x9e3779b9u);
                const int seg=std::max(4,int(src.width/48));
                const float segEvent=hashCell(x/seg,y/2,seed+0x243f6a88u,tick);
                if(segEvent<clamp01(band.fragmentation)*.24f)mask*=.12f;
                const float rowNoise=hashCell(0,y/2,seed+0x85ebca6bu,tick)-.5f;
                const float wave=std::sin((v-cy)*std::max(1.0f,band.waveFrequency)*2.0f*kPi+t*2.0f);
                const float jump=hashCell(x/seg,y,seed+0xc2b2ae35u,tick)<clamp01(band.fragmentation)*.18f
                    ? (hashCell(x/seg+7,y+9,seed+0x165667b1u,tick)-.5f)*band.horizontalShiftPx*2.0f:0.0f;
                const float shift=rowNoise*band.horizontalShiftPx+wave*band.waveAmplitudePx+jump;
                const float sx=float(x)+shift;
                const float smear=std::max(0.0f,band.smearPx);
                float sr=sample(src,sx-smear*.5f,y,0),sg=sample(src,sx,y,1),sb=sample(src,sx+smear*.5f,y,2);
                if(std::fabs(band.chromaShiftPx)>1.0e-5f) {
                    sr=sample(src,sx-band.chromaShiftPx,y,0);
                    sb=sample(src,sx+band.chromaShiftPx,y,2);
                }
                const float drop=hashCell(x/seg,y/2,seed+0x27d4eb2fu,tick);
                if(drop<clamp01(band.dropoutProbability)) {
                    const float dark=hashCell(x/seg+3,y+5,seed+0x68bc21ebu,tick);
                    const float loss=dark<.55f?0.08f:1.15f;
                    sr=sr*loss;sg=sg*loss;sb=sb*loss;
                }
                const float flicker=1.0f-clamp01(band.flicker)*hashCell(0,y,seed+0x51ed270bu,tick);
                const float amount=clamp01(mask*band.intensity*master*flicker);
                rr+=(sr-rr)*amount;gg+=(sg-gg)*amount;bb+=(sb-bb)*amount;
                (void)halfH;
            }
            const float mix=baseMix;
            float* dp=dr+x*4;
            dp[0]=sp[0]+(rr-sp[0])*mix;dp[1]=sp[1]+(gg-sp[1])*mix;dp[2]=sp[2]+(bb-sp[2])*mix;dp[3]=alpha;
        }
    }
    return true;
}

} // namespace vhs90
