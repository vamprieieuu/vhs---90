#include "VHS90BloomEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=32,h=24;std::vector<float>s(w*h*4,0),out(s.size()),again(s.size()),off(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;s[i+3]=1.0f;}
 const size_t center=((h/2)*w+w/2)*4;s[center]=s[center+1]=s[center+2]=1.0f;
 ConstImageView si{s.data(),w,h,w*4};ImageView oi{out.data(),w,h,w*4},ai{again.data(),w,h,w*4},zi{off.data(),w,h,w*4};
 BloomParams p;p.threshold=.5f;p.intensity=1.0f;p.spread=2.0f;p.levels=3;
 assert(processBloom(si,oi,p));assert(processBloom(si,ai,p));assert(out==again);
 assert(out[center+4]>0.0f || out[center-4]>0.0f || out[center+w*4]>0.0f);
 BloomParams zero;zero.intensity=0;assert(processBloom(si,zi,zero));assert(off==s);
 std::vector<float>semi=s,semiOut(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.5f;semi[i+1]*=.5f;semi[i+2]*=.5f;semi[i+3]=.5f;}
 ConstImageView ssi{semi.data(),w,h,w*4};ImageView so{semiOut.data(),w,h,w*4};p.premultipliedAlpha=true;assert(processBloom(ssi,so,p));
 for(size_t i=3;i<semiOut.size();i+=4)assert(std::fabs(semiOut[i]-.5f)<1e-6f);
 return 0;}
