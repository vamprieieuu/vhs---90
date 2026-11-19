#include "VHS90BloomEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
struct Pixel{float r,g,b;};
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline Pixel sample(const std::vector<Pixel>&a,int w,int h,float x,float y) noexcept {
 x=std::clamp(x,0.0f,float(w-1));y=std::clamp(y,0.0f,float(h-1));
 const int x0=int(x),y0=int(y),x1=std::min(x0+1,w-1),y1=std::min(y0+1,h-1);
 const float fx=x-x0,fy=y-y0;const Pixel&A=a[size_t(y0)*w+x0],&B=a[size_t(y0)*w+x1],&C=a[size_t(y1)*w+x0],&D=a[size_t(y1)*w+x1];
 auto m=[&](float av,float bv,float cv,float dv){const float ab=av+(bv-av)*fx,cd=cv+(dv-cv)*fx;return ab+(cd-ab)*fy;};
 return {m(A.r,B.r,C.r,D.r),m(A.g,B.g,C.g,D.g),m(A.b,B.b,C.b,D.b)};
}
void blur(std::vector<Pixel>&a,int w,int h,int radius) {
 if(radius<=0)return;const float sigma=std::max(0.8f,float(radius)*0.65f);
 std::vector<float>weights(size_t(radius*2+1));float sum=0;
 for(int i=-radius;i<=radius;++i){const float q=std::exp(-float(i*i)/(2*sigma*sigma));weights[size_t(i+radius)]=q;sum+=q;}
 for(float&v:weights)v/=sum;std::vector<Pixel>tmp(a.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){Pixel p{0,0,0};for(int d=-radius;d<=radius;++d){const Pixel&s=a[size_t(y)*w+std::clamp(x+d,0,w-1)];const float q=weights[size_t(d+radius)];p.r+=s.r*q;p.g+=s.g*q;p.b+=s.b*q;}tmp[size_t(y)*w+x]=p;}
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){Pixel p{0,0,0};for(int d=-radius;d<=radius;++d){const Pixel&s=tmp[size_t(std::clamp(y+d,0,h-1))*w+x];const float q=weights[size_t(d+radius)];p.r+=s.r*q;p.g+=s.g*q;p.b+=s.b*q;}a[size_t(y)*w+x]=p;}
}
}
bool processBloom(ConstImageView src,ImageView dst,const BloomParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.threshold)||!std::isfinite(p.intensity)||
    !std::isfinite(p.spread)||!std::isfinite(p.veil)||p.spread<0.0f||p.intensity<0.0f)
  return false;
 if(p.intensity<=0.0f) {
  for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
  return true;
 }
 try {
  const int w=src.width,h=src.height,levels=p.fast?1:std::clamp(p.levels,1,5);
  std::vector<Pixel>accum(size_t(w)*h,{0,0,0});
  for(int level=0;level<levels;++level) {
   const int scale=p.fast?2:(1<<(level+1));
   const int lw=std::max(1,(w+scale-1)/scale),lh=std::max(1,(h+scale-1)/scale);
   std::vector<Pixel>small(size_t(lw)*lh);
   for(int y=0;y<lh;++y)for(int x=0;x<lw;++x) {
    float r=0,g=0,b=0,n=0;
    for(int oy=0;oy<scale&&y*scale+oy<h;++oy)for(int ox=0;ox<scale&&x*scale+ox<w;++ox) {
     const float*px=src.rgba+size_t(y*scale+oy)*src.rowStrideFloats+(x*scale+ox)*4;
     const float a=std::clamp(px[3],0.0f,1.0f);if(a<=1.0e-6f)continue;
     float rr=px[0],gg=px[1],bb=px[2];if(p.premultipliedAlpha){rr/=a;gg/=a;bb/=a;}
     r+=rr*a;g+=gg*a;b+=bb*a;n+=a;
    }
    if(n>1.0e-6f){r/=n;g/=n;b/=n;}
    const float lum=.2126f*r+.7152f*g+.0722f*b;
    const float bright=std::max(lum-p.threshold,0.0f)/std::max(lum,1.0e-5f);
    small[size_t(y)*lw+x]={r*bright,g*bright,b*bright};
   }
   blur(small,lw,lh,std::clamp(int(std::round(p.spread*.75f)),1,6));
   const float weight=p.fast?1.0f:1.0f/float(level+1);
   for(int y=0;y<h;++y)for(int x=0;x<w;++x){const Pixel q=sample(small,lw,lh,(float(x)+.5f)/scale-.5f,(float(y)+.5f)/scale-.5f);Pixel&d=accum[size_t(y)*w+x];d.r+=q.r*weight;d.g+=q.g*weight;d.b+=q.b*weight;}
  }
  const float amount=std::clamp(p.intensity,0.0f,4.0f)/float(levels),veil=std::max(0.0f,p.veil);
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){
   const float*px=src.rgba+y*src.rowStrideFloats+x*4;float*out=dst.rgba+y*dst.rowStrideFloats+x*4;const float a=px[3];
   if(a<=1.0e-6f){std::copy_n(px,4,out);continue;}
   const Pixel&b=accum[size_t(y)*w+x];float r=px[0],g=px[1],bl=px[2];
   if(p.premultipliedAlpha){r/=a;g/=a;bl/=a;}
   r+=b.r*amount+veil;g+=b.g*amount+veil;bl+=b.b*amount+veil;
   if(p.premultipliedAlpha){r*=a;g*=a;bl*=a;}
   out[0]=r;out[1]=g;out[2]=bl;out[3]=a;
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
