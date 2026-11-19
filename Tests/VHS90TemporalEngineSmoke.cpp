#include "VHS90TemporalEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=24,h=16;std::vector<float>cur(w*h*4),prev(cur.size()),a(cur.size()),b(cur.size()),off(cur.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;cur[i]=.1f;cur[i+1]=.2f;cur[i+2]=.3f;cur[i+3]=1.0f;prev[i]=.8f;prev[i+1]=.1f;prev[i+2]=.05f;prev[i+3]=1.0f;}
 ConstImageView ci{cur.data(),w,h,w*4},pi{prev.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},oi{off.data(),w,h,w*4};
 TemporalGhostParams p;p.amount=.6f;p.offsetXPixels=1.5f;p.fieldWeave=.4f;
 assert(processTemporalGhost(ci,pi,ai,p));assert(processTemporalGhost(ci,pi,bi,p));assert(a==b&&a!=cur);
 TemporalGhostParams zero;zero.amount=0.0f;assert(processTemporalGhost(ci,pi,oi,zero));assert(off==cur);
 std::vector<float>semi=cur,semiOut(cur.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.35f;semi[i+1]*=.35f;semi[i+2]*=.35f;semi[i+3]=.35f;}
 ConstImageView si{semi.data(),w,h,w*4};ImageView so{semiOut.data(),w,h,w*4};p.premultipliedAlpha=true;assert(processTemporalGhost(si,pi,so,p));
 for(size_t i=3;i<semiOut.size();i+=4)assert(std::fabs(semiOut[i]-.35f)<1e-6f);
 return 0;}
