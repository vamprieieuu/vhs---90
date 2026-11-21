#include "VHS90ImageGradeEngine.h"
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
inline RGB straightAt(ConstImageView s,int x,int y,bool premul) noexcept {
 x=std::clamp(x,0,s.width-1);y=std::clamp(y,0,s.height-1);
 const float*p=s.rgba+y*s.rowStrideFloats+x*4;RGB c{p[0],p[1],p[2]};
 if(premul&&p[3]>1.0e-6f){c.r/=p[3];c.g/=p[3];c.b/=p[3];}return c;
}
inline float clamp01(float v) noexcept{return std::clamp(v,0.0f,1.0f);}
}
bool processImageGrade(ConstImageView src,ImageView dst,const ImageGradeParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.edgeEnhancement)||!std::isfinite(p.chromaEdgeEnhancement)||!std::isfinite(p.chromaSharpenCutoffHz)||p.chromaSharpenCutoffHz<=0.0f||!std::isfinite(p.fade)||!std::isfinite(p.blackLift)||
    !std::isfinite(p.contrast)||!std::isfinite(p.brightness)||!std::isfinite(p.saturation)||
    !std::isfinite(p.chromaIGain)||!std::isfinite(p.chromaQGain)||!std::isfinite(p.chromaIOffset)||!std::isfinite(p.chromaQOffset)||!std::isfinite(p.chromaQuantization)||
    !std::isfinite(p.vignette)||!std::isfinite(p.flicker))return false;
 const float edge=clamp01(p.edgeEnhancement),chromaEdge=clamp01(p.chromaEdgeEnhancement),fade=clamp01(p.fade),blackLift=clamp01(p.blackLift),contrast=std::clamp(p.contrast,0.0f,3.0f);
 const float sat=std::clamp(p.saturation,0.0f,5.0f),vignette=clamp01(p.vignette),flicker=clamp01(p.flicker);
 const float iGain=std::clamp(p.chromaIGain,0.0f,3.0f),qGain=std::clamp(p.chromaQGain,0.0f,3.0f);
 const float iOffset=std::clamp(p.chromaIOffset,-0.5f,0.5f),qOffset=std::clamp(p.chromaQOffset,-0.5f,0.5f);
 const float chromaStep=std::clamp(p.chromaQuantization,0.0f,0.0833333f);
 if(edge==0&&chromaEdge==0&&fade==0&&contrast==1&&p.brightness==0&&blackLift==0&&sat==1&&iGain==1&&qGain==1&&iOffset==0&&qOffset==0&&chromaStep==0&&vignette==0&&flicker==0){
  for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;
 }
 const float flickerValue=std::sin(float(p.frame)*0.73f)*flicker*0.10f+
   (hash01(0u,0u,p.seed,p.frame)*2.0f-1.0f)*flicker*0.20f;
 const float fadeLuma=1.0f-0.15f*fade,fadeLift=0.09f*fade,fadeSat=1.0f-0.25f*fade;
 const float cx=std::max(1.0f,float(src.width-1)*0.5f),cy=std::max(1.0f,float(src.height-1)*0.5f);
 const float maxD=std::sqrt(2.0f);
 // Three cascaded first-order low-pass stages model the source VHS playback
 // chroma unsharp path. The input is full-width RGB, so use the full pixel
 // sample rate while preserving the source's 4:2:2 spatial cutoff.
 constexpr float kChromaSampleRate=14318182.0f;
 const float tau=1.0f/(p.chromaSharpenCutoffHz*2.0f*kPi);
 const float chromaAlpha=(1.0f/kChromaSampleRate)/(tau+(1.0f/kChromaSampleRate));
 for(int y=0;y<src.height;++y){float*dr=dst.rgba+y*dst.rowStrideFloats;
  float lowI1=0.0f,lowI2=0.0f,lowI3=0.0f,lowQ1=0.0f,lowQ2=0.0f,lowQ3=0.0f;
  for(int x=0;x<src.width;++x){const float*px=src.rgba+y*src.rowStrideFloats+x*4;const float a=px[3];float*out=dr+x*4;
   if(a<=1.0e-6f){std::copy_n(px,4,out);continue;}
   RGB rgb=straightAt(src,x,y,p.premultipliedAlpha);YIQ yiq=rgbToYiq(rgb);
   if(edge>0.0f){
    // URP Retro VHS used a blurred reference image for unsharp edge recovery.
    // Use a normalized 3x3 Gaussian reference so sharpening is isotropic and
    // does not create a one-pixel horizontal halo on vertical/diagonal edges.
    float blur=4.0f*yiq.y;
    blur+=2.0f*rgbToYiq(straightAt(src,x-1,y,p.premultipliedAlpha)).y;
    blur+=2.0f*rgbToYiq(straightAt(src,x+1,y,p.premultipliedAlpha)).y;
    blur+=2.0f*rgbToYiq(straightAt(src,x,y-1,p.premultipliedAlpha)).y;
    blur+=2.0f*rgbToYiq(straightAt(src,x,y+1,p.premultipliedAlpha)).y;
    blur+=rgbToYiq(straightAt(src,x-1,y-1,p.premultipliedAlpha)).y;
    blur+=rgbToYiq(straightAt(src,x+1,y-1,p.premultipliedAlpha)).y;
    blur+=rgbToYiq(straightAt(src,x-1,y+1,p.premultipliedAlpha)).y;
    blur+=rgbToYiq(straightAt(src,x+1,y+1,p.premultipliedAlpha)).y;
    blur*=1.0f/16.0f;
    const float detail=yiq.y-blur;
    const float deadZone=0.0015f;
    const float cleanDetail=std::copysign(std::max(0.0f,std::fabs(detail)-deadZone),detail);
    yiq.y+=edge*0.90f*cleanDetail;
   }
   if(chromaEdge>0.0f){
    // Three-pole I/Q low-pass reference followed by independent chroma
    // unsharp gain; each scanline resets to neutral chroma (I=Q=0).
    lowI1+=chromaAlpha*(yiq.i-lowI1);lowI2+=chromaAlpha*(lowI1-lowI2);lowI3+=chromaAlpha*(lowI2-lowI3);
    lowQ1+=chromaAlpha*(yiq.q-lowQ1);lowQ2+=chromaAlpha*(lowQ1-lowQ2);lowQ3+=chromaAlpha*(lowQ2-lowQ3);
    yiq.i+=(yiq.i-lowI3)*(0.85f*chromaEdge);
    yiq.q+=(yiq.q-lowQ3)*(0.85f*chromaEdge);
   }
   yiq.y=yiq.y*fadeLuma+fadeLift;
   const float pedestal=(46.0f/255.0f)*blackLift; yiq.y=yiq.y*(1.0f-pedestal)+pedestal;
   yiq.i*=fadeSat;yiq.q*=fadeSat;
   yiq.y=(yiq.y-0.5f)*contrast+0.5f+p.brightness;
   yiq.i=yiq.i*sat*fadeSat*iGain+iOffset;
   yiq.q=yiq.q*sat*fadeSat*qGain+qOffset;
   if(chromaStep>1.0e-6f){
    // Quantize chroma magnitude while preserving its phase. Independent I/Q
    // rounding rotates hue and can create unrelated false-colour patches.
    const float chromaMagnitude=std::sqrt(yiq.i*yiq.i+yiq.q*yiq.q);
    if(chromaMagnitude>1.0e-6f){
     const float quantizedMagnitude=std::round(chromaMagnitude/chromaStep)*chromaStep;
     const float scale=quantizedMagnitude/chromaMagnitude;
     yiq.i*=scale;yiq.q*=scale;
    }
   }
   const float dx=(float(x)-cx)/cx,dy=(float(y)-cy)/cy;
   const float d=std::sqrt(dx*dx+dy*dy)/maxD;
   const float vig=1.0f-std::min(0.80f,vignette*0.70f)*d*d;
   yiq.y=yiq.y*vig+flickerValue/255.0f;
   rgb=yiqToRgb(yiq);if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
   out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
  }
 }
 return true;
}
} // namespace vhs90
