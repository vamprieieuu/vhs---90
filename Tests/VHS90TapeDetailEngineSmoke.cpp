#include "VHS90TapeDetailEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=160,h=12;std::vector<float>s(w*h*4),full(s.size()),low(s.size()),repeat(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){const size_t i=(size_t(y)*w+x)*4;const float v=(x%2)?1.0f:0.0f;s[i]=v;s[i+1]=.25f;s[i+2]=.7f;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView fi{full.data(),w,h,w*4},li{low.data(),w,h,w*4},ri{repeat.data(),w,h,w*4};
 TapeDetailParams p;assert(processTapeDetail(si,fi,p));assert(full==s);
 p.detail=.25f;p.minimumWidth=1;assert(processTapeDetail(si,li,p));assert(processTapeDetail(si,ri,p));assert(low==repeat&&low!=s);
 for(size_t i=3;i<low.size();i+=4)assert(low[i]==1.0f);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.4f;semi[i+1]*=.4f;semi[i+2]*=.4f;semi[i+3]=.4f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView oi{out.data(),w,h,w*4};p.premultipliedAlpha=true;assert(processTapeDetail(pi,oi,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.4f)<1e-6f);
 return 0;}
