#include "VHS90TapeTransportEngine.h"
#include <cassert>
#include <vector>
int main(){using namespace vhs90;constexpr int w=32,h=16;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;s[i]=float(x)/float(w-1);s[i+1]=float(y)/float(h-1);s[i+2]=.4f;s[i+3]=(x<8)?0.0f:1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4};
 TapeTransportParams p;p.seed=9;p.frame=14;p.drumFlutterPixels=1.2f;p.flutterPixels=.5f;p.stickSlipPixels=.2f;p.lineJitterPixels=.1f;
 assert(processTapeTransportWarp(si,ai,p));assert(processTapeTransportWarp(si,bi,p));assert(a==b);
 p.frame=15;assert(processTapeTransportWarp(si,ci,p));assert(a!=c);
 TapeTransportParams off;std::fill(b.begin(),b.end(),0.0f);assert(processTapeTransportWarp(si,bi,off));assert(b==s);
 for(size_t i=3;i<a.size();i+=4)assert(a[i]==0.0f||a[i]==1.0f);
 return 0;}
