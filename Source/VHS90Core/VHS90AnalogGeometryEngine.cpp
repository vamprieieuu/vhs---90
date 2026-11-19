#include "VHS90AnalogGeometryEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline float sampleChannel(ConstImageView s,float x,float y,int ch) noexcept {
 x=std::clamp(x,0.0f,float(s.width-1));y=std::clamp(y,0.0f,float(s.height-1));
 const int x0=int(x),y0=int(y),x1=std::min(x0+1,s.width-1),y1=std::min(y0+1,s.height-1);
 const float fx=x-x0,fy=y-y0;
 const float a=s.rgba[y0*s.rowStrideFloats+x0*4+ch],b=s.rgba[y0*s.rowStrideFloats+x1*4+ch];
 const float c=s.rgba[y1*s.rowStrideFloats+x0*4+ch],d=s.rgba[y1*s.rowStrideFloats+x1*4+ch];
 const float ab=a+(b-a)*fx,cd=c+(d-c)*fx;return ab+(cd-ab)*fy;
}
inline float clamp01(float x) noexcept {return std::clamp(x,0.0f,1.0f);}
}
bool processAnalogGeometry(ConstImageView src,ImageView dst,const AnalogGeometryParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.lineJitterPixels)||!std::isfinite(p.lineJitterCorrelationRows)||
    !std::isfinite(p.topSkewPixels)||!std::isfinite(p.topSkewDecayRows)||
    !std::isfinite(p.edgePincushionPixels)||!std::isfinite(p.edgePincushionPower)||!std::isfinite(p.interlaceAmount))return false;
 if(p.lineJitterPixels==0.0f&&p.topSkewPixels==0.0f&&p.edgePincushionPixels==0.0f&&p.interlaceAmount==0.0f) {
  for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;
 }
 for(int y=0;y<src.height;++y) {
  const float yn=src.height>1?(2.0f*float(y)/float(src.height-1)-1.0f):0.0f;
  const float decay=std::exp(-float(y)/std::max(1.0f,p.topSkewDecayRows));
  const float topNoise=hash01(7u,0u,p.seed,p.frame)*2.0f-1.0f;
  const float topShift=topNoise*p.topSkewPixels*decay;
  const float jitter=valueNoise1D(float(y)/std::max(1.0f,p.lineJitterCorrelationRows),p.seed+0x9e3779b9u,p.frame)*2.0f-1.0f;
  const float rowShift=topShift+jitter*p.lineJitterPixels;
  const float interlace=clamp01(p.interlaceAmount);
  const float fieldDir=(y&1)?-1.0f:1.0f;
  const float fieldPhase=std::sin(float(p.frame)*0.73f)*interlace*std::max(1.0f,float(src.width)*0.004f);
  const float fieldShift=fieldPhase*fieldDir;
  const float fieldYShift=0.15f*interlace*fieldDir;
  float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x) {
   const float xn=src.width>1?(2.0f*float(x)/float(src.width-1)-1.0f):0.0f;
   const float bend=p.edgePincushionPixels*xn*std::pow(std::abs(yn),std::max(0.1f,p.edgePincushionPower));
   const float sx=float(x)-rowShift-bend-fieldShift;
   const float sy=std::clamp(float(y)+fieldYShift,0.0f,float(src.height-1));
   for(int c=0;c<4;++c)dr[x*4+c]=sampleChannel(src,sx,sy,c);
  }
 }
 return true;
}
bool processScanlines(ConstImageView src,ImageView dst,const ScanlineParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount)||!std::isfinite(p.frequency)||
    !std::isfinite(p.phase)||!std::isfinite(p.speed)||p.frequency<0.0f)return false;
 const float amount=clamp01(p.amount);
 if(amount<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 const float framePhase=float(p.frame)*p.speed;
 for(int y=0;y<src.height;++y) {
  const float phase=(float(y)+p.phase)*p.frequency+framePhase;
  const float gain=1.0f-amount*(0.5f+0.5f*std::sin(6.28318530718f*phase));
  const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x) {
   const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];
   if(a<=1.0e-6f){std::copy_n(px,4,out);continue;}
   RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
   YIQ yiq=rgbToYiq(rgb);yiq.y*=gain;rgb=yiqToRgb(yiq);
   if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
   out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
  }
 }
 return true;
}
} // namespace vhs90
