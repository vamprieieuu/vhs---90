#include "VHS90AnalogNoiseEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
inline float gaussian(std::uint32_t x,std::uint32_t y,std::uint32_t seed,std::int64_t frame) noexcept {
 const float u1=std::max(hash01(x,y,seed,frame),1.0e-7f);
 const float u2=hash01(x+0x9e37u,y+0x85ebu,seed^0xA511E9B3u,frame);
 return std::clamp(std::sqrt(-2.0f*std::log(u1))*std::cos(2.0f*kPi*u2),-3.0f,3.0f);
}
inline float smoothNoise1D(float x,std::uint32_t seed) noexcept {
 const int cell=int(std::floor(x));const float f=x-float(cell),t=f*f*(3.0f-2.0f*f);
 const float a=hash01(std::uint32_t(cell),0u,seed,0),b=hash01(std::uint32_t(cell+1),0u,seed,0);
 return a+(b-a)*t;
}
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
}
bool processAnalogNoise(ConstImageView src,ImageView dst,const AnalogNoiseParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.lumaAmount)||!std::isfinite(p.chromaAmount)||
    !std::isfinite(p.compositeAmount)||!std::isfinite(p.chromaLoss)||!std::isfinite(p.chromaPhaseNoise)||!std::isfinite(p.hueBiasDegrees)||!std::isfinite(p.tapeLineAmount)||!std::isfinite(p.tapeLineSpeed)||!std::isfinite(p.framesPerSecond)||
    p.lumaAmount<0.0f||p.chromaAmount<0.0f||p.compositeAmount<0.0f||p.tapeLineAmount<0.0f||p.framesPerSecond<=0.0f) return false;
 const float ly=std::clamp(p.lumaAmount,0.0f,1.0f),ch=std::clamp(p.chromaAmount,0.0f,1.0f),co=std::clamp(p.compositeAmount,0.0f,1.0f),loss=std::clamp(p.chromaLoss,0.0f,5000.0f)/65000.0f,tape=std::clamp(p.tapeLineAmount,0.0f,1.0f),phaseNoise=std::clamp(p.chromaPhaseNoise,0.0f,1.0f);
 if(ly<=0.0f&&ch<=0.0f&&co<=0.0f&&loss<=0.0f&&tape<=0.0f&&phaseNoise<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 const int lw=std::max(1,p.lumaCellWidth),lh=std::max(1,p.lumaCellHeight),cw=std::max(1,p.chromaCellWidth),chh=std::max(1,p.chromaCellHeight);
 const float angle=p.hueBiasDegrees*kPi/180.0f,di=std::cos(angle),dq=std::sin(angle);
 const float time=float(p.frame)*p.tapeLineSpeed/p.framesPerSecond;
 for(int y=0;y<src.height;++y){const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  float phaseCos=1.0f,phaseSin=0.0f;
  if(phaseNoise>0.0f){const float phase=(hash01(0u,std::uint32_t(y),p.seed^0xD1B54A35u,p.frame)-0.5f)*2.0f*phaseNoise*6.28318530718f;phaseCos=std::cos(phase);phaseSin=std::sin(phase);}
  for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];
   if(a<=1.0e-6f){std::copy_n(px,4,out);continue;}
   RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
   YIQ q=rgbToYiq(rgb);
   if(ly>0){const float n=gaussian(std::uint32_t(x/lw),std::uint32_t(y/lh),p.seed^0xC0FFEE11u,p.frame);q.y+=n*ly*0.10f;}
   if(ch>0){const float n=gaussian(std::uint32_t(x/cw),std::uint32_t(y/chh),p.seed^0x55AA7733u,p.frame);q.i+=n*ch*0.10f*di;q.q+=n*ch*0.10f*dq;}
   if(co>0){const float n=gaussian(std::uint32_t(x),std::uint32_t(y),p.seed^0x1234ABCDu,p.frame);q.y+=n*co*0.035f;}
   if(loss>0.0f&&hash01(0u,std::uint32_t(y),p.seed^0xC4A05EEDu,p.frame)<loss){q.i*=0.08f;q.q*=0.08f;}
   if(tape>0.0f) {
    const float line=smoothNoise1D(float(y)*0.012f+time,p.seed^0x13579BDFu)*
                     smoothNoise1D(float(y)*0.091f+200.0f+time,p.seed^0x2468ACE0u)*
                     smoothNoise1D(float(y)*0.917f+421.0f+time,p.seed^0xB5297A4Du);
    const float mask=hash01(std::uint32_t(x),std::uint32_t(y),p.seed^0x68E31DA4u,p.frame);
    if(line*(mask*mask*mask+0.3f)>=1.0f-tape) q.y+=0.22f*tape;
   }
   if(phaseNoise>0.0f){const float qi=q.i*phaseCos-q.q*phaseSin,qq=q.i*phaseSin+q.q*phaseCos;q.i=qi;q.q=qq;}
   rgb=yiqToRgb(q);if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
   out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
  }
 }
 return true;
}
} // namespace vhs90
