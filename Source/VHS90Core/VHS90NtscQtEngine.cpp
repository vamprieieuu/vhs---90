#include "VHS90NtscQtEngine.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <vector>

namespace vhs90 {
namespace {
using C = std::complex<double>;
constexpr double kPi = 3.1415926535897932384626433832795;
std::uint32_t mix32(std::uint32_t x) noexcept {
    x ^= x >> 16; x *= 0x7feb352du; x ^= x >> 15;
    x *= 0x846ca68bu; x ^= x >> 16; return x;
}
double uniform(std::uint32_t seed, std::int64_t frame, int row, int bin, int lane) noexcept {
    std::uint32_t f=static_cast<std::uint32_t>(frame)^static_cast<std::uint32_t>(static_cast<std::uint64_t>(frame)>>32);
    std::uint32_t h=seed ^ mix32(f+0x9e3779b9u) ^ mix32(static_cast<std::uint32_t>(row)*0x85ebca6bu);
    h ^= mix32(static_cast<std::uint32_t>(bin)*0xc2b2ae35u+static_cast<std::uint32_t>(lane)*0x27d4eb2du);
    return (double(mix32(h)&0x00ffffffu)+0.5)/16777216.0;
}
bool isPowerOfTwo(std::size_t n) noexcept { return n && (n&(n-1))==0; }
std::size_t nextPowerOfTwo(std::size_t n) {
    std::size_t p=1;
    while(p<n) { if(p>std::numeric_limits<std::size_t>::max()/2) throw std::bad_alloc(); p*=2; }
    return p;
}
void fftPower2(std::vector<C>& a,bool inverse) {
    const std::size_t n=a.size();
    for(std::size_t i=1,j=0;i<n;++i) {
        std::size_t bit=n>>1;
        for(;j&bit;bit>>=1) j^=bit;
        j^=bit;
        if(i<j) std::swap(a[i],a[j]);
    }
    for(std::size_t len=2;len<=n;len<<=1) {
        const double angle=(inverse?2.0:-2.0)*kPi/double(len);
        const C root(std::cos(angle),std::sin(angle));
        for(std::size_t i=0;i<n;i+=len) {
            C w(1.0,0.0);
            for(std::size_t j=0;j<len/2;++j) {
                const C u=a[i+j],v=a[i+j+len/2]*w;
                a[i+j]=u+v;a[i+j+len/2]=u-v;w*=root;
            }
        }
        if(len>n/2) break; // avoids size_t overflow on the final left shift
    }
    if(inverse) for(C& v:a) v/=double(n);
}
void fft(std::vector<C>& a,bool inverse) {
    if(inverse) {
        for(C& v:a) v=std::conj(v);
        fft(a,false);
        for(C& v:a) v=std::conj(v)/double(a.size());
        return;
    }
    const std::size_t n=a.size();
    if(isPowerOfTwo(n)) { fftPower2(a,false); return; }
    const std::size_t m=nextPowerOfTwo(2*n-1);
    std::vector<C> x(m,C{}),b(m,C{});
    for(std::size_t i=0;i<n;++i) {
        const double phase=kPi*double(i)*double(i)/double(n);
        const C chirp(std::cos(phase),-std::sin(phase));
        const C kernel(std::cos(phase),std::sin(phase));
        x[i]=a[i]*chirp;b[i]=kernel;
        if(i) b[m-i]=kernel;
    }
    fftPower2(x,false);fftPower2(b,false);
    for(std::size_t i=0;i<m;++i)x[i]*=b[i];
    fftPower2(x,true);
    a.resize(n);
    for(std::size_t i=0;i<n;++i) {
        const double phase=kPi*double(i)*double(i)/double(n);
        a[i]=x[i]*C(std::cos(phase),-std::sin(phase));
    }
}
void spectralMaskRow(float* row,int width,int fieldRows,int rowIndex,
                     const NtscQtParams& p) {
    std::vector<C> spectrum(static_cast<std::size_t>(width));
    for(int x=0;x<width;++x)spectrum[static_cast<std::size_t>(x)]=C(row[x],0.0);
    fft(spectrum,false);
    const int crow=fieldRows/2,ccol=width/2;
    const int maskH=std::min(crow,std::max(0,int(1.0f+p.alpha*float(crow))));
    const int start=int(float(ccol)*(1.0f-p.frequencyNoiseSize));
    const int stop=int(float(ccol)*(1.0f+p.frequencyNoiseSize));
    for(int k=0;k<width;++k) {
        const int centered=(k<=width/2)?k:k-width;
        const int shiftedIndex=centered+ccol;
        const bool keep=(shiftedIndex>=ccol-maskH && shiftedIndex<ccol+maskH);
        const bool noiseBand=p.frequencyNoiseSize>0.0f &&
            (shiftedIndex<start || shiftedIndex>=stop);
        const double noise=noiseBand?double(p.frequencyNoiseAmplitude)*0.5:0.0;
        const double nr=uniform(p.seed,p.frame,rowIndex,k,0);
        const double ni=uniform(p.seed,p.frame,rowIndex,k,1);
        const double mr=(keep?1.0:0.0)+nr*noise-noise*0.5;
        const double mi=(keep?1.0:0.0)+ni*noise-noise*0.5;
        spectrum[static_cast<std::size_t>(k)]=C(spectrum[static_cast<std::size_t>(k)].real()*mr,
                                                spectrum[static_cast<std::size_t>(k)].imag()*mi);
    }
    fft(spectrum,true);
    for(int x=0;x<width;++x)row[x]=static_cast<float>(spectrum[static_cast<std::size_t>(x)].real());
}
void preEmphasize(float* row,int width,float amount,float cutoffMHz) {
    if(amount<=0.0f)return;
    const double rate=315000000.0/88.0*4.0;
    const double alpha=(1.0/rate)/(1.0/(2.0*kPi*double(cutoffMHz))+1.0/rate);
    double previous=16.0;
    for(int x=0;x<width;++x) {
        const double sample=row[x];
        previous=alpha*sample+(1.0-alpha)*previous;
        row[x]=static_cast<float>(sample+(sample-previous)*double(amount));
    }
}
inline float clampFinite(float v,float lo,float hi,float fallback) noexcept {
    return std::isfinite(v)?std::clamp(v,lo,hi):fallback;
}
}

bool processNtscQtSignal(ConstImageView src,ImageView dst,const NtscQtParams& raw) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||
       dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||
       src.rgba==dst.rgba)return false;
    NtscQtParams p=raw;
    p.alpha=clampFinite(p.alpha,0.0f,1.0f,0.65f);
    p.frequencyNoiseSize=clampFinite(p.frequencyNoiseSize,0.0f,1.0f,0.0f);
    p.frequencyNoiseAmplitude=clampFinite(p.frequencyNoiseAmplitude,0.0f,5.0f,2.0f);
    p.preEmphasis=clampFinite(p.preEmphasis,0.0f,8.0f,0.0f);
    p.preEmphasisCutoffMHz=clampFinite(p.preEmphasisCutoffMHz,0.05f,6.0f,1.0f);
    if(p.fieldMode<0||p.fieldMode>2)return false;
    try {
        const int w=src.width,h=src.height;
        const std::size_t pixels=static_cast<std::size_t>(w)*static_cast<std::size_t>(h);
        std::vector<float> y(pixels),i(pixels),q(pixels);
        for(int row=0;row<h;++row)for(int x=0;x<w;++x) {
            const float* in=src.rgba+static_cast<std::ptrdiff_t>(row)*src.rowStrideFloats+x*4;
            float a=in[3];RGB rgb{in[0],in[1],in[2]};
            if(p.premultipliedAlpha&&a>1.0e-8f){rgb.r/=a;rgb.g/=a;rgb.b/=a;}
            const YIQ yiq=rgbToYiq(rgb);
            const std::size_t at=static_cast<std::size_t>(row)*w+x;
            y[at]=yiq.y;i[at]=yiq.i;q[at]=yiq.q;
        }
        const int selectedRows=p.fieldMode==0?h:(h+(p.fieldMode==1?1:0))/2;
        for(int row=0;row<h;++row) {
            if(p.fieldMode==1&&(row&1))continue;
            if(p.fieldMode==2&&!(row&1))continue;
            const std::size_t base=static_cast<std::size_t>(row)*w;
            if(p.preEmphasis>0.0f)preEmphasize(y.data()+base,w,p.preEmphasis,p.preEmphasisCutoffMHz);
            if(p.spectralRingingEnabled) {
                spectralMaskRow(y.data()+base,w,selectedRows,row,p);
                spectralMaskRow(i.data()+base,w,selectedRows,row,p);
                spectralMaskRow(q.data()+base,w,selectedRows,row,p);
            }
        }
        for(int row=0;row<h;++row)for(int x=0;x<w;++x) {
            const std::size_t at=static_cast<std::size_t>(row)*w+x;
            const RGB rgb=yiqToRgb({y[at],i[at],q[at]});
            const float* original=src.rgba+static_cast<std::ptrdiff_t>(row)*src.rowStrideFloats+x*4;
            float* out=dst.rgba+static_cast<std::ptrdiff_t>(row)*dst.rowStrideFloats+x*4;
            out[3]=original[3];
            if(p.premultipliedAlpha){out[0]=rgb.r*out[3];out[1]=rgb.g*out[3];out[2]=rgb.b*out[3];}
            else{out[0]=rgb.r;out[1]=rgb.g;out[2]=rgb.b;}
        }
        return true;
    } catch(...) { return false; }
}
} // namespace vhs90
