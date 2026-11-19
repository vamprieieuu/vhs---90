#include "VHS90TapeSurfaceEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=48,h=32;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;s[i]=.25f+.5f*float(x)/float(w-1);s[i+1]=.35f;s[i+2]=.65f-.3f*float(y)/float(h-1);s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4};
 TapeSurfaceParams p;p.seed=17;p.frame=12;p.defectDensity=.9f;p.edgeLift=.2f;assert(processTapeSurface(si,ai,p));assert(processTapeSurface(si,bi,p));assert(a==b);
 p.frame=13;assert(processTapeSurface(si,ci,p));assert(a!=c);
 TapeSurfaceParams off;off.defectDensity=0;off.edgeLift=0;off.surfaceRoughness=0;std::fill(b.begin(),b.end(),0);assert(processTapeSurface(si,bi,off));assert(b==s);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.4f;semi[i+1]*=.4f;semi[i+2]*=.4f;semi[i+3]=.4f;}
 ConstImageView ssi{semi.data(),w,h,w*4};ImageView so{out.data(),w,h,w*4};p.frame=12;p.premultipliedAlpha=true;assert(processTapeSurface(ssi,so,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.4f)<1e-6f);
 for(size_t i=0;i<out.size();i+=4)assert(std::isfinite(out[i])&&std::isfinite(out[i+1])&&std::isfinite(out[i+2]));
 const auto x=sampleTapeSurface(.31f,.42f,p),y=sampleTapeSurface(.31f,.42f,p);assert(x.magnetization==y.magnetization&&x.spacingLift==y.spacingLift);return 0;}
