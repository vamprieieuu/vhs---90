#include "VHS90ReceiverInstabilityEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline float sample(const float*row,int w,float x,int c) noexcept {
 x=std::clamp(x,0.0f,float(w-1));const int x0=int(x),x1=std::min(x0+1,w-1);const float t=x-float(x0);
 return row[x0*4+c]+(row[x1*4+c]-row[x0*4+c])*t;
}
}
bool processReceiverInstability(ConstImageView src,ImageView dst,const ReceiverInstabilityParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.afcHunt)||!std::isfinite(p.chromaFlutter)||!std::isfinite(p.ycCrosstalk)||!std::isfinite(p.horizontalSyncNoise)||!std::isfinite(p.verticalSyncNoise)||!std::isfinite(p.burstNoise))return false;
 const float afc=std::clamp(p.afcHunt,0.0f,1.0f),flutter=std::clamp(p.chromaFlutter,0.0f,1.0f),cross=std::clamp(p.ycCrosstalk,0.0f,1.0f),hsync=std::clamp(p.horizontalSyncNoise,0.0f,1.0f),vsync=std::clamp(p.verticalSyncNoise,0.0f,1.0f),burst=std::clamp(p.burstNoise,0.0f,1.0f);
 if(afc<=0&&flutter<=0&&cross<=0&&hsync<=0&&vsync<=0&&burst<=0){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 const float phase=0.0051f*float(p.seed)+0.0023f*float(p.frame);
 const int verticalOffset=int(std::lround(std::sin(phase*0.37f+0.4f)*vsync*2.0f));
 for(int y=0;y<src.height;++y){
  const float paired=float(std::min(y,(y+src.height/2)%src.height));
  const float linePhase=phase*0.41f+0.062f*paired;
  const float baseShift=(0.08f+7.2f*afc)*(std::sin(linePhase)+0.42f*std::sin(phase*0.19f+0.029f*paired+2.0f));
  const float slipProb=0.003f+0.12f*afc*afc;
  const float slip=(hash01(std::uint32_t(y),11u,p.seed,p.frame)<slipProb)?
      (hash01(std::uint32_t(y),12u,p.seed,p.frame)<0.5f?-1.0f:1.0f)*(0.7f+6.5f*afc):0.0f;
  const float micro=(hash01(std::uint32_t(y),13u,p.seed,p.frame)*2.0f-1.0f)*(0.02f+0.75f*afc);
  const float fastDecay=7.0f+13.0f*(1.0f-afc),slowDecay=34.0f+76.0f*(1.0f-afc);
  const float burstPhase=(hash01(std::uint32_t(y),41u,p.seed,p.frame)*2.0f-1.0f)*(0.25f*burst);
  const float phaseJit=(0.22f+2.4f*flutter)*flutter*std::sin(phase*0.31f+0.043f*paired)+
      (hash01(std::uint32_t(y),21u,p.seed,p.frame)*2.0f-1.0f)*(0.65f*flutter)+burstPhase;
  const float ampJit=1.0f+(0.06f+1.18f*flutter)*flutter*std::sin(phase*0.47f+0.071f*paired+0.9f);
  const int sourceY=std::clamp(y+verticalOffset,0,src.height-1);
  const float*sr=src.rgba+sourceY*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  const float hsyncShift=(hash01(std::uint32_t(y),51u,p.seed,p.frame)*2.0f-1.0f)*(2.5f*hsync);
  for(int x=0;x<src.width;++x){
   const float left=std::exp(-float(x)/slowDecay),sync=std::exp(-float(x)/fastDecay);
   const float shift=afc*(baseShift*left+(slip+micro)*sync)+hsyncShift;
   const float sx=float(x)-shift;const float alpha=sample(sr,src.width,sx,3);
   if(alpha<=1.0e-6f){std::copy_n(sr+x*4,4,dr+x*4);continue;}
   RGB rgb{sample(sr,src.width,sx,0),sample(sr,src.width,sx,1),sample(sr,src.width,sx,2)};
   if(p.premultipliedAlpha){rgb.r/=alpha;rgb.g/=alpha;rgb.b/=alpha;}
   YIQ yiq=rgbToYiq(rgb);
   if(flutter>0){
    const float angle=phaseJit*(kPi*0.5f);
    const float ii=yiq.i*std::cos(angle)-yiq.q*std::sin(angle);
    const float qq=yiq.i*std::sin(angle)+yiq.q*std::cos(angle);
    const float corr=(hash01(std::uint32_t(x),std::uint32_t(y),p.seed^0xA51u,p.frame)*2.0f-1.0f)*(0.012f*flutter);
    yiq.i=(ii*ampJit)+corr;yiq.q=(qq*ampJit)-corr*0.65f;
   }
   if(cross>0&&x>0&&x+1<src.width){
    const float l=rgbToYiq({sample(sr,src.width,float(x)-1.0f,0),sample(sr,src.width,float(x)-1.0f,1),sample(sr,src.width,float(x)-1.0f,2)}).y;
    const float r=rgbToYiq({sample(sr,src.width,float(x)+1.0f,0),sample(sr,src.width,float(x)+1.0f,1),sample(sr,src.width,float(x)+1.0f,2)}).y;
    const float hp=yiq.y-0.5f*(l+r);
    const float edge=std::clamp(std::abs(hp)/(0.02f+0.20f*(1.0f-cross)),0.0f,1.0f);
    const float carrier=std::sin((kPi*0.5f)*float(x)+((y&1)?kPi:0.0f)+0.72f);
    yiq.i+=hp*edge*cross*0.45f*carrier;yiq.q+=hp*edge*cross*0.32f*std::cos((kPi*0.5f)*float(x)+0.72f);
   }
   rgb=yiqToRgb(yiq);if(p.premultipliedAlpha){rgb.r*=alpha;rgb.g*=alpha;rgb.b*=alpha;}
   float*out=dr+x*4;out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=alpha;
  }
 }
 return true;
}
} // namespace vhs90
