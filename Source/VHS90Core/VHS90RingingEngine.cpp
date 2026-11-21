#include "VHS90RingingEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
constexpr float kTaps[7]={.16f,-.30f,.42f,1.0f,-.42f,.30f,-.16f};
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline int reflect101(int x,int n) noexcept { if(n<=1)return 0;while(x<0||x>=n){if(x<0)x=-x;if(x>=n)x=2*n-x-2;}return x; }
inline float clamp01(float v) noexcept{return std::clamp(v,0.0f,1.0f);}
inline float sample(const std::vector<float>&v,int w,int h,int x,int y) noexcept {
 return v[std::size_t(reflect101(y,h))*w+reflect101(x,w)];
}
}
bool processRinging(ConstImageView src,ImageView dst,const RingingParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.strength)||!std::isfinite(p.power))return false;
 const float amount=clamp01(p.strength),power=std::clamp(p.power,0.0f,8.0f);
 if(amount<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 try{
  const int w=src.width,h=src.height;const std::size_t n=std::size_t(w)*h;
  std::vector<float> luma(n),blurX(n),blur(n),edge(n),ring(n);
  const float sigma=(1.25f+power*.15f)*std::max(.1f,float(w)/720.0f);
  const int radius=std::clamp(int(std::ceil(3.0f*sigma)),1,32);
  std::vector<float> kernel(std::size_t(radius*2+1));float sum=0;
  for(int k=-radius;k<=radius;++k){float v=std::exp(-.5f*float(k*k)/(sigma*sigma));kernel[std::size_t(k+radius)]=v;sum+=v;}
  for(float&v:kernel)v/=sum;
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){
   const float*px=src.rgba+y*src.rowStrideFloats+x*4;const float a=px[3];
   float r=px[0],g=px[1],b=px[2];if(p.premultipliedAlpha&&a>1e-6f){r/=a;g/=a;b/=a;}
   luma[std::size_t(y)*w+x]=rgbToYiq({r,g,b}).y;
  }
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){float v=0;for(int k=-radius;k<=radius;++k)v+=sample(luma,w,h,x+k,y)*kernel[std::size_t(k+radius)];blurX[std::size_t(y)*w+x]=v;}
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){float v=0;for(int k=-radius;k<=radius;++k)v+=blurX[std::size_t(reflect101(y+k,h))*w+x]*kernel[std::size_t(k+radius)];blur[std::size_t(y)*w+x]=v;}
  for(std::size_t i=0;i<n;++i)edge[i]=luma[i]-blur[i];
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){float v=0;for(int k=-3;k<=3;++k)v+=sample(edge,w,h,x+k,y)*kTaps[k+3];ring[std::size_t(y)*w+x]=v;}
  const int forward=std::max(1,int(std::lround(std::max(1.0f,power)*std::max(.1f,float(w)/720.0f))));
  const int backward=std::max(1,int(std::lround(std::max(1.0f,power*.5f)*std::max(.1f,float(w)/720.0f))));
  for(int y=0;y<h;++y){const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
   for(int x=0;x<w;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];if(a<=1e-6f){std::copy_n(px,4,out);continue;}
    RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
    YIQ yiq=rgbToYiq(rgb);const std::size_t idx=std::size_t(y)*w+x;
    const float echo=sample(edge,w,h,x+forward,y)-sample(edge,w,h,x-backward,y);
    yiq.y+=ring[idx]*amount*(1.05f+power*.22f)+echo*amount*.22f;
    rgb=yiqToRgb(yiq);if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
    out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
   }
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
