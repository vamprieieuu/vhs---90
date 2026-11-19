#include "VHS90HeadConditionEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
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
 const int head=int((p.frame%2+2)%2);
 const float headWear=std::clamp(wear*(head?1.0f+0.4f*mismatch:1.0f-0.4f*mismatch)+(head?0.25f*mismatch:0.0f),0.0f,1.0f);
 const float event=rnd(p.seed,p.frame,81u);
 const float debris=std::clamp(dirt*0.5f*(head?0.9f:1.2f)+0.02f+1.2f*buildup*buildup+((event<0.015f+0.08f*buildup)?0.15f+0.65f*rnd(p.seed,p.frame,82u):0.0f),0.0f,2.0f);
 const float film=grease*(0.25f+0.75f*rnd(p.seed,p.frame/4,83u));
 const float spacing=0.3f*headWear+debris+film*0.7f;
 const float gap=0.3f+0.35f*headWear+0.18f*debris+0.2f*film;
 const float lumaGain=gain(4.0f,spacing,gap),chromaGain=gain(0.62937f,spacing,gap);
 const float efficiency=std::pow(10.0f,-(headWear*10.0f+(head?mismatch*3.0f:0.0f))/20.0f);
 const float colorBoost=1.0f/std::max(0.15f,efficiency*chromaGain);
 for(int y=0;y<src.height;++y){const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];if(a<=1e-6f){std::copy_n(px,4,out);continue;}
   const RGB c=read(src,x,y,p.premultipliedAlpha);const YIQ q=rgbToYiq(c);
   const float yl=rgbToYiq(read(src,x-1,y,p.premultipliedAlpha)).y,yr=rgbToYiq(read(src,x+1,y,p.premultipliedAlpha)).y;
   const float low=(yl+2.0f*q.y+yr)*0.25f;
   YIQ damaged{low+(q.y-low)*lumaGain,q.i*chromaGain*colorBoost,q.q*chromaGain*colorBoost};
   RGB result=yiqToRgb({q.y+(damaged.y-q.y)*amount,q.i+(damaged.i-q.i)*amount,q.q+(damaged.q-q.q)*amount});
   if(p.premultipliedAlpha){result.r*=a;result.g*=a;result.b*=a;}
   out[0]=result.r;out[1]=result.g;out[2]=result.b;out[3]=a;
  }
 }
 return true;
}
} // namespace vhs90
