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

 // AGC Pump must respond on ordinary mid-level footage, not only near-white peaks.
 RfInstabilityParams agcOnly;agcOnly.agcPump=1.0f;agcOnly.seed=17;agcOnly.frame=21;
 std::vector<float>agcA(s.size()),agcB(s.size());
 ImageView agcAv{agcA.data(),w,h,w*4},agcBv{agcB.data(),w,h,w*4};
 assert(processRfInstability(si,agcAv,agcOnly));assert(agcA!=s);
 agcOnly.frame=35;assert(processRfInstability(si,agcBv,agcOnly));assert(agcA!=agcB);

 // RF Drift must displace scanlines horizontally instead of merely changing
 // the whole frame's gain or opacity.
 std::vector<float>gradient(s.size()),driftOut(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){const size_t k=(size_t(y)*w+x)*4;gradient[k]=float(x)/float(w-1);gradient[k+1]=.4f;gradient[k+2]=.2f;gradient[k+3]=1.0f;}
 ConstImageView gi{gradient.data(),w,h,w*4};ImageView go{driftOut.data(),w,h,w*4};
 RfInstabilityParams driftOnly;driftOnly.drift=1.0f;driftOnly.frame=22;driftOnly.seed=31;
 assert(processRfInstability(gi,go,driftOnly));assert(driftOut!=gradient);
 std::vector<float>premul=s,out(s.size());for(size_t i=0;i<premul.size();i+=4){premul[i]*=.25f;premul[i+1]*=.25f;premul[i+2]*=.25f;premul[i+3]=.25f;}
 ConstImageView pi{premul.data(),w,h,w*4};ImageView po{out.data(),w,h,w*4};assert(processRfInstability(pi,po,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.25f)<1e-6f);
 return 0;}
