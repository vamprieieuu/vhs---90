#include "VHS90ThreeBandEQEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
struct BandState {float low[4]{},high[4]{},history[3]{};};
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline float coefficient(float cutoffCyclesPerPixel) noexcept {
 return 2.0f*std::sin(3.14159265358979323846f*cutoffCyclesPerPixel);
}
inline float filterBand(BandState&st,float sample,float lo,float hi,float gl,float gm,float gh) noexcept {
 st.low[0]+=lo*(sample-st.low[0]);st.high[0]+=hi*(sample-st.high[0]);
 for(int i=1;i<4;++i){st.low[i]+=lo*(st.low[i-1]-st.low[i]);st.high[i]+=hi*(st.high[i-1]-st.high[i]);}
 const float low=st.low[3],mid=st.high[3]-st.low[3],high=st.history[2]-st.high[3];
 st.history[2]=st.history[1];st.history[1]=st.history[0];st.history[0]=sample;
 return low*gl+mid*gm+high*gh;
}
}
bool processThreeBandEq(ConstImageView src,ImageView dst,const ThreeBandEqParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount))return false;
 const float amount=std::clamp(p.amount,0.0f,1.0f);
 if(amount<=0){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 // The original NTSC-CRT coefficients are based on a 14.31818 MHz sample clock.
 const float yLo=coefficient(1.5e6f/14.31818e6f),yHi=coefficient(3.0e6f/14.31818e6f);
 const float iLo=coefficient(80.0e3f/14.31818e6f),iHi=coefficient(1.15e6f/14.31818e6f);
 const float qLo=coefficient(80.0e3f/14.31818e6f),qHi=coefficient(1.0e6f/14.31818e6f);
 for(int y=0;y<src.height;++y){BandState ys,is,qs;const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];if(a<=1.0e-6f){std::copy_n(px,4,out);continue;}
   RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
   YIQ yiq=rgbToYiq(rgb);
   const float fy=filterBand(ys,yiq.y,yLo,yHi,1.0f,.125f,.14f);
   const float fi=filterBand(is,yiq.i,iLo,iHi,1.0f,1.0f,.02f);
   const float fq=filterBand(qs,yiq.q,qLo,qHi,1.0f,1.0f,0.0f);
   yiq.y+=(fy-yiq.y)*amount;yiq.i+=(fi-yiq.i)*amount;yiq.q+=(fq-yiq.q)*amount;
   rgb=yiqToRgb(yiq);if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
   out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
  }
 }
 return true;
}
} // namespace vhs90
