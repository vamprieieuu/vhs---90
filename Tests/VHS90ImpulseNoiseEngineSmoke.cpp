#include "VHS90ImpulseNoiseEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=80,h=32;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size()),z(s.size());
 for(size_t i=0;i<s.size();i+=4){s[i]=.42f;s[i+1]=.44f;s[i+2]=.46f;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4},zi{z.data(),w,h,w*4};
 ImpulseNoiseParams p;p.amount=.8f;p.seed=7;p.frame=19;
 assert(processImpulseNoise(si,ai,p));assert(processImpulseNoise(si,bi,p));assert(a==b&&a!=s);
 p.frame=20;assert(processImpulseNoise(si,ci,p));assert(a!=c);
 ImpulseNoiseParams off;assert(processImpulseNoise(si,zi,off));assert(z==s);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.4f;semi[i+1]*=.4f;semi[i+2]*=.4f;semi[i+3]=.4f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView po{out.data(),w,h,w*4};assert(processImpulseNoise(pi,po,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.4f)<1e-6f);
 return 0;}
