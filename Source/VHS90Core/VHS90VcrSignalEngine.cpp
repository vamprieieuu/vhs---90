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
} // namespace vhs90
