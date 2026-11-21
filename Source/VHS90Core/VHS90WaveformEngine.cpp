#include "VHS90WaveformEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace vhs90 {
namespace {
constexpr int kWidth=720,kHeight=480;
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline float clamp01(float v) noexcept{return std::clamp(v,0.0f,1.0f);}
inline void sampleInput(ConstImageView src,float x,float y,bool premul,float out[4]) noexcept {
 x=std::clamp(x,0.0f,float(src.width-1));y=std::clamp(y,0.0f,float(src.height-1));
 const int x0=int(x),y0=int(y),x1=std::min(x0+1,src.width-1),y1=std::min(y0+1,src.height-1);
 const float fx=x-x0,fy=y-y0;
 const float* a=src.rgba+y0*src.rowStrideFloats+x0*4;
 const float* b=src.rgba+y0*src.rowStrideFloats+x1*4;
 const float* c=src.rgba+y1*src.rowStrideFloats+x0*4;
 const float* d=src.rgba+y1*src.rowStrideFloats+x1*4;
 for(int k=0;k<4;++k){const float ab=a[k]+(b[k]-a[k])*fx,cd=c[k]+(d[k]-c[k])*fx;out[k]=ab+(cd-ab)*fy;}
 if(premul&&out[3]>1.0e-6f){out[0]/=out[3];out[1]/=out[3];out[2]/=out[3];}
}
inline float sampleByte(const std::vector<std::uint8_t>& rgb,float x,float y,int channel) noexcept {
 x=std::clamp(x,0.0f,float(kWidth-1));y=std::clamp(y,0.0f,float(kHeight-1));
 const int x0=int(x),y0=int(y),x1=std::min(x0+1,kWidth-1),y1=std::min(y0+1,kHeight-1);
 const float fx=x-x0,fy=y-y0;
 auto at=[&](int xx,int yy){return float(rgb[(std::size_t(yy)*kWidth+xx)*3+channel])/255.0f;};
 const float a=at(x0,y0)+(at(x1,y0)-at(x0,y0))*fx;
 const float b=at(x0,y1)+(at(x1,y1)-at(x0,y1))*fx;
 return a+(b-a)*fy;
}
}
bool processNtscWaveformRoundTrip(ConstImageView src,ImageView dst,const NtscWaveformParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.strength))return false;
 const float strength=clamp01(p.strength);
 if(strength<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 try {
  std::vector<std::uint8_t> packed(std::size_t(kWidth)*kHeight*3);
  for(int y=0;y<kHeight;++y)for(int x=0;x<kWidth;++x){
   const float sx=(float(x)+0.5f)*float(src.width)/float(kWidth)-0.5f;
   const float sy=(float(y)+0.5f)*float(src.height)/float(kHeight)-0.5f;
   float rgba[4];sampleInput(src,sx,sy,p.premultipliedAlpha,rgba);
   const std::size_t i=(std::size_t(y)*kWidth+x)*3;
   packed[i]=std::uint8_t(std::lround(clamp01(rgba[0])*255.0f));
   packed[i+1]=std::uint8_t(std::lround(clamp01(rgba[1])*255.0f));
   packed[i+2]=std::uint8_t(std::lround(clamp01(rgba[2])*255.0f));
  }
  rf2::NtscEncoder encoder(p.signal);
  std::vector<float> waveform;
  encoder.EncodeFrameAtFloat(packed.data(),p.frame,&waveform);
  rf2::NtscEffects effects(p.signal);effects.controls()=p.effects;
  if(effects.HasActiveEffects())effects.Apply(&waveform);
  rf2::NtscDecoder decoder(p.signal);decoder.controls()=p.decoder;
  std::vector<std::uint8_t> decoded;decoder.Decode(waveform,&decoded);
  if(decoded.size()!=std::size_t(kWidth)*kHeight*3)return false;
  for(int y=0;y<src.height;++y){const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
   for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float alpha=px[3];
    if(alpha<=1.0e-6f){std::copy_n(px,4,out);continue;}
    const float dx=(float(x)+0.5f)*float(kWidth)/float(src.width)-0.5f;
    const float dy=(float(y)+0.5f)*float(kHeight)/float(src.height)-0.5f;
    float r=sampleByte(decoded,dx,dy,0),g=sampleByte(decoded,dx,dy,1),b=sampleByte(decoded,dx,dy,2);
    float originalR=px[0],originalG=px[1],originalB=px[2];
    if(p.premultipliedAlpha){originalR/=alpha;originalG/=alpha;originalB/=alpha;}
    r=originalR+(r-originalR)*strength;g=originalG+(g-originalG)*strength;b=originalB+(b-originalB)*strength;
    if(p.premultipliedAlpha){r*=alpha;g*=alpha;b*=alpha;}
    out[0]=r;out[1]=g;out[2]=b;out[3]=alpha;
   }
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
