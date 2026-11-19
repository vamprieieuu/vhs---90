#include "VHS90TapeDetailEngine.h"
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
inline float sample(const std::vector<float>&v,int n,float x) noexcept {
 x=std::clamp(x,0.0f,float(n-1));const int a=int(x),b=std::min(a+1,n-1);const float t=x-float(a);
 return v[std::size_t(a)]+(v[std::size_t(b)]-v[std::size_t(a)])*t;
}
}
bool processTapeDetail(ConstImageView src,ImageView dst,const TapeDetailParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.detail)||p.minimumWidth<1)return false;
 const float detail=std::clamp(p.detail,0.06f,1.0f);
 const int lowWidth=std::clamp(std::max(p.minimumWidth,int(std::lround(float(src.width)*detail))),1,src.width);
 if(lowWidth>=src.width){
  for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
  return true;
 }
 try {
  std::vector<float> full(std::size_t(src.width)),low(std::size_t(lowWidth));
  for(int y=0;y<src.height;++y){
   const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
   for(int x=0;x<src.width;++x){
    const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];
    if(a<=1.0e-6f){std::copy_n(px,4,out);continue;}
    RGB rgb{px[0],px[1],px[2]};
    if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
    const YIQ yiq=rgbToYiq(rgb);full[std::size_t(x)]=yiq.y;
   }
   // Linear downsample in source-pixel coordinates, then linear reconstruction.
   for(int x=0;x<lowWidth;++x){
    const float sx=(float(x)+0.5f)*float(src.width)/float(lowWidth)-0.5f;
    low[std::size_t(x)]=sample(full,src.width,sx);
   }
   for(int x=0;x<src.width;++x){
    const float lx=(float(x)+0.5f)*float(lowWidth)/float(src.width)-0.5f;
    const float yLow=sample(low,lowWidth,lx);
    const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];
    if(a<=1.0e-6f)continue;
    RGB rgb{px[0],px[1],px[2]};
    if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
    const YIQ original=rgbToYiq(rgb);RGB result=yiqToRgb({yLow,original.i,original.q});
    if(p.premultipliedAlpha){result.r*=a;result.g*=a;result.b*=a;}
    out[0]=result.r;out[1]=result.g;out[2]=result.b;out[3]=a;
   }
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
