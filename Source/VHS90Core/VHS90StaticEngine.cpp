#include "VHS90StaticEngine.h"
#include <algorithm>
#include <cmath>

namespace vhs90 {
namespace {
inline float clamp01(float v) noexcept { return std::max(0.0f, std::min(1.0f, v)); }
inline float clampf(float v,float lo,float hi) noexcept { return std::max(lo,std::min(hi,v)); }
inline float smooth01(float t) noexcept { t=clamp01(t); return t*t*(3.0f-2.0f*t); }
inline float lerp(float a,float b,float t) noexcept { return a+(b-a)*t; }
inline float hashCell(int x,int y,std::uint32_t seed,std::int64_t tick) noexcept {
    return hash01(static_cast<std::uint32_t>(x),static_cast<std::uint32_t>(y),seed,tick);
}
inline RGB readSource(ConstImageView src,int x,int y) noexcept {
    const float* p=src.rgba+y*src.rowStrideFloats+x*4;
    return {p[0],p[1],p[2]};
}
inline float readAlpha(ConstImageView src,int x,int y) noexcept {
    return src.rgba[y*src.rowStrideFloats+x*4+3];
}
inline float patternWeight(const StaticParams& p,float v) noexcept {
    switch(p.mode) {
        case StaticPatternMode::TopStrip: {
            const float h=clamp01(p.topBandHeight);
            if(h<=0.0f) return 0.0f;
            return 1.0f-smooth01((v-h)/std::max(0.0001f,h*0.12f));
        }
        case StaticPatternMode::BottomWeightedStreaks:
            return std::pow(clamp01(v),std::max(0.1f,p.bottomExponent));
        case StaticPatternMode::SplitBandChromatic:
            return 1.0f;
        default: return 1.0f;
    }
}
inline float horizontalSignal(int cellX,int cellY,const StaticParams& p,std::int64_t tick) noexcept {
    const float rowEvent=hashCell(0,cellY,p.seed+0x68bc21ebu,tick);
    const float cellEvent=hashCell(cellX,cellY,p.seed,tick);
    const float rowValue=(hashCell(0,cellY+31,p.seed+0x85ebca6bu,tick)-0.5f)*2.0f;
    const float fragment=(hashCell(cellX+17,cellY+73,p.seed+0xc2b2ae35u,tick)-0.5f)*2.0f;
    if(rowEvent<clamp01(p.density)*0.18f) return rowValue;
    if(cellEvent<clamp01(p.density)) return fragment;
    return 0.0f;
}
}

bool processStaticPattern(ConstImageView src,ImageView dst,
                          const StaticParams& p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||
       dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||
       src.rgba==dst.rgba) return false;

    const std::int64_t tick=p.animated
        ? static_cast<std::int64_t>(std::floor(static_cast<double>(p.frame)*std::max(0.0f,p.animationSpeed)))
        : 0;
    const int cell=std::max(1,static_cast<int>(std::lround(std::max(0.25f,p.cellSizePx))));
    const int streak=std::max(1,static_cast<int>(std::lround(std::max(1.0f,p.streakLengthPx))));
    const int blackoutFrames=std::max(1,p.blackoutDurationFrames);
    const std::int64_t blackoutTick=p.animated ? p.frame/blackoutFrames : 0;
    const bool blackout=p.mode==StaticPatternMode::StaticToBlackout &&
        hashCell(static_cast<int>(blackoutTick),0,p.seed+0x27d4eb2fu,0)<clamp01(p.blackoutProbability);
    const float frameFlicker=1.0f-clamp01(p.flicker)*hashCell(0,0,p.seed+0x165667b1u,tick);
    const float angle=p.angleDegrees*0.01745329251994329577f;
    const float ca=std::cos(angle),sa=std::sin(angle);
    const float sourceMix=clamp01(p.sourceMix);
    const float intensity=clamp01(p.intensity);

    for(int y=0;y<src.height;++y) {
        const float v=(float(y)+0.5f)/float(src.height);
        const float weight=patternWeight(p,v);
        for(int x=0;x<src.width;++x) {
            const float alpha=readAlpha(src,x,y);
            const RGB srcPixel=readSource(src,x,y);
            RGB out=srcPixel;
            if(alpha>0.0f) {
                if(intensity<=0.0f || ((weight<=0.0f || p.density<=0.0f) && sourceMix>=1.0f && !blackout)) {
                    float* dstPixel=dst.rgba+y*dst.rowStrideFloats+x*4;
                    dstPixel[0]=srcPixel.r; dstPixel[1]=srcPixel.g; dstPixel[2]=srcPixel.b; dstPixel[3]=alpha;
                    continue;
                }
                const int cx=x/cell;
                const int cy=y/cell;
                const float horizontal=horizontalSignal(x/streak,cy,p,tick);
                const float speckle=hashCell(cx,cy,p.seed+0x9e3779b9u,tick)<clamp01(p.density)
                    ? (hashCell(cx+19,cy+11,p.seed+0x85ebca6bu,tick)-0.5f)*2.0f : 0.0f;
                float lumaNoise=0.0f;
                float chromaI=0.0f;
                float chromaQ=0.0f;
                float greenBoost=0.0f;
                bool pixelBlackout=blackout;

                switch(p.mode) {
                    case StaticPatternMode::DarkFieldLumaStreaks:
                        lumaNoise=horizontal*p.lumaAmount+speckle*p.lumaAmount*0.35f;
                        break;
                    case StaticPatternMode::MultiPhaseGreenLine: {
                        const int phases=std::max(2,p.phaseCount);
                        const int phase=static_cast<int>((std::max<std::int64_t>(0,p.frame)+y/3)%phases);
                        lumaNoise=horizontal*p.lumaAmount*(phase==0?1.0f:0.35f);
                        const float line=hashCell(0,y/2,p.seed+0x51ed270bu,tick);
                        if(phase==0&&line>0.42f) greenBoost=clamp01(p.greenLineAmount)*(0.35f+line*0.65f);
                        chromaI=(hashCell(cx+7,cy+3,p.seed+0x94d049bbu,tick)-0.5f)*p.chromaAmount;
                        break;
                    }
                    case StaticPatternMode::TwoStageAngularRibbon: {
                        const float fx=float(x)/float(std::max(1,src.width-1));
                        const float fy=float(y)/float(std::max(1,src.height-1));
                        const float rx=fx*ca+fy*sa;
                        const float ry=-fx*sa+fy*ca;
                        const float ribbon=std::fabs(ry-(0.45f+0.045f*std::sin(rx*18.0f+float(tick)*0.08f)));
                        const bool topStage=fy<clamp01(p.topBandHeight);
                        const bool ribbonStage=ribbon<clampf(p.ribbonWidth,0.005f,0.5f);
                        if(topStage||ribbonStage) {
                            lumaNoise=horizontal*p.lumaAmount*(topStage?0.7f:1.0f);
                            chromaI=(hashCell(cx+5,cy+9,p.seed+0x51ed270bu,tick)-0.5f)*p.chromaAmount*2.0f;
                            chromaQ=(hashCell(cx+13,cy+17,p.seed+0x94d049bbu,tick)-0.5f)*p.chromaAmount*2.0f;
                        }
                        break;
                    }
                    case StaticPatternMode::DiagonalSyncCollapse: {
                        const float fx=float(x)/float(std::max(1,src.width-1));
                        const float fy=float(y)/float(std::max(1,src.height-1));
                        const float diag=fy-fx*std::tan(angle)-float(tick)*0.0015f;
                        const float band=1.0f-smooth01(std::fabs(diag-0.28f)/std::max(0.005f,p.ribbonWidth));
                        if(band>0.0f) {
                            lumaNoise=horizontal*p.lumaAmount*band;
                            const float collapse=hashCell(x/4,y/2,p.seed+0x27d4eb2fu,tick);
                            if(collapse<clamp01(p.blackoutProbability)*band) pixelBlackout=true;
                            chromaI=(hashCell(cx+3,cy+11,p.seed+0x51ed270bu,tick)-0.5f)*p.chromaAmount*band*2.0f;
                            chromaQ=(hashCell(cx+17,cy+7,p.seed+0x94d049bbu,tick)-0.5f)*p.chromaAmount*band*2.0f;
                        }
                        break;
                    }
                    case StaticPatternMode::DarkFieldSpeckles:
                        lumaNoise=speckle*p.lumaAmount;
                        chromaI=(hashCell(cx+7,cy+3,p.seed+0x51ed270bu,tick)-0.5f)*p.chromaAmount*2.0f;
                        chromaQ=(hashCell(cx+13,cy+5,p.seed+0x94d049bbu,tick)-0.5f)*p.chromaAmount*2.0f;
                        break;
                    case StaticPatternMode::ColoredSpeckles: {
                        if(hashCell(cx,cy,p.seed+0x243f6a88u,tick)<clamp01(p.density)) {
                            const float nr=(hashCell(cx+3,cy+7,p.seed+0x243f6a88u,tick)-0.5f)*2.0f;
                            const float ng=(hashCell(cx+11,cy+19,p.seed+0x85a308d3u,tick)-0.5f)*2.0f;
                            const float nb=(hashCell(cx+23,cy+5,p.seed+0x13198a2eu,tick)-0.5f)*2.0f;
                            lumaNoise=((nr+ng+nb)/3.0f)*p.lumaAmount;
                            chromaI=(nr-ng)*0.5f*p.chromaAmount;
                            chromaQ=(nb-(nr+ng)*0.5f)*0.5f*p.chromaAmount;
                        }
                        break;
                    }
                    case StaticPatternMode::VerticalColumns: {
                        const int column=x/cell;
                        const int segment=y/streak;
                        const float event=hashCell(column,segment,p.seed+0x243f6a88u,tick);
                        if(event<clamp01(p.density)) {
                            lumaNoise=(hashCell(column+5,segment+13,p.seed+0xb7e15162u,tick)-0.5f)*2.0f*p.lumaAmount;
                            chromaI=(hashCell(column+11,segment+3,p.seed+0x51ed270bu,tick)-0.5f)*2.0f*p.chromaAmount;
                            chromaQ=(hashCell(column+19,segment+7,p.seed+0x94d049bbu,tick)-0.5f)*2.0f*p.chromaAmount;
                        }
                        break;
                    }
                    case StaticPatternMode::MonochromeStreaks:
                        lumaNoise=horizontal*p.lumaAmount;
                        break;
                    case StaticPatternMode::SplitBandChromatic:
                        if(v<clamp01(p.splitPosition)) {
                            lumaNoise=horizontal*p.lumaAmount;
                            if(std::fabs(horizontal)>1.0e-6f) {
                                chromaI=(hashCell(cx+3,cy+9,p.seed+0x51ed270bu,tick)-0.5f)*p.chromaAmount*2.0f;
                                chromaQ=(hashCell(cx+17,cy+5,p.seed+0x94d049bbu,tick)-0.5f)*p.chromaAmount*2.0f;
                            }
                        } else {
                            const int column=x/cell;
                            const int segment=y/streak;
                            const float verticalEvent=hashCell(column,segment,p.seed+0x243f6a88u,tick);
                            if(verticalEvent<clamp01(p.density)) {
                                lumaNoise=(hashCell(column+5,segment+13,p.seed+0xb7e15162u,tick)-0.5f)*p.lumaAmount*2.0f;
                                chromaI=(hashCell(column+11,segment+3,p.seed+0x51ed270bu,tick)-0.5f)*p.chromaAmount*2.0f;
                                chromaQ=(hashCell(column+19,segment+7,p.seed+0x94d049bbu,tick)-0.5f)*p.chromaAmount*2.0f;
                            }
                        }
                        break;
                    case StaticPatternMode::TopStrip:
                    case StaticPatternMode::BottomWeightedStreaks:
                    case StaticPatternMode::FullFrameHorizontal:
                    case StaticPatternMode::StaticToBlackout:
                    default:
                        lumaNoise=horizontal*p.lumaAmount;
                        chromaI=(hashCell(cx+23,cy+2,p.seed+0x51ed270bu,tick)-0.5f)*p.chromaAmount*2.0f;
                        chromaQ=(hashCell(cx+29,cy+4,p.seed+0x94d049bbu,tick)-0.5f)*p.chromaAmount*2.0f;
                        break;
                }

                const RGB generatedBase{p.baseLuma,p.baseLuma,p.baseLuma};
                RGB straightSource=srcPixel;
                if(p.premultipliedAlpha && alpha>1.0e-6f) {
                    straightSource.r/=alpha; straightSource.g/=alpha; straightSource.b/=alpha;
                }
                RGB base{lerp(generatedBase.r,straightSource.r,sourceMix),
                         lerp(generatedBase.g,straightSource.g,sourceMix),
                         lerp(generatedBase.b,straightSource.b,sourceMix)};
                const float noiseWeight=weight*intensity*frameFlicker;
                if(noiseWeight>0.0f) {
                    YIQ q=rgbToYiq(base);
                    q.y+=lumaNoise*noiseWeight;
                    q.i+=chromaI*noiseWeight;
                    q.q+=chromaQ*noiseWeight;
                    out=yiqToRgb(q);
                    out.g+=greenBoost*noiseWeight;
                } else {
                    out=base;
                }

                if(pixelBlackout) {
                    out.r=0.0f; out.g=0.0f; out.b=0.0f;
                }
                if(p.premultipliedAlpha) {
                    out.r*=alpha; out.g*=alpha; out.b*=alpha;
                }
            }
            float* dstPixel=dst.rgba+y*dst.rowStrideFloats+x*4;
            dstPixel[0]=out.r; dstPixel[1]=out.g; dstPixel[2]=out.b; dstPixel[3]=alpha;
        }
    }
    return true;
}

} // namespace vhs90
