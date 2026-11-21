#include "VHS90DropoutEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=160,h=64;std::vector<float>s(w*h*4),z(s.size()),a(s.size()),b(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(size_t(y)*w+x)*4;s[i]=.35f;s[i+1]=.55f;s[i+2]=.7f;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView zi{z.data(),w,h,w*4},ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4};
 DropoutParams p;assert(processDropouts(si,zi,p));assert(z==s);p.amount=.8f;p.seed=42;p.frame=8;
 assert(processDropouts(si,ai,p));assert(processDropouts(si,bi,p));assert(a==b&&a!=s);
 std::size_t changed=0;for(size_t i=0;i<a.size();i+=4)if(std::fabs(a[i]-s[i])>1e-5f||std::fabs(a[i+1]-s[i+1])>1e-5f||std::fabs(a[i+2]-s[i+2])>1e-5f)++changed;
 assert(changed>0&&changed<std::size_t(w*h)*3/4);
 for(size_t i=3;i<a.size();i+=4)assert(a[i]==1.0f);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.4f;semi[i+1]*=.4f;semi[i+2]*=.4f;semi[i+3]=.4f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView oi{out.data(),w,h,w*4};p.premultipliedAlpha=true;assert(processDropouts(pi,oi,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.4f)<1e-6f);
 return 0;}
