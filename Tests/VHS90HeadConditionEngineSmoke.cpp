#include "VHS90HeadConditionEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=40,h=24;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),z(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){auto i=(y*w+x)*4;float v=(x%4<2)?0.9f:0.1f;s[i]=v;s[i+1]=0.3f;s[i+2]=1-v;s[i+3]=1;}
 ConstImageView in{s.data(),w,h,w*4};ImageView oa{a.data(),w,h,w*4},ob{b.data(),w,h,w*4},oz{z.data(),w,h,w*4};
 HeadConditionParams p;p.amount=1;p.wear=.8f;p.dirt=.7f;p.buildup=.9f;p.mismatch=.4f;p.grease=.5f;p.frame=17;
 assert(processHeadCondition(in,oa,p));assert(processHeadCondition(in,ob,p));assert(a==b&&a!=s);
 p.amount=0;assert(processHeadCondition(in,oz,p));assert(z==s);
 HeadConditionParams early=p,late=p;early.amount=1;late.amount=1;early.frame=0;late.frame=240;
 std::vector<float> earlyOut(s.size()),lateOut(s.size());ImageView earlyView{earlyOut.data(),w,h,w*4},lateView{lateOut.data(),w,h,w*4};
 assert(processHeadCondition(in,earlyView,early));assert(processHeadCondition(in,lateView,late));assert(earlyOut!=lateOut);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.35f;semi[i+1]*=.35f;semi[i+2]*=.35f;semi[i+3]=.35f;}
 ConstImageView si{semi.data(),w,h,w*4};ImageView so{out.data(),w,h,w*4};p.amount=1;assert(processHeadCondition(si,so,p));for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.35f)<1e-6f);
}
