#include "VHS90AnalogGlowEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=48,h=32;std::vector<float>s(w*h*4),z(s.size()),a(s.size()),b(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(size_t(y)*w+x)*4;float v=(x==w/2&&y==h/2)?1.0f:.05f;s[i]=v;s[i+1]=v;s[i+2]=v;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView zi{z.data(),w,h,w*4},ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4};
 AnalogGlowParams p;assert(processAnalogGlow(si,zi,p));assert(z==s);p.amount=.9f;
 assert(processAnalogGlow(si,ai,p));assert(processAnalogGlow(si,bi,p));assert(a==b&&a!=s);
 for(size_t i=3;i<a.size();i+=4)assert(a[i]==1.0f);
 // Glow must be highlight-selective: spread around the hot pixel, leave dark field untouched.
 const size_t center=(size_t(h/2)*w+w/2)*4;
 const size_t near=(size_t(h/2)*w+w/2+5)*4;
 const size_t far=(size_t(0)*w+0)*4;
 assert(a[near]>s[near]);
 assert(std::fabs(a[far]-s[far])<1e-6f);
 std::vector<float> dark(s.size()),darkOut(s.size());
 for(size_t i=0;i<dark.size();i+=4){dark[i]=dark[i+1]=dark[i+2]=.12f;dark[i+3]=1.0f;}
 ConstImageView darkView{dark.data(),w,h,w*4};ImageView darkOutView{darkOut.data(),w,h,w*4};
 assert(processAnalogGlow(darkView,darkOutView,p));assert(darkOut==dark);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.4f;semi[i+1]*=.4f;semi[i+2]*=.4f;semi[i+3]=.4f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView oi{out.data(),w,h,w*4};p.premultipliedAlpha=true;assert(processAnalogGlow(pi,oi,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.4f)<1e-6f);
 return 0;}
