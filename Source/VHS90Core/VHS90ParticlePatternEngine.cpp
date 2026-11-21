#include "VHS90ParticlePatternEngine.h"
#include <algorithm>
#include <cmath>

namespace vhs90 {
namespace {
constexpr float kPi = 3.14159265358979323846f;
inline float clamp01(float x) noexcept { return std::max(0.0f, std::min(1.0f, x)); }
inline float clampf(float x,float lo,float hi) noexcept { return std::max(lo,std::min(hi,x)); }
inline float hashCell(int x,int y,std::uint32_t seed,std::int64_t tick) noexcept {
    return hash01(static_cast<std::uint32_t>(x),static_cast<std::uint32_t>(y),seed,tick);
}
inline float distributionWeight(ParticleDistribution d,float v) noexcept {
    switch(d) {
        case ParticleDistribution::TopWeighted: return 1.0f-0.75f*v;
        case ParticleDistribution::BottomWeighted: return 0.25f+0.75f*v;
        case ParticleDistribution::CenterWeighted: return 0.2f+0.8f*(1.0f-std::fabs(v*2.0f-1.0f));
        case ParticleDistribution::EdgeWeighted: return 0.2f+0.8f*std::fabs(v*2.0f-1.0f);
        case ParticleDistribution::Uniform:
        default: return 1.0f;
    }
}
inline float smoothCoverage(float d,float radius) noexcept {
    radius=std::max(0.25f,radius);
    const float t=clamp01((radius+0.75f-d)/1.5f);
    return t*t*(3.0f-2.0f*t);
}
}

bool processParticlePattern(ConstImageView src,ImageView dst,const ParticlePatternParams& p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||
       dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||
       src.rgba==dst.rgba) return false;
    if(!std::isfinite(p.intensity)||!std::isfinite(p.density)||
       !std::isfinite(p.particleSizePx)||!std::isfinite(p.streakLengthPx)||
       !std::isfinite(p.orientationDegrees)||!std::isfinite(p.sourceMix)) return false;

    const float intensity=clamp01(p.intensity);
    const float density=clamp01(p.density);
    const float sourceMix=clamp01(p.sourceMix);
    const float cellPitch=std::max(2.0f,p.particleSizePx*2.8f+p.streakLengthPx*0.12f);
    const int cell=std::max(2,static_cast<int>(std::lround(cellPitch)));
    const std::int64_t tick=p.animated
        ? static_cast<std::int64_t>(std::floor(static_cast<double>(p.frame)*std::max(0.0f,p.animationSpeed)))
        : 0;
    const float baseAngle=p.orientationDegrees*kPi/180.0f;
    const float cs=std::cos(baseAngle),sn=std::sin(baseAngle);
    const float baseSize=std::max(0.25f,p.particleSizePx);
    const float length=std::max(1.0f,p.streakLengthPx);
    const float aspect=std::max(0.2f,p.aspectRatio);

    for(int y=0;y<src.height;++y) {
        const float v=(float(y)+0.5f)/float(src.height);
        float* outRow=dst.rgba+y*dst.rowStrideFloats;
        const float* srcRow=src.rgba+y*src.rowStrideFloats;
        for(int x=0;x<src.width;++x) {
            const float* sp=srcRow+x*4;
            float* dp=outRow+x*4;
            const float alpha=sp[3];
            if(alpha<=0.0f || intensity<=0.0f || density<=0.0f) {
                std::copy_n(sp,4,dp);
                continue;
            }
            const int gx=x/cell,gy=y/cell;
            const std::uint32_t seed=p.seed;
            const float event=hashCell(gx,gy,seed,tick);
            const float weight=distributionWeight(p.distribution,v);
            float distribution=weight;
            if(p.mode==ParticlePatternMode::BottomWeightedStreaks)
                distribution*=0.25f+0.75f*v*v;
            if(event>=clamp01(density*distribution)) {
                const float base=p.backgroundLuma*(1.0f-sourceMix)*(p.premultipliedAlpha?alpha:1.0f)+sp[0]*sourceMix;
                const float baseG=p.backgroundLuma*(1.0f-sourceMix)*(p.premultipliedAlpha?alpha:1.0f)+sp[1]*sourceMix;
                const float baseB=p.backgroundLuma*(1.0f-sourceMix)*(p.premultipliedAlpha?alpha:1.0f)+sp[2]*sourceMix;
                dp[0]=base;dp[1]=baseG;dp[2]=baseB;dp[3]=alpha;
                continue;
            }

            const float ox=hashCell(gx+11,gy+3,seed+0x9e3779b9u,tick);
            const float oy=hashCell(gx+5,gy+17,seed+0x85ebca6bu,tick);
            const float sizeN=hashCell(gx+23,gy+29,seed+0xc2b2ae35u,tick);
            const float size=baseSize*(1.0f+(sizeN-0.5f)*2.0f*clamp01(p.sizeVariation));
            float angle=baseAngle;
            float halfLong=length*0.5f;
            float halfShort=std::max(0.35f,size*0.5f);
            switch(p.mode) {
                case ParticlePatternMode::DarkFieldSpeckles:
                    halfLong=std::max(0.5f,size*0.65f); halfShort=std::max(0.5f,size*0.5f); break;
                case ParticlePatternMode::HorizontalFragments:
                case ParticlePatternMode::BottomWeightedStreaks:
                    angle=baseAngle; halfLong=std::max(1.0f,length*(0.35f+ox*0.9f)); halfShort=std::max(0.4f,size*0.35f); break;
                case ParticlePatternMode::ColoredSpeckles:
                    halfLong=std::max(0.6f,size*0.7f); halfShort=std::max(0.6f,size*0.7f); break;
                case ParticlePatternMode::VerticalColumns:
                    angle=baseAngle+kPi*0.5f; halfLong=std::max(1.0f,length*(0.5f+oy*0.8f)); halfShort=std::max(0.4f,size*0.35f); break;
            }
            const float cx=(float(gx)+ox)*float(cell);
            const float cy=(float(gy)+oy)*float(cell);
            const float dx=float(x)-cx,dy=float(y)-cy;
            const float ca=std::cos(angle),sa=std::sin(angle);
            const float along=std::fabs(dx*ca+dy*sa);
            const float across=std::fabs(-dx*sa+dy*ca);
            const float norm=std::sqrt((along*along)/(halfLong*halfLong)+(across*across)/(halfShort*halfShort));
            float mask=smoothCoverage(norm,1.0f);
            if(mask<=0.0f) {
                const float base=p.backgroundLuma*(1.0f-sourceMix)*(p.premultipliedAlpha?alpha:1.0f)+sp[0]*sourceMix;
                const float baseG=p.backgroundLuma*(1.0f-sourceMix)*(p.premultipliedAlpha?alpha:1.0f)+sp[1]*sourceMix;
                const float baseB=p.backgroundLuma*(1.0f-sourceMix)*(p.premultipliedAlpha?alpha:1.0f)+sp[2]*sourceMix;
                dp[0]=base;dp[1]=baseG;dp[2]=baseB;dp[3]=alpha;
                continue;
            }
            const float flickerN=hashCell(gx,gy,seed+0x165667b1u,tick);
            const float flicker=1.0f-clamp01(p.flicker)*flickerN;
            const float raw=hashCell(gx+31,gy+47,seed+0x27d4eb2fu,tick);
            float particle=clampf((0.35f+raw*0.9f)*p.brightness,0.0f,2.0f);
            particle=clampf((particle-0.5f)*p.contrast+0.5f,0.0f,2.0f)*flicker;
            const float chroma=clamp01(p.chromaAmount);
            float pr=particle,pg=particle,pb=particle;
            if(p.mode==ParticlePatternMode::ColoredSpeckles || chroma>0.0f) {
                const float hr=hashCell(gx+7,gy+13,seed+0x51ed270bu,tick);
                const float hg=hashCell(gx+19,gy+2,seed+0x94d049bbu,tick);
                const float hb=hashCell(gx+3,gy+37,seed+0x243f6a88u,tick);
                pr=particle*(1.0f-chroma)+clampf(hr*1.6f,0.0f,1.8f)*particle*chroma;
                pg=particle*(1.0f-chroma)+clampf(hg*1.6f,0.0f,1.8f)*particle*chroma;
                pb=particle*(1.0f-chroma)+clampf(hb*1.6f,0.0f,1.8f)*particle*chroma;
            }
            const float baseR=p.backgroundLuma*(1.0f-sourceMix)*(p.premultipliedAlpha?alpha:1.0f)+sp[0]*sourceMix;
            const float baseG=p.backgroundLuma*(1.0f-sourceMix)*(p.premultipliedAlpha?alpha:1.0f)+sp[1]*sourceMix;
            const float baseB=p.backgroundLuma*(1.0f-sourceMix)*(p.premultipliedAlpha?alpha:1.0f)+sp[2]*sourceMix;
            const float a=clamp01(mask*intensity);
            const float pa=p.premultipliedAlpha?alpha:1.0f;
            dp[0]=baseR+(pr*pa-baseR)*a;
            dp[1]=baseG+(pg*pa-baseG)*a;
            dp[2]=baseB+(pb*pa-baseB)*a;
            dp[3]=alpha;
        }
    }
    return true;
}

} // namespace vhs90
