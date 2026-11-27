#include "VHS90ChromaShiftEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace vhs90 {
namespace {
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline RGB straight(const float*row,int x,bool premul) noexcept {
 const float a=row[x*4+3];RGB c{row[x*4],row[x*4+1],row[x*4+2]};
 if(premul&&a>1.0e-6f){c.r/=a;c.g/=a;c.b/=a;}return c;
}
inline float yOf(RGB c) noexcept{return .299f*c.r+.587f*c.g+.114f*c.b;}
inline float crOf(RGB c) noexcept{return .713f*(c.r-yOf(c));}
inline float cbOf(RGB c) noexcept{return .564f*(c.b-yOf(c));}
inline float sampleChannel(ConstImageView src,int y,float x,int channel,bool premul,float centerAlpha) noexcept {
 x=std::clamp(x,0.0f,float(src.width-1));const int x0=int(std::floor(x)),x1=std::min(x0+1,src.width-1);const float t=x-float(x0);
 const float* row=src.rgba+y*src.rowStrideFloats;
 auto get=[&](int px){const float* p=row+px*4;const float a=p[3];if(premul){return a>1.0e-6f?p[channel]/a:0.0f;}return p[channel];};
 const float value=get(x0)*(1.0f-t)+get(x1)*t;
 return premul?value*centerAlpha:value;
}
}
bool processChromaChannelShift(ConstImageView src,ImageView dst,const ChromaChannelShiftParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.shiftPixels))return false;
 const float shift=std::clamp(p.shiftPixels,-64.0f,64.0f);
 if(std::abs(shift)<1.0e-6f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 for(int y=0;y<src.height;++y){const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float alpha=px[3];
   if(alpha<=1.0e-6f){std::copy_n(px,4,out);continue;}
   const RGB center=straight(sr,x,p.premultipliedAlpha);const float y0=yOf(center);
   const int xCr0=int(std::floor(std::clamp(float(x)+shift,0.0f,float(src.width-1))));
   const int xCb0=int(std::floor(std::clamp(float(x)-shift,0.0f,float(src.width-1))));
   const RGB crA=straight(sr,xCr0,p.premultipliedAlpha),cbA=straight(sr,xCb0,p.premultipliedAlpha);
   const float cr=crOf(crA),cb=cbOf(cbA);
   RGB result{y0+cr/0.713f,y0-(0.714f*cr+0.344f*cb)/0.587f,y0+cb/0.564f};
   if(p.premultipliedAlpha){result.r*=alpha;result.g*=alpha;result.b*=alpha;}
   out[0]=result.r;out[1]=result.g;out[2]=result.b;out[3]=alpha;
  }
 }
 return true;
}
bool processRgbChannelRegistration(ConstImageView src,ImageView dst,const RgbChannelRegistrationParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.redOffsetPixels)||!std::isfinite(p.greenOffsetPixels)||!std::isfinite(p.blueOffsetPixels))return false;
 float ro=std::clamp(p.redOffsetPixels,-64.0f,64.0f),go=std::clamp(p.greenOffsetPixels,-64.0f,64.0f),bo=std::clamp(p.blueOffsetPixels,-64.0f,64.0f);
 switch(p.mode){
  case RgbRegistrationMode::RedCyan: go=0.0f;bo=0.0f;break;
  case RgbRegistrationMode::GreenMagenta: ro=0.0f;bo=0.0f;break;
  case RgbRegistrationMode::BlueYellow: ro=0.0f;go=0.0f;break;
  default: break;
 }
 if(std::abs(ro)<1.0e-6f&&std::abs(go)<1.0e-6f&&std::abs(bo)<1.0e-6f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 for(int y=0;y<src.height;++y){const float* sr=src.rgba+y*src.rowStrideFloats;float* dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x){const float* px=sr+x*4;float* out=dr+x*4;const float alpha=px[3];
   if(alpha<=1.0e-6f){std::copy_n(px,4,out);continue;}
   out[0]=sampleChannel(src,y,float(x)+ro,0,p.premultipliedAlpha,alpha);
   out[1]=sampleChannel(src,y,float(x)+go,1,p.premultipliedAlpha,alpha);
   out[2]=sampleChannel(src,y,float(x)+bo,2,p.premultipliedAlpha,alpha);
   out[3]=alpha;
  }
 }
 return true;
}
} // namespace vhs90
