#include "vhs90_tape_physics.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
inline float sinc(float x) noexcept { if(std::abs(x)<1e-5f)return 1.0f;return std::sin(x)/x; }
inline uint64_t mix64(uint64_t x) noexcept { x^=x>>30;x*=0xbf58476d1ce4e5b9ULL;x^=x>>27;x*=0x94d049bb133111ebULL;x^=x>>31;return x; }
inline float randomSigned(uint64_t seed,int64_t frame,int x,int lane) noexcept {
    const uint64_t h=mix64(seed^(uint64_t(frame)*0x9e3779b97f4a7c15ULL)^(uint64_t(uint32_t(x))*0xd1b54a32d192ed03ULL)^(uint64_t(uint32_t(lane))*0x94d049bb133111ebULL));
    return float((h>>40)&0xffffffu)*(1.0f/16777216.0f)*2.0f-1.0f;
}
inline void sampleRgba(const std::vector<float>&src,int w,int h,float x,float y,float out[4]) noexcept {
    x=std::clamp(x,0.0f,float(w-1));y=std::clamp(y,0.0f,float(h-1));
    const int x0=int(x),y0=int(y),x1=std::min(x0+1,w-1),y1=std::min(y0+1,h-1);
    const float fx=x-x0,fy=y-y0;
    const size_t i00=(size_t(y0)*w+x0)*4,i10=(size_t(y0)*w+x1)*4,i01=(size_t(y1)*w+x0)*4,i11=(size_t(y1)*w+x1)*4;
    for(int c=0;c<4;++c){const float a=src[i00+c]+(src[i10+c]-src[i00+c])*fx,b=src[i01+c]+(src[i11+c]-src[i01+c])*fx;out[c]=a+(b-a)*fy;}
}
}
float headReadbackGain(float frequencyHz,const HeadReadbackParams&p) noexcept {
    const float speed=std::max(p.tapeSpeedMetersPerSecond,0.01f);
    const float gap=p.headGapMicrons*1e-6f,spacing=p.headToTapeSpacingMicrons*1e-6f;
    const float wavelength=speed/std::max(std::abs(frequencyHz),1.0f);
    const float gapTerm=sinc(kPi*gap/std::max(wavelength,1e-12f));
    const float spacingTerm=std::exp(-2.0f*kPi*spacing/std::max(wavelength,1e-12f));
    const float azimuth=std::sin(p.azimuthMismatchDegrees*kPi/180.0f);
    const float azimuthTerm=sinc(kPi*p.trackWidthMicrons*1e-6f*azimuth/std::max(wavelength,1e-12f));
    const float condition=std::clamp(1.0f-p.headWear,0.0f,1.0f)*std::clamp(1.0f-p.headDirt,0.0f,1.0f);
    return std::clamp(std::abs(gapTerm*spacingTerm*azimuthTerm)*condition,0.0f,1.0f);
}
void renderLorentzianHeadReadback(const MagneticTransition*t,std::size_t count,float sampleRate,float sampleStart,int sampleCount,const HeadReadbackParams&p,float*out) noexcept {
    if(!out||sampleCount<=0||sampleRate<=0)return;
    std::fill(out,out+sampleCount,0.0f);
    const float speed=std::max(p.tapeSpeedMetersPerSecond,0.01f);
    const float halfWidthSeconds=(p.headGapMicrons*0.5e-6f+2.0f*p.headToTapeSpacingMicrons*1e-6f)/speed;
    const float halfWidth=std::max(0.35f,halfWidthSeconds*sampleRate);
    const float gain=std::clamp(1.0f-p.headWear,0.0f,1.0f)*std::clamp(1.0f-p.headDirt,0.0f,1.0f);
    const float norm=halfWidth/kPi;
    for(std::size_t i=0;i<count;++i){const auto&tr=t[i];const float center=tr.positionSamples-sampleStart;const int lo=std::max(0,int(std::floor(center-halfWidth*12.0f)));const int hi=std::min(sampleCount-1,int(std::ceil(center+halfWidth*12.0f)));for(int s=lo;s<=hi;++s){const float dx=float(s)-center;const float pulse=norm/(dx*dx+halfWidth*halfWidth);out[s]+=tr.polarity*tr.strength*gain*pulse;}}
}
void applyTapeTransportWarp(FloatRgbaView im,const TapeTransportParams&p){
    if(!im.pixels||im.width<=0||im.height<=0||im.rowStrideFloats<im.width*4)return;
    const int w=im.width,h=im.height;std::vector<float>src(size_t(w)*h*4);
    for(int y=0;y<h;++y)std::copy(im.pixels+size_t(y)*im.rowStrideFloats,im.pixels+size_t(y)*im.rowStrideFloats+w*4,src.data()+size_t(y)*w*4);
    const float fps=std::max(p.framesPerSecond,1.0f),time=float(p.frameIndex)/fps;
    for(int y=0;y<h;++y){
        const float line=float(y)/std::max(1,h-1);
        const float drum=std::sin(2.0f*kPi*(time*p.fieldRateHz+line*0.035f))*p.drumFlutterPixels;
        const float flutter=std::sin(2.0f*kPi*(time*p.flutterHz+line*0.17f))*p.flutterPixels;
        const float stick=randomSigned(p.seed,p.frameIndex,y/3,1)*p.stickSlipPixels;
        const float jitter=randomSigned(p.seed,p.frameIndex,y,2)*p.lineJitterPixels;
        const float tracking=p.trackingOffsetPixels+p.trackingSlopePixels*(line-0.5f);
        const float drift=time*p.slowDriftPixelsPerSecond;
        const float shift=drum+flutter+stick+jitter+tracking+drift;
        float*dst=im.pixels+size_t(y)*im.rowStrideFloats;
        for(int x=0;x<w;++x){float q[4];sampleRgba(src,w,h,float(x)-shift,float(y),q);dst[x*4]=q[0];dst[x*4+1]=q[1];dst[x*4+2]=q[2];}
    }
}
} // namespace vhs90
