#include "VHS90TapeSurfaceEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=48,h=32;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;s[i]=.25f+.5f*float(x)/float(w-1);s[i+1]=.35f;s[i+2]=.65f-.3f*float(y)/float(h-1);s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4};
 TapeSurfaceParams p;p.seed=17;p.frame=12;p.defectDensity=.9f;p.edgeLift=.2f;p.pinholeAmount=.5f;p.flakeAmount=.6f;p.moldAmount=.7f;p.vaselineAmount=.4f;assert(processTapeSurface(si,ai,p));assert(processTapeSurface(si,bi,p));assert(a==b);
 p.frame=13;assert(processTapeSurface(si,ci,p));assert(a!=c);
 TapeSurfaceParams off;off.defectDensity=0;off.edgeLift=0;off.surfaceRoughness=0;std::fill(b.begin(),b.end(),0);assert(processTapeSurface(si,bi,off));assert(b==s);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.4f;semi[i+1]*=.4f;semi[i+2]*=.4f;semi[i+3]=.4f;}
 ConstImageView ssi{semi.data(),w,h,w*4};ImageView so{out.data(),w,h,w*4};p.frame=12;p.premultipliedAlpha=true;assert(processTapeSurface(ssi,so,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.4f)<1e-6f);
 for(size_t i=0;i<out.size();i+=4)assert(std::isfinite(out[i])&&std::isfinite(out[i+1])&&std::isfinite(out[i+2]));
 constexpr int sw=32,sh=8;std::vector<float>stripe(sw*sh*4),soft(sw*sh*4);
 for(int yy=0;yy<sh;++yy)for(int xx=0;xx<sw;++xx){size_t k=(yy*sw+xx)*4;float v=(xx&1)?0.9f:0.1f;stripe[k]=stripe[k+1]=stripe[k+2]=v;stripe[k+3]=1.0f;}
 ConstImageView sti{stripe.data(),sw,sh,sw*4};ImageView sto{soft.data(),sw,sh,sw*4};
 TapeSurfaceParams spacing;spacing.defectDensity=0;spacing.surfaceRoughness=0;spacing.edgeLift=1;assert(processTapeSurface(sti,sto,spacing));
 float before=0,after=0;for(int xx=0;xx<sw-1;++xx){before+=std::fabs(stripe[xx*4]-stripe[(xx+1)*4]);after+=std::fabs(soft[xx*4]-soft[(xx+1)*4]);}
 assert(after<before*0.5f);
 const auto x=sampleTapeSurface(.31f,.42f,p),y=sampleTapeSurface(.31f,.42f,p);assert(x.magnetization==y.magnetization&&x.spacingLift==y.spacingLift);
 const auto physicalA=sampleTapeSurfaceAtMm(128.375,4.25f,p),physicalB=sampleTapeSurfaceAtMm(128.375,4.25f,p),physicalC=sampleTapeSurfaceAtMm(128.475,4.25f,p);
 assert(physicalA.magnetization==physicalB.magnetization&&physicalA.spacingLift==physicalB.spacingLift&&physicalA.shed==physicalB.shed);
 assert(std::isfinite(physicalA.magnetization)&&std::isfinite(physicalA.spacingLift)&&std::isfinite(physicalA.film)&&std::isfinite(physicalA.shed));
 assert(physicalA.magnetization!=physicalC.magnetization||physicalA.spacingLift!=physicalC.spacingLift||physicalA.film!=physicalC.film);
 return 0;}
