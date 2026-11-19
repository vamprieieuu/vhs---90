#include "VHS90HeadConditionEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
constexpr float kAngle=5.97f*kPi/180.0f;
constexpr double kAdvancePerField=33.35/(60000.0/1001.0);
constexpr float kTrackLength=5.8f*1000.0f/(60000.0f/1001.0f);
inline std::uint32_t mix(std::uint32_t x) noexcept { x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;return x; }
inline float rnd(std::uint32_t seed,std::int64_t frame,std::uint32_t lane) noexcept {
 const auto f=std::uint32_t(frame)^std::uint32_t(std::uint64_t(frame)>>32);
 return float(mix(seed^(f*0x9e3779b9u)^(lane*0x85ebca6bu))>>8)*(1.0f/16777216.0f);
}
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline RGB read(ConstImageView s,int x,int y,bool premul) noexcept {
 x=std::clamp(x,0,s.width-1);y=std::clamp(y,0,s.height-1);const float*p=s.rgba+y*s.rowStrideFloats+x*4;RGB c{p[0],p[1],p[2]};
 if(premul&&p[3]>1e-6f){c.r/=p[3];c.g/=p[3];c.b/=p[3];}return c;
}
struct HeadReadState { float debris=0.0f, grease=0.0f; };
inline HeadReadState simulateHeadState(const HeadConditionParams& p,int head,float dirt,float buildup,float greaseTarget) noexcept {
 HeadReadState state{};
 const float pickup=0.02f+1.2f*buildup*buildup;
 const float proxyShed=std::clamp(0.025f+0.11f*dirt+0.035f*buildup,0.0f,0.35f);
 const float dt=1.0f/59.94005994f;
 const std::int64_t last=std::max<std::int64_t>(0,std::min<std::int64_t>(p.frame*2+1,1000000));
 TapeSurfaceParams tape=p.tapeSurface;tape.seed=p.seed;
 for(std::int64_t field=0;field<=last;++field) {
  if(int(field&1)!=head) continue;
  float shed=proxyShed,film=greaseTarget;
  if(p.usePhysicalTapeSamples) {
   shed=0.0f;film=0.0f;
   for(int i=0;i<12;++i) {
    const float along=(float(i)+0.5f)/12.0f*kTrackLength;
    const double x=double(field)*kAdvancePerField/double(tape.tapeSpeed+1u)+double(along*std::cos(kAngle));
    const float y=1.0f+along*std::sin(kAngle);
    const auto sample=sampleTapeSurfaceAtMm(x,y,tape,false,true);
    shed+=sample.shed;film+=sample.film;
   }
   shed/=12.0f;film/=12.0f;
   // Head dirt and buildup remain deck-side contributions, not tape map values.
   shed+=0.11f*dirt+0.035f*buildup;
   film=std::max(film,greaseTarget);
  }
  state.debris+=pickup*shed*dt-state.debris*0.5f*dt;
  const float event=rnd(p.seed,field,81u+std::uint32_t(head)*17u);
  if(event<std::min(pickup*shed*0.03f,0.25f))
   state.debris+=0.15f+0.65f*rnd(p.seed,field,82u+std::uint32_t(head)*17u);
  if(rnd(p.seed,field,83u+std::uint32_t(head)*17u)<0.0005f+0.004f*state.debris*state.debris)
   state.debris*=0.25f*rnd(p.seed,field,84u+std::uint32_t(head)*17u);
  state.debris=std::clamp(state.debris,0.0f,6.0f);
  state.grease+=(film-state.grease)*(film>state.grease?0.2f:0.003f);
 }
 return state;
}
inline float gain(float mhz,float spacing,float gap) noexcept {
 const float writingSpeed=5.8f;
 const float loss=std::exp(-2.0f*kPi*spacing*mhz/writingSpeed);
 const float x=kPi*gap*mhz/writingSpeed;
 return loss*(std::fabs(x)<1e-6f?1.0f:std::fabs(std::sin(x)/x));
}
}
bool processHeadCondition(ConstImageView src,ImageView dst,const HeadConditionParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.wear)||!std::isfinite(p.dirt)||!std::isfinite(p.buildup)||!std::isfinite(p.mismatch)||!std::isfinite(p.grease)||!std::isfinite(p.amount))return false;
 const float amount=std::clamp(p.amount,0.0f,1.0f),wear=std::clamp(p.wear,0.0f,1.0f),dirt=std::clamp(p.dirt,0.0f,1.0f),buildup=std::clamp(p.buildup,0.0f,1.0f),mismatch=std::clamp(p.mismatch,0.0f,1.0f),grease=std::clamp(p.grease,0.0f,1.0f);
 if(amount<=0.0f||wear+dirt+buildup+mismatch+grease<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 struct HeadModel { float lumaGain,chromaGain,colorBoost; };
 HeadModel models[2]{};
 for(int head=0;head<2;++head) {
  const float headWear=std::clamp(wear*(head?1.0f+0.4f*mismatch:1.0f-0.4f*mismatch)+(head?0.25f*mismatch:0.0f),0.0f,1.0f);
  const HeadReadState state=simulateHeadState(p,head,dirt,buildup,grease);
  const float bakedDirt=dirt*0.5f*(head?0.9f:1.2f);
  const float spacing=0.3f*headWear+bakedDirt+state.debris+state.grease*0.7f;
  const float gap=0.3f+0.35f*headWear+0.18f*state.debris+0.2f*state.grease;
  const float efficiency=std::pow(10.0f,-(headWear*10.0f+(head?mismatch*3.0f:0.0f))/20.0f);
  models[head]={gain(4.0f,spacing,gap),gain(0.62937f,spacing,gap),
                1.0f/std::max(0.15f,efficiency*gain(0.62937f,spacing,gap))};
 }
 for(int y=0;y<src.height;++y){const int head=(int((p.frame%2+2)%2)+y)&1;const HeadModel model=models[head];const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];if(a<=1e-6f){std::copy_n(px,4,out);continue;}
   const RGB c=read(src,x,y,p.premultipliedAlpha);const YIQ q=rgbToYiq(c);
   const float yl=rgbToYiq(read(src,x-1,y,p.premultipliedAlpha)).y,yr=rgbToYiq(read(src,x+1,y,p.premultipliedAlpha)).y;
   const float low=(yl+2.0f*q.y+yr)*0.25f;
   YIQ damaged{low+(q.y-low)*model.lumaGain,q.i*model.chromaGain*model.colorBoost,q.q*model.chromaGain*model.colorBoost};
   RGB result=yiqToRgb({q.y+(damaged.y-q.y)*amount,q.i+(damaged.i-q.i)*amount,q.q+(damaged.q-q.q)*amount});
   if(p.premultipliedAlpha){result.r*=a;result.g*=a;result.b*=a;}
   out[0]=result.r;out[1]=result.g;out[2]=result.b;out[3]=a;
  }
 }
 return true;
}
} // namespace vhs90
