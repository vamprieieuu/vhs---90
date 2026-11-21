#include "VHS90ChromaShiftEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=80,h=8;std::vector<float>s(w*h*4),z(s.size()),a(s.size()),b(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){const size_t i=(size_t(y)*w+x)*4;const float v=(x%2)?1.0f:0.0f;s[i]=v;s[i+1]=.2f;s[i+2]=1.0f-v;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView zi{z.data(),w,h,w*4},ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4};
 ChromaChannelShiftParams p;assert(processChromaChannelShift(si,zi,p));assert(z==s);
 p.shiftPixels=4;assert(processChromaChannelShift(si,ai,p));assert(processChromaChannelShift(si,bi,p));assert(a==b&&a!=s);
 for(size_t i=3;i<a.size();i+=4)assert(a[i]==1.0f);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.5f;semi[i+1]*=.5f;semi[i+2]*=.5f;semi[i+3]=.5f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView oi{out.data(),w,h,w*4};p.premultipliedAlpha=true;assert(processChromaChannelShift(pi,oi,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.5f)<1e-6f);
 std::vector<float> reg(s.size()),reg2(s.size());ImageView ri{reg.data(),w,h,w*4},r2i{reg2.data(),w,h,w*4};
 RgbChannelRegistrationParams rp;assert(processRgbChannelRegistration(si,ri,rp));assert(reg==s);
 rp.redOffsetPixels=2.5f;rp.blueOffsetPixels=-3.0f;assert(processRgbChannelRegistration(si,ri,rp));assert(processRgbChannelRegistration(si,r2i,rp));assert(reg==reg2&&reg!=s);
 for(size_t i=3;i<reg.size();i+=4)assert(reg[i]==1.0f);
 rp.redOffsetPixels=2.5f;rp.blueOffsetPixels=-3.0f;assert(processRgbChannelRegistration(pi,oi,rp));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.5f)<1e-6f);
 return 0;}
