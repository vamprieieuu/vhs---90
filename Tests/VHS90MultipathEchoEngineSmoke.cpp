#include "VHS90MultipathEchoEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=96,h=24;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size()),z(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;const float v=x<18?1.0f:0.08f;s[i]=v;s[i+1]=v*.8f;s[i+2]=v*.6f;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4},zi{z.data(),w,h,w*4};
 MultipathEchoParams p;p.gain=.4f;p.delayPixels=16;p.ensemble=.8f;p.seed=3;p.frame=8;
 assert(processMultipathEcho(si,ai,p));assert(processMultipathEcho(si,bi,p));assert(a==b&&a!=s);
 p.frame=9;assert(processMultipathEcho(si,ci,p));assert(a!=c);
 MultipathEchoParams off;assert(processMultipathEcho(si,zi,off));assert(z==s);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.3f;semi[i+1]*=.3f;semi[i+2]*=.3f;semi[i+3]=.3f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView po{out.data(),w,h,w*4};assert(processMultipathEcho(pi,po,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.3f)<1e-6f);
 return 0;}
