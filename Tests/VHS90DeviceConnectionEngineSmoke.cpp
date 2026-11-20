#include "VHS90DeviceConnectionEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
using namespace vhs90;
int main(){
 const int w=96,h=64;std::vector<float> src(std::size_t(w*h*4),0.0f);
 for(int i=0;i<w*h;++i){src[std::size_t(i)*4]=0.08f;src[std::size_t(i)*4+1]=0.08f;src[std::size_t(i)*4+2]=0.08f;src[std::size_t(i)*4+3]=1.0f;}
 ConstImageView in{src.data(),w,h,w*4};DeviceConnectionParams p;p.amount=.9f;p.density=.45f;p.nodeSize=2.0f;p.speed=1.0f;p.seed=712;p.frame=20;
 std::vector<float>a(src.size()),b(src.size()),c(src.size()),off(src.size());ImageView oa{a.data(),w,h,w*4},ob{b.data(),w,h,w*4},oc{c.data(),w,h,w*4},oo{off.data(),w,h,w*4};
 assert(processDeviceConnections(in,oa,p));assert(processDeviceConnections(in,ob,p));assert(a==b);assert(a!=src);
 p.frame=21;assert(processDeviceConnections(in,oc,p));assert(a!=c);
 p.amount=0;assert(processDeviceConnections(in,oo,p));assert(off==src);
 p.amount=.8f;p.seed++;assert(processDeviceConnections(in,oc,p));assert(a!=c);
 p.density=1.0f;assert(processDeviceConnections(in,oc,p));assert(a!=c);
 p.distribution=1.0f;assert(processDeviceConnections(in,oc,p));assert(a!=c);
 for(std::size_t i=0;i<src.size();i+=4)assert(std::abs(a[i+3]-1.0f)<1e-6f);
 return 0;
}
