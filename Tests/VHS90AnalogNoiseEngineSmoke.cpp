#include "VHS90AnalogNoiseEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=48,h=24;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size()),off(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t k=(y*w+x)*4;s[k]=.4f;s[k+1]=.45f;s[k+2]=.5f;s[k+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4},oi{off.data(),w,h,w*4};
 AnalogNoiseParams p;p.lumaAmount=.8f;p.chromaAmount=.7f;p.compositeAmount=.3f;p.hueBiasDegrees=45;p.seed=5;p.frame=8;
 assert(processAnalogNoise(si,ai,p));assert(processAnalogNoise(si,bi,p));assert(a==b&&a!=s);
 p.seed=6;assert(processAnalogNoise(si,ci,p));assert(a!=c);
 AnalogNoiseParams tape;tape.tapeLineAmount=.85f;tape.tapeLineSpeed=1.0f;tape.framesPerSecond=29.97f;tape.seed=17;tape.frame=9;
 std::vector<float> tapeA(s.size()),tapeB(s.size()),tapeNext(s.size()),tapeOff(s.size());
 ImageView ta{tapeA.data(),w,h,w*4},tb{tapeB.data(),w,h,w*4},tn{tapeNext.data(),w,h,w*4},to{tapeOff.data(),w,h,w*4};
 assert(processAnalogNoise(si,ta,tape));assert(processAnalogNoise(si,tb,tape));assert(tapeA==tapeB&&tapeA!=s);
 tape.frame=120;assert(processAnalogNoise(si,tn,tape));assert(tapeNext!=tapeA);
 tape.tapeLineAmount=0;assert(processAnalogNoise(si,to,tape));assert(tapeOff==s);
 AnalogNoiseParams z;assert(processAnalogNoise(si,oi,z));assert(off==s);
 std::vector<float>semi=s,semiOut(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.3f;semi[i+1]*=.3f;semi[i+2]*=.3f;semi[i+3]=.3f;}
 ConstImageView ssi{semi.data(),w,h,w*4};ImageView so{semiOut.data(),w,h,w*4};p.premultipliedAlpha=true;assert(processAnalogNoise(ssi,so,p));
 for(size_t i=3;i<semiOut.size();i+=4)assert(std::fabs(semiOut[i]-.3f)<1e-6f);
    AnalogNoiseParams loss;loss.chromaLoss=5000;loss.seed=91;loss.frame=4;
    std::vector<float> lossA(s.size()),lossB(s.size());ImageView lossAView{lossA.data(),w,h,w*4},lossBView{lossB.data(),w,h,w*4};
    assert(processAnalogNoise(si,lossAView,loss));assert(processAnalogNoise(si,lossBView,loss));assert(lossA==lossB&&lossA!=s);
 AnalogNoiseParams phase;phase.chromaPhaseNoise=0.45f;phase.seed=33;phase.frame=8;
 std::vector<float> phaseA(s.size()),phaseB(s.size()),phaseOff(s.size());ImageView pa{phaseA.data(),w,h,w*4},pb{phaseB.data(),w,h,w*4},po{phaseOff.data(),w,h,w*4};
 assert(processAnalogNoise(si,pa,phase));assert(processAnalogNoise(si,pb,phase));assert(phaseA==phaseB&&phaseA!=s);
 phase.chromaPhaseNoise=0.0f;assert(processAnalogNoise(si,po,phase));assert(phaseOff==s);

 AnalogNoiseParams vhs6;vhs6.grainAmount=.75f;vhs6.grainSize=3;vhs6.lumaAmount=.6f;vhs6.chromaAmount=.5f;vhs6.shadowBias=.8f;vhs6.noiseDetail=.7f;vhs6.seed=123;vhs6.frame=15;
 std::vector<float> vhs6a(s.size()),vhs6b(s.size()),vhs6c(s.size()),vhs6off(s.size());ImageView v6a{vhs6a.data(),w,h,w*4},v6b{vhs6b.data(),w,h,w*4},v6c{vhs6c.data(),w,h,w*4},v6o{vhs6off.data(),w,h,w*4};
 assert(processAnalogNoise(si,v6a,vhs6));assert(processAnalogNoise(si,v6b,vhs6));assert(vhs6a==vhs6b&&vhs6a!=s);
 vhs6.seed++;assert(processAnalogNoise(si,v6c,vhs6));assert(vhs6a!=vhs6c);
 vhs6.proceduralNoiseEnabled=false;assert(processAnalogNoise(si,v6o,vhs6));assert(vhs6off==s);
 return 0;}
