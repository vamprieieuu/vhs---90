#include "VHS90RfInstabilityEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline float clamp01(float x) noexcept {return std::clamp(x,0.0f,1.0f);}
inline float sample(const float* row,int width,float x,int channel) noexcept {
 x=std::clamp(x,0.0f,float(width-1));const int x0=int(x),x1=std::min(x0+1,width-1);
 const float t=x-float(x0);return row[x0*4+channel]+(row[x1*4+channel]-row[x0*4+channel])*t;
}
}
bool processRfInstability(ConstImageView src,ImageView dst,const RfInstabilityParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.drift)||!std::isfinite(p.hum)||
    !std::isfinite(p.amNonlinearity)||!std::isfinite(p.agcPump)||!std::isfinite(p.blackPedestal))return false;
 const float drift=clamp01(p.drift),hum=clamp01(p.hum),am=clamp01(p.amNonlinearity),agc=clamp01(p.agcPump);
 if(drift==0.0f&&hum==0.0f&&am==0.0f&&agc==0.0f&&p.blackPedestal==0.0f) {
  for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
  return true;
 }
 const float framePhase=0.0042f*float(p.frame)+0.0039f*float(p.seed);
 const float humPeriod=3.0f-1.0f*hum;
 const float humPhase=framePhase+float(p.frame)*(2.0f*kPi/std::max(1.0f,30.0f*humPeriod));
 float lumaEnergy=0.0f,transientEnergy=0.0f;std::size_t energyCount=0;
 if(agc>0.0f){
  for(int y=0;y<src.height;++y){const float*row=src.rgba+y*src.rowStrideFloats;float previous=0.0f;bool havePrevious=false;
   for(int x=0;x<src.width;++x){const float a=row[x*4+3];if(a<=1.0e-6f)continue;
    RGB rgb{row[x*4],row[x*4+1],row[x*4+2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
    const float lum=rgbToYiq(rgb).y;
    lumaEnergy+=lum;
    if(havePrevious)transientEnergy+=std::abs(lum-previous);
    previous=lum;havePrevious=true;++energyCount;
   }
  }
 }
 const float invCount=energyCount?1.0f/float(energyCount):0.0f;
 const float meanLuma=lumaEnergy*invCount;
 const float sceneDrive=std::clamp((meanLuma-0.12f)/0.58f,0.0f,1.0f);
 const float transDrive=std::clamp(transientEnergy*invCount/0.045f,0.0f,1.0f);
 // AGC pumping follows the average picture level and temporal envelope. It
 // remains active on ordinary footage instead of requiring near-white peaks.
 const float eventDrive=std::clamp(0.68f*sceneDrive+0.32f*transDrive,0.0f,1.0f);
 const float pulse=0.5f+0.5f*std::sin(framePhase*0.47f+0.6f);
 const float pump=agc*(0.06f+0.52f*eventDrive)*pulse;
 const float hiKnee=0.68f-0.20f*am,loKnee=0.40f-0.12f*am;
 const float hiComp=0.010f+0.085f*am,loComp=0.013f+0.110f*am;
 for(int y=0;y<src.height;++y) {
  const float rowPosition=(float(y)+0.5f)/float(src.height);
  const float paired=float(std::min(y,(y+src.height/2)%src.height));
  const float humLinePhase=humPhase+paired*(0.8f+4.2f*hum)*((2.0f*kPi)/std::max(1.0f,float(src.height)));
  const float humOffset=(0.5f+8.8f*hum)*std::sin(humLinePhase+0.2f)+
                        (0.2f+4.6f*hum)*std::sin(2.0f*humLinePhase+1.7f);
  const float humGain=1.0f+(0.003f+0.042f*hum)*std::sin(humLinePhase+0.9f);
  // RF drift is time-base instability: slow, medium and stochastic line
  // displacement. It must not behave like a global exposure/opacity slider.
  const float slowDrift=std::sin(2.0f*kPi*(rowPosition*1.15f-framePhase*0.07f));
  const float mediumDrift=std::sin(2.0f*kPi*(rowPosition*4.3f+framePhase*0.13f));
  const float randomDrift=valueNoise1D(rowPosition*17.0f+framePhase*0.05f,p.seed,p.frame/4)-0.5f;
  const float shift=drift*(0.6f+10.0f*slowDrift+3.0f*mediumDrift+8.0f*randomDrift);
  const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x) {
   const float sx=float(x)-shift;const float alpha=sample(sr,src.width,sx,3);
   if(alpha<=1.0e-6f){std::copy_n(sr+x*4,4,dr+x*4);continue;}
   RGB rgb{sample(sr,src.width,sx,0),sample(sr,src.width,sx,1),sample(sr,src.width,sx,2)};
   if(p.premultipliedAlpha){rgb.r/=alpha;rgb.g/=alpha;rgb.b/=alpha;}
   YIQ yiq=rgbToYiq(rgb);
   float luma=yiq.y;
   const float ripple=(0.12f+3.2f*hum)*std::sin(humLinePhase+0.0095f*float(x)+
                       0.25f*std::sin(framePhase*0.71f+0.0032f*float(x)));
   const float interference=(0.03f+1.1f*hum)*std::sin((kPi*0.5f)*float(x)+0.63f*humLinePhase);
   luma=p.blackPedestal+(luma-p.blackPedestal)*humGain+humOffset*0.01f+ripple*0.01f+interference*0.01f;
   if(pump>0.0001f){const float linePump=pump*(0.86f+0.14f*std::sin(framePhase*0.53f+0.024f*float(y)));
    const float knee=0.24f+0.14f*(1.0f-agc);float centered=luma-p.blackPedestal;
    if(centered>knee)centered=knee+(centered-knee)*(1.0f-0.92f*linePump);
    else if(centered>0.0f)centered*=1.0f-0.20f*linePump*(centered/(knee+0.01f));
    luma=p.blackPedestal+centered;
   }
   float v=luma-p.blackPedestal;
   if(v>hiKnee){const float over=v-hiKnee;v=hiKnee+over/(1.0f+hiComp*over);}
   else if(v< -loKnee){const float under=-loKnee-v;v=-loKnee-under/(1.0f+loComp*under);}
   yiq.y=p.blackPedestal+v;
   rgb=yiqToRgb(yiq);if(p.premultipliedAlpha){rgb.r*=alpha;rgb.g*=alpha;rgb.b*=alpha;}
   float*out=dr+x*4;out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=alpha;
  }
 }
 return true;
}
} // namespace vhs90
