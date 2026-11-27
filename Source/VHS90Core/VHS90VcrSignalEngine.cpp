#include "VHS90VcrSignalEngine.h"
#include <algorithm>
#include <cmath>
namespace vhs90 {
namespace {
constexpr double kPi=3.1415926535897932384626433832795;
constexpr double kTau=2.0*kPi;
constexpr double kWritingSpeedUmPerUs=5.8;
constexpr double kFmLowMHz=1.3,kFmHighMHz=6.6;
constexpr double kInputChromaMHz=1.0;
double bessel0(double x) noexcept {
 double sum=1.0,term=1.0;
 for(int k=1;k<30;++k){const double q=x/(2.0*double(k));term*=q*q;sum+=term;}
 return sum;
}
}
double vcrCarrierToNoiseDb() noexcept {
 const double band=(kFmHighMHz-kFmLowMHz)/(kVcrSampleRateMHz/2.0);
 return 10.0*std::log10(0.5/(double(kVcrNoise)*double(kVcrNoise)*band));
}
double vcrChannelGain(double mhz,double spacingUm,double gapUm) noexcept {
 const double loss=std::exp(-2.0*kPi/kWritingSpeedUmPerUs*spacingUm*mhz);
 const double x=kPi*gapUm*mhz/kWritingSpeedUmPerUs;
 return loss*(std::fabs(x)<1e-6?1.0:std::fabs(std::sin(x)/x));
}
std::vector<float> buildVcrSpacingBank(float gapUm) {
 const std::size_t taps=2u*kVcrSpacingReach+1u;
 std::vector<float> out(std::size_t(kVcrSpacingLevels)*taps,0.0f);
 const double half=std::max(double(gapUm)/kWritingSpeedUmPerUs*kVcrSampleRateMHz/2.0,0.5);
 for(std::uint32_t i=0;i<kVcrSpacingLevels;++i){
  const double t=double(i)/double(kVcrSpacingLevels-1u);
  const double d=double(kVcrMaxSpacingUm)*t*t;
  const double c=std::max(d/kWritingSpeedUmPerUs*kVcrSampleRateMHz,1e-4);
  for(int k=-int(kVcrSpacingReach);k<=int(kVcrSpacingReach);++k){
   const double kd=double(k);
   const double w=(std::atan((kd+half)/c)-std::atan((kd-half)/c))/(2.0*kPi*half);
   out[std::size_t(i)*taps+std::size_t(k+int(kVcrSpacingReach))]=float(w);
  }
 }
 return out;
}
std::vector<VcrComplexTap> buildVcrFmBandTaps() {
 std::vector<VcrComplexTap> out;out.reserve(2u*kVcrFmReach+1u);
 constexpr double beta=6.0;
 const double denom=bessel0(beta);
 const double w1=kTau*kFmLowMHz/kVcrSampleRateMHz,w2=kTau*kFmHighMHz/kVcrSampleRateMHz;
 for(int k=-int(kVcrFmReach);k<=int(kVcrFmReach);++k){
  const double kd=double(k);
  double re=0.0,im=0.0;
  if(k==0)re=(w2-w1)/(2.0*kPi);
  else {re=(std::sin(w2*kd)-std::sin(w1*kd))/(2.0*kPi*kd);im=(std::cos(w1*kd)-std::cos(w2*kd))/(2.0*kPi*kd);}
  const double r=kd/double(kVcrFmReach);
  const double win=bessel0(beta*std::sqrt(std::max(0.0,1.0-r*r)))/denom;
  out.push_back({float(2.0*re*win),float(2.0*im*win)});
 }
 return out;
}
bool applyVcrSpacingBank(const float*input,const float*spacing,std::size_t count,float gapUm,float*output) {
 if(!input||!spacing||!output||count==0||!std::isfinite(gapUm)||gapUm<0.0f)return false;
 const auto bank=buildVcrSpacingBank(gapUm);constexpr int reach=int(kVcrSpacingReach),taps=2*reach+1;
 for(std::size_t i=0;i<count;++i){
  const float level=std::clamp(spacing[i]/kVcrMaxSpacingUm,0.0f,1.0f)*float(kVcrSpacingLevels-1u);
  const int lo=int(level),hi=std::min(lo+1,int(kVcrSpacingLevels-1u));const float f=level-float(lo);
  double a=0.0,b=0.0;
  for(int k=-reach;k<=reach;++k){const auto j=std::size_t(std::clamp<std::ptrdiff_t>(std::ptrdiff_t(i)+k,0,std::ptrdiff_t(count-1)));
   a+=double(input[j])*bank[std::size_t(lo)*taps+std::size_t(k+reach)];
   b+=double(input[j])*bank[std::size_t(hi)*taps+std::size_t(k+reach)];
  }
  output[i]=float(a+(b-a)*f);
 }
 return true;
}
bool applyVcrFmBandpass(const float*input,std::size_t count,float*real,float*imag) {
 if(!input||!real||!imag||count==0)return false;
 const auto taps=buildVcrFmBandTaps();constexpr int reach=int(kVcrFmReach);
 for(std::size_t i=0;i<count;++i){double re=0.0,im=0.0;
  for(int k=-reach;k<=reach;++k){const auto j=std::size_t(std::clamp<std::ptrdiff_t>(std::ptrdiff_t(i)+k,0,std::ptrdiff_t(count-1)));
   const auto&t=taps[std::size_t(k+reach)];re+=double(input[j])*t.real;im+=double(input[j])*t.imag;
  }
  real[i]=float(re);imag[i]=float(im);
 }
 return true;
}
std::array<float,5> buildVcrLowPass(double mhz) noexcept {
 const double k=std::tan(kPi*mhz/kVcrSampleRateMHz),s2=std::sqrt(2.0);
 const double norm=1.0/(1.0+s2*k+k*k),b0=k*k*norm;
 return {float(b0),float(2.0*b0),float(b0),float(2.0*(k*k-1.0)*norm),float((1.0-s2*k+k*k)*norm)};
}
std::array<float,3> buildVcrShelf(double zeroMHz,double poleMHz) noexcept {
 const double a=2.0*kVcrSampleRateMHz/(kTau*zeroMHz),b=2.0*kVcrSampleRateMHz/(kTau*poleMHz);
 return {float((1.0+a)/(1.0+b)),float((1.0-a)/(1.0+b)),float((1.0-b)/(1.0+b))};
}
std::array<float,5> buildVcrInputTrap() noexcept {
 const double w0=kPi/4.0,q=1.0,alpha=std::sin(w0)/(2.0*q),a0=1.0+alpha;
 return {float(1.0/a0),float(-2.0*std::cos(w0)/a0),float(1.0/a0),float(-2.0*std::cos(w0)/a0),float((1.0-alpha)/a0)};
}
int vcrInputChromaDelaySamples() noexcept {
 return int(std::lround(2.0*std::sqrt(2.0)/(kTau*kInputChromaMHz)*kVcrSampleRateMHz));
}

namespace {
constexpr double kVcrTau=6.283185307179586476925286766559;
constexpr double kEmphasisZero=1.0/(kVcrTau*1.3);
constexpr double kEmphasisPole=5.0/(kVcrTau*1.3);
std::uint32_t vcrMix(std::uint32_t x) noexcept {
    x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;return x;
}
double vcrUniform(std::uint32_t seed,std::int64_t frame,int line,std::size_t sample,std::uint32_t lane) noexcept {
    const std::uint32_t f=static_cast<std::uint32_t>(frame)^static_cast<std::uint32_t>(static_cast<std::uint64_t>(frame)>>32);
    std::uint32_t h=seed^vcrMix(f+0x9e3779b9u)^vcrMix(static_cast<std::uint32_t>(line)*0x85ebca6bu);
    h^=vcrMix(static_cast<std::uint32_t>(sample)*0xc2b2ae35u+lane*0x27d4eb2du);
    return (double(vcrMix(h)&0x00ffffffu)+0.5)/16777216.0;
}
double vcrGaussian(std::uint32_t seed,std::int64_t frame,int line,std::size_t sample) noexcept {
    const double a=std::max(1.0e-12,vcrUniform(seed,frame,line,sample,0));
    const double b=vcrUniform(seed^0x51a7u,frame,line,sample,1);
    return std::sqrt(-2.0*std::log(a))*std::cos(kVcrTau*b);
}
void firstOrder(const float* in,std::size_t n,const std::array<float,3>& c,float* out) noexcept {
    double x1=n?in[0]:0.0,y1=x1;
    for(std::size_t i=0;i<n;++i){const double x=in[i];const double y=double(c[0])*x+double(c[1])*x1-double(c[2])*y1;out[i]=float(y);x1=x;y1=y;}
}
void biquadPass(const float* in,std::size_t n,const std::array<float,5>& c,float* out,bool reverse) noexcept {
    double x1=0.0,x2=0.0,y1=0.0,y2=0.0;
    if(n){const double edge=reverse?in[n-1]:in[0];x1=x2=y1=y2=edge;}
    for(std::size_t step=0;step<n;++step){
        const std::size_t i=reverse?n-1-step:step;
        const double x=in[i];
        const double y=double(c[0])*x+double(c[1])*x1+double(c[2])*x2-double(c[3])*y1-double(c[4])*y2;
        out[i]=float(y);x2=x1;x1=x;y2=y1;y1=y;
    }
}
void zeroPhaseBiquad(std::vector<float>& data,const std::array<float,5>& c,std::vector<float>& scratch) {
    biquadPass(data.data(),data.size(),c,scratch.data(),false);
    biquadPass(scratch.data(),scratch.size(),c,data.data(),true);
}
}
bool processVcrSignalLine(const float* y,const float* inI,const float* inQ,
                          const float* spacing,std::size_t count,const VcrSignalParams& raw,
                          float* outY,float* outI,float* outQ) noexcept {
    if(!y||!inI||!inQ||!spacing||!outY||!outI||!outQ||count<2)return false;
    VcrSignalParams p=raw;
    if(!std::isfinite(p.amount)||!std::isfinite(p.spacingUm)||!std::isfinite(p.gapUm)||
       !std::isfinite(p.noise)||!std::isfinite(p.dropoutThreshold)||p.gapUm<0.0f||
       p.spacingUm<0.0f||p.noise<0.0f||p.dropoutThreshold<0.0f||p.dropoutThreshold>1.0f||
       p.colorPhaseQuarterTurns<-3||p.colorPhaseQuarterTurns>3)return false;
    p.amount=std::clamp(p.amount,0.0f,1.0f);
    if(p.amount<=0.0f){std::copy_n(y,count,outY);std::copy_n(inI,count,outI);std::copy_n(inQ,count,outQ);return true;}
    try {
        std::vector<float> emphasis(count),rf(count),rfSpaced(count),fmR(count),fmI(count);
        std::vector<float> cu(count),cuSpaced(count),mixI(count),mixQ(count);
        std::vector<float> demod(count),low(count),scratch(count),spacingClamped(count);
        const auto em=buildVcrShelf(kEmphasisZero,kEmphasisPole);
        const auto de=buildVcrShelf(kEmphasisPole,kEmphasisZero);
        const auto lumaLp=buildVcrLowPass(3.0);
        const auto chromaLp=buildVcrLowPass(0.55);
        firstOrder(y,count,em,emphasis.data());
        double phase=kVcrTau*vcrUniform(p.seed,p.frame,p.lineIndex,0,9);
        const double dt=1.0/kVcrSampleRateMHz;
        const double cuStep=kVcrTau*(40.0/63.556)/kVcrSampleRateMHz;
        for(std::size_t i=0;i<count;++i){
            const double ire=std::clamp(double(emphasis[i]),-82.0,226.0);
            const double mhz=3.4+(ire+40.0)/140.0;
            phase+=kVcrTau*mhz*dt;
            rf[i]=float(std::cos(phase)+double(p.noise)*vcrGaussian(p.seed,p.frame,p.lineIndex,i));
            const double angle=cuStep*double(i)+double(p.colorPhaseQuarterTurns)*(kVcrTau/4.0);
            cu[i]=0.55f*(inI[i]*float(std::cos(angle))+inQ[i]*float(std::sin(angle)));
            spacingClamped[i]=std::clamp(spacing[i]+p.spacingUm,0.0f,kVcrMaxSpacingUm);
        }
        if(!applyVcrSpacingBank(rf.data(),spacingClamped.data(),count,p.gapUm,rfSpaced.data())||
           !applyVcrSpacingBank(cu.data(),spacingClamped.data(),count,p.gapUm,cuSpaced.data())||
           !applyVcrFmBandpass(rfSpaced.data(),count,fmR.data(),fmI.data()))return false;
        double envelope=0.0,peak=0.0;
        for(std::size_t i=0;i<std::min<std::size_t>(count,120);++i)
            peak=std::max(peak,std::hypot(double(fmR[i]),double(fmI[i])));
        peak=std::max(peak,1.0e-6);
        bool dropout=false;int hold=0;float lastGood=0.0f;
        double prevR=fmR[0],prevI=fmI[0];
        for(std::size_t i=0;i<count;++i){
            const double re=fmR[i],im=fmI[i];
            const double dphi=std::atan2(prevR*im-prevI*re,prevR*re+prevI*im);
            prevR=re;prevI=im;
            const double mhz=std::clamp(dphi/(kVcrTau*dt),1.0,7.5);
            double decoded=(mhz-3.4)*140.0-40.0;
            const double mag=std::hypot(re,im);
            envelope+=0.12*(mag-envelope);
            const double on=std::max(0.9*double(p.noise),double(p.dropoutThreshold)*peak);
            const double off=std::max(1.36*double(p.noise),1.22*double(p.dropoutThreshold)*peak);
            if(dropout){if(envelope>off){if(--hold<=0)dropout=false;}else hold=14;}
            else if(envelope<on){dropout=true;hold=14;}
            if(dropout&&p.dropoutCompensation)decoded=lastGood;
            else lastGood=float(decoded);
            demod[i]=float(decoded);
            const double angle=cuStep*double(i)+double(p.colorPhaseQuarterTurns)*(kVcrTau/4.0);
            mixI[i]=2.0f*cuSpaced[i]*float(std::cos(angle))/0.55f;
            mixQ[i]=2.0f*cuSpaced[i]*float(std::sin(angle))/0.55f;
        }
        biquadPass(demod.data(),count,lumaLp,low.data(),false);
        biquadPass(low.data(),count,lumaLp,scratch.data(),true);
        firstOrder(scratch.data(),count,de,low.data());
        for(int round=0;round<2;++round){
            zeroPhaseBiquad(mixI,chromaLp,scratch);
            zeroPhaseBiquad(mixQ,chromaLp,scratch);
        }
        for(std::size_t i=0;i<count;++i){
            outY[i]=y[i]+(low[i]-y[i])*p.amount;
            outI[i]=inI[i]+(mixI[i]-inI[i])*p.amount;
            outQ[i]=inQ[i]+(mixQ[i]-inQ[i])*p.amount;
        }
        return true;
    } catch(...) { return false; }
}

} // namespace vhs90
