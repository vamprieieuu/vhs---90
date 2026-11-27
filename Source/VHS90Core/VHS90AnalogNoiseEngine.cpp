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
inline std::uint32_t vhs6riftHash(std::uint32_t x) noexcept {
 x=((x>>16)^x)*0x45d9f3bu; x=((x>>16)^x)*0x45d9f3bu; return (x>>16)^x;
}
inline float vhs6riftHash2D(int x,int y,std::uint32_t seed) noexcept {
 const std::uint32_t h=seed^(std::uint32_t(x)*374761393u)^(std::uint32_t(y)*668265263u);
 return float(vhs6riftHash(h)&0x00ffffffu)/16777216.0f;
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
 if(!valid(src,dst)||!std::isfinite(p.lumaAmount)||!std::isfinite(p.grainAmount)||
    !std::isfinite(p.noiseDetail)||!std::isfinite(p.shadowBias)||!std::isfinite(p.chromaAmount)||!std::isfinite(p.hueNoiseAmount)||
    !std::isfinite(p.compositeAmount)||!std::isfinite(p.chromaLoss)||!std::isfinite(p.chromaPhaseNoise)||
    !std::isfinite(p.hueBiasDegrees)||!std::isfinite(p.tapeLineAmount)||!std::isfinite(p.tapeLineSpeed)||
    !std::isfinite(p.framesPerSecond)||!std::isfinite(p.signalNoiseIntensity)||!std::isfinite(p.signalNoiseFrequency)||
    !std::isfinite(p.noiseSize)||!std::isfinite(p.noiseDensity)||!std::isfinite(p.noiseDistribution)||
    !std::isfinite(p.horizontalLineIntensity)||!std::isfinite(p.animationSpeed)||p.lumaAmount<0.0f||p.grainAmount<0.0f||
    p.chromaAmount<0.0f||p.compositeAmount<0.0f||p.tapeLineAmount<0.0f||p.framesPerSecond<=0.0f) return false;

 const float signalIntensity=std::clamp(p.signalNoiseIntensity,0.0f,1.0f);
 const float ly=std::clamp(p.lumaAmount,0.0f,1.0f)*signalIntensity;
 const float grain=p.proceduralNoiseEnabled?std::clamp(p.grainAmount,0.0f,1.0f):0.0f;
 const float detail=std::clamp(p.noiseDetail,0.0f,1.0f),shadowBias=std::clamp(p.shadowBias,0.0f,1.0f);
 const float ch=std::clamp(p.chromaAmount,0.0f,1.0f)*signalIntensity;
 const float hueNoise=std::clamp(p.hueNoiseAmount,0.0f,1.0f)*signalIntensity;
 const float co=std::clamp(p.compositeAmount,0.0f,1.0f)*signalIntensity;
 const float loss=std::clamp(p.chromaLoss,0.0f,5000.0f)/65000.0f;
 const float tape=std::clamp(p.tapeLineAmount+p.horizontalLineIntensity,0.0f,1.0f)*signalIntensity;
 const float phaseNoise=std::clamp(p.chromaPhaseNoise,0.0f,1.0f)*signalIntensity;
 const float frequency=std::clamp(p.signalNoiseFrequency,0.1f,10.0f);
 const float noiseSize=std::clamp(p.noiseSize,0.25f,16.0f);
 const float noiseDensity=std::clamp(p.noiseDensity,0.0f,1.0f);
 const float noiseDistribution=std::clamp(p.noiseDistribution,0.0f,1.0f);
 const float animationSpeed=std::clamp(p.animationSpeed,0.0f,4.0f);
 const int noiseCell=std::clamp(int(std::lround(noiseSize/frequency)),1,64);
 const std::int64_t noiseFrame=static_cast<std::int64_t>(std::floor(double(p.frame)*animationSpeed));

 const bool pixelSignalNoiseActive=noiseDensity>0.0f&&(ly>0.0f||ch>0.0f||hueNoise>0.0f||co>0.0f||phaseNoise>0.0f);
 if(grain<=0.0f&&!pixelSignalNoiseActive&&loss<=0.0f&&tape<=0.0f) {
  for(int y=0;y<src.height;++y) std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
  return true;
 }

 const float angle=p.hueBiasDegrees*kPi/180.0f,di=std::cos(angle),dq=std::sin(angle);
 const float time=float(p.frame)*p.tapeLineSpeed*animationSpeed/p.framesPerSecond;
 // Keep the original VHS6RIFT grain cadence unchanged; the new animation speed
 // controls only the unified signal-noise layer and horizontal interference.
 const int frameGrain=int(std::floor(double(p.frame)*60.0/std::max(1.0,double(p.framesPerSecond))));
 const int frameLuma=int(std::floor(double(p.frame)*120.0*animationSpeed/std::max(1.0,double(p.framesPerSecond))));
 for(int y=0;y<src.height;++y) {
  const float*sr=src.rgba+y*src.rowStrideFloats; float*dr=dst.rgba+y*dst.rowStrideFloats;
  float phaseCos=1.0f,phaseSin=0.0f;
  if(phaseNoise>0.0f) {
   const float phase=(hash01(0u,std::uint32_t(y/noiseCell),p.seed^0xD1B54A35u,noiseFrame)-0.5f)*2.0f*phaseNoise*6.28318530718f;
   phaseCos=std::cos(phase); phaseSin=std::sin(phase);
  }
  for(int x=0;x<src.width;++x) {
   const float*px=sr+x*4; float*out=dr+x*4; const float a=px[3];
   if(a<=1.0e-6f) { std::copy_n(px,4,out); continue; }
   RGB rgb{px[0],px[1],px[2]}; if(p.premultipliedAlpha) { rgb.r/=a;rgb.g/=a;rgb.b/=a; }
   YIQ q=rgbToYiq(rgb);
   const int nx=x/noiseCell,ny=y/noiseCell;
   const float cluster=hash01(std::uint32_t(nx/4),std::uint32_t(ny/4),p.seed^0xA24BAED5u,noiseFrame);
   const float localDensity=std::clamp(noiseDensity*(1.0f-noiseDistribution+noiseDistribution*(0.25f+1.5f*cluster)),0.0f,1.0f);
   const float noiseMask=hash01(std::uint32_t(nx),std::uint32_t(ny),p.seed^0x9FB21C65u,noiseFrame)<localDensity?1.0f:0.0f;
   if(co>0.0f&&noiseMask>0.0f) { const float n=gaussian(std::uint32_t(nx),std::uint32_t(ny),p.seed^0x1234ABCDu,noiseFrame); q.y+=n*co*0.035f*noiseMask; }
   if(loss>0.0f&&hash01(0u,std::uint32_t(y),p.seed^0xC4A05EEDu,p.frame)<loss) { q.i*=0.08f;q.q*=0.08f; }
   if(tape>0.0f) {
    const float line=smoothNoise1D(float(y)*0.012f+time,p.seed^0x13579BDFu)*
                     smoothNoise1D(float(y)*0.091f+200.0f+time,p.seed^0x2468ACE0u)*
                     smoothNoise1D(float(y)*0.917f+421.0f+time,p.seed^0xB5297A4Du);
    const float mask=hash01(std::uint32_t(x),std::uint32_t(y),p.seed^0x68E31DA4u,noiseFrame);
    if(line*(mask*mask*mask+0.3f)>=1.0f-tape) q.y+=0.22f*tape;
   }
   if(phaseNoise>0.0f&&noiseMask>0.0f) { const float qi=q.i*phaseCos-q.q*phaseSin,qq=q.i*phaseSin+q.q*phaseCos;q.i=qi;q.q=qq; }
   if(hueNoise>0.0f&&noiseMask>0.0f) { const float a=(hash01(std::uint32_t(nx),std::uint32_t(ny),p.seed^0xA3C59AC3u,noiseFrame)-0.5f)*hueNoise*0.50f*noiseMask; const float cs=std::cos(a),sn=std::sin(a); const float ni=q.i*cs-q.q*sn,nq=q.i*sn+q.q*cs;q.i=ni;q.q=nq; }
   rgb=yiqToRgb(q);
   // VHS6RIFT's original procedural grain remains an independent layer.
   if(grain>0.0f||ly>0.0f||ch>0.0f) {
    const float shadowWeight=(1.0f-std::clamp(q.y,0.0f,1.0f))*shadowBias+(1.0f-shadowBias);
    const int gs=std::max(1,p.grainSize);
    const float particle=vhs6riftHash2D(x/gs,(y+frameGrain)/gs,p.seed+777u)-0.5f;
    const float grainAdd=particle*grain*0.4f*shadowWeight;
    const float detailScale=0.5f+detail*1.5f;
    const int lx=int(std::floor(float(nx)*detailScale)),lyi=int(std::floor(float(ny)*detailScale));
    const float lNoise=(vhs6riftHash2D(lx,lyi,p.seed+std::uint32_t(frameLuma))-0.5f)*ly*0.35f*shadowWeight*noiseMask;
    rgb.r+=grainAdd+lNoise; rgb.g+=grainAdd+lNoise; rgb.b+=grainAdd+lNoise;
    if(ch>0.001f&&noiseMask>0.0f) {
     const float nr=(vhs6riftHash2D(nx,ny,p.seed+111u+std::uint32_t(noiseFrame))-0.5f)*ch*0.35f*noiseMask;
     const float nb=(vhs6riftHash2D(nx,ny,p.seed+222u+std::uint32_t(noiseFrame))-0.5f)*ch*0.35f*noiseMask;
     const YIQ chromaNoise=rgbToYiq({nr,0.0f,nb});
     const float ni=chromaNoise.i*di-chromaNoise.q*dq,nq=chromaNoise.i*dq+chromaNoise.q*di;
     const RGB rotated=yiqToRgb({0.0f,ni,nq});
     rgb.r+=rotated.r;rgb.g+=rotated.g;rgb.b+=rotated.b;
    }
   }
   if(p.premultipliedAlpha) { rgb.r*=a;rgb.g*=a;rgb.b*=a; }
   out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
  }
 }
 return true;
}

} // namespace vhs90
