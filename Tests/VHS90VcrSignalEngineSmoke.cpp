#include "VHS90VcrSignalEngine.h"
#include <cassert>
#include <cmath>
#include <cstddef>
int main(){using namespace vhs90;
 assert(std::fabs(kVcrSampleRateMHz-28.6363636364)<1e-8);
 assert(kVcrSamplesPerLine==1820&&kVcrSegmentSamples==16&&kVcrSpacingLevels==64&&kVcrSpacingReach==40&&kVcrFmReach==47);
 assert(vcrCarrierToNoiseDb()>30.0&&vcrCarrierToNoiseDb()<40.0);
 assert(vcrChannelGain(4.0,0.0,0.3)>0.0&&vcrChannelGain(4.0,2.0,0.3)<vcrChannelGain(4.0,0.0,0.3));
 const auto bank=buildVcrSpacingBank(0.3f);assert(bank.size()==64u*81u);
 for(std::size_t i=0;i<64;++i)for(std::size_t k=0;k<81;++k){assert(std::isfinite(bank[i*81+k]));assert(std::fabs(bank[i*81+k]-bank[i*81+(80-k)])<1e-6f);}
 const auto fm=buildVcrFmBandTaps();assert(fm.size()==95);assert(std::fabs(fm[47].imag)<1e-7f);
 for(std::size_t k=0;k<47;++k){assert(std::fabs(fm[k].real-fm[94-k].real)<1e-6f);assert(std::fabs(fm[k].imag+fm[94-k].imag)<1e-6f);}
 const auto lp=buildVcrLowPass(3.0);const auto em=buildVcrShelf(1.0/(2.0*3.14159265358979323846*1.3),5.0/(2.0*3.14159265358979323846*1.3));const auto trap=buildVcrInputTrap();
 for(float x:lp)assert(std::isfinite(x));for(float x:em)assert(std::isfinite(x));for(float x:trap)assert(std::isfinite(x));
 assert(vcrInputChromaDelaySamples()>0);
 constexpr std::size_t n=256,mid=n/2;float input[n]{},spacing0[n]{},spacing4[n],out0[n]{},out4[n]{},fmR[n]{},fmI[n]{};
 input[mid]=1.0f;for(std::size_t i=0;i<n;++i)spacing4[i]=4.0f;
 assert(applyVcrSpacingBank(input,spacing0,n,.3f,out0));assert(applyVcrSpacingBank(input,spacing4,n,.3f,out4));
 assert(out0[mid]>out4[mid]&&out4[mid]>0.0f);
 assert(applyVcrFmBandpass(input,n,fmR,fmI));assert(fmR[mid]>0.0f);
 assert(std::fabs(fmR[mid-1]-fmR[mid+1])<1e-6f&&std::fabs(fmI[mid-1]+fmI[mid+1])<1e-6f);
 return 0;}
