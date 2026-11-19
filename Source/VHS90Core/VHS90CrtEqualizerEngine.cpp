#include "VHS90CrtEqualizerEngine.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace vhs90 {
namespace {
constexpr int kPi14=8192,kTwoPi14=16384,kMask14=16383;
constexpr int kSin15[18]={0x0000,0x0c88,0x18f8,0x2528,0x30f8,0x3c50,0x4718,0x5130,0x5a80,0x62f0,0x6a68,0x70e0,0x7640,0x7a78,0x7d88,0x7f60,0x8000,0x7f60};
inline int sinTable(int n) noexcept {const int f=n&255,i=(n>>8)&255,a=kSin15[i],b=kSin15[i+1];return a+(((b-a)*f)>>8);}
inline void sincos14(int n,int&sn,int&cs) noexcept {
 n&=kMask14;const int h=n&((kTwoPi14>>1)-1);
 if(h>((kTwoPi14>>2)-1)){cs=-sinTable(h-(kTwoPi14>>2));sn=sinTable((kTwoPi14>>1)-h);}
 else{cs=sinTable((kTwoPi14>>2)-h);sn=sinTable(h);}
 if(n>((kTwoPi14>>1)-1)){cs=-cs;sn=-sn;}
}
inline int coefficient(int cutoffHz) noexcept {
 constexpr int sampleRate=14318182;
 int sn=0,cs=0;sincos14(int((std::int64_t(kPi14)*cutoffHz)/sampleRate),sn,cs);
 return 4*sn; // Q16 coefficient for 2*sin(pi*fc/fs)
}
struct BandFilter {
 int low[4]{},high[4]{},history[3]{};
 int lowCoeff=0,highCoeff=0;
 int gains[3]{65536,65536,65536};
 void reset() noexcept {for(int&i:low)i=0;for(int&i:high)i=0;for(int&i:history)i=0;}
 int process(int sample) noexcept {
  low[0]+=(int((std::int64_t(lowCoeff)*(sample-low[0])+32768)>>16));
  high[0]+=(int((std::int64_t(highCoeff)*(sample-high[0])+32768)>>16));
  for(int i=1;i<4;++i){low[i]+=int((std::int64_t(lowCoeff)*(low[i-1]-low[i])+32768)>>16);high[i]+=int((std::int64_t(highCoeff)*(high[i-1]-high[i])+32768)>>16);}
  int bands[3]={low[3],high[3]-low[3],history[2]-high[3]};
  for(int i=0;i<3;++i)bands[i]=int((std::int64_t(bands[i])*gains[i])>>16);
  history[2]=history[1];history[1]=history[0];history[0]=sample;
  return bands[0]+bands[1]+bands[2];
 }
};
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline float clamp01(float x) noexcept{return std::clamp(x,0.0f,1.0f);}
}
bool processCrtThreeBandEqualizer(ConstImageView src,ImageView dst,const CrtEqualizerParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount))return false;
 const float amount=clamp01(p.amount);
 if(amount<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 try{
  BandFilter yFilter,iFilter,qFilter;
  yFilter.lowCoeff=coefficient(1500000);yFilter.highCoeff=coefficient(3000000);
  yFilter.gains[0]=65536;yFilter.gains[1]=8192;yFilter.gains[2]=9175;
  iFilter.lowCoeff=coefficient(80000);iFilter.highCoeff=coefficient(1150000);
  iFilter.gains[0]=65536;iFilter.gains[1]=65536;iFilter.gains[2]=1311;
  qFilter.lowCoeff=coefficient(80000);qFilter.highCoeff=coefficient(1000000);
  qFilter.gains[0]=65536;qFilter.gains[1]=65536;qFilter.gains[2]=0;
  for(int y=0;y<src.height;++y){
   yFilter.reset();iFilter.reset();qFilter.reset();
   const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
   for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];if(a<=1e-6f){std::copy_n(px,4,out);continue;}
    RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
    const YIQ original=rgbToYiq(rgb);
    const int yIn=int(std::lround(original.y*255.0f)),iIn=int(std::lround(original.i*255.0f)),qIn=int(std::lround(original.q*255.0f));
    const YIQ filtered{float(yFilter.process(yIn))/255.0f,float(iFilter.process(iIn))/255.0f,float(qFilter.process(qIn))/255.0f};
    YIQ mixed{original.y+(filtered.y-original.y)*amount,original.i+(filtered.i-original.i)*amount,original.q+(filtered.q-original.q)*amount};
    rgb=yiqToRgb(mixed);if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
    out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
   }
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
