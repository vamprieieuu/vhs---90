#include "VHS90TapeTransportEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
inline std::uint32_t mix32(std::uint32_t x) noexcept { x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;return x; }
inline float randomSigned(std::uint32_t seed,std::int64_t frame,int x,int lane) noexcept {
 const std::uint32_t f=std::uint32_t(frame)^std::uint32_t(std::uint64_t(frame)>>32);
 const std::uint32_t h=mix32(seed^(f*0x9e3779b9u)^(std::uint32_t(x)*0xd1b54a35u)^(std::uint32_t(lane)*0x94d049bbu));
 return float(h)/4294967295.0f*2.0f-1.0f;
}
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
        s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}

inline float gaussianHash(std::uint32_t seed,int epoch,int row,int lane) noexcept {
 const float u1=std::max(1.0e-7f,hash01(std::uint32_t(row),std::uint32_t(lane),seed,std::int64_t(epoch)));
 const float u2=hash01(std::uint32_t(row),std::uint32_t(lane+1),seed,std::int64_t(epoch));
 return std::sqrt(-2.0f*std::log(u1))*std::cos(2.0f*kPi*u2);
}
std::vector<float> slowProfile(std::uint32_t seed,int epoch,int height,float amplitude,float gain,float pole,float vScale) {
 const float scale=std::max(1.0e-6f,vScale);
 const float scaledPole=std::pow(std::min(std::abs(pole),0.999999f),1.0f/scale);
 const float scaledGain=gain*std::sqrt(std::max(1.0e-12f,(1.0f-scaledPole*scaledPole)/(1.0f-pole*pole)));
 std::vector<float> out(static_cast<std::size_t>(height),0.0f);float previous=0.0f;
 for(int y=0;y<height;++y){const float raw=gaussianHash(seed,epoch,y,17)*amplitude;previous=scaledGain*raw+scaledPole*previous;out[std::size_t(y)]=previous;}
 return out;
}
inline bool inactive(const TapeTransportParams&p) noexcept {
 return p.slowDriftPixelsPerSecond==0.0f&&p.drumFlutterPixels==0.0f&&
        p.flutterPixels==0.0f&&p.stickSlipPixels==0.0f&&p.lineJitterPixels==0.0f&&
        p.trackingOffsetPixels==0.0f&&p.trackingSlopePixels==0.0f&&p.servoJitterPixels==0.0f&&p.edgeWavePixels==0.0f;
}
inline void sample(const std::vector<float>&src,int w,int h,float x,float y,float out[4]) noexcept {
 x=std::clamp(x,0.0f,float(w-1));y=std::clamp(y,0.0f,float(h-1));
 const int x0=int(x),y0=int(y),x1=std::min(x0+1,w-1),y1=std::min(y0+1,h-1);
 const float fx=x-x0,fy=y-y0;
 const size_t a=(size_t(y0)*w+x0)*4,b=(size_t(y0)*w+x1)*4,c=(size_t(y1)*w+x0)*4,d=(size_t(y1)*w+x1)*4;
 for(int k=0;k<4;++k){const float ab=src[a+k]+(src[b+k]-src[a+k])*fx,cd=src[c+k]+(src[d+k]-src[c+k])*fx;out[k]=ab+(cd-ab)*fy;}
}
}
bool processTapeTransportWarp(ConstImageView src,ImageView dst,const TapeTransportParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.framesPerSecond)||!std::isfinite(p.fieldRateHz)||
    !std::isfinite(p.slowDriftPixelsPerSecond)||!std::isfinite(p.drumFlutterPixels)||
    !std::isfinite(p.flutterHz)||!std::isfinite(p.flutterPixels)||
    !std::isfinite(p.stickSlipPixels)||!std::isfinite(p.lineJitterPixels)||
    !std::isfinite(p.trackingOffsetPixels)||!std::isfinite(p.trackingSlopePixels)||!std::isfinite(p.servoJitterPixels)||!std::isfinite(p.edgeWavePixels)||!std::isfinite(p.horizontalScale)||!std::isfinite(p.verticalScale)||p.horizontalScale<=0||p.verticalScale<=0) return false;
 if(inactive(p)){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 try {
  const int w=src.width,h=src.height;std::vector<float>copy(size_t(w)*h*4);
  for(int y=0;y<h;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,w*4,copy.data()+size_t(y)*w*4);
  const float fps=std::max(p.framesPerSecond,1.0f),time=float(p.frame)/fps;
  const int epochFrames=4,epoch=int(p.frame/epochFrames);const float epochMix=float(p.frame%epochFrames)/float(epochFrames);
  std::vector<float> servo,servoNext,edge,edgeNext;
  if(p.servoJitterPixels>0){servo=slowProfile(p.seed^0x51A7u,epoch,h,p.servoJitterPixels*.55f,.055f,.945f,p.verticalScale);servoNext=slowProfile(p.seed^0x51A7u,epoch+1,h,p.servoJitterPixels*.55f,.055f,.945f,p.verticalScale);}
  if(p.edgeWavePixels>0){const float amp=p.edgeWavePixels*.62f*p.horizontalScale;edge=slowProfile(p.seed^0xED63u,epoch,h,amp,.08f,.92f,p.verticalScale);edgeNext=slowProfile(p.seed^0xED63u,epoch+1,h,amp,.08f,.92f,p.verticalScale);}
  for(int y=0;y<h;++y){
   const float line=float(y)/float(std::max(1,h-1));
   const float drum=std::sin(2.0f*kPi*(time*p.fieldRateHz+line*0.035f))*p.drumFlutterPixels;
   const float flutter=std::sin(2.0f*kPi*(time*p.flutterHz+line*0.17f))*p.flutterPixels;
   const float stick=randomSigned(p.seed,p.frame,y/3,1)*p.stickSlipPixels;
   const float jitter=randomSigned(p.seed,p.frame,y,2)*p.lineJitterPixels;
   const float tracking=p.trackingOffsetPixels+p.trackingSlopePixels*(line-0.5f);
   const float drift=time*p.slowDriftPixelsPerSecond;
   const float servoShift=servo.empty()?0.0f:servo[std::size_t(y)]*(1.0f-epochMix)+servoNext[std::size_t(y)]*epochMix;
   const float edgeShift=edge.empty()?0.0f:edge[std::size_t(y)]*(1.0f-epochMix)+edgeNext[std::size_t(y)]*epochMix;
   const float shift=drum+flutter+stick+jitter+tracking+drift+servoShift+edgeShift;
   float*row=dst.rgba+y*dst.rowStrideFloats;
   for(int x=0;x<w;++x){float q[4];sample(copy,w,h,float(x)-shift,float(y),q);row[x*4]=q[0];row[x*4+1]=q[1];row[x*4+2]=q[2];row[x*4+3]=q[3];}
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
