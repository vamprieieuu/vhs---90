#include "VHS90SyncCollapseLabEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace vhs90 {
namespace {
inline float clamp01(float v) noexcept { return std::clamp(v, 0.0f, 1.0f); }
inline float smooth01(float v) noexcept { v=clamp01(v); return v*v*(3.0f-2.0f*v); }
inline float smoothEdge(float a,float b,float x) noexcept { return b<=a?(x<a?0.0f:1.0f):smooth01((x-a)/(b-a)); }
inline float hash(int x,int y,std::uint32_t seed,std::int64_t tick) noexcept {
    return hash01(static_cast<std::uint32_t>(x),static_cast<std::uint32_t>(y),seed,tick);
}
inline float sample(ConstImageView src,float x,float y,int channel) noexcept {
    x=std::clamp(x,0.0f,float(src.width-1)); y=std::clamp(y,0.0f,float(src.height-1));
    const int x0=int(std::floor(x)),y0=int(std::floor(y));
    const int x1=std::min(src.width-1,x0+1),y1=std::min(src.height-1,y0+1);
    const float tx=x-float(x0),ty=y-float(y0);
    const float* p00=src.rgba+y0*src.rowStrideFloats+x0*4;
    const float* p10=src.rgba+y0*src.rowStrideFloats+x1*4;
    const float* p01=src.rgba+y1*src.rowStrideFloats+x0*4;
    const float* p11=src.rgba+y1*src.rowStrideFloats+x1*4;
    const float a=p00[channel]+(p10[channel]-p00[channel])*tx;
    const float b=p01[channel]+(p11[channel]-p01[channel])*tx;
    return a+(b-a)*ty;
}
inline float luma(float r,float g,float b) noexcept { return 0.299f*r+0.587f*g+0.114f*b; }
inline float lerp(float a,float b,float t) noexcept { return a+(b-a)*t; }
struct EventState { float bright=0.0f,dark=0.0f; };
inline EventState evaluateEvent(const SyncCollapseLabParams& p,double time) noexcept {
    if(!p.lumaEnabled) return {};
    if(p.animationMode==SyncDamageAnimationMode::ManualKeyframed)
        return {clamp01(p.manualBrightPulse),clamp01(p.manualDarkCollapse)};
    const float frequency=std::max(0.05f,p.eventFrequencyHz);
    const double phase=std::max(0.0,time)*double(frequency);
    const double eventIndexD=std::floor(phase);
    const auto eventIndex=static_cast<std::int64_t>(eventIndexD);
    const float cycle=float(phase-eventIndexD);
    const float duration=std::clamp(p.eventDurationSeconds*frequency,0.02f,0.92f);
    const float start=hash(int(eventIndex),7,p.seed+0x9e3779b9u,0)*(1.0f-duration);
    const float age=(cycle-start)/duration;
    if(age<0.0f||age>1.0f) return {};
    const float sharp=clamp01(p.transitionSharpness);
    const float edge=std::clamp(0.22f-sharp*0.20f,0.008f,0.22f);
    const float envelope=smoothEdge(0.0f,edge,age)*(1.0f-smoothEdge(1.0f-edge,1.0f,age));
    const float eventChoice=hash(int(eventIndex),19,p.seed+0x85ebca6bu,0);
    const float brightProbability=clamp01(p.brightEventProbability);
    const float darkProbability=clamp01(p.darkEventProbability);
    const float totalProbability=brightProbability+darkProbability;
    if(totalProbability<=0.0f||eventChoice>=std::min(1.0f,totalProbability)) return {};
    const float polarityChoice=hash(int(eventIndex),23,p.seed+0xc2b2ae35u,0);
    const bool isBright=polarityChoice<brightProbability/std::max(1.0e-6f,totalProbability);
    return isBright?EventState{envelope*clamp01(p.brightFlashIntensity),0.0f}
                   :EventState{0.0f,envelope*clamp01(p.darkCollapseIntensity)};
}
}

bool processSyncCollapseLab(ConstImageView src,ImageView dst,const SyncCollapseLabParams& p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||src.rgba==dst.rgba||
       !std::isfinite(p.timeSeconds)||!std::isfinite(p.effectAmount)||!std::isfinite(p.horizontalTearAmount)||
       !std::isfinite(p.bandIntensity)||!std::isfinite(p.bandDensity)||!std::isfinite(p.bandHeight)||
       !std::isfinite(p.bandIrregularity)||!std::isfinite(p.horizontalDisplacementPx)||
       !std::isfinite(p.boundaryFragmentation)||!std::isfinite(p.fullFrameCoverage)||
       !std::isfinite(p.lumaInstability)||!std::isfinite(p.eventFrequencyHz)||
       !std::isfinite(p.eventDurationSeconds)||!std::isfinite(p.streakLengthPx)||
       !std::isfinite(p.animationSpeed)||!std::isfinite(p.blendWithInput)) return false;
    const float effect=clamp01(p.effectAmount);
    if(!p.enabled||effect<=0.0f) {
        for(int y=0;y<src.height;++y) std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
        return true;
    }
    const double time=p.animationMode==SyncDamageAnimationMode::Frozen?0.0:
        std::max(0.0,p.timeSeconds)*double(std::max(0.0f,p.animationSpeed));
    const float temporal=clamp01(p.temporalVariation);
    const std::int64_t tick=temporal>0.0f?static_cast<std::int64_t>(std::floor(time*(1.0+18.0*temporal))):0;
    const EventState event=evaluateEvent(p,time);
    const float coverage=clamp01(p.fullFrameCoverage),coverageTop=(1.0f-coverage)*0.5f;
    const int bandCount=std::clamp(int(std::lround(p.bandDensity)),1,32);
    const float bandHeight=clamp01(p.bandHeight),irregularity=clamp01(p.bandIrregularity);
    const float tear=clamp01(p.horizontalTearAmount),bandDrive=clamp01(p.bandIntensity);
    const float fragmentation=clamp01(p.boundaryFragmentation);
    const float dropoutDensity=p.dropoutsEnabled?clamp01(p.dropoutStreakDensity):0.0f;
    const float dropoutFragment=clamp01(p.horizontalFragmentation);
    const float blendInput=clamp01(p.blendWithInput);
    const float displacement=std::max(0.0f,p.horizontalDisplacementPx);
    const int streakLength=std::max(2,int(std::lround(std::max(2.0f,p.streakLengthPx))));
    const int segmentWidth=std::max(4,streakLength);
    const float magenta=p.chromaEnabled?clamp01(p.subtleChromaTint)*clamp01(p.magentaTint):0.0f;
    const float green=p.chromaEnabled?clamp01(p.subtleChromaTint)*clamp01(p.greenTint):0.0f;
    const float rgbOffset=p.chromaEnabled?std::clamp(p.rgbOffsetPx,0.0f,8.0f):0.0f;
    const float bleed=p.chromaEnabled?clamp01(p.chromaBleed):0.0f;
    const float framePulse=p.lumaEnabled?(event.bright-event.dark):0.0f;

    for(int y=0;y<src.height;++y) {
        const float v=(float(y)+0.5f)/float(src.height);
        const bool inside=coverage>0.0f&&v>=coverageTop&&v<=coverageTop+coverage;
        const float localV=inside?(v-coverageTop)/coverage:0.0f;
        const float rowNoise=hash(0,y/3,p.seed+0x68bc21ebu,tick);
        const float rowWave=std::sin(float(time)*2.3f+float(y)*0.041f)*temporal;
        const float rowShiftBase=(rowNoise-0.5f)*2.0f*displacement*tear*(0.35f+temporal*0.65f)
                                +rowWave*displacement*tear*0.12f;
        const int bandIndex=inside?std::clamp(int(localV*float(bandCount)),0,bandCount-1):-1;
        float bandMask=0.0f,bandLevel=0.035f;
        if(bandIndex>=0) {
            const float bandCoord=localV*float(bandCount)-float(bandIndex);
            const float bandRandom=hash(bandIndex,0,p.seed+0x243f6a88u,tick);
            const float center=0.5f+(bandRandom-0.5f)*irregularity*0.52f+(rowNoise-0.5f)*irregularity*0.12f;
            const float halfHeight=std::max(0.006f,bandHeight*(0.22f+0.30f*bandRandom));
            const float distance=std::fabs(bandCoord-center);
            bandMask=1.0f-smoothEdge(halfHeight,halfHeight+0.045f+irregularity*0.035f,distance);
            const float rowFragment=hash(0,y/2,p.seed+0x94d049bbu,tick);
            if(rowFragment<fragmentation*0.28f) bandMask*=0.12f;
            const float brightLevel=0.58f+0.30f*bandRandom;
            const float darkLevel=0.008f+0.075f*hash(bandIndex,13,p.seed+0x51ed270bu,tick);
            bandLevel=(bandIndex&1)==0?brightLevel:darkLevel;
        }
        const float localVariation=(hash(0,y/2,p.seed+0x165667b1u,tick)-0.5f)*clamp01(p.localBandLumaVariation)*0.28f;
        const float rowEventLuma=framePulse*(0.20f+0.80f*bandMask)*clamp01(p.lumaInstability);
        const float rowTear=bandMask*displacement*tear*((rowNoise-0.5f)*1.25f+rowWave*0.35f);
        for(int x=0;x<src.width;++x) {
            const float* original=src.rgba+y*src.rowStrideFloats+x*4;
            float* output=dst.rgba+y*dst.rowStrideFloats+x*4;
            const float alpha=original[3];
            if(alpha<=0.0f) { std::copy_n(original,4,output); continue; }
            const int segment=x/segmentWidth;
            const float segmentRand=hash(segment,y/2,p.seed+0xc2b2ae35u,tick);
            const float segmentMask=segmentRand<dropoutFragment*0.35f?0.12f:1.0f;
            const float dropoutRow=hash(0,y/2,p.seed+0xd3a2646cu,tick);
            const bool dropout=p.dropoutsEnabled&&dropoutRow<dropoutDensity*0.24f&&segmentRand>dropoutFragment*0.42f;
            const float dropoutType=hash(segment+17,y/2,p.seed+0x27d4eb2fu,tick);
            const float dropoutStrength=dropout?(dropoutType>0.58f?clamp01(p.streakBrightness):clamp01(p.darkDropoutAmount)):0.0f;
            const float rowShift=rowShiftBase*bandMask+rowTear+(dropout?(segmentRand-0.5f)*streakLength*0.65f:0.0f);
            const float sx=float(x)+rowShift;
            const float rX=sx-rgbOffset-bleed*3.0f,bX=sx+rgbOffset+bleed*3.0f;
            float r=sample(src,rX,float(y),0);
            float g=sample(src,sx,float(y),1);
            float b=sample(src,bX,float(y),2);
            if(p.premultipliedAlpha) {
                const float alphaR=sample(src,rX,float(y),3),alphaG=sample(src,sx,float(y),3),alphaB=sample(src,bX,float(y),3);
                r=alphaR>1.0e-6f?r/alphaR:0.0f;
                g=alphaG>1.0e-6f?g/alphaG:0.0f;
                b=alphaB>1.0e-6f?b/alphaB:0.0f;
            }
            const float generatedBase=0.025f+0.035f*hash(int(std::lround((float(x)+rowShift)/8.0f)),y/4,p.seed+0x13198a2eu,tick);
            if(p.inputMode==SyncDamageInputMode::GeneratedSignal) r=g=b=generatedBase;
            const float sourceLuma=p.inputMode==SyncDamageInputMode::GeneratedSignal?generatedBase:luma(r,g,b);
            const float baseLuma=sourceLuma;
            const float bandBlend=bandMask*bandDrive*(0.55f+0.45f*tear)*segmentMask;
            float damagedLuma=lerp(baseLuma,bandLevel+localVariation,bandBlend);
            damagedLuma=clamp01(damagedLuma+rowEventLuma);
            if(dropout) {
                const float along=float(x%segmentWidth)/float(segmentWidth);
                const float edge=std::min(1.0f,std::min(along*5.0f,(1.0f-along)*5.0f));
                const float dropoutMix=dropoutStrength*edge;
                const float target=dropoutType>0.58f?0.92f:(1.0f-dropoutStrength*0.98f);
                damagedLuma=lerp(damagedLuma,target,dropoutMix);
            }
            const float chromaActivity=clamp01(bandMask*bandDrive+dropoutStrength*0.35f);
            const float tintR=magenta*chromaActivity*0.12f-green*chromaActivity*0.025f;
            const float tintG=green*chromaActivity*0.10f-magenta*chromaActivity*0.045f;
            const float tintB=magenta*chromaActivity*0.12f-green*chromaActivity*0.035f;
            const float deltaLuma=damagedLuma-sourceLuma;
            r+=deltaLuma+tintR;g+=deltaLuma+tintG;b+=deltaLuma+tintB;
            const float sourceR=p.premultipliedAlpha&&alpha>1.0e-6f?original[0]/alpha:original[0];
            const float sourceG=p.premultipliedAlpha&&alpha>1.0e-6f?original[1]/alpha:original[1];
            const float sourceB=p.premultipliedAlpha&&alpha>1.0e-6f?original[2]/alpha:original[2];
            const float outR=lerp(r,sourceR,blendInput),outG=lerp(g,sourceG,blendInput),outB=lerp(b,sourceB,blendInput);
            const float finalR=p.premultipliedAlpha?outR*alpha:outR;
            const float finalG=p.premultipliedAlpha?outG*alpha:outG;
            const float finalB=p.premultipliedAlpha?outB*alpha:outB;
            output[0]=effect>=1.0f?finalR:original[0]+(finalR-original[0])*effect;
            output[1]=effect>=1.0f?finalG:original[1]+(finalG-original[1])*effect;
            output[2]=effect>=1.0f?finalB:original[2]+(finalB-original[2])*effect;
            output[3]=alpha;
        }
    }
    return true;
}
} // namespace vhs90
