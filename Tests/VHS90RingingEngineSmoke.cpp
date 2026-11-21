#include "VHS90RingingEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=128,h=48;std::vector<float>s(w*h*4),off(s.size()),a(s.size()),b(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(size_t(y)*w+x)*4;float v=x<64?.08f:.92f;s[i]=v;s[i+1]=v*.9f;s[i+2]=v*.8f;s[i+3]=1;}
 ConstImageView si{s.data(),w,h,w*4};ImageView oi{off.data(),w,h,w*4},ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4};
 RingingParams p;assert(processRinging(si,oi,p));assert(off==s);p.strength=.75f;p.power=2;
 assert(processRinging(si,ai,p));assert(processRinging(si,bi,p));assert(a==b&&a!=s);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.4f;semi[i+1]*=.4f;semi[i+2]*=.4f;semi[i+3]=.4f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView po{out.data(),w,h,w*4};assert(processRinging(pi,po,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.4f)<1e-6f);
 return 0;}
