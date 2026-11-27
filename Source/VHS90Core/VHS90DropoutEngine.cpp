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
  std::vector<float> luma(count),inPhase(count),quadrature(count),alpha(count);
  std::vector<std::uint8_t> affected(count,0u);
  for(int row=0;row<h;++row)for(int x=0;x<w;++x){
   const float*px=src.rgba+row*src.rowStrideFloats+x*4;const std::size_t n=std::size_t(row)*w+x;
   alpha[n]=px[3];RGB rgb{px[0],px[1],px[2]};
   if(p.premultipliedAlpha&&px[3]>1.0e-6f){rgb.r/=px[3];rgb.g/=px[3];rgb.b/=px[3];}
   const YIQ v=rgbToYiq(rgb);luma[n]=v.y;inPhase[n]=v.i;quadrature[n]=v.q;
  }
  const std::uint32_t frame=std::uint32_t(p.frame)^std::uint32_t(std::uint64_t(p.frame)>>32);
  Rng rng(p.seed^(frame*0x9E3779B9u)^0xD1B54A35u);
  // Tape dropouts are sparse, elongated RF-loss marks, not broad opaque bars.
  const float eventRate=amount*(3.0f+0.22f*float(h))/std::max(0.65f,p.verticalScale);
  const int events=std::min(768,rng.poisson(eventRate));
  for(int e=0;e<events;++e){
   const int row=rng.integer(0,h-1);
   const float sizeRoll=rng.uniform();
   float nominal=0.0f;
   if(sizeRoll<0.56f) nominal=2.0f+rng.uniform()*std::max(5.0f,float(w)*0.055f);
   else if(sizeRoll<0.91f) nominal=7.0f+rng.uniform()*std::max(12.0f,float(w)*0.16f);
   else nominal=18.0f+rng.uniform()*std::max(24.0f,float(w)*0.38f);
   const int length=std::clamp(int(std::lround(nominal*p.horizontalScale)),1,w);
   const int x0=rng.integer(0,std::max(0,w-length));
   const int bandHeight=std::clamp(1+(rng.uniform()<0.16f?1:0)+(rng.uniform()<0.035f?1:0),1,
                                   std::max(1,int(std::lround(2.0f*p.verticalScale))));
   const int y0=std::clamp(row-bandHeight/2,0,std::max(0,h-bandHeight));
   const float depth=0.42f+0.53f*amount*rng.uniform();
   const bool whiteDrop=rng.uniform()>0.78f;
   const float target=whiteDrop?(0.72f+0.25f*rng.uniform()):(0.005f+0.12f*rng.uniform());
   const float grain=(0.008f+0.035f*amount);
   const float taper=std::max(1.0f,float(length)*0.10f);
   for(int yy=y0;yy<std::min(h,y0+bandHeight);++yy){
    const float rowWeight=bandHeight==1?1.0f:std::sin((float(yy-y0)+0.5f)/float(bandHeight)*3.14159265f);
    for(int xx=x0;xx<x0+length;++xx){
     const float along=float(xx-x0)+0.5f;
     const float edge=std::clamp(std::min(along/taper,(float(length)-along)/taper),0.0f,1.0f);
     const float ragged=0.76f+0.24f*rng.uniform();
     const float mix=std::clamp(depth*rowWeight*edge*ragged,0.0f,0.96f);
     const std::size_t at=std::size_t(yy)*w+xx;affected[at]=1u;
     const float localNoise=rng.normal()*grain;
     luma[at]=luma[at]*(1.0f-mix)+(target+localNoise)*mix;
     const float chromaKeep=1.0f-mix*(0.78f+0.18f*amount);
     inPhase[at]*=chromaKeep;quadrature[at]*=chromaKeep;
    }
   }
  }
  // A small population of isolated oxide pinholes complements the longer streaks.
  const int speckles=std::min<int>(int(std::min<std::size_t>(count,200000)),
                                   rng.poisson(float(count)*amount*0.00032f));
  for(int n=0;n<speckles;++n){
   const std::size_t at=std::size_t(rng.next())%count;affected[at]=1u;
   const float target=rng.uniform()>0.72f?0.92f:0.015f;
   luma[at]=luma[at]*(0.12f+0.20f*rng.uniform())+target*(0.80f-0.20f*rng.uniform());
   inPhase[at]*=0.12f;quadrature[at]*=0.12f;
  }
  for(int row=0;row<h;++row)for(int x=0;x<w;++x){
   const std::size_t n=std::size_t(row)*w+x;const float a=alpha[n];float*out=dst.rgba+row*dst.rowStrideFloats+x*4;
   const float*original=src.rgba+row*src.rowStrideFloats+x*4;
   if(a<=1.0e-6f||affected[n]==0u){std::copy_n(original,4,out);continue;}
   RGB rgb=yiqToRgb({luma[n],inPhase[n],quadrature[n]});
   if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
   out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
