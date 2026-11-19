#include "VHS90TapeSurfaceEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
inline std::uint64_t mix64(std::uint64_t x) noexcept { x^=x>>30;x*=0xbf58476d1ce4e5b9ULL;x^=x>>27;x*=0x94d049bb133111ebULL;x^=x>>31;return x; }
inline std::uint64_t hashCell(std::uint32_t seed,std::int64_t x,std::int64_t y,std::uint32_t lane=0) noexcept {
 return mix64(std::uint64_t(seed)^(std::uint64_t(x)*0x9e3779b97f4a7c15ULL)^(std::uint64_t(y)*0xd1b54a32d192ed03ULL)^(std::uint64_t(lane)*0x94d049bb133111ebULL));
}
inline float unit(std::uint64_t h) noexcept { return float((h>>40)&0xffffffu)*(1.0f/16777216.0f); }
inline float smooth(float x) noexcept { x=std::clamp(x,0.0f,1.0f);return x*x*(3.0f-2.0f*x); }
inline float falloffEllipse(float dx,float dy,float rx,float ry) noexcept {
 if(rx<=0.0f||ry<=0.0f)return 0.0f;const float q=dx*dx/(rx*rx)+dy*dy/(ry*ry);return q>=1.0f?0.0f:1.0f-smooth(q);
}
inline RGB straightAt(ConstImageView s,int x,int y,bool premul) noexcept { x=std::clamp(x,0,s.width-1);y=std::clamp(y,0,s.height-1);const float*p=s.rgba+y*s.rowStrideFloats+x*4;RGB c{p[0],p[1],p[2]};if(premul&&p[3]>1.0e-6f){c.r/=p[3];c.g/=p[3];c.b/=p[3];}return c; }
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline bool inactive(const TapeSurfaceParams&p) noexcept { return p.defectDensity<=0.0f&&p.edgeLift<=0.0f&&p.surfaceRoughness<=0.0f; }
}
TapeSurfaceSample sampleTapeSurface(float u,float v,const TapeSurfaceParams&p) noexcept {
 const float cu=std::max(p.alongCellSize,1.0e-4f),cv=std::max(p.acrossCellSize,1.0e-4f);
 const auto ix=static_cast<std::int64_t>(std::floor(u/cu)),iy=static_cast<std::int64_t>(std::floor(v/cv));
 TapeSurfaceSample out;const float density=std::clamp(p.defectDensity,0.0f,1.0f);
 for(int oy=-1;oy<=1;++oy)for(int ox=-3;ox<=3;++ox){
  const auto cx=ix+ox,cy=iy+oy;const auto h=hashCell(p.seed,cx,cy);if(unit(h)>density*0.42f)continue;
  const float cu0=(float(cx)+unit(hashCell(p.seed,cx,cy,1)))*cu,cv0=(float(cy)+unit(hashCell(p.seed,cx,cy,2)))*cv;
  const float du=u-cu0,dv=v-cv0,type=unit(hashCell(p.seed,cx,cy,3));
  const float scale=0.25f+2.5f*std::pow(unit(hashCell(p.seed,cx,cy,4)),2.2f);
  const float ru=cu*scale,rv=cv*(0.06f+0.42f*unit(hashCell(p.seed,cx,cy,5)));
  if(type<0.42f){const float f=falloffEllipse(du,dv,ru,rv);out.magnetization*=1.0f-std::clamp(p.oxideLoss*f,0.0f,0.95f);}
  else if(type<0.68f){const float f=falloffEllipse(du,dv,ru*2.2f,rv*0.12f);out.magnetization*=1.0f-std::clamp(p.scratchDensity*f,0.0f,0.9f);out.roughness+=f*p.surfaceRoughness;}
  else if(type<0.86f){const float f=falloffEllipse(du,dv,ru*1.5f,rv*0.28f);out.creaseLoss=std::max(out.creaseLoss,std::clamp(p.creaseStrength*f,0.0f,1.0f));out.spacingLift+=f*p.debrisAmount*0.8f;}
  else{const float f=falloffEllipse(du,dv,ru*0.45f,rv*0.65f);out.spacingLift+=f*p.debrisAmount;out.roughness+=f*p.surfaceRoughness*1.5f;}
 }
 const float edge=std::min(v,1.0f-v),edgeProfile=1.0f-smooth(std::clamp(edge/0.06f,0.0f,1.0f));
 out.spacingLift+=edgeProfile*std::max(0.0f,p.edgeLift);
 out.roughness+=std::abs(std::sin((u*173.0f+v*29.0f)*kPi))*p.surfaceRoughness*0.15f;
 out.magnetization=std::clamp(out.magnetization,0.0f,1.0f);out.spacingLift=std::clamp(out.spacingLift,0.0f,1.0f);
 out.roughness=std::clamp(out.roughness,0.0f,0.25f);out.creaseLoss=std::clamp(out.creaseLoss,0.0f,1.0f);return out;
}
bool processTapeSurface(ConstImageView src,ImageView dst,const TapeSurfaceParams&p) noexcept {
 if(!valid(src,dst))return false;
 if(inactive(p)){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 for(int y=0;y<src.height;++y){const float v=(float(y)+0.5f)/float(src.height);const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];if(a<=1e-6f){std::copy_n(px,4,out);continue;}
   const float u=(float(x)+0.5f)/float(src.width)+float(p.frame)*p.tapeAdvancePerFrame;const auto d=sampleTapeSurface(u,v,p);
   RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
   YIQ q=rgbToYiq(rgb);const float mag=d.magnetization*(1.0f-d.creaseLoss);
   // NTSCRT/VcrStage channelGain: head spacing damages high-frequency FM luma
   // more than the lower-frequency color-under carrier.
   const float spacingUm=4.0f*d.spacingLift,headGapUm=1.0f,writingSpeed=5.8f;
   const auto channelGain=[&](float mhz) noexcept {
    const float spacing=std::exp(-2.0f*kPi*spacingUm*mhz/writingSpeed);
    const float aperture=kPi*headGapUm*mhz/writingSpeed;
    const float gap=std::fabs(aperture)<1.0e-5f?1.0f:std::fabs(std::sin(aperture)/aperture);
    return spacing*gap;
   };
   const float lumaGain=channelGain(4.0f),chromaGain=channelGain(0.62937f);
   const float leftY=rgbToYiq(straightAt(src,std::max(0,x-1),y,p.premultipliedAlpha)).y;
   const float rightY=rgbToYiq(straightAt(src,std::min(src.width-1,x+1),y,p.premultipliedAlpha)).y;
   const float lowY=(leftY+2.0f*q.y+rightY)*0.25f;
   q.y=(lowY+(q.y-lowY)*lumaGain)*mag;
   const float ca=mag*chromaGain;q.i*=ca;q.q*=ca;
   const auto nh=hashCell(p.seed^0xA511E9B3u,static_cast<std::int64_t>(std::floor(u*4096.0f)),static_cast<std::int64_t>(std::floor(v*1024.0f)),static_cast<std::uint32_t>(p.frame));
   q.y+=(unit(nh)*2.0f-1.0f)*d.roughness;rgb=yiqToRgb(q);if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
   out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
  }
 }return true;
}
} // namespace vhs90
