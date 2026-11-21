#include "VHS90WaveformEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=96,h=64;std::vector<float>s(w*h*4),off(s.size()),a(s.size()),b(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){const size_t i=(size_t(y)*w+x)*4;s[i]=float(x)/float(w-1);s[i+1]=float(y)/float(h-1);s[i+2]=((x/8)&1)?1.0f:.05f;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView oi{off.data(),w,h,w*4},ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4};
 NtscWaveformParams p;p.strength=0;assert(processNtscWaveformRoundTrip(si,oi,p));assert(off==s);
 p.strength=1;p.frame=12;p.effects.random_seed=123;p.decoder.random_seed=456;
 assert(processNtscWaveformRoundTrip(si,ai,p));assert(processNtscWaveformRoundTrip(si,bi,p));assert(a==b&&a!=s);
 for(size_t i=3;i<a.size();i+=4)assert(a[i]==1.0f);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.35f;semi[i+1]*=.35f;semi[i+2]*=.35f;semi[i+3]=.35f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView po{out.data(),w,h,w*4};p.premultipliedAlpha=true;assert(processNtscWaveformRoundTrip(pi,po,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.35f)<1e-6f);
 return 0;}
