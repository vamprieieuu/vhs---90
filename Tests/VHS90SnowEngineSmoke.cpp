#include "VHS90SnowEngine.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    using namespace vhs90;
    constexpr int w=96,h=64;
    std::vector<float> src(w*h*4),a(src.size()),b(src.size()),c(src.size());
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        const std::size_t i=(std::size_t(y)*w+x)*4;
        src[i]=0.15f+0.65f*float(x)/float(w-1);
        src[i+1]=0.12f+0.55f*float(y)/float(h-1);
        src[i+2]=0.25f+0.25f*float((x+y)%9)/8.0f;
        src[i+3]=1.0f;
    }
    ConstImageView input{src.data(),w,h,w*4};
    ImageView av{a.data(),w,h,w*4},bv{b.data(),w,h,w*4},cv{c.data(),w,h,w*4};
    SnowParams p; p.enabled=true; p.intensity=1.0f; p.density=0.82f; p.sizePx=2.2f;
    p.distribution=0.75f; p.contrast=0.95f; p.brightness=0.72f; p.color=0.9f;
    p.colored=true; p.temporalVariation=1.0f; p.randomness=1.0f;
    p.animationSpeed=1.0f; p.seed=0x5150u; p.timeSeconds=0.25;
    assert(processSnow(input,av,p)); assert(processSnow(input,bv,p));
    assert(a==b && a!=src);

    SnowParams later=p; later.timeSeconds=0.90;
    assert(processSnow(input,cv,later)); assert(c!=a);

    // Zero temporal variation freezes the snow field regardless of composition time.
    std::vector<float> frozenA(src.size()),frozenB(src.size());
    ImageView frozenAv{frozenA.data(),w,h,w*4},frozenBv{frozenB.data(),w,h,w*4};
    SnowParams frozen=p; frozen.temporalVariation=0.0f; frozen.timeSeconds=0.0;
    assert(processSnow(input,frozenAv,frozen));
    frozen.timeSeconds=9.0; assert(processSnow(input,frozenBv,frozen));
    assert(frozenA==frozenB);

    // Snow can be disabled without disabling the independent background-noise layer.
    std::vector<float> clean(src.size()),backgroundA(src.size()),backgroundB(src.size());
    ImageView cleanView{clean.data(),w,h,w*4},backgroundViewA{backgroundA.data(),w,h,w*4},backgroundViewB{backgroundB.data(),w,h,w*4};
    SnowParams off; off.enabled=false;
    assert(processSnow(input,cleanView,off)); assert(clean==src);
    off.backgroundNoiseIntensity=0.65f; off.seed=19;
    assert(processSnow(input,backgroundViewA,off)); assert(backgroundA!=src);
    off.intensity=0.0f; off.density=0.0f; off.sizePx=15.0f;
    assert(processSnow(input,backgroundViewB,off)); assert(backgroundA==backgroundB);

    // Background noise does not turn on the snow particles.
    SnowParams snowOnly=p; snowOnly.backgroundNoiseIntensity=0.0f;
    std::vector<float> snowOutput(src.size()),snowOff(src.size());
    ImageView snowView{snowOutput.data(),w,h,w*4},snowOffView{snowOff.data(),w,h,w*4};
    assert(processSnow(input,snowView,snowOnly));
    snowOnly.enabled=false; assert(processSnow(input,snowOffView,snowOnly));
    assert(snowOff==src);

    // Monochrome and colored snow are distinct, and seed changes the particle field.
    std::vector<float> mono(src.size()),colored(src.size()),otherSeed(src.size());
    ImageView monoView{mono.data(),w,h,w*4},coloredView{colored.data(),w,h,w*4},seedView{otherSeed.data(),w,h,w*4};
    SnowParams monoP=p; monoP.colored=false;
    assert(processSnow(input,monoView,monoP));
    SnowParams colorP=p; colorP.colored=true;
    assert(processSnow(input,coloredView,colorP));
    assert(mono!=colored);
    colorP.seed+=1; assert(processSnow(input,seedView,colorP));
    assert(otherSeed!=colored);

    // Preserve transparent and premultiplied alpha.
    std::vector<float> transparent=src,transparentOut(src.size());
    for(std::size_t i=0;i<transparent.size();i+=4) {
        transparent[i]=transparent[i+1]=transparent[i+2]=transparent[i+3]=0.0f;
    }
    ConstImageView transparentInput{transparent.data(),w,h,w*4};
    ImageView transparentView{transparentOut.data(),w,h,w*4};
    assert(processSnow(transparentInput,transparentView,p)); assert(transparentOut==transparent);

    std::vector<float> semi=src,semiOut(src.size());
    for(std::size_t i=0;i<semi.size();i+=4) {
        semi[i]*=0.4f; semi[i+1]*=0.4f; semi[i+2]*=0.4f; semi[i+3]=0.4f;
    }
    ConstImageView semiInput{semi.data(),w,h,w*4};
    ImageView semiView{semiOut.data(),w,h,w*4};
    assert(processSnow(semiInput,semiView,p));
    for(std::size_t i=3;i<semiOut.size();i+=4) assert(std::fabs(semiOut[i]-0.4f)<1.0e-6f);
    return 0;
}
