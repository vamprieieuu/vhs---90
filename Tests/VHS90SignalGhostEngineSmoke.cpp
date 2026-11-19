#include "VHS90SignalGhostEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=128,h=16;std::vector<float>s(w*h*4),z(s.size()),a(s.size()),b(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(size_t(y)*w+x)*4;float v=(x<24)?1.0f:0.0f;s[i]=v;s[i+1]=.2f;s[i+2]=.4f;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView zi{z.data(),w,h,w*4},ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4};
 SignalGhostParams p;assert(processSignalGhost(si,zi,p));assert(z==s);p.amount=.7f;p.chromaDelayPixels=6;
 assert(processSignalGhost(si,ai,p));assert(processSignalGhost(si,bi,p));assert(a==b&&a!=s);
 for(size_t i=3;i<a.size();i+=4)assert(a[i]==1.0f);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.5f;semi[i+1]*=.5f;semi[i+2]*=.5f;semi[i+3]=.5f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView oi{out.data(),w,h,w*4};p.premultipliedAlpha=true;assert(processSignalGhost(pi,oi,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.5f)<1e-6f);
 return 0;}
