#include "VHS90ReceiverInstabilityEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=72,h=28;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size()),z(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;float v=(x%9<4)?.8f:.18f;s[i]=v;s[i+1]=.42f;s[i+2]=1.0f-v*.5f;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4},zi{z.data(),w,h,w*4};
 ReceiverInstabilityParams p;p.afcHunt=.6f;p.chromaFlutter=.7f;p.ycCrosstalk=.5f;p.seed=11;p.frame=5;
 assert(processReceiverInstability(si,ai,p));assert(processReceiverInstability(si,bi,p));assert(a==b&&a!=s);
 p.frame=6;assert(processReceiverInstability(si,ci,p));assert(a!=c);
 ReceiverInstabilityParams off;assert(processReceiverInstability(si,zi,off));assert(z==s);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.35f;semi[i+1]*=.35f;semi[i+2]*=.35f;semi[i+3]=.35f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView po{out.data(),w,h,w*4};assert(processReceiverInstability(pi,po,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.35f)<1e-6f);
 return 0;}
