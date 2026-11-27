#include "VHS90SyncCollapseLabEngine.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    using namespace vhs90;
    constexpr int w=96,h=72;
    std::vector<float> src(w*h*4),a(src.size()),b(src.size()),c(src.size());
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        const std::size_t i=(std::size_t(y)*w+x)*4;
        src[i]=float(x)/float(w-1); src[i+1]=float(y)/float(h-1);
        src[i+2]=0.25f+0.35f*float((x+y)%13)/12.0f; src[i+3]=1.0f;
    }
    ConstImageView input{src.data(),w,h,w*4};
    ImageView av{a.data(),w,h,w*4},bv{b.data(),w,h,w*4},cv{c.data(),w,h,w*4};
    SyncCollapseLabParams p;
    p.seed=0x9027u; p.frame=12; p.timeSeconds=0.22;
    p.bandDensity=8.0f; p.bandHeight=0.62f; p.bandIrregularity=0.9f;
    p.horizontalTearAmount=0.9f; p.bandIntensity=0.9f;
    p.dropoutStreakDensity=0.9f; p.streakLengthPx=14.0f;
    p.blendWithInput=0.25f;
    assert(processSyncCollapseLab(input,av,p));
    assert(processSyncCollapseLab(input,bv,p));
    assert(a==b && a!=src);

    SyncCollapseLabParams next=p; next.timeSeconds=0.71;
    assert(processSyncCollapseLab(input,cv,next));
    assert(c!=a);

    // Manual/keyframed event values drive independent bright and dark pulses.
    std::vector<float> bright(src.size()),dark(src.size());
    ImageView brightView{bright.data(),w,h,w*4},darkView{dark.data(),w,h,w*4};
    SyncCollapseLabParams manual=p; manual.animationMode=SyncDamageAnimationMode::ManualKeyframed;
    manual.manualBrightPulse=1.0f; manual.manualDarkCollapse=0.0f;
    assert(processSyncCollapseLab(input,brightView,manual));
    manual.manualBrightPulse=0.0f; manual.manualDarkCollapse=1.0f;
    assert(processSyncCollapseLab(input,darkView,manual));
    assert(bright!=dark);

    // Automatic bright/dark event probabilities are independently controllable.
    std::vector<float> brightOnly(src.size()),darkOnly(src.size()),noEvents(src.size());
    ImageView brightOnlyView{brightOnly.data(),w,h,w*4},darkOnlyView{darkOnly.data(),w,h,w*4},noEventsView{noEvents.data(),w,h,w*4};
    SyncCollapseLabParams eventParams=p; eventParams.timeSeconds=0.5; eventParams.eventFrequencyHz=1.0f;
    eventParams.eventDurationSeconds=10.0f; eventParams.bandIntensity=0.0f; eventParams.horizontalTearAmount=0.0f;
    eventParams.horizontalDisplacementPx=0.0f; eventParams.dropoutsEnabled=false; eventParams.chromaEnabled=false;
    eventParams.brightEventProbability=1.0f; eventParams.darkEventProbability=0.0f;
    assert(processSyncCollapseLab(input,brightOnlyView,eventParams));
    eventParams.brightEventProbability=0.0f; eventParams.darkEventProbability=1.0f;
    assert(processSyncCollapseLab(input,darkOnlyView,eventParams));
    assert(brightOnly!=darkOnly);
    eventParams.brightEventProbability=0.0f; eventParams.darkEventProbability=0.0f;
    assert(processSyncCollapseLab(input,noEventsView,eventParams));
    std::vector<float> noLumaEvents(src.size()); ImageView noLumaEventsView{noLumaEvents.data(),w,h,w*4};
    SyncCollapseLabParams lumaDisabled=eventParams; lumaDisabled.lumaEnabled=false;
    assert(processSyncCollapseLab(input,noLumaEventsView,lumaDisabled));
    assert(noEvents==noLumaEvents);

    // Frozen mode must ignore composition time and produce a stable still.
    std::vector<float> frozenA(src.size()),frozenB(src.size());
    ImageView frozenAv{frozenA.data(),w,h,w*4},frozenBv{frozenB.data(),w,h,w*4};
    SyncCollapseLabParams frozen=p; frozen.animationMode=SyncDamageAnimationMode::Frozen;
    frozen.timeSeconds=0.0; assert(processSyncCollapseLab(input,frozenAv,frozen));
    frozen.timeSeconds=12.5; assert(processSyncCollapseLab(input,frozenBv,frozen));
    assert(frozenA==frozenB);

    // Generated-signal mode is independent of source visibility; full input blend is identity.
    std::vector<float> generated(src.size()),identity(src.size());
    ImageView generatedView{generated.data(),w,h,w*4},identityView{identity.data(),w,h,w*4};
    SyncCollapseLabParams generatedParams=p;
    generatedParams.inputMode=SyncDamageInputMode::GeneratedSignal;
    generatedParams.blendWithInput=0.0f;
    assert(processSyncCollapseLab(input,generatedView,generatedParams));
    assert(generated!=src);
    std::vector<float> alternateSource=src,generatedFromAlternate(src.size());
    for(std::size_t i=0;i<alternateSource.size();i+=4) {
        alternateSource[i]=1.0f-alternateSource[i];
        alternateSource[i+1]=1.0f-alternateSource[i+1];
        alternateSource[i+2]=1.0f-alternateSource[i+2];
    }
    ConstImageView alternateInput{alternateSource.data(),w,h,w*4};
    ImageView generatedAlternateView{generatedFromAlternate.data(),w,h,w*4};
    assert(processSyncCollapseLab(alternateInput,generatedAlternateView,generatedParams));
    assert(generatedFromAlternate==generated);
    generatedParams.blendWithInput=1.0f;
    assert(processSyncCollapseLab(input,identityView,generatedParams));
    assert(identity==src);

    // Disabled and zero-strength states are exact bypasses.
    SyncCollapseLabParams bypass=p; bypass.enabled=false;
    assert(processSyncCollapseLab(input,identityView,bypass)); assert(identity==src);
    bypass=p; bypass.effectAmount=0.0f;
    assert(processSyncCollapseLab(input,identityView,bypass)); assert(identity==src);

    // Preserve transparent pixels and premultiplied alpha.
    std::vector<float> transparent=src,transparentOut(src.size());
    for(std::size_t i=0;i<transparent.size();i+=4) {
        transparent[i]=0.0f; transparent[i+1]=0.0f; transparent[i+2]=0.0f; transparent[i+3]=0.0f;
    }
    ConstImageView transparentInput{transparent.data(),w,h,w*4};
    ImageView transparentView{transparentOut.data(),w,h,w*4};
    assert(processSyncCollapseLab(transparentInput,transparentView,p));
    assert(transparentOut==transparent);

    std::vector<float> semi=src,semiOut(src.size());
    for(std::size_t i=0;i<semi.size();i+=4) {
        semi[i]*=0.35f; semi[i+1]*=0.35f; semi[i+2]*=0.35f; semi[i+3]=0.35f;
    }
    ConstImageView semiInput{semi.data(),w,h,w*4};
    ImageView semiView{semiOut.data(),w,h,w*4};
    assert(processSyncCollapseLab(semiInput,semiView,p));
    for(std::size_t i=3;i<semiOut.size();i+=4) assert(std::fabs(semiOut[i]-0.35f)<1.0e-6f);

    return 0;
}
