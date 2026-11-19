#include "VHS90ChromaLossEngine.h"
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
inline float clamp01(float v) noexcept{return std::clamp(v,0.0f,1.0f);}
}
bool processChromaLoss(ConstImageView src,ImageView dst,const ChromaLossParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount))return false;
 const float amount=clamp01(p.amount);
 if(amount<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 try{
  std::vector<float> rowScale(std::size_t(src.height),1.0f);
  const std::uint32_t frame=std::uint32_t(p.frame)^std::uint32_t(std::uint64_t(p.frame)>>32);
  const float chance=amount*0.077f;
  for(int y=0;y<src.height;++y)if(hash01(std::uint32_t(y),0xC4A0u,p.seed,frame)<chance)rowScale[std::size_t(y)]=.08f;
  for(int y=0;y<src.height;++y){const float scale=rowScale[std::size_t(y)];const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
   for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];if(a<=1e-6f){std::copy_n(px,4,out);continue;}
    RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
    YIQ yiq=rgbToYiq(rgb);yiq.i*=scale;yiq.q*=scale;rgb=yiqToRgb(yiq);
    if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
    out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
   }
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
