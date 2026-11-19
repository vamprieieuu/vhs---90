#include "VHS90IIRFilter.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace vhs90 {
namespace {
constexpr float kPi = 3.14159265358979323846f;

struct TransferFunction {
    std::vector<float> num;
    std::vector<float> den;
};

std::vector<float> multiplyPolynomial(const std::vector<float>& a,
                                      const std::vector<float>& b) {
    std::vector<float> out(a.size()+b.size()-1,0.0f);
    for(std::size_t i=0;i<a.size();++i)
        for(std::size_t j=0;j<b.size();++j)
            out[i+j]+=a[i]*b[j];
    return out;
}

TransferFunction makeFilter(float cutoff,float rate,IIRFilterDesign design) {
    cutoff=std::max(1.0f,std::min(cutoff,rate*0.499f));
    if(design==IIRFilterDesign::Butterworth2) {
        const float omega=2.0f*kPi*cutoff/rate;
        const float sn=std::sin(omega), cs=std::cos(omega);
        const float alpha=sn/(2.0f*0.70710678118f);
        const float a0=1.0f+alpha;
        TransferFunction f;
        f.num={(1.0f-cs)*0.5f/a0,(1.0f-cs)/a0,(1.0f-cs)*0.5f/a0,0.0f};
        f.den={1.0f,-2.0f*cs/a0,(1.0f-alpha)/a0,0.0f};
        return f;
    }

    const float tau=1.0f/(2.0f*kPi*cutoff);
    const float dt=1.0f/rate;
    const float alpha=dt/(tau+dt);
    const float pole=-(1.0f-alpha);
    std::vector<float> num{alpha};
    std::vector<float> den{1.0f,pole};
    num=multiplyPolynomial(multiplyPolynomial(num,num),num);
    den=multiplyPolynomial(multiplyPolynomial(den,den),den);
    num.resize(den.size(),0.0f);
    return {std::move(num),std::move(den)};
}

void initializeState(const TransferFunction& f,float value,std::vector<float>& z) {
    const std::size_t n=std::max(f.num.size(),f.den.size());
    z.assign(n,0.0f);
    if(value==0.0f||n<2) return;
    const float a0=f.den[0];
    if(std::fabs(a0)<1.0e-12f) return;
    float bsum=0.0f;
    for(std::size_t i=1;i<n;++i) {
        const float ni=i<f.num.size()?f.num[i]/a0:0.0f;
        const float di=i<f.den.size()?f.den[i]/a0:0.0f;
        bsum+=ni-di*(f.num[0]/a0);
    }
    float dsum=0.0f;
    for(float d:f.den) dsum+=d/a0;
    if(std::fabs(dsum)<1.0e-12f) return;
    z[0]=bsum/dsum;
    float asum=1.0f,csum=0.0f;
    for(std::size_t i=1;i<n-1;++i) {
        const float ni=i<f.num.size()?f.num[i]/a0:0.0f;
        const float di=i<f.den.size()?f.den[i]/a0:0.0f;
        asum+=di;
        csum+=ni-di*(f.num[0]/a0);
        z[i]=(asum*z[0]-csum)*value;
    }
    z[0]*=value;
}

void filterRow(const std::vector<float>& src,std::vector<float>& dst,
               const TransferFunction& f,int delay,IIRInitialCondition initial,
               std::vector<float>& z) {
    const std::size_t n=src.size();
    const std::size_t len=std::max(f.num.size(),f.den.size());
    initializeState(f,initial==IIRInitialCondition::FirstSample?src[0]:0.0f,z);
    dst.resize(n);
    const int d=std::max(0,delay);
    for(std::size_t i=0;i<n+static_cast<std::size_t>(d);++i) {
        const float sample=src[std::min(i,n-1)];
        const float y=z[0]+f.num[0]*sample;
        for(std::size_t k=0;k<len-1;++k) {
            const float nk=k+1<f.num.size()?f.num[k+1]:0.0f;
            const float dk=k+1<f.den.size()?f.den[k+1]:0.0f;
            z[k]=z[k+1]+nk*sample-dk*y;
        }
        if(i>=static_cast<std::size_t>(d)) dst[i-static_cast<std::size_t>(d)]=y;
    }
}

inline RGB unassociate(RGB c,float a,bool premul) noexcept {
    if(premul&&a>1.0e-6f) { c.r/=a; c.g/=a; c.b/=a; }
    return c;
}
}

bool processYiqBandwidth(ConstImageView src,ImageView dst,
                         const YiqBandwidthParams& p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||
       dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||
       src.rgba==dst.rgba||p.sampleRateHz<=0.0f) return false;
    if(p.iCutoffHz<0.0f||p.qCutoffHz<0.0f||p.lumaCutoffHz<0.0f||
       !std::isfinite(p.sampleRateHz)||!std::isfinite(p.iCutoffHz)||
       !std::isfinite(p.qCutoffHz)||!std::isfinite(p.lumaCutoffHz)||!std::isfinite(p.verticalLumaSigmaRows)||
       p.verticalLumaRadiusRows<0||p.verticalLumaRadiusRows>32||p.verticalLumaSigmaRows<=0.0f) return false;
    if(p.iCutoffHz==0.0f&&p.qCutoffHz==0.0f&&p.lumaCutoffHz==0.0f&&p.verticalLumaRadiusRows==0) {
        for(int y=0;y<src.height;++y)
            std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
        return true;
    }

    try {
        const int w=src.width;
        std::vector<float> y(w),i(w),q(w),yf(w),ifilt(w),qf(w),state;
        TransferFunction fi, fq, fy;
        if(p.iCutoffHz>0.0f) fi=makeFilter(p.iCutoffHz,p.sampleRateHz,p.design);
        if(p.qCutoffHz>0.0f) fq=makeFilter(p.qCutoffHz,p.sampleRateHz,p.design);
        if(p.lumaCutoffHz>0.0f) fy=makeFilter(p.lumaCutoffHz,p.sampleRateHz,p.design);

        for(int row=0;row<src.height;++row) {
            const float* srcRow=src.rgba+row*src.rowStrideFloats;
            float* dstRow=dst.rgba+row*dst.rowStrideFloats;
            for(int x=0;x<w;++x) {
                const float* px=srcRow+x*4;
                const float a=px[3];
                const RGB rgb=unassociate({px[0],px[1],px[2]},a,p.premultipliedAlpha);
                const YIQ yiq=rgbToYiq(rgb);
                y[x]=yiq.y; i[x]=yiq.i; q[x]=yiq.q;
            }
            if(p.iCutoffHz>0.0f) filterRow(i,ifilt,fi,p.iDelaySamples,p.initial,state); else ifilt=i;
            if(p.qCutoffHz>0.0f) filterRow(q,qf,fq,p.qDelaySamples,p.initial,state); else qf=q;
            if(p.lumaCutoffHz>0.0f) filterRow(y,yf,fy,p.lumaDelaySamples,p.initial,state); else yf=y;
            for(int x=0;x<w;++x) {
                const float* px=srcRow+x*4;
                const float a=px[3];
                RGB out=yiqToRgb({yf[x],ifilt[x],qf[x]});
                if(p.premultipliedAlpha) { out.r*=a; out.g*=a; out.b*=a; }
                float* dstPx=dstRow+x*4;
                dstPx[0]=out.r; dstPx[1]=out.g; dstPx[2]=out.b; dstPx[3]=a;
            }
        }
        if(p.verticalLumaRadiusRows>0) {
            const std::size_t count=std::size_t(src.width)*std::size_t(src.height);
            std::vector<float> lumaPlane(count),iPlane(count),qPlane(count),alphaPlane(count);
            for(int row=0;row<src.height;++row) {
                const float* rowPx=dst.rgba+row*dst.rowStrideFloats;
                for(int x=0;x<src.width;++x) {
                    const float* px=rowPx+x*4;const float a=px[3];
                    RGB rgb=unassociate({px[0],px[1],px[2]},a,p.premultipliedAlpha);
                    const YIQ q=rgbToYiq(rgb);const std::size_t k=std::size_t(row)*src.width+x;
                    lumaPlane[k]=q.y;iPlane[k]=q.i;qPlane[k]=q.q;alphaPlane[k]=a;
                }
            }
            const int radius=p.verticalLumaRadiusRows;const float sigma=p.verticalLumaSigmaRows;
            float weights[65]{};float total=0.0f;
            for(int d=-radius;d<=radius;++d){const float q=std::exp(-float(d*d)/(2.0f*sigma*sigma));weights[d+radius]=q;total+=q;}
            for(int d=0;d<radius*2+1;++d)weights[d]/=total;
            for(int row=0;row<src.height;++row) {
                float* rowPx=dst.rgba+row*dst.rowStrideFloats;
                for(int x=0;x<src.width;++x) {
                    float filteredY=0.0f;
                    for(int d=-radius;d<=radius;++d) {
                        const int yy=std::clamp(row+d,0,src.height-1);
                        filteredY+=lumaPlane[std::size_t(yy)*src.width+x]*weights[d+radius];
                    }
                    const std::size_t k=std::size_t(row)*src.width+x;
                    RGB out=yiqToRgb({filteredY,iPlane[k],qPlane[k]});
                    if(p.premultipliedAlpha){out.r*=alphaPlane[k];out.g*=alphaPlane[k];out.b*=alphaPlane[k];}
                    float* px=rowPx+x*4;px[0]=out.r;px[1]=out.g;px[2]=out.b;px[3]=alphaPlane[k];
                }
            }
        }
        return true;
    } catch(...) {
        return false;
    }
}

} // namespace vhs90
