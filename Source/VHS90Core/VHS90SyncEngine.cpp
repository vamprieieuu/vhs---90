#include "VHS90SyncEngine.h"
#include <algorithm>
#include <cmath>
namespace vhs90 {
namespace {
inline float clamp01(float v) noexcept { return std::max(0.0f,std::min(1.0f,v)); }
inline float smooth01(float t) noexcept { t=clamp01(t); return t*t*(3.0f-2.0f*t); }
inline float smoothStep(float a,float b,float x) noexcept { return a==b?(x<a?0.0f:1.0f):smooth01((x-a)/(b-a)); }
inline float lerp(float a,float b,float t) noexcept { return a+(b-a)*t; }
inline float hash2(int x,int y,std::uint32_t seed) noexcept { return hash01(std::uint32_t(x),std::uint32_t(y),seed,0); }
inline float valueNoise(float x,float y,std::uint32_t seed) noexcept {
    const int x0=int(std::floor(x)),y0=int(std::floor(y));
    const float tx=smooth01(x-float(x0)),ty=smooth01(y-float(y0));
    const float a=lerp(hash2(x0,y0,seed),hash2(x0+1,y0,seed),tx);
    const float b=lerp(hash2(x0,y0+1,seed),hash2(x0+1,y0+1,seed),tx);
    return lerp(a,b,ty);
}
inline float sample(ConstImageView src,float x,float y,int ch) noexcept {
    x=std::max(0.0f,std::min(float(src.width-1),x)); y=std::max(0.0f,std::min(float(src.height-1),y));
    const int x0=int(std::floor(x)),y0=int(std::floor(y)),x1=std::min(src.width-1,x0+1),y1=std::min(src.height-1,y0+1);
    const float tx=x-x0,ty=y-y0;
    const float* p00=src.rgba+y0*src.rowStrideFloats+x0*4;
    const float* p10=src.rgba+y0*src.rowStrideFloats+x1*4;
    const float* p01=src.rgba+y1*src.rowStrideFloats+x0*4;
    const float* p11=src.rgba+y1*src.rowStrideFloats+x1*4;
    return lerp(lerp(p00[ch],p10[ch],tx),lerp(p01[ch],p11[ch],tx),ty);
}
inline float circularDistance(float a,float b) noexcept { const float d=std::fabs(a-b); return std::min(d,1.0f-d); }
inline float wrapY(float y,int height) noexcept {
    const float h=static_cast<float>(height);
    y=std::fmod(y,h);
    return y<0.0f?y+h:y;
}
}
bool processSyncCollapse(ConstImageView src,ImageView dst,const SyncCollapseParams& p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||src.rgba==dst.rgba||
       !std::isfinite(p.timeSeconds)||!std::isfinite(p.strength)) return false;
    const float strength=clamp01(p.strength);
    if(strength<=0.0f) {
        for(int y=0;y<src.height;++y) std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
        return true;
    }
    const float time=float(p.timeSeconds);
    const float glitchPhase=time*std::max(0.0f,p.eventRateHz),glitchTickF=std::floor(glitchPhase);
    const std::int64_t glitchTick=static_cast<std::int64_t>(glitchTickF);
    const float glitchAge=glitchPhase-glitchTickF;
    const bool glitchEvent=hash2(int(glitchTick),19,p.seed+0x9e3779b9u)>1.0f-clamp01(p.eventFrequency);
    const float attack=std::max(0.0001f,p.eventAttackFraction);
    const float releaseStart=std::max(attack,p.eventReleaseStartFraction);
    const float releaseEnd=std::max(releaseStart+0.0001f,p.eventReleaseEndFraction);
    const float envelope=smoothStep(0.0f,attack,glitchAge)*(1.0f-smoothStep(releaseStart,releaseEnd,glitchAge));
    const float active=glitchEvent?envelope:0.0f;
    const float tearCenter=hash2(int(glitchTick),43,p.seed+0x85ebca6bu);
    const float tearWidth=lerp(p.tearWidthMin,p.tearWidthMax,hash2(int(glitchTick),71,p.seed+0xc2b2ae35u));
    const float secondCenter=std::fmod(tearCenter+p.secondaryTearOffsetMin+
        (p.secondaryTearOffsetMax-p.secondaryTearOffsetMin)*hash2(int(glitchTick),11,p.seed+0x165667b1u),1.0f);
    const float direction=hash2(int(glitchTick),9,p.seed+0x27d4eb2fu)*2.0f-1.0f;
    const float syncPhase=time*std::max(0.0f,p.syncEventRateHz),syncTickF=std::floor(syncPhase);
    const int syncTick=static_cast<int>(syncTickF);
    const float syncProgress=clamp01((syncPhase-syncTickF)/std::max(0.001f,p.syncEventWindowFraction));
    const bool syncWindow=(syncPhase-syncTickF)<clamp01(p.syncEventWindowFraction);
    const bool syncEvent=syncWindow&&hash2(syncTick,241,p.seed+0x51ed270bu)>1.0f-clamp01(p.syncEventProbability);
    const bool rollChoice=hash2(syncTick,92,p.seed+0x94d049bbu)>0.52f;
    const float roll=syncEvent&&rollChoice&&p.enableVerticalRoll?(0.10f+syncProgress*0.90f)*p.syncRollStrength:0.0f;
    const float trackingY=std::fmod(time*p.trackingLineSpeed+p.trackingLineWobble*std::sin(time*0.19f)+1.0f,1.0f);
    for(int y=0;y<src.height;++y) {
        const float v=(float(y)+0.5f)/float(src.height),signalLine=v*float(src.height);
        const float slow=valueNoise(signalLine*0.018f,time*0.82f,p.seed+0x68bc21ebu)*2.0f-1.0f;
        const float fast=valueNoise(signalLine*0.095f+17.0f,time*3.70f,p.seed+0xd3a2646cu)*2.0f-1.0f;
        const float lineTimebase=slow*0.76f+fast*0.24f;
        const float primary=1.0f-smoothStep(tearWidth,tearWidth*2.4f,circularDistance(v,tearCenter));
        const float secondary=(1.0f-smoothStep(tearWidth*0.45f,tearWidth*1.45f,circularDistance(v,secondCenter)))*p.secondaryTearStrength;
        const float tearShape=std::max(primary,secondary);
        const float textureNoise=valueNoise(signalLine*0.055f,time*5.1f+glitchTickF,p.seed+0x243f6a88u);
        const float tearTexture=1.0f-clamp01(p.lineTextureAmount)*(1.0f-textureNoise);
        const float glitchBand=active*tearShape;
        const float trackingDistance=circularDistance(v,trackingY);
        const float trackingBand=p.enableTrackingLine?std::pow(std::max(0.0f,1.0f-trackingDistance*42.0f),3.0f):0.0f;
        const float rowShift=lineTimebase*(p.slowTimebasePx*0.76f+p.fastTimebasePx*0.24f)
            +glitchBand*(direction+lineTimebase*0.24f)*p.displacementPx*tearTexture;
        const float rollPixels=roll*float(src.height);
        for(int x=0;x<src.width;++x) {
            const float alpha=src.rgba[y*src.rowStrideFloats+x*4+3];
            const float* original=src.rgba+y*src.rowStrideFloats+x*4;
            float* out=dst.rgba+y*dst.rowStrideFloats+x*4;
            if(alpha<=0.0f) { out[0]=original[0]; out[1]=original[1]; out[2]=original[2]; out[3]=alpha; continue; }
            // Sync collapse is timing/geometry failure, not a full-frame grain layer.
            // Noise is confined to the actual tear band below.
            const float trackingPulse=trackingBand*std::sin(time*17.0f)*p.trackingLineStrength;
            const float sx=float(x)+rowShift+trackingPulse;
            // A vertical sync roll is a raster wrap, not a clamped edge smear.
            const float sy=rollPixels>0.0f?wrapY(float(y)+rollPixels,src.height):float(y);
            float r=sample(src,sx,sy,0),g=sample(src,sx,sy,1),b=sample(src,sx,sy,2);
            const float sourceA=sample(src,sx,sy,3);
            const float bandDamage=glitchBand;
            const float noise=bandDamage*(hash01(std::uint32_t(x/3),std::uint32_t(y),p.seed+31u,p.frame)-0.5f)*clamp01(p.noiseAmount);
            const float lumaGain=1.0f-bandDamage*0.55f-trackingBand*p.trackingLineStrength;
            if(p.premultipliedAlpha) {
                if(sourceA>1.0e-6f) { r/=sourceA; g/=sourceA; b/=sourceA; }
                else { r=0.0f; g=0.0f; b=0.0f; }
            }
            r=r*lumaGain+noise; g=g*lumaGain+noise; b=b*lumaGain+noise;
            if(p.premultipliedAlpha) { r*=alpha; g*=alpha; b*=alpha; }
            out[0]=original[0]+(r-original[0])*strength;
            out[1]=original[1]+(g-original[1])*strength;
            out[2]=original[2]+(b-original[2])*strength;
            out[3]=alpha;
        }
    }
    return true;
}
} // namespace vhs90
