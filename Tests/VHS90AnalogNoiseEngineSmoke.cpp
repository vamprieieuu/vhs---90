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
 AnalogNoiseParams z;assert(processAnalogNoise(si,oi,z));assert(off==s);
 std::vector<float>semi=s,semiOut(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.3f;semi[i+1]*=.3f;semi[i+2]*=.3f;semi[i+3]=.3f;}
 ConstImageView ssi{semi.data(),w,h,w*4};ImageView so{semiOut.data(),w,h,w*4};p.premultipliedAlpha=true;assert(processAnalogNoise(ssi,so,p));
 for(size_t i=3;i<semiOut.size();i+=4)assert(std::fabs(semiOut[i]-.3f)<1e-6f);
    AnalogNoiseParams loss;loss.chromaLoss=5000;loss.seed=91;loss.frame=4;
    std::vector<float> lossA(s.size()),lossB(s.size());ImageView lossAView{lossA.data(),w,h,w*4},lossBView{lossB.data(),w,h,w*4};
    assert(processAnalogNoise(si,lossAView,loss));assert(processAnalogNoise(si,lossBView,loss));assert(lossA==lossB&&lossA!=s);
 return 0;}
