#include "VHS90CrtEqualizerEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=128,h=32;std::vector<float>s(w*h*4),z(s.size()),a(s.size()),b(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(size_t(y)*w+x)*4;float v=(x%8<4)?.9f:.1f;s[i]=v;s[i+1]=.4f;s[i+2]=.7f;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView zi{z.data(),w,h,w*4},ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4};
 CrtEqualizerParams p;assert(processCrtThreeBandEqualizer(si,zi,p));assert(z==s);p.amount=1.0f;
 assert(processCrtThreeBandEqualizer(si,ai,p));assert(processCrtThreeBandEqualizer(si,bi,p));assert(a==b&&a!=s);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.5f;semi[i+1]*=.5f;semi[i+2]*=.5f;semi[i+3]=.5f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView oi{out.data(),w,h,w*4};assert(processCrtThreeBandEqualizer(pi,oi,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.5f)<1e-6f);
 return 0;}
