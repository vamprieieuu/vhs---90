#include "VHS90ImpulseNoiseEngine.h"
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
}
bool processImpulseNoise(ConstImageView src,ImageView dst,const ImpulseNoiseParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount))return false;
 const float amount=std::clamp(p.amount,0.0f,1.0f);
 if(amount<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 try {
  const int w=src.width,h=src.height;
  std::vector<float> impulses(std::size_t(w)*h,0.0f);
  const float eventDrive=1.0f+58.0f*amount;
  const int events=std::max(1,int(std::lround(eventDrive*(0.4f+0.6f*hash01(0u,0u,p.seed,p.frame)))));
  for(int e=0;e<events;++e){
   const auto id=std::uint32_t(e+1);
   const int y=std::min(h-1,int(std::lround(hash01(id,1u,p.seed,p.frame)*float(std::max(1,h-1)))));
   const int start=std::min(w-1,int(std::lround(hash01(id,2u,p.seed,p.frame)*float(std::max(1,w-1)))));
   const int len=1+int(std::lround((4.0f+38.0f*amount)*hash01(id,3u,p.seed,p.frame)));
   const float sign=hash01(id,4u,p.seed,p.frame)<0.58f?1.0f:-1.0f;
   float amp=(0.10f+0.86f*amount)*sign,walk=0.0f;
   for(int x=start;x<std::min(w,start+len);++x){
    const float noise=hash01(std::uint32_t(x),id^0xA53u,p.seed,p.frame)*2.0f-1.0f;
    walk=0.76f*walk+noise*(0.005f+0.038f*amount);
    impulses[std::size_t(y)*w+x]+=amp+walk;
    amp*=0.70f;
   }
  }
  for(int y=0;y<h;++y){const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
   for(int x=0;x<w;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];
    if(a<=1.0e-6f){std::copy_n(px,4,out);continue;}
    RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
    YIQ yiq=rgbToYiq(rgb);yiq.y+=impulses[std::size_t(y)*w+x];rgb=yiqToRgb(yiq);
    if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
    out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
   }
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
