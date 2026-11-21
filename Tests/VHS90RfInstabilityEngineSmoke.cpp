#include "VHS90RfInstabilityEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=64,h=32;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size()),z(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){const auto k=(y*w+x)*4;s[k]=.45f;s[k+1]=.42f;s[k+2]=.38f;s[k+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4},zi{z.data(),w,h,w*4};
 RfInstabilityParams p;p.drift=.8f;p.hum=.6f;p.amNonlinearity=.7f;p.agcPump=.65f;p.seed=17;p.frame=21;
 assert(processRfInstability(si,ai,p));assert(processRfInstability(si,bi,p));assert(a==b&&a!=s);
 p.frame=22;assert(processRfInstability(si,ci,p));assert(a!=c);
 RfInstabilityParams off;assert(processRfInstability(si,zi,off));assert(z==s);
 std::vector<float>premul=s,out(s.size());for(size_t i=0;i<premul.size();i+=4){premul[i]*=.25f;premul[i+1]*=.25f;premul[i+2]*=.25f;premul[i+3]=.25f;}
 ConstImageView pi{premul.data(),w,h,w*4};ImageView po{out.data(),w,h,w*4};assert(processRfInstability(pi,po,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.25f)<1e-6f);
 return 0;}
