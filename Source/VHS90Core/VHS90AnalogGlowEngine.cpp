#include "VHS90AnalogGlowEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
struct RGBf{float r,g,b;};
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline int reflect101(int x,int n) noexcept {if(n<=1)return 0;while(x<0||x>=n){if(x<0)x=-x;if(x>=n)x=2*n-x-2;}return x;}
}
bool processAnalogGlow(ConstImageView src,ImageView dst,const AnalogGlowParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount)||!std::isfinite(p.horizontalScale)||p.horizontalScale<=0)return false;
 const float amount=std::clamp(p.amount,0.0f,1.0f);
 if(amount<=0){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 try{
  const int w=src.width,h=src.height;const std::size_t n=std::size_t(w)*h;
  const float sigma=(1.8f+amount*5.0f)*p.horizontalScale;
  const int radius=std::clamp(int(std::ceil(3.0f*sigma)),1,64);
  std::vector<float>weights(std::size_t(radius*2+1));float sum=0;
  for(int k=-radius;k<=radius;++k){const float v=std::exp(-.5f*float(k*k)/(sigma*sigma));weights[std::size_t(k+radius)]=v;sum+=v;}
  for(float&v:weights)v/=sum;
  std::vector<RGBf> masked(n),blurred(n),rowBuffer;rowBuffer.resize(static_cast<std::size_t>(w));
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){const float*px=src.rgba+y*src.rowStrideFloats+x*4;const float a=px[3];float r=px[0],g=px[1],b=px[2];if(p.premultipliedAlpha&&a>1e-6f){r/=a;g/=a;b/=a;}
   const float lum=.299f*r+.587f*g+.114f*b;const float mask=std::clamp((lum-.38f)/.62f,0.0f,1.0f);
   masked[std::size_t(y)*w+x]={r*mask,g*mask,b*mask};
  }
  for(int y=0;y<h;++y){
   for(int x=0;x<w;++x){RGBf v{0,0,0};for(int k=-radius;k<=radius;++k){const RGBf&q=masked[std::size_t(y)*w+reflect101(x+k,w)];const float a=weights[std::size_t(k+radius)];v.r+=q.r*a;v.g+=q.g*a;v.b+=q.b*a;}rowBuffer[std::size_t(x)]=v;}
   std::copy(rowBuffer.begin(),rowBuffer.end(),masked.begin()+std::size_t(y)*w);
  }
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){RGBf v{0,0,0};for(int k=-radius;k<=radius;++k){const RGBf&q=masked[std::size_t(reflect101(y+k,h))*w+x];const float a=weights[std::size_t(k+radius)];v.r+=q.r*a;v.g+=q.g*a;v.b+=q.b*a;}blurred[std::size_t(y)*w+x]=v;}
  constexpr float gainR=.95f,gainG=.78f,gainB=.55f;
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){const float*px=src.rgba+y*src.rowStrideFloats+x*4;float*out=dst.rgba+y*dst.rowStrideFloats+x*4;const float a=px[3];if(a<=1e-6f){std::copy_n(px,4,out);continue;}
   float r=px[0],g=px[1],b=px[2];if(p.premultipliedAlpha){r/=a;g/=a;b/=a;}
   const RGBf&q=blurred[std::size_t(y)*w+x];r=std::clamp(r+q.r*gainR*amount,0.0f,1.0f);g=std::clamp(g+q.g*gainG*amount,0.0f,1.0f);b=std::clamp(b+q.b*gainB*amount,0.0f,1.0f);
   if(p.premultipliedAlpha){r*=a;g*=a;b*=a;}out[0]=r;out[1]=g;out[2]=b;out[3]=a;
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
