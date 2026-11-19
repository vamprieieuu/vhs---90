#include "VHS90SignalGhostEngine.h"
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
inline float shifted(const std::vector<float>&v,std::size_t rowBase,int w,int x,int shift) noexcept {
 const int sx=std::clamp(x-shift,0,w-1);return v[rowBase+std::size_t(sx)];
}
}
bool processSignalGhost(ConstImageView src,ImageView dst,const SignalGhostParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount)||!std::isfinite(p.chromaDelayPixels)||!std::isfinite(p.horizontalScale)||p.horizontalScale<=0)return false;
 const float amount=std::clamp(p.amount,0.0f,1.0f);
 const int delay=std::max(0,int(std::lround(p.chromaDelayPixels)));
 if(amount<=0.0f&&delay<=0){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 try{
  const int w=src.width,h=src.height;const std::size_t n=std::size_t(w)*h;
  std::vector<float> lum(n),ii(n),qq(n);
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){const float*px=src.rgba+y*src.rowStrideFloats+x*4;RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha&&px[3]>1e-6f){rgb.r/=px[3];rgb.g/=px[3];rgb.b/=px[3];}const YIQ v=rgbToYiq(rgb);const size_t k=size_t(y)*w+x;lum[k]=v.y;ii[k]=v.i;qq[k]=v.q;}
  const int echoDelay=std::max(1,int(std::lround(float(std::max(1,delay))*p.horizontalScale+float(w)*amount*.035f)));
  const int secondDelay=std::max(echoDelay+1,int(std::lround(float(echoDelay)*2.35f)));
  const int iDelay=std::max(1,int(std::lround(float(delay)*p.horizontalScale)));
  const int qDelay=std::max(1,iDelay/2);
  std::vector<float> outY(n),outI(n),outQ(n);
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){const size_t k=size_t(y)*w+x;
   float yy=lum[k];
   if(amount>0){const float first=shifted(lum,size_t(y)*w,w,x,echoDelay);yy=yy*(1.0f-amount*.18f)+first*amount*.15f;const float second=shifted(lum,size_t(y)*w,w,x,secondDelay);yy+=second*amount*.06f;}
   const float delayedI=delay>0?shifted(ii,size_t(y)*w,w,x,iDelay):ii[k];
   const float delayedQ=delay>0?shifted(qq,size_t(y)*w,w,x,qDelay):qq[k];
   outY[k]=yy;outI[k]=delay>0?ii[k]*.35f+delayedI*.65f:ii[k];outQ[k]=delay>0?qq[k]*.45f+delayedQ*.55f:qq[k];
  }
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){const size_t k=size_t(y)*w+x;const float*px=src.rgba+y*src.rowStrideFloats+x*4;float*out=dst.rgba+y*dst.rowStrideFloats+x*4;const float a=px[3];if(a<=1e-6f){std::copy_n(px,4,out);continue;}RGB rgb=yiqToRgb({outY[k],outI[k],outQ[k]});if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;}
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
