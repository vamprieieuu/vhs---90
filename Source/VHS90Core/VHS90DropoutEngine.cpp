#include "VHS90DropoutEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace vhs90 {
namespace {
struct Rng {
 std::uint32_t s;
 explicit Rng(std::uint32_t seed):s(seed?seed:0xA341316Cu){}
 std::uint32_t next() noexcept {s^=s<<13;s^=s>>17;s^=s<<5;return s;}
 float uniform() noexcept{return float(next()>>8)*(1.0f/16777216.0f);}
 float normal() noexcept {const float u1=std::max(1.0e-7f,uniform()),u2=uniform();return std::sqrt(-2.0f*std::log(u1))*std::cos(6.28318530718f*u2);}
 int integer(int low,int high) noexcept {if(high<=low)return low;return low+int(next()%std::uint32_t(high-low+1));}
 int poisson(float lambda) noexcept {
  if(lambda<=0)return 0;const float limit=std::exp(-lambda);float p=1.0f;int k=0;
  do{++k;p*=std::max(1.0e-7f,uniform());}while(p>limit&&k<512);
  return k-1;
 }
};
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline float clamp01(float x) noexcept{return std::clamp(x,0.0f,1.0f);}
}
bool processDropouts(ConstImageView src,ImageView dst,const DropoutParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount)||!std::isfinite(p.horizontalScale)||!std::isfinite(p.verticalScale)||p.horizontalScale<=0||p.verticalScale<=0)return false;
 const float amount=clamp01(p.amount);
 if(amount<=0){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 try{
  const int w=src.width,h=src.height;const std::size_t count=std::size_t(w)*h;
  std::vector<float> y(count),i(count),q(count),alpha(count);\n  std::vector<std::uint8_t> affected(count,0u);
  for(int row=0;row<h;++row)for(int x=0;x<w;++x){
   const float*px=src.rgba+row*src.rowStrideFloats+x*4;const std::size_t n=std::size_t(row)*w+x;
   alpha[n]=px[3];RGB rgb{px[0],px[1],px[2]};
   if(p.premultipliedAlpha&&px[3]>1.0e-6f){rgb.r/=px[3];rgb.g/=px[3];rgb.b/=px[3];}
   const YIQ v=rgbToYiq(rgb);y[n]=v.y;i[n]=v.i;q[n]=v.q;
  }
  const std::uint32_t frame=std::uint32_t(p.frame)^std::uint32_t(std::uint64_t(p.frame)>>32);
  Rng rng(p.seed^(frame*0x9E3779B9u)^0xD1B54A35u);
  const float expected=amount*std::max(1.0f,float(h)/(22.0f*p.verticalScale));
  const int bands=std::min(512,rng.poisson(expected));
  for(int n=0;n<bands;++n){
   const int row=rng.integer(0,h-1);
   const int maxBand=std::max(2,int(std::lround((2.0f+amount*7.0f)*p.verticalScale)));
   const int band=rng.integer(1,maxBand),y1=std::min(h,row+band);
   const int x0=rng.integer(0,w-1),minLength=std::max(2,w/10),length=rng.integer(minLength,w);
   const int x1=std::min(w,x0+length);if(x1<=x0)continue;
   const float mix=std::clamp(.24f+amount*(.28f+.44f*rng.uniform()),0.0f,.96f);
   const float target=(rng.uniform()>.35f)?235.0f/255.0f:18.0f/255.0f;
   const float sigma=(18.0f+amount*22.0f)/255.0f;
   for(int yy=row;yy<y1;++yy)for(int xx=x0;xx<x1;++xx){
    const std::size_t at=std::size_t(yy)*w+xx;
    y[at]=y[at]*(1.0f-mix)+(target+rng.normal()*sigma)*mix;
    const float chromaKeep=1.0f-mix*.75f;i[at]*=chromaKeep;q[at]*=chromaKeep;
   }
  }
  const float chance=std::min(1.0f,amount*.0018f/(p.horizontalScale*p.verticalScale));
  const int speckles=std::min<int>(int(std::min<std::size_t>(count,200000)),rng.poisson(float(count)*chance));
  for(int n=0;n<speckles;++n){const std::size_t at=std::size_t(rng.next())%count;affected[at]=1u;y[at]=rng.uniform();}
  for(int row=0;row<h;++row)for(int x=0;x<w;++x){
   const std::size_t n=std::size_t(row)*w+x;const float a=alpha[n];float*out=dst.rgba+row*dst.rowStrideFloats+x*4;
   if(a<=1.0e-6f){std::copy_n(src.rgba+row*src.rowStrideFloats+x*4,4,out);continue;}
   RGB rgb=yiqToRgb({y[n],i[n],q[n]});if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
   out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
