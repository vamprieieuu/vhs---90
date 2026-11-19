#include "VHS90AnalogGeometryEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=40,h=24;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size()),scan(s.size()),off(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;s[i]=float(x)/float(w-1);s[i+1]=float(y)/float(h-1);s[i+2]=.4f;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4},sc{scan.data(),w,h,w*4},oi{off.data(),w,h,w*4};
 AnalogGeometryParams p;p.lineJitterPixels=1.5f;p.topSkewPixels=3.0f;p.edgePincushionPixels=2.0f;p.seed=17;p.frame=6;
 assert(processAnalogGeometry(si,ai,p));assert(processAnalogGeometry(si,bi,p));assert(a==b&&a!=s);
 p.frame=7;assert(processAnalogGeometry(si,ci,p));assert(a!=c);
 AnalogGeometryParams zero;assert(processAnalogGeometry(si,oi,zero));assert(off==s);
 ScanlineParams sp;sp.amount=.7f;sp.frequency=.5f;assert(processScanlines(si,sc,sp));assert(scan!=s);
 std::fill(off.begin(),off.end(),0.0f);sp.amount=0;assert(processScanlines(si,oi,sp));assert(off==s);
 for(size_t i=3;i<scan.size();i+=4)assert(scan[i]==1.0f);
 return 0;}
