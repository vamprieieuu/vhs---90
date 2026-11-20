#include "VHS90AnalogGeometryEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline float sampleChannel(ConstImageView s,float x,float y,int ch) noexcept {
 x=std::clamp(x,0.0f,float(s.width-1));y=std::clamp(y,0.0f,float(s.height-1));
 const int x0=int(x),y0=int(y),x1=std::min(x0+1,s.width-1),y1=std::min(y0+1,s.height-1);
 const float fx=x-x0,fy=y-y0;
 const float a=s.rgba[y0*s.rowStrideFloats+x0*4+ch],b=s.rgba[y0*s.rowStrideFloats+x1*4+ch];
 const float c=s.rgba[y1*s.rowStrideFloats+x0*4+ch],d=s.rgba[y1*s.rowStrideFloats+x1*4+ch];
 const float ab=a+(b-a)*fx,cd=c+(d-c)*fx;return ab+(cd-ab)*fy;
}
inline float clamp01(float x) noexcept {return std::clamp(x,0.0f,1.0f);}
inline float fade5(float t) noexcept {return t*t*t*(t*(t*6.0f-15.0f)+10.0f);}
inline float gradient2D(float x,float y,std::uint32_t seed) noexcept {
 const int ix=int(std::floor(x)),iy=int(std::floor(y));const float fx=x-float(ix),fy=y-float(iy);
 const float u=fade5(fx),v=fade5(fy);
 auto dot=[&](int dx,int dy) noexcept {
  const std::uint32_t h=hash32(std::uint32_t(ix+dx)*0x9e3779b9u^std::uint32_t(iy+dy)*0x85ebca6bu^seed);
  constexpr float g[8][2]={{1,0},{-1,0},{0,1},{0,-1},{.70710678f,.70710678f},{-.70710678f,.70710678f},{.70710678f,-.70710678f},{-.70710678f,-.70710678f}};
  const auto& v=g[h&7u];return v[0]*(fx-float(dx))+v[1]*(fy-float(dy));
 };
 const float a=dot(0,0)+(dot(1,0)-dot(0,0))*u;
 const float b=dot(0,1)+(dot(1,1)-dot(0,1))*u;
 return (a+(b-a)*v)*1.41421356f;
}
inline float fbm2D(float x,float y,std::uint32_t seed,int detail) noexcept {
 float sum=0.0f,weight=1.0f,total=0.0f,frequency=1.0f;
 const int octaves=std::clamp(detail,1,5);
 for(int octave=0;octave<octaves;++octave){
  sum+=gradient2D(x*frequency,y*frequency,seed+std::uint32_t(octave)*0x9e3779b9u)*weight;
  total+=weight;frequency*=2.0f;weight*=0.70710678f;
 }
 return total>0.0f?sum/total:0.0f;
}
}
bool processAnalogGeometry(ConstImageView src,ImageView dst,const AnalogGeometryParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.lineJitterPixels)||!std::isfinite(p.lineJitterCorrelationRows)||
    !std::isfinite(p.topSkewPixels)||!std::isfinite(p.topSkewDecayRows)||
    !std::isfinite(p.edgePincushionPixels)||!std::isfinite(p.edgePincushionPower)||!std::isfinite(p.interlaceAmount)||!std::isfinite(p.verticalRoll)||!std::isfinite(p.edgeWaveAmplitudePixels)||!std::isfinite(p.edgeWaveFrequency)||!std::isfinite(p.edgeWaveSpeed)||!std::isfinite(p.horizontalScale)||!std::isfinite(p.verticalScale)||!std::isfinite(p.lensDistortion)||!std::isfinite(p.digitalZoom)||!std::isfinite(p.roundedOverscan)||p.horizontalScale<=0.0f||p.verticalScale<=0.0f||p.digitalZoom<1.0f||p.edgeWaveFrequency<0.0f)return false;
 if(p.lineJitterPixels==0.0f&&p.topSkewPixels==0.0f&&p.edgePincushionPixels==0.0f&&p.interlaceAmount==0.0f&&p.verticalRoll==0.0f&&p.edgeWaveAmplitudePixels==0.0f&&!p.vhs43Frame&&p.lensDistortion==0.0f&&p.digitalZoom==1.0f) {
  for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;
 }
 const float rollOffset=std::sin(float(p.frame)*0.23f+float(p.seed)*0.00017f)*clamp01(p.verticalRoll)*float(src.height)*0.045f;
 const float aspect=float(src.width)/float(src.height);
 const float playbackWidth=p.vhs43Frame?std::min(1.0f,(4.0f/3.0f)/std::max(0.001f,aspect)):1.0f;
 const float playbackLeft=0.5f-playbackWidth*0.5f;
 const float playbackRight=0.5f+playbackWidth*0.5f;
 const float zoom=std::max(1.0f,p.digitalZoom);
 std::vector<float> edgeWave;
 if(p.edgeWaveAmplitudePixels>0.0f) {
  edgeWave.resize(std::size_t(src.height));
  const float offset=float(hash01(0u,0u,p.seed^0x6A09E667u,0))*float(src.height);
  const float time=float(p.frame)*p.edgeWaveSpeed;
  for(int row=0;row<src.height;++row) {
   const float n=fbm2D((float(row)+offset)*p.edgeWaveFrequency,time,p.seed^0xBB67AE85u,p.edgeWaveDetail);
   edgeWave[std::size_t(row)]=n*p.edgeWaveAmplitudePixels;
  }
 }
 for(int y=0;y<src.height;++y) {
  const float yn=src.height>1?(2.0f*float(y)/float(src.height-1)-1.0f):0.0f;
  const float decay=std::exp(-float(y)/std::max(1.0f,p.topSkewDecayRows));
  const float topNoise=hash01(7u,0u,p.seed,p.frame)*2.0f-1.0f;
  const float topShift=topNoise*p.topSkewPixels*decay;
  const float jitter=valueNoise1D(float(y)/std::max(1.0f,p.lineJitterCorrelationRows),p.seed+0x9e3779b9u,p.frame)*2.0f-1.0f;
  const float rowShift=topShift+jitter*p.lineJitterPixels+(edgeWave.empty()?0.0f:edgeWave[std::size_t(y)]);
  const float interlace=clamp01(p.interlaceAmount);
  const float fieldDir=(y&1)?-1.0f:1.0f;
  const float fieldPhase=std::sin(float(p.frame)*0.73f)*interlace*std::max(1.0f,float(src.width)*0.004f);
  const float fieldShift=fieldPhase*fieldDir;
  const float fieldYShift=0.15f*interlace*fieldDir;
  float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x) {
   const float xn=src.width>1?(2.0f*float(x)/float(src.width-1)-1.0f):0.0f;
   const float bend=p.edgePincushionPixels*xn*std::pow(std::abs(yn),std::max(0.1f,p.edgePincushionPower));
   const float u=src.width>1?float(x)/float(src.width-1):0.5f;
   const float v=src.height>1?float(y)/float(src.height-1):0.5f;
   if(p.vhs43Frame&&(u<playbackLeft||u>playbackRight)){
    const float alpha=src.rgba[y*src.rowStrideFloats+x*4+3];
    dr[x*4]=dr[x*4+1]=dr[x*4+2]=0.0f;dr[x*4+3]=alpha;continue;
   }
   const float frameU=p.vhs43Frame?(u-playbackLeft)/std::max(1.0e-6f,playbackWidth):u;
   float sourceU=p.vhs43Frame?0.5f+(frameU-0.5f)*playbackWidth:u;
   sourceU=0.5f+(sourceU-0.5f)/zoom;
   if(p.lensDistortion!=0.0f){
    float lx=(sourceU-0.5f)*2.0f*aspect,ly=(v-0.5f)*2.0f;
    const float r2=lx*lx+ly*ly;const float scale=1.0f+p.lensDistortion*r2;
    lx*=scale;ly*=scale;sourceU=0.5f+(lx/std::max(0.001f,aspect))*0.5f;
   }
   const float sx=sourceU*float(src.width-1)-rowShift-bend-fieldShift;
   float sy=std::fmod(v*float(src.height-1)+rollOffset+fieldYShift,float(src.height));if(sy<0.0f)sy+=float(src.height);
   for(int c=0;c<4;++c)dr[x*4+c]=sampleChannel(src,sx,sy,c);
  }
 }
 return true;
}
bool processScanlines(ConstImageView src,ImageView dst,const ScanlineParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount)||!std::isfinite(p.frequency)||
    !std::isfinite(p.phase)||!std::isfinite(p.speed)||p.frequency<0.0f)return false;
 const float amount=clamp01(p.amount);
 if(amount<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 const float framePhase=float(p.frame)*p.speed;
 for(int y=0;y<src.height;++y) {
  const float phase=(float(y)+p.phase)*p.frequency+framePhase;
  const float gain=1.0f-amount*(0.5f+0.5f*std::sin(6.28318530718f*phase));
  const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x) {
   const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];
   if(a<=1.0e-6f){std::copy_n(px,4,out);continue;}
   RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
   YIQ yiq=rgbToYiq(rgb);yiq.y*=gain;rgb=yiqToRgb(yiq);
   if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}
   out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
  }
 }
 return true;
}
} // namespace vhs90
