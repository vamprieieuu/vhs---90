#include "VHS90TapeTransportEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
constexpr float kTrackAngle=5.97f*kPi/180.0f;
constexpr double kAdvancePerField=33.35/(60000.0/1001.0);
constexpr double kControlOffsetMm=79.244;
constexpr float kControlYmm=0.35f;
inline std::uint32_t mix32(std::uint32_t x) noexcept { x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;return x; }
inline float randomSigned(std::uint32_t seed,std::int64_t frame,int x,int lane) noexcept {
 const std::uint32_t f=std::uint32_t(frame)^std::uint32_t(std::uint64_t(frame)>>32);
 const std::uint32_t h=mix32(seed^(f*0x9e3779b9u)^(std::uint32_t(x)*0xd1b54a35u)^(std::uint32_t(lane)*0x94d049bbu));
 return float(h)/4294967295.0f*2.0f-1.0f;
}
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline float gaussianHash(std::uint32_t seed,int epoch,int row,int lane) noexcept {
 const float u1=std::max(1.0e-7f,hash01(std::uint32_t(row),std::uint32_t(lane),seed,std::int64_t(epoch)));
 const float u2=hash01(std::uint32_t(row),std::uint32_t(lane+1),seed,std::int64_t(epoch));
 return std::sqrt(-2.0f*std::log(u1))*std::cos(2.0f*kPi*u2);
}
float controlTrackServoOffset(const TapeTransportParams& p) noexcept {
 const std::int64_t last=std::max<std::int64_t>(0,std::min<std::int64_t>(p.frame,500000));
 if(p.controlTrackServoAmount<=0.0f) return 0.0f;
 float delta=0.0f,speed=0.0f,kick=0.0f,integrator=0.0f,phase=0.0f,rate=0.0f;
 TapeSurfaceParams tape=p.controlTrackTape;
 for(std::int64_t f=0;f<=last;++f) {
  const float motor=0.004f*randomSigned(p.seed^69u,f,0,0)+0.006f*randomSigned(p.seed^60u,f,0,1);
  delta+=(motor+speed+kick);kick=0.0f;
  if(p.pictureSearchSpeed>1.02f){phase=std::fmod(phase+rate+2.0f,2.0f);rate=std::clamp(p.pictureSearchSpeed-1.0f,0.0f,8.0f);continue;}
  if(rate!=0.0f||phase!=0.0f){float wrapped=std::fmod(phase+rate+1.0f,2.0f)-1.0f;delta+=wrapped;phase=0.0f;rate=0.0f;}
  const float pulse=std::clamp(p.controlTrackPulseStrength,0.0f,1.5f);
  float physicalAmplitude=1.0f,physicalLift=0.0f;
  if(p.usePhysicalControlTrackMap){
   const double x=double(f*2)*kAdvancePerField/double(tape.tapeSpeed+1u)+kControlOffsetMm;
   const auto sample=sampleTapeSurfaceAtMm(x,kControlYmm,tape,false,true);
   const float remanence=std::pow(10.0f,-std::clamp(tape.oxideLoss,0.0f,1.85f)*10.0f/20.0f);
   physicalAmplitude=sample.magnetization/std::max(remanence,1.0e-5f);
   physicalLift=sample.spacingLift+sample.film;
   physicalAmplitude*=std::exp(-2.0f*kPi*physicalLift/12.0f);
  }
  const float variation=0.85f+0.3f*(randomSigned(p.seed^66u,f,0,0)*0.5f+0.5f);
  const bool detected=pulse*physicalAmplitude*(1.0f-std::clamp(p.controlTrackPulseDropout,0.0f,1.0f))*variation>0.5f;
  const float falseChance=std::max(std::clamp(p.controlTrackFalsePulseChance,0.0f,1.0f),p.usePhysicalControlTrackMap?0.5f*std::clamp((physicalLift-0.8f)/(2.5f-0.8f),0.0f,1.0f):0.0f);
  const float falseRoll=randomSigned(p.seed^67u,f,0,0)*0.5f+0.5f;
  float error=0.0f;bool measured=detected;
  if(falseRoll<falseChance){measured=true;error=randomSigned(p.seed^68u,f,0,0)*0.5f;}
  else if(detected)error=std::clamp(delta-p.trackingTargetPixels,-1.0f,1.0f);
  if(measured){speed=-integrator;kick=-0.377f*error*std::clamp(p.controlTrackServoAmount,0.0f,1.0f);integrator=std::clamp(integrator+0.0986f*error*std::clamp(p.controlTrackServoAmount,0.0f,1.0f),-0.15f,0.15f);}
 }
 return delta;
}
inline std::uint64_t sourceMix64(std::uint64_t z) noexcept {
 z+=0x9E3779B97F4A7C15ULL;z=(z^(z>>30))*0xBF58476D1CE4E5B9ULL;z=(z^(z>>27))*0x94D049BB133111EBULL;return z^(z>>31);
}
inline double sourceUniform(std::uint64_t seed,std::uint64_t purpose,std::int64_t a,std::int64_t b) noexcept {
 const auto h=sourceMix64(seed^sourceMix64(purpose^sourceMix64(std::uint64_t(a)^sourceMix64(std::uint64_t(b)))));
 return double(h>>11)*(1.0/9007199254740992.0);
}
inline double sourceValue(std::uint64_t seed,std::uint64_t purpose,double x) noexcept {
 const double i=std::floor(x),f=x-i;
 const double a=sourceUniform(seed,purpose,static_cast<std::int64_t>(i),0)*2.0-1.0;
 const double b=sourceUniform(seed,purpose,static_cast<std::int64_t>(i)+1,0)*2.0-1.0;
 const double s=f*f*(3.0-2.0*f);return a+(b-a)*s;
}
inline double wrapTrackDelta(double x) noexcept {
 double r=std::fmod(x,2.0);if(r>1.0)r-=2.0;else if(r<=-1.0)r+=2.0;return r;
}
std::vector<float> physicalStickySlipProfile(const TapeTransportParams& p,int width,int height) {
 std::vector<float> profile(static_cast<std::size_t>(height),0.0f);
 const double sticky=std::clamp(double(p.controlTrackTape.debrisAmount),0.0,1.0);
 const double mold=std::clamp(double(p.controlTrackTape.moldAmount),0.0,1.0);
 if(sticky<=0.0&&mold<=0.0&&p.backTension==0.0f)return profile;
 const std::int64_t frame=std::max<std::int64_t>(0,p.frame);
 const std::int64_t firstField=std::max<std::int64_t>(0,frame*2-8),lastField=frame*2+1;
 const int activePerField=std::max(1,(height+1)/2);
 const int vbiPerField=std::max(0,int(std::lround(double(activePerField)*20.0/242.5)));
 const double totalLines=262.5,lineUs=63.556;
 const double advance=33.35/(60000.0/1001.0)/double(std::min(p.controlTrackTape.tapeSpeed,2u)+1u);
 const double trackLength=5.8*1000.0/(60000.0/1001.0);
 const double lineLength=trackLength/totalLines;
 const double angle=5.97*3.14159265358979323846/180.0;
 const double trackOffsetLines=advance*std::cos(angle)/lineLength;
 const std::uint64_t seed=p.seed;
 double delta=0.0;
 double lag[3]={0.0,0.0,0.0};
 double breakAt[3]={8.0,6.0,5.0};
 for(std::int64_t field=firstField;field<=lastField;++field){
  for(int line=0;line<263;++line){
   const double t=(double(field)*totalLines+double(line))*lineUs*1.0e-6;
   const double x=double(field)*advance+double(line)/totalLines*advance;
   const double localMold=mold*std::max(sourceValue(seed,63,x/9.0),0.0);
   const double mu=sticky*(0.55+0.45*sourceValue(seed,62,x/25.0))+0.4*localMold;
   const double drag=sticky*sticky*0.05*(1.0+sourceValue(seed,61,t/0.8));
   delta-=drag/totalLines;
   const double perLine=33.35/(double(std::min(p.controlTrackTape.tapeSpeed,2u)+1u))*lineUs*1.0e-3;
   for(int guide=0;guide<3;++guide){
    const double grip=mu*double(guide==0?1.0:(guide==1?0.75:0.55));
    if(grip<0.1){lag[guide]*=0.8;continue;}
    lag[guide]+=double(guide==0?0.3:(guide==1?0.2:0.15))*perLine;
    if(lag[guide]>=breakAt[guide]){
     lag[guide]=-0.3*lag[guide];
     const double u=sourceUniform(seed,std::uint64_t(64+guide),field*263+line,0);
     breakAt[guide]=(1.5+7.0*grip)*(1.0+0.35*std::clamp(double(p.backTension),-1.0,1.0))*(0.4+1.4*u*u);
    }
   }
   const int parity=int(field-frame*2);
   const int localRow=line-vbiPerField;
   const int outY=parity*activePerField+localRow;
   if(parity<0||parity>1||outY<0||outY>=height)continue;
   const double slip=delta+(lag[0]+lag[1]+lag[2])/(advance*1000.0);
   const double along=double(line)/totalLines*trackLength;const double strainUs=8.0e-4*std::clamp(double(p.backTension),-1.0,1.0)*along/lineLength*lineUs;const double tbe=lineUs*0.45*std::tanh(trackOffsetLines*wrapTrackDelta(slip)/0.45)+strainUs;
   profile[static_cast<std::size_t>(outY)]=float(tbe/52.656*double(width));
  }
 }
 return profile;
}
std::vector<float> slowProfile(std::uint32_t seed,int epoch,int height,float amplitude,float gain,float pole,float vScale) {
 const float scale=std::max(1.0e-6f,vScale),scaledPole=std::pow(std::min(std::abs(pole),0.999999f),1.0f/scale);
 const float scaledGain=gain*std::sqrt(std::max(1.0e-12f,(1.0f-scaledPole*scaledPole)/(1.0f-pole*pole)));
 std::vector<float> out(static_cast<std::size_t>(height),0.0f);float previous=0.0f;
 for(int y=0;y<height;++y){const float raw=gaussianHash(seed,epoch,y,17)*amplitude;previous=scaledGain*raw+scaledPole*previous;out[std::size_t(y)]=previous;}return out;
}
inline bool inactive(const TapeTransportParams&p) noexcept {
 return p.slowDriftPixelsPerSecond==0.0f&&p.drumFlutterPixels==0.0f&&p.flutterPixels==0.0f&&p.stickSlipPixels==0.0f&&p.lineJitterPixels==0.0f&&p.trackingOffsetPixels==0.0f&&p.trackingSlopePixels==0.0f&&p.servoJitterPixels==0.0f&&p.controlTrackServoAmount==0.0f&&p.controlTrackTape.debrisAmount<=0.0f&&p.controlTrackTape.moldAmount<=0.0f&&p.backTension==0.0f;
}
inline void sample(const std::vector<float>&src,int w,int h,float x,float y,float out[4]) noexcept {
 x=std::clamp(x,0.0f,float(w-1));y=std::clamp(y,0.0f,float(h-1));const int x0=int(x),y0=int(y),x1=std::min(x0+1,w-1),y1=std::min(y0+1,h-1);
 const float fx=x-x0,fy=y-y0;const size_t a=(size_t(y0)*w+x0)*4,b=(size_t(y0)*w+x1)*4,c=(size_t(y1)*w+x0)*4,d=(size_t(y1)*w+x1)*4;
 for(int k=0;k<4;++k){const float ab=src[a+k]+(src[b+k]-src[a+k])*fx,cd=src[c+k]+(src[d+k]-src[c+k])*fx;out[k]=ab+(cd-ab)*fy;}
}
}
bool processTapeTransportWarp(ConstImageView src,ImageView dst,const TapeTransportParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.framesPerSecond)||!std::isfinite(p.fieldRateHz)||!std::isfinite(p.slowDriftPixelsPerSecond)||!std::isfinite(p.drumFlutterPixels)||!std::isfinite(p.flutterHz)||!std::isfinite(p.flutterPixels)||!std::isfinite(p.stickSlipPixels)||!std::isfinite(p.lineJitterPixels)||!std::isfinite(p.trackingOffsetPixels)||!std::isfinite(p.trackingSlopePixels)||!std::isfinite(p.servoJitterPixels)||!std::isfinite(p.controlTrackServoAmount)||!std::isfinite(p.controlTrackPulseStrength)||!std::isfinite(p.controlTrackPulseDropout)||!std::isfinite(p.controlTrackFalsePulseChance)||!std::isfinite(p.pictureSearchSpeed)||!std::isfinite(p.trackingTargetPixels)||!std::isfinite(p.backTension)||!std::isfinite(p.verticalScale)||p.verticalScale<=0)return false;
 if(inactive(p)){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 try{
  const int w=src.width,h=src.height;std::vector<float>copy(size_t(w)*h*4);for(int y=0;y<h;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,w*4,copy.data()+size_t(y)*w*4);
  const float fps=std::max(p.framesPerSecond,1.0f),time=float(p.frame)/fps,controlServo=controlTrackServoOffset(p);
  const int epochFrames=4,epoch=int(p.frame/epochFrames);const float epochMix=float(p.frame%epochFrames)/float(epochFrames);
  std::vector<float>servo,servoNext,stickySlip;if(p.controlTrackTape.debrisAmount>0.0f||p.controlTrackTape.moldAmount>0.0f||p.backTension!=0.0f)stickySlip=physicalStickySlipProfile(p,w,h);if(p.servoJitterPixels>0){servo=slowProfile(p.seed^0x51A7u,epoch,h,p.servoJitterPixels*.55f,.055f,.945f,p.verticalScale);servoNext=slowProfile(p.seed^0x51A7u,epoch+1,h,p.servoJitterPixels*.55f,.055f,.945f,p.verticalScale);}
  for(int y=0;y<h;++y){const float line=float(y)/float(std::max(1,h-1)),drum=std::sin(2*kPi*(time*p.fieldRateHz+line*.035f))*p.drumFlutterPixels,flutter=std::sin(2*kPi*(time*p.flutterHz+line*.17f))*p.flutterPixels,stick=randomSigned(p.seed,p.frame,y/3,1)*p.stickSlipPixels,jitter=randomSigned(p.seed,p.frame,y,2)*p.lineJitterPixels,tracking=p.trackingOffsetPixels+p.trackingSlopePixels*(line-.5f),drift=time*p.slowDriftPixelsPerSecond,servoShift=servo.empty()?0.0f:servo[std::size_t(y)]*(1-epochMix)+servoNext[std::size_t(y)]*epochMix,materialSlip=stickySlip.empty()?0.0f:stickySlip[std::size_t(y)],shift=drum+flutter+stick+jitter+tracking+drift+servoShift+controlServo+materialSlip;
   float*row=dst.rgba+y*dst.rowStrideFloats;for(int x=0;x<w;++x){float q[4];sample(copy,w,h,float(x)-shift,float(y),q);row[x*4]=q[0];row[x*4+1]=q[1];row[x*4+2]=q[2];row[x*4+3]=q[3];}}
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
