#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <vector>
namespace vhs90 {
// NTSCRT VCR sample-domain constants: 8x NTSC color subcarrier.
constexpr double kVcrSampleRateMHz = 8.0 * (315.0 / 88.0);
constexpr std::uint32_t kVcrSamplesPerLine = 1820;
constexpr std::uint32_t kVcrSegmentSamples = 16;
constexpr std::uint32_t kVcrSpacingLevels = 64;
constexpr std::uint32_t kVcrSpacingReach = 40;
constexpr std::uint32_t kVcrFmReach = 47;
constexpr float kVcrNoise = 0.022f;
constexpr float kVcrMaxSpacingUm = 4.0f;
struct VcrComplexTap { float real=0.0f, imag=0.0f; };
double vcrCarrierToNoiseDb() noexcept;
bool applyVcrSpacingBank(const float* input,const float* spacingUm,std::size_t count,float gapUm,float* output);
bool applyVcrFmBandpass(const float* input,std::size_t count,float* outputReal,float* outputImag);
double vcrChannelGain(double mhz,double spacingUm,double gapUm) noexcept;
std::vector<float> buildVcrSpacingBank(float gapUm);
std::vector<VcrComplexTap> buildVcrFmBandTaps();
std::array<float,5> buildVcrLowPass(double mhz) noexcept;
std::array<float,3> buildVcrShelf(double zeroMHz,double poleMHz) noexcept;
std::array<float,5> buildVcrInputTrap() noexcept;
int vcrInputChromaDelaySamples() noexcept;
} // namespace vhs90
