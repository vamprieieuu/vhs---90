#include "VHS90TrackReadEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=48,h=32;std::vector<float>s(w*h*4),off(s.size()),again(s.size()),zero(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;float v=(y<10?1.0f:(y<20?.05f:.8f));s[i]=v;s[i+1]=.3f;s[i+2]=1-v;s[i+3]=1;}
 ConstImageView si{s.data(),w,h,w*4};ImageView oi{off.data(),w,h,w*4},ai{again.data(),w,h,w*4},zi{zero.data(),w,h,w*4};
 TrackReadParams p;p.amount=1;p.trackingOffset=.65f;p.speed=2;assert(processTrackRead(si,oi,p));assert(processTrackRead(si,ai,p));assert(off==again&&off!=s);
 p.amount=0;assert(processTrackRead(si,zi,p));assert(zero==s);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.4f;semi[i+1]*=.4f;semi[i+2]*=.4f;semi[i+3]=.4f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView po{out.data(),w,h,w*4};p.amount=1;assert(processTrackRead(pi,po,p));for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.4f)<1e-6f);return 0;}
