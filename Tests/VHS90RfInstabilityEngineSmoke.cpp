#include "VHS90RfInstabilityEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=64,h=32;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size()),z(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){const auto k=(y*w+x)*4;s[k]=.45f;s[k+1]=.42f;s[k+2]=.38f;s[k+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4},zi{z.data(),w,h,w*4};
 RfInstabilityParams p;p.drift=.8f;p.hum=.6f;p.amNonlinearity=.7f;p.agcPump=.65f;p.seed=17;p.frame=21;
 assert(processRfInstability(si,ai,p));assert(processRfInstability(si,bi,p));assert(a==b&&a!=s);
 p.frame=22;assert(processRfInstability(si,ci,p));assert(a!=c);
 RfInstabilityParams off;assert(processRfInstability(si,zi,off));assert(z==s);
 // Isolate each receiver control so one combined preset cannot hide an inert control.
 RfInstabilityParams isolated;isolated.seed=17;isolated.frame=21;
 std::vector<float> one(s.size());
 ImageView oneView{one.data(),w,h,w*4};
 isolated.drift=.8f;assert(processRfInstability(si,oneView,isolated));assert(one!=s);
 for(size_t k=0;k<one.size();k+=4){assert(one[k]>0.0f&&one[k]<1.0f);assert(one[k+1]>0.0f&&one[k+1]<1.0f);assert(one[k+2]>0.0f&&one[k+2]<1.0f);}
 isolated={};isolated.seed=17;isolated.frame=21;isolated.hum=.8f;
 assert(processRfInstability(si,oneView,isolated));assert(one!=s);
 isolated={};isolated.seed=17;isolated.frame=21;isolated.amNonlinearity=.8f;
 assert(processRfInstability(si,oneView,isolated));assert(one!=s);
 // Regression: Drift alone must remain an affine luma change; neither AM
 // compression nor AGC pumping may leak in while their controls are zero.
 std::vector<float> twoLevels(s.size()),driftOnly(s.size());
 for(int yy=0;yy<h;++yy)for(int xx=0;xx<w;++xx){const auto k=(yy*w+xx)*4;const float v=xx<w/2?.20f:.95f;twoLevels[k]=twoLevels[k+1]=twoLevels[k+2]=v;twoLevels[k+3]=1.0f;}
 ConstImageView twoLevelsView{twoLevels.data(),w,h,w*4};
 isolated={};isolated.seed=17;isolated.frame=21;isolated.drift=.8f;
 ImageView driftOnlyView{driftOnly.data(),w,h,w*4};
 assert(processRfInstability(twoLevelsView,driftOnlyView,isolated));
 const int sampleY=h/2,sampleLowX=w/4,sampleHighX=3*w/4;
 const auto lowAt=(size_t(sampleY)*w+sampleLowX)*4,highAt=(size_t(sampleY)*w+sampleHighX)*4;
 const float inputDelta=twoLevels[highAt]-twoLevels[lowAt];
 const float outputDelta=driftOnly[highAt]-driftOnly[lowAt];
 assert(outputDelta/inputDelta>0.99f);
 std::vector<float> levelPattern(s.size());
 for(int yy=0;yy<h;++yy)for(int xx=0;xx<w;++xx){const auto k=(yy*w+xx)*4;const float v=xx<w/2?.18f:.96f;levelPattern[k]=levelPattern[k+1]=levelPattern[k+2]=v;levelPattern[k+3]=1.0f;}
 ConstImageView levelView{levelPattern.data(),w,h,w*4};
 assert(processRfInstability(levelView,oneView,isolated));
 float highDelta=0,lowDelta=0;for(int yy=0;yy<h;++yy)for(int xx=0;xx<w;++xx){const auto k=(yy*w+xx)*4;const float d=std::fabs(one[k]-levelPattern[k]);if(xx<w/2)lowDelta+=d;else highDelta+=d;}
 assert(highDelta>lowDelta*2.0f);
 isolated={};isolated.seed=17;isolated.frame=21;isolated.agcPump=.9f;
 std::vector<float> white(w*h*4,1.0f),pumpA(s.size()),pumpB(s.size());
 for(size_t k=3;k<white.size();k+=4)white[k]=1.0f;
 ConstImageView whiteView{white.data(),w,h,w*4};ImageView pumpAView{pumpA.data(),w,h,w*4},pumpBView{pumpB.data(),w,h,w*4};
 assert(processRfInstability(whiteView,pumpAView,isolated));isolated.frame=22;
 assert(processRfInstability(whiteView,pumpBView,isolated));assert(pumpA!=white&&pumpB!=white&&pumpA!=pumpB);
 std::vector<float>premul=s,out(s.size());for(size_t i=0;i<premul.size();i+=4){premul[i]*=.25f;premul[i+1]*=.25f;premul[i+2]*=.25f;premul[i+3]=.25f;}
 ConstImageView pi{premul.data(),w,h,w*4};ImageView po{out.data(),w,h,w*4};assert(processRfInstability(pi,po,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.25f)<1e-6f);
 return 0;}
