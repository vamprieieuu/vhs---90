#include "VHS90TrackReadEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
inline float sinc(float x) noexcept { return std::fabs(x)<1.0e-5f?1.0f:std::sin(x)/x; }
inline float azimuthLoss(float overlapInSpTrackWidths,float frequencyMHz) noexcept {
 // NTSCRT TapeFormat::azimuth_loss: x = pi * width_um * tan(12deg) / wavelength_um.
 // The raster overlap is normalized to one 58 um SP track width.
 constexpr float kTrackWidthUm=58.0f;
 constexpr float kWritingSpeedUmPerUs=5.8f;
 constexpr float kTanAzimuth=0.21255656f;
 const float widthUm=overlapInSpTrackWidths*kTrackWidthUm;
 const float x=kPi*widthUm*kTanAzimuth*frequencyMHz/kWritingSpeedUmPerUs;
 return sinc(x);
}
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline RGB read(ConstImageView s,int x,int y,bool premul) noexcept {
 x=std::clamp(x,0,s.width-1);y=std::clamp(y,0,s.height-1);
 const float*p=s.rgba+y*s.rowStrideFloats+x*4;RGB c{p[0],p[1],p[2]};
 if(premul&&p[3]>1e-6f){c.r/=p[3];c.g/=p[3];c.b/=p[3];}return c;
}
}
bool processTrackRead(ConstImageView src,ImageView dst,const TrackReadParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount)||!std::isfinite(p.trackingOffset)||p.speed<0||p.speed>2)return false;
 const float amount=std::clamp(p.amount,0.0f,1.0f),delta=std::clamp(p.trackingOffset,-1.5f,1.5f);
 if(amount<=0.0f||(p.speed==0&&std::fabs(delta)<1e-5f)){
  for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
  return true;
 }
 const float pitch=(p.speed==0?1.0f:(p.speed==1?0.5f:1.0f/3.0f));
 const float headWidth=(p.speed==0?1.0f:0.4483f); // normalized to an SP head width
 const float trackWidth=std::min(headWidth,pitch);
 const float center=float(std::round(delta));
 const float lo=delta*pitch-headWidth*0.5f,hi=delta*pitch+headWidth*0.5f;
 const float vbi=float(std::max(12,int(std::lround(float(src.height)*20.0f/242.5f))));
 const float rasterScale=(float(src.height)+vbi)/262.5f;
 const float rowsPerTrack=(1.5f/float(p.speed+1))*rasterScale;
 float weights[3]={0,0,0},fm[3]={0,0,0},color[3]={0,0,0};
 float sum=0;
 for(int t=0;t<3;++t){
  const int k=int(center)-1+t;
  const float overlap=std::max(0.0f,std::min(hi,float(k)*pitch+trackWidth*.5f)-std::max(lo,float(k)*pitch-trackWidth*.5f));
  const float share=overlap; // normalized to one 58 µm SP track
  const bool opposite=(k%2)!=0;
  const float fmLoss=opposite?azimuthLoss(overlap,4.0f):1.0f;
  const float chromaLoss=opposite?azimuthLoss(overlap,40.0f/63.556f):1.0f;
  weights[t]=share;fm[t]=share*fmLoss;color[t]=share*chromaLoss;sum+=share;
 }
 if(sum<1e-5f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 // Keep absolute overlap shares: LP/EP narrow heads intentionally read less tape.
 for(int y=0;y<src.height;++y){float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x){const float*px=src.rgba+y*src.rowStrideFloats+x*4;const float a=px[3];float*out=dr+x*4;
   if(a<=1e-6f){std::copy_n(px,4,out);continue;}
   YIQ mixed{0,0,0};
   for(int t=0;t<3;++t){if(weights[t]<=0)continue;const int k=int(center)-1+t;const int rowOffset=int(std::lround((center-float(k))*rowsPerTrack));const RGB c=read(src,x,y+rowOffset,p.premultipliedAlpha);const YIQ q=rgbToYiq(c);
    mixed.y+=q.y*fm[t];mixed.i+=q.i*color[t];mixed.q+=q.q*color[t];
   }
   const RGB original=read(src,x,y,p.premultipliedAlpha);const YIQ base=rgbToYiq(original);
   mixed.y=base.y+(mixed.y-base.y)*amount;mixed.i=base.i+(mixed.i-base.i)*amount;mixed.q=base.q+(mixed.q-base.q)*amount;
   RGB result=yiqToRgb(mixed);if(p.premultipliedAlpha){result.r*=a;result.g*=a;result.b*=a;}
   out[0]=result.r;out[1]=result.g;out[2]=result.b;out[3]=a;
  }
 }
 return true;
}
} // namespace vhs90
