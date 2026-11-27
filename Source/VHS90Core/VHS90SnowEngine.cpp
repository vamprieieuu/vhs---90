#include "VHS90SnowEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace vhs90 {
namespace {
inline float clamp01(float v) noexcept { return std::clamp(v,0.0f,1.0f); }
inline float lerp(float a,float b,float t) noexcept { return a+(b-a)*t; }
inline float smooth01(float v) noexcept { v=clamp01(v); return v*v*(3.0f-2.0f*v); }
inline float hash(int x,int y,std::uint32_t seed,std::int64_t tick) noexcept {
    return hash01(static_cast<std::uint32_t>(x),static_cast<std::uint32_t>(y),seed,tick);
}
}

bool processSnow(ConstImageView src,ImageView dst,const SnowParams& p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||src.rgba==dst.rgba||
       !std::isfinite(p.timeSeconds)||!std::isfinite(p.intensity)||!std::isfinite(p.density)||
       !std::isfinite(p.sizePx)||!std::isfinite(p.distribution)||!std::isfinite(p.contrast)||
       !std::isfinite(p.brightness)||!std::isfinite(p.color)||!std::isfinite(p.temporalVariation)||
       !std::isfinite(p.randomness)||!std::isfinite(p.animationSpeed)||
       !std::isfinite(p.backgroundNoiseIntensity)) return false;

    const float snowIntensity=p.enabled?clamp01(p.intensity):0.0f;
    const float background=clamp01(p.backgroundNoiseIntensity);
    if(snowIntensity<=0.0f&&background<=0.0f) {
        for(int y=0;y<src.height;++y)
            std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
        return true;
    }

    const float density=clamp01(p.density),distribution=clamp01(p.distribution);
    const float contrast=clamp01(p.contrast),brightness=clamp01(p.brightness);
    const float colorStrength=p.colored?clamp01(p.color):0.0f;
    const float temporal=clamp01(p.temporalVariation),randomness=clamp01(p.randomness);
    const float sizePx=std::clamp(p.sizePx,0.35f,16.0f);
    const float cellSize=std::max(2.0f,sizePx*3.0f);
    const double time=std::max(0.0,p.timeSeconds)*double(std::max(0.0f,p.animationSpeed));
    const std::int64_t tick=temporal>0.0f?static_cast<std::int64_t>(std::floor(time*(1.0+30.0*temporal))):0;
    const float backgroundAmplitude=background*0.085f;

    for(int y=0;y<src.height;++y) {
        const int cellY=int(float(y)/cellSize);
        const float localY=float(y)-float(cellY)*cellSize;
        for(int x=0;x<src.width;++x) {
            const float* input=src.rgba+y*src.rowStrideFloats+x*4;
            float* output=dst.rgba+y*dst.rowStrideFloats+x*4;
            const float alpha=input[3];
            if(alpha<=0.0f) { std::copy_n(input,4,output); continue; }

            float r=input[0],g=input[1],b=input[2];
            if(p.premultipliedAlpha&&alpha>1.0e-6f) { r/=alpha;g/=alpha;b/=alpha; }
            const float backgroundNoise=(hash(x,y,p.seed+0x9e3779b9u,tick)-0.5f)*2.0f*backgroundAmplitude;
            r+=backgroundNoise; g+=backgroundNoise; b+=backgroundNoise;

            if(snowIntensity>0.0f) {
                const int cellX=int(float(x)/cellSize);
                const float localX=float(x)-float(cellX)*cellSize;
                const float particleRand=hash(cellX,cellY,p.seed+0x85ebca6bu,tick);
                const float cluster=hash(cellX/4,cellY/4,p.seed+0xc2b2ae35u,tick);
                const float localDensity=clamp01(density*lerp(1.0f,0.25f+1.5f*cluster,distribution));
                const float centerJitterX=lerp(cellSize*0.5f,hash(cellX,cellY,p.seed+0x27d4eb2fu,tick)*cellSize,randomness);
                const float centerJitterY=lerp(cellSize*0.5f,hash(cellX,cellY,p.seed+0x165667b1u,tick)*cellSize,randomness);
                const float radiusRand=lerp(0.5f,hash(cellX,cellY,p.seed+0x94d049bbu,tick),randomness);
                const float radius=sizePx*(0.22f+0.58f*radiusRand);
                const float dx=localX-centerJitterX,dy=localY-centerJitterY;
                const float distance=std::sqrt(dx*dx+dy*dy);
                const float particleMask=particleRand<localDensity
                    ?1.0f-smooth01((distance-radius*0.72f)/std::max(0.01f,radius*0.28f)):0.0f;
                const float snowMask=particleMask*snowIntensity;
                if(snowMask>0.0f) {
                    const float valueRand=lerp(0.5f,hash(cellX+11,cellY+7,p.seed+0x51ed270bu,tick),randomness);
                    const float snowValue=clamp01(0.5f+(valueRand-0.5f)*2.0f*contrast+(brightness-0.5f)*1.15f);
                    const float redRand=hash(cellX+3,cellY+19,p.seed+0x1234567bu,tick);
                    const float greenRand=hash(cellX+17,cellY+5,p.seed+0x31415926u,tick);
                    const float blueRand=hash(cellX+29,cellY+13,p.seed+0x27182818u,tick);
                    const float snowR=lerp(snowValue,snowValue*(0.48f+0.52f*redRand),colorStrength);
                    const float snowG=lerp(snowValue,snowValue*(0.48f+0.52f*greenRand),colorStrength);
                    const float snowB=lerp(snowValue,snowValue*(0.48f+0.52f*blueRand),colorStrength);
                    r=lerp(r,snowR,snowMask); g=lerp(g,snowG,snowMask); b=lerp(b,snowB,snowMask);
                }
            }

            r=std::clamp(r,0.0f,1.0f); g=std::clamp(g,0.0f,1.0f); b=std::clamp(b,0.0f,1.0f);
            output[0]=p.premultipliedAlpha?r*alpha:r;
            output[1]=p.premultipliedAlpha?g*alpha:g;
            output[2]=p.premultipliedAlpha?b*alpha:b;
            output[3]=alpha;
        }
    }
    return true;
}
} // namespace vhs90
