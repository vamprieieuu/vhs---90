#include "VHS90TapeSurfaceEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
constexpr float kTapeWidthMm=12.65f;
constexpr float kTrackStartMm=1.0f;
constexpr float kTrackAngle=5.97f*kPi/180.0f;
constexpr float kTrackLengthMm=5.8f*1000.0f/(60000.0f/1001.0f);
constexpr float kLineLengthMm=kTrackLengthMm/262.5f;
constexpr float kLinePeriodUs=63.556f;
constexpr float kActiveStartUs=9.4f;
constexpr float kActiveLengthUs=52.656f;
constexpr double kAdvancePerFieldMm=33.35/(60000.0/1001.0);
inline std::uint64_t mix64(std::uint64_t x) noexcept { x^=x>>30;x*=0xbf58476d1ce4e5b9ULL;x^=x>>27;x*=0x94d049bb133111ebULL;x^=x>>31;return x; }
inline std::uint64_t hashCell(std::uint32_t seed,std::int64_t x,std::int64_t y,std::uint32_t lane=0) noexcept {
 return mix64(std::uint64_t(seed)^(std::uint64_t(x)*0x9e3779b97f4a7c15ULL)^(std::uint64_t(y)*0xd1b54a32d192ed03ULL)^(std::uint64_t(lane)*0x94d049bb133111ebULL));
}
inline float unit(std::uint64_t h) noexcept { return float((h>>40)&0xffffffu)*(1.0f/16777216.0f); }
inline std::uint32_t ntscHash(std::uint32_t a,std::uint32_t b,std::uint32_t c,std::uint32_t d) noexcept {
 std::uint32_t h=a*0x8DA6B343u^b*0xD8163841u^c*0xCB1AB31Fu^d*0x165667B1u;
 h^=h>>15;h*=0x2C1B3C6Du;h^=h>>12;h*=0x297A2D39u;h^=h>>15;return h;
}
inline float uni(std::uint32_t a,std::uint32_t b,std::int32_t c,std::int32_t d) noexcept {
 return (float(ntscHash(a,b,std::uint32_t(c),std::uint32_t(d))>>8)+0.5f)*(1.0f/16777216.0f);
}
inline float smooth(float x) noexcept { x=std::clamp(x,0.0f,1.0f);return x*x*(3.0f-2.0f*x); }
inline float smoothstep(float e0,float e1,float x) noexcept { const float t=std::clamp((x-e0)/(e1-e0),0.0f,1.0f);return t*t*(3.0f-2.0f*t); }
inline float falloffEllipse(float dx,float dy,float rx,float ry) noexcept {
 if(rx<=0.0f||ry<=0.0f)return 0.0f;const float q=dx*dx/(rx*rx)+dy*dy/(ry*ry);return q>=1.0f?0.0f:1.0f-smooth(q);
}
struct PhysicalPosition { std::int32_t base; float offset,y; };
inline PhysicalPosition makePosition(double x,float y) noexcept {
 const double tenths=x*10.0,whole=std::floor(tenths);
 return {static_cast<std::int32_t>(std::clamp(whole,double(INT32_MIN),double(INT32_MAX))),float(tenths-whole),y};
}
inline std::pair<std::int32_t,float> cell(PhysicalPosition p,std::int32_t q) noexcept {
 const float whole=std::floor(p.offset);const std::int32_t t=p.base+static_cast<std::int32_t>(whole);
 std::int32_t c=t/q;if(t%q!=0&&t<0)--c;
 return {c,(float(t-c*q)+(p.offset-whole))/float(q)};
}
inline float dx(PhysicalPosition p,std::int32_t c,std::int32_t q,float frac) noexcept {
 return float(c*q-p.base)+frac*float(q)-p.offset;
}
inline float noise1(std::uint32_t s,std::uint32_t k,std::int32_t i,float f) noexcept {
 const float a=uni(s,k,i,0),b=uni(s,k,i+1,0);return a+(b-a)*smooth(f);
}
inline float noise2(std::uint32_t s,std::uint32_t k,std::int32_t i,float fi,std::int32_t j,float fj) noexcept {
 const float a=uni(s,k,i,j),b=uni(s,k,i+1,j),c=uni(s,k,i,j+1),d=uni(s,k,i+1,j+1);
 const float u=smooth(fi),v=smooth(fj),lo=a+(b-a)*u,hi=c+(d-c)*u;return lo+(hi-lo)*v;
}
inline RGB straightAt(ConstImageView s,int x,int y,bool premul) noexcept {
 x=std::clamp(x,0,s.width-1);y=std::clamp(y,0,s.height-1);const float*p=s.rgba+y*s.rowStrideFloats+x*4;RGB c{p[0],p[1],p[2]};
 if(premul&&p[3]>1.0e-6f){c.r/=p[3];c.g/=p[3];c.b/=p[3];}return c;
}
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline bool inactive(const TapeSurfaceParams&p) noexcept { return p.defectDensity<=0.0f&&p.edgeLift<=0.0f&&p.surfaceRoughness<=0.0f&&p.oxideLoss<=0.0f&&p.scratchDensity<=0.0f&&p.creaseStrength<=0.0f&&p.moldAmount<=0.0f&&p.vaselineAmount<=0.0f&&p.debrisAmount<=0.0f&&p.pinholeAmount<=0.0f&&p.flakeAmount<=0.0f; }
}
double tapeWidthMm() noexcept { return double(kTapeWidthMm); }
double tapeTrackStartMm() noexcept { return double(kTrackStartMm); }
double tapeTrackLengthMm() noexcept { return double(kTrackLengthMm); }
double tapeTrackPitchUm(std::uint32_t speed) noexcept { return kAdvancePerFieldMm/double(speed+1u)*std::sin(kTrackAngle)*1000.0; }
double tapeTrackOffsetLines(std::uint32_t speed) noexcept {
 const double lineLength=kTrackLengthMm/262.5;
 return kAdvancePerFieldMm/double(speed+1u)*std::cos(kTrackAngle)/lineLength;
}
TapePointMm tapeTrackPoint(std::int64_t field,double along,double across,std::uint32_t speed) noexcept {
 const double origin=double(field)*kAdvancePerFieldMm/double(speed+1u);
 return {origin+along*std::cos(kTrackAngle)-across*std::sin(kTrackAngle),
         double(kTrackStartMm)+along*std::sin(kTrackAngle)+across*std::cos(kTrackAngle)};
}
namespace {
const float* tapeCurve(TapeDamageControl c) noexcept {
 static constexpr float wear[11]={0,.259f,.648f,.955f,1.202f,1.391f,1.540f,1.640f,1.724f,1.785f,1.850f};
 static constexpr float drop[11]={0,.195f,.473f,.754f,.931f,1.120f,1.322f,1.607f,2.124f,2.480f,3.0f};
 static constexpr float scratch[11]={0,.759f,1.281f,1.822f,2.073f,2.289f,2.516f,2.777f,3.069f,3.430f,4.0f};
 static constexpr float edge[11]={0,.703f,.743f,.800f,.888f,.934f,.987f,1.056f,1.190f,1.329f,1.45f};
 switch(c){case TapeDamageControl::Wear:return wear;case TapeDamageControl::Dropouts:return drop;case TapeDamageControl::Scratches:return scratch;case TapeDamageControl::EdgeDamage:return edge;default:return nullptr;}
}
float topEndAmount(float k,float gain) noexcept { k=std::clamp(k,0.0f,1.0f);return k<=.5f?k:k+gain*(k-.5f)/.5f; }
float topEndKnob(float a,float gain) noexcept { a=std::max(0.0f,a);return a<=.5f?a:(a+gain)/(1.0f+2.0f*gain); }
float oldAmount(TapeDamageControl c,float k) noexcept {
 float gain=0.0f;switch(c){case TapeDamageControl::Wear:gain=.85f;break;case TapeDamageControl::Dropouts:gain=2.0f;break;case TapeDamageControl::Scratches:gain=3.0f;break;case TapeDamageControl::EdgeDamage:gain=.45f;break;default:break;}
 return topEndAmount(k,gain);
}
}
float ntscrtTapeAmount(TapeDamageControl c,float k) noexcept {
 if(const float*curve=tapeCurve(c)){const float x=std::clamp(k,0.0f,1.0f)*10.0f;const int i=std::min(9,int(x));return curve[i]+(curve[i+1]-curve[i])*(x-float(i));}
 return topEndAmount(k,4.0f);
}
float ntscrtTapeKnobForAmount(TapeDamageControl c,float amount) noexcept {
 if(const float*curve=tapeCurve(c)){
  const float a=std::clamp(amount,0.0f,curve[10]);
  for(int i=0;i<10;++i)if(a<=curve[i+1]){const float span=curve[i+1]-curve[i];return (float(i)+(span>0?(a-curve[i])/span:0.0f))/10.0f;}
  return 1.0f;
 }
 return topEndKnob(amount,4.0f);
}
float ntscrtTapeAmountIn027(TapeDamageControl c,float k) noexcept { return oldAmount(c,k); }
float ntscrtTransportFlutterAmount(float k) noexcept { return topEndAmount(k,4.0f); }
float ntscrtTapeWearAmount(float k) noexcept { return ntscrtTapeAmount(TapeDamageControl::Wear,k); }
float ntscrtTapeDropoutAmount(float k) noexcept { return ntscrtTapeAmount(TapeDamageControl::Dropouts,k); }
float ntscrtTapeScratchAmount(float k) noexcept { return ntscrtTapeAmount(TapeDamageControl::Scratches,k); }
float ntscrtTapeEdgeAmount(float k) noexcept { return ntscrtTapeAmount(TapeDamageControl::EdgeDamage,k); }
float ntscrtTapeCreaseAmount(float k) noexcept { return ntscrtTapeAmount(TapeDamageControl::Creases,k); }
std::uint32_t ntscrtTapeSeed(std::uint32_t seed) noexcept { return seed*7919u+17u; }
TapeSurfaceParams makeTapeSurfaceParams(const TapeMaterialControls& c) noexcept {
 TapeSurfaceParams p{};
 const float master=std::clamp(c.master/100.0f,0.0f,1.0f);
 const auto knob=[&](float v) noexcept { return master*std::clamp(v/100.0f,0.0f,1.0f); };
 p.seed=ntscrtTapeSeed(std::max(1u,c.cassetteSeed));
 p.frame=c.frame;
 p.tapeSpeed=std::min(c.tapeSpeed,2u);
 p.trackingOffsetTracks=std::clamp(c.trackingOffsetTracks,-8.0f,8.0f);
 p.oxideLoss=ntscrtTapeAmount(TapeDamageControl::Wear,knob(c.wear));
 p.defectDensity=ntscrtTapeAmount(TapeDamageControl::Dropouts,knob(c.dropouts));
 p.scratchDensity=ntscrtTapeAmount(TapeDamageControl::Scratches,knob(c.scratches));
 p.creaseStrength=ntscrtTapeAmount(TapeDamageControl::Creases,knob(c.creases));
 p.edgeLift=ntscrtTapeAmount(TapeDamageControl::EdgeDamage,knob(c.edgeDamage));
 p.moldAmount=knob(c.mold);
 p.vaselineAmount=knob(c.vaseline);
 p.debrisAmount=knob(c.sticky);
 p.pinholeAmount=knob(c.pinholes);
 p.flakeAmount=knob(c.flakes);
 p.surfaceRoughness=0.25f*knob(c.surfaceRoughness);
 return p;
}
TapeSurfaceSample sampleTapeSurfaceAtMm(double xMm,float y,const TapeSurfaceParams&p,bool detail,bool flakes) noexcept {
 const PhysicalPosition pos=makePosition(xMm,y);const std::uint32_t s=p.seed;
 const float wear=std::clamp(p.oxideLoss,0.0f,1.85f),drop=std::clamp(p.defectDensity,0.0f,3.0f);
 const float scratches=std::clamp(p.scratchDensity,0.0f,4.0f),creases=std::clamp(p.creaseStrength,0.0f,5.0f);
 const float edge=std::clamp(p.edgeLift,0.0f,1.45f),mold=std::clamp(p.moldAmount,0.0f,1.0f);
 const float vas=std::clamp(p.vaselineAmount,0.0f,1.0f),sticky=std::clamp(p.debrisAmount,0.0f,1.0f);
 TapeSurfaceSample o;o.magnetization=std::pow(10.0f,-wear*10.0f/20.0f);o.roughness=0.0f;o.spacingLift=0.015f+0.1f*wear+std::clamp(p.surfaceRoughness,0.0f,0.25f);o.shed=0.02f*wear+0.9f*sticky*sticky;
 const auto [c5,f5]=cell(pos,5);const float yy=y*10.0f;const auto cy=static_cast<std::int32_t>(std::floor(yy));
 o.magnetization*=1.0f+(0.04f+0.1f*wear)*(2.0f*noise2(s,1,c5,f5,cy,yy-float(cy))-1.0f);
 const float pin=std::clamp(std::max(p.pinholeAmount,(0.05f+6.0f*drop+2.0f*wear)*0.01f),0.0f,1.0f);
 if(detail&&pin>0){const auto [cx,fx]=cell(pos,1);(void)fx;const auto py=static_cast<std::int32_t>(std::floor(y*10.0f));
  for(int j=-1;j<=1;++j)for(int i=-1;i<=1;++i){const auto ci=cx+i,cj=py+j;if(uni(s,11,ci,cj)>=pin)continue;
   const float ddx=dx(pos,ci,1,uni(s,12,ci,cj))*0.1f,ddy=(float(cj)+uni(s,13,ci,cj))*0.1f-y;
   const float r=0.003f*std::pow(15.0f,uni(s,14,ci,cj)*uni(s,14,ci,cj)),d=std::sqrt(ddx*ddx+ddy*ddy);
   if(d<r)o.magnetization*=smoothstep(r*0.6f,r,d);
  }
 }
 const float flake=std::clamp(std::max(p.flakeAmount,0.001f+0.6f*drop*drop+0.06f*wear+0.06f*mold),0.0f,1.0f);
 if(flakes&&flake>0){const auto [cx,fx]=cell(pos,10);(void)fx;const auto py=static_cast<std::int32_t>(std::floor(y));
  for(int j=-1;j<=1;++j)for(int i=-1;i<=1;++i){const auto ci=cx+i,cj=py+j;if(uni(s,21,ci,cj)>=flake)continue;
   const float ddx=dx(pos,ci,10,uni(s,22,ci,cj))*0.1f,ddy=float(cj)+uni(s,23,ci,cj)-y,u=uni(s,24,ci,cj);
   const float r=0.04f*std::pow(8.0f,u*u)*(1.0f+0.6f*std::max(0.0f,drop-1.0f));
   const float e=0.6f+0.8f*uni(s,25,ci,cj),d=std::sqrt((ddx/e)*(ddx/e)+(ddy*e)*(ddy*e));
   if(d<r*1.5f){o.magnetization*=smoothstep(r*0.8f,r,d);o.shed+=0.5f*(1.0f-smoothstep(r,r*1.5f,d));o.spacingLift+=0.15f*(1.0f-smoothstep(r,r*1.5f,d));}
  }
 }
 if(detail&&scratches>0){const auto lane=static_cast<std::int32_t>(std::floor(y*4.0f));const auto [sx,fx]=cell(pos,3000);const auto [c80,f80]=cell(pos,80);const auto [c15,f15]=cell(pos,15);const auto [c200,f200]=cell(pos,200);const auto [c120,f120]=cell(pos,120);
  for(int l=-1;l<=1;++l){const auto li=lane+l;if(uni(s,31,li,sx)>=scratches*0.3f)continue;const float a=uni(s,32,li,sx),b=uni(s,33,li,sx);if(fx<std::min(a,b)*0.6f||fx>0.4f+std::max(a,b)*0.6f)continue;
   const auto ss=s^std::uint32_t(li)*0x9E3779B9u^std::uint32_t(sx)*0x85EBCA6Bu;
   const float weave=0.03f*(2*noise1(ss,37,c80,f80)-1)+0.012f*(2*noise1(ss,38,c15,f15)-1);
   const float yc=(float(li)+0.15f+0.7f*uni(s,34,li,sx))*0.25f+weave,u=uni(s,35,li,sx);
   const float w=(0.006f+0.074f*u*u*u)*(0.6f+0.4f*scratches)*(0.5f+noise1(ss,39,c200,f200));
   const float depth=smoothstep(0.3f,0.55f,noise1(ss,40,c120,f120)),dy=std::fabs(y-yc);
   if(dy<w*0.5f){o.magnetization*=1.0f-0.95f*depth;o.spacingLift+=0.15f*depth*scratches;}else if(dy<w*1.5f)o.spacingLift+=0.35f*depth*scratches*(1.0f-(dy-w*0.5f)/w);
  }
 }
 if(creases>0){const auto [sx,unused]=cell(pos,400);(void)unused;for(int k=-1;k<=1;++k){const auto ci=sx+k;if(uni(s,41,ci,0)>=creases*0.9f)continue;
  const float off=dx(pos,ci,400,uni(s,42,ci,0))*0.1f,slant=std::tan((uni(s,43,ci,0)-0.5f)*0.7f);
  const bool fromBottom=uni(s,44,ci,0)<0.5f;const float extent=kTapeWidthMm*(0.25f+0.75f*uni(s,45,ci,0)),yy2=fromBottom?y:kTapeWidthMm-y;
  if(yy2>=extent)continue;const float reach=smoothstep(extent,extent*0.8f,yy2),w=0.3f+2.7f*std::pow(uni(s,46,ci,0),1.5f);
  const float peak=(0.5f+3.5f*std::pow(uni(s,47,ci,0),1.5f))*(0.4f+0.6f*creases),dd=(off+yy2*slant)/w;
  if(std::fabs(dd)>=4)continue;const float profile=std::exp(-dd*dd)*(0.75f+0.25f*std::cos(dd*9.0f));o.spacingLift+=peak*profile*reach;o.magnetization*=1.0f-0.3f*profile*reach;
 }}
 if(edge>0){for(std::uint32_t side=0;side<2;++side){const std::uint32_t k=51+side*4;const float yy2=side==0?y:kTapeWidthMm-y;const auto [c60,f60]=cell(pos,600);
  const float stretch=noise1(s,k,c60,f60),damaged=smoothstep(edge*0.85f,edge*0.85f-0.1f,stretch);if(damaged<=0)continue;
  const auto [c20,f20]=cell(pos,200);const float reach=0.3f+1.9f*edge*(0.5f+0.5f*noise1(s,k+1,c20,f20));if(yy2>=reach)continue;
  const auto [c1,f1]=cell(pos,12);const float crinkle=noise1(s,k+2,c1,f1);o.spacingLift+=damaged*edge*(1.0f+2.5f*crinkle*crinkle)*std::pow(1.0f-yy2/reach,0.7f);
 }}
 if(mold>0){const auto [cx,fx]=cell(pos,40);(void)fx;const auto cy=static_cast<std::int32_t>(std::floor(y/4.0f));
  for(int j=-1;j<=1;++j)for(int i=-1;i<=1;++i){const auto ci=cx+i,cj=cy+j;const float ycen=(float(cj)+uni(s,63,ci,cj))*4.0f;if(ycen<=-1||ycen>=kTapeWidthMm+1)continue;
   const float edgeNear=1.0f-std::min(std::min(std::fabs(ycen),std::fabs(kTapeWidthMm-ycen))/6.3f,1.0f);if(uni(s,61,ci,cj)>=std::min(mold*(0.3f+1.3f*edgeNear),0.95f))continue;
   const float ddx=dx(pos,ci,40,uni(s,62,ci,cj))*0.1f,ddy=ycen-y,d=std::sqrt(ddx*ddx+ddy*ddy),big=(0.35f+2.0f*std::pow(uni(s,64,ci,cj),1.3f))*(0.6f+0.4f*mold);if(d>=big*1.4f)continue;
   const float turn=(std::atan2(ddy,ddx)/(2*kPi)+0.5f)*6.0f;const int ti=int(std::floor(turn));const float a0=uni(s,65,ci*7+ti%6,cj),a1=uni(s,65,ci*7+(ti+1)%6,cj);
   const float rr=big*(0.7f+0.6f*(a0+(a1-a0)*smooth(turn-float(ti)))),dens=(1.0f-smoothstep(rr*0.35f,rr,d))*(0.55f+0.45f*noise2(s,66,cell(pos,3).first,cell(pos,3).second,static_cast<int>(std::floor(y*3.3f)),y*3.3f-std::floor(y*3.3f)));
   o.spacingLift+=dens*(0.5f+2.5f*mold);o.magnetization*=1.0f-0.55f*dens;o.shed+=dens*2.0f;
  }
 }
 if(vas>0){const auto [sx,fx]=cell(pos,600);if(unit(ntscHash(s,71,std::uint32_t(sx),0))<0.2f+0.8f*vas){
  const float a=0.4f*uni(s,72,sx,0),b=0.6f+0.4f*uni(s,73,sx,0),taper=smoothstep(a,a+0.05f,fx)*smoothstep(b,b-0.05f,fx);
  if(taper>0){const auto [c8,f8]=cell(pos,80);const float yy2=y*5.0f;const int cy=int(std::floor(yy2));const float streak=std::pow(noise2(s,74,c8,f8,cy,yy2-float(cy)),1.5f);o.film=vas*1.8f*(0.25f+0.75f*streak)*taper;}
 }}
 o.magnetization=std::max(0.0f,o.magnetization);o.spacingLift=std::clamp(o.spacingLift,0.0f,8.0f);o.roughness=std::clamp(o.roughness,0.0f,0.25f);o.film=std::clamp(o.film,0.0f,2.0f);return o;
}
TapeSurfaceSample sampleTapeSurface(float u,float v,const TapeSurfaceParams&p) noexcept {
 // Raster rows advance along the helical track; horizontal samples move
 // through the recorded NTSC line. This is TapeGeometry::along_track/point.
 const float totalLines=float(std::max(1,p.rasterActiveLines+p.rasterVbiLines));
 const float fieldLine=(float(p.rasterVbiLines)+v*float(std::max(1,p.rasterActiveLines)))/totalLines;
 const float along=fieldLine*kTrackLengthMm+((kActiveStartUs+u*kActiveLengthUs)/kLinePeriodUs)*kLineLengthMm;
 const double fieldAdvance=kAdvancePerFieldMm/double(p.tapeSpeed+1u);
 const double origin=double(p.frame*2)*fieldAdvance+double(along*std::cos(kTrackAngle));
 const float centerY=kTrackStartMm+along*std::sin(kTrackAngle);
 const float headWidthMm=p.tapeSpeed==0?0.058f:0.026f;
 const float trackPitchMm=float(kAdvancePerFieldMm/double(p.tapeSpeed+1u)*std::sin(kTrackAngle));
 TapeSurfaceSample out{};out.magnetization=0.0f;
 for(int k=0;k<4;++k){
  const float across=-p.trackingOffsetTracks*trackPitchMm+(float(k)-1.5f)*0.25f*headWidthMm;
  const double x=origin-double(across*std::sin(kTrackAngle));
  const float y=centerY+across*std::cos(kTrackAngle);
  const auto q=sampleTapeSurfaceAtMm(x,y,p,true,true);
  out.magnetization+=q.magnetization*0.25f;out.spacingLift+=q.spacingLift*0.25f;
  out.roughness+=q.roughness*0.25f;out.creaseLoss+=q.creaseLoss*0.25f;
  out.film+=q.film*0.25f;out.shed+=q.shed*0.25f;
 }
 return out;
}
bool processTapeSurface(ConstImageView src,ImageView dst,const TapeSurfaceParams&p) noexcept {
 if(!valid(src,dst))return false;
 if(inactive(p)){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 TapeSurfaceParams mapParams=p;mapParams.rasterActiveLines=src.height;mapParams.rasterVbiLines=std::max(12,int(std::lround(float(src.height)*20.0f/242.5f)));
 for(int y=0;y<src.height;++y){const float v=(float(y)+0.5f)/float(src.height);const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];if(a<=1e-6f){std::copy_n(px,4,out);continue;}
   const float u=(float(x)+0.5f)/float(src.width);const auto d=sampleTapeSurface(u,v,mapParams);
   RGB rgb{px[0],px[1],px[2]};if(p.premultipliedAlpha){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
   YIQ q=rgbToYiq(rgb);const float mag=d.magnetization*(1.0f-std::clamp(d.creaseLoss,0.0f,1.0f));
   const float spacingUm=std::clamp(d.spacingLift+d.film,0.0f,8.0f),headGapUm=1.0f,writingSpeed=5.8f;
   const auto channelGain=[&](float mhz) noexcept {const float spacing=std::exp(-2.0f*kPi*spacingUm*mhz/writingSpeed);const float aperture=kPi*headGapUm*mhz/writingSpeed;const float gap=std::fabs(aperture)<1e-5f?1.0f:std::fabs(std::sin(aperture)/aperture);return spacing*gap;};
   const float lumaGain=channelGain(4.0f),chromaGain=channelGain(0.62937f);
   const float stickyBase=0.02f*std::clamp(p.oxideLoss,0.0f,1.85f)+0.9f*std::pow(std::clamp(p.debrisAmount,0.0f,1.0f),2.0f);
   const float localShed=std::clamp(d.shed-stickyBase,0.0f,2.0f);
   const float shedLoss=std::clamp(d.shed*0.08f,0.0f,0.24f);
   const float leftY=rgbToYiq(straightAt(src,std::max(0,x-1),y,p.premultipliedAlpha)).y,rightY=rgbToYiq(straightAt(src,std::min(src.width-1,x+1),y,p.premultipliedAlpha)).y;
   const float lowY=(leftY+2.0f*q.y+rightY)*0.25f;q.y=(lowY+(q.y-lowY)*lumaGain)*mag*(1.0f-shedLoss)*(1.0f-0.65f*localShed);
   const float ca=mag*chromaGain*(1.0f-shedLoss)*(1.0f-0.85f*localShed);q.i*=ca;q.q*=ca;
   const auto shedCell=hashCell(p.seed,static_cast<std::int64_t>(std::floor(u*160.0f)),static_cast<std::int64_t>(std::floor(v*80.0f)),0x53484544u);
   q.y+=(unit(hashCell(p.seed,static_cast<std::int64_t>(std::floor(u*4096.0f)),static_cast<std::int64_t>(std::floor(v*1024.0f)),static_cast<std::uint32_t>(p.frame)))*2.0f-1.0f)*d.roughness;
   q.y+=(unit(shedCell)*2.0f-1.0f)*std::clamp(d.shed,0.0f,1.5f)*0.006f;
   rgb=yiqToRgb(q);if(p.premultipliedAlpha){rgb.r*=a;rgb.g*=a;rgb.b*=a;}out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;out[3]=a;
  }
 }return true;
}
} // namespace vhs90
