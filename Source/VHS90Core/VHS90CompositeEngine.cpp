#include "VHS90CompositeEngine.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace vhs90 {
namespace {
constexpr float kIMult[4]={1.0f,0.0f,-1.0f,0.0f};
constexpr float kQMult[4]={0.0f,1.0f,0.0f,-1.0f};
inline int mod4(std::int64_t v) noexcept { int r=int(v%4); return r<0?r+4:r; }
inline int phaseShift(CompositePhase mode,int offset,std::int64_t frame,int line) noexcept {
    if(mode==CompositePhase::Degrees0) return 0;
    if(mode==CompositePhase::Degrees180) return mod4(((frame+line*2)&2)+offset);
    return mod4(frame+offset+line);
}
inline RGB readStraight(ConstImageView src,int x,int y,bool premul) noexcept {
    const float* p=src.rgba+y*src.rowStrideFloats+x*4;
    RGB c{p[0],p[1],p[2]};
    if(premul&&p[3]>1.0e-6f) { c.r/=p[3]; c.g/=p[3]; c.b/=p[3]; }
    return c;
}
inline void demodulateChroma(float chroma,int index,int xi,float* i,float* q,int width) noexcept {
    const int offset=(index+xi)&3;
    const float im=-chroma*kIMult[offset];
    const float qm=-chroma*kQMult[offset];
    if(index<width-1) { i[index+1]=im*0.5f; q[index+1]=qm*0.5f; }
    i[index]+=im; q[index]+=qm;
    if(index>0) { i[index-1]+=im*0.5f; q[index-1]+=qm*0.5f; }
}
}

bool processCompositeRoundTrip(ConstImageView src,ImageView dst,
                               const CompositeParams& p) noexcept {
    if(!src.rgba||!dst.rgba||src.width<=0||src.height<=0||
       dst.width!=src.width||dst.height!=src.height||
       src.rowStrideFloats<src.width*4||dst.rowStrideFloats<dst.width*4||
       src.rgba==dst.rgba) return false;
    if(!std::isfinite(p.strength)||!std::isfinite(p.compositeSignalStrength)) return false;
    const float strength=std::max(0.0f,std::min(1.0f,p.strength));
    const float compositeStrength=std::max(0.0f,std::min(1.0f,p.compositeSignalStrength));
    if(strength<=0.0f) {
        for(int y=0;y<src.height;++y)
            std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
        return true;
    }

    try {
        const int w=src.width,h=src.height;
        const std::size_t count=std::size_t(w)*std::size_t(h);
        std::vector<float> geometricallyWarped;
        std::vector<float> transported;
        std::vector<float> tapeDamaged;
        std::vector<float> prefiltered;
        ConstImageView input=src;
        if(p.geometryEnabled) {
            geometricallyWarped.resize(count*4);
            ImageView geometryView{geometricallyWarped.data(),w,h,w*4};
            if(!processAnalogGeometry(src,geometryView,p.geometry)) return false;
            input={geometricallyWarped.data(),w,h,w*4};
        }
        if(p.tapeTransportEnabled) {
            transported.resize(count*4);
            ImageView transportView{transported.data(),w,h,w*4};
            if(!processTapeTransportWarp(input,transportView,p.tapeTransport)) return false;
            input={transported.data(),w,h,w*4};
        }
        if(p.tapeSurfaceEnabled) {
            tapeDamaged.resize(count*4);
            ImageView tapeView{tapeDamaged.data(),w,h,w*4};
            if(!processTapeSurface(input,tapeView,p.tapeSurface)) return false;
            input={tapeDamaged.data(),w,h,w*4};
        }
        if(compositeStrength>0.0f && p.inputBandwidthEnabled) {
            prefiltered.resize(count*4);
            ImageView pre{prefiltered.data(),w,h,w*4};
            YiqBandwidthParams bp=p.inputBandwidth; bp.premultipliedAlpha=p.premultipliedAlpha;
            if(!processYiqBandwidth(input,pre,bp)) return false;
            input={prefiltered.data(),w,h,w*4};
        }

        std::vector<float> decoded;
        if(compositeStrength>0.0f) {
            std::vector<float> y(count),i(count),q(count),decodedY(count);
            for(int row=0;row<h;++row) for(int x=0;x<w;++x) {
                const std::size_t idx=std::size_t(row)*w+x;
                const RGB rgb=readStraight(input,x,row,p.premultipliedAlpha);
                const YIQ yiq=rgbToYiq(rgb);
                y[idx]=yiq.y; i[idx]=yiq.i; q[idx]=yiq.q;
            }
    
            for(int row=0;row<h;++row) {
                const int xi=phaseShift(p.phase,p.phaseOffset,p.frame,row);
                const std::size_t base=std::size_t(row)*w;
                for(int x=0;x<w;++x) {
                    const int phase=(x+xi)&3;
                    const float palSign=(p.palMode&&p.palSwitch&&(row&1))?-1.0f:1.0f;
                    y[base+x]+=i[base+x]*kIMult[phase]+q[base+x]*palSign*kQMult[phase];
                }
            }
    
            std::fill(i.begin(),i.end(),0.0f);
            std::fill(q.begin(),q.end(),0.0f);
    
            std::vector<float> scratch(w);
            for(int row=0;row<h;++row) {
                const int xi=phaseShift(p.phase,p.phaseOffset,p.frame,row);
                const std::size_t base=std::size_t(row)*w;
                if(p.demodulator==CompositeDemodulator::Box || h<2) {
                    for(int x=0;x<w;++x) {
                        const float c=y[base+std::min(x+2,w-1)];
                        const float a0=x>0?y[base+x-1]:16.0f/255.0f;
                        const float a1=y[base+x];
                        const float a2=y[base+std::min(x+1,w-1)];
                        const float a3=y[base+std::min(x+2,w-1)];
                        scratch[x]=(a0+a1+a2+a3)*0.25f;
                        const float chroma=c-scratch[x];
                        demodulateChroma(chroma,x,xi,i.data()+base,q.data()+base,w);
                    }
                    std::copy(scratch.begin(),scratch.end(),decodedY.begin()+base);
                } else {
                    const int prev=row==0?1:row-1;
                    const int next=row==h-1?h-2:row+1;
                    for(int x=0;x<w;++x) {
                        const float cur=y[base+x];
                        float blended=cur;
                        if(p.demodulator==CompositeDemodulator::OneLineComb) {
                            blended=(cur+y[std::size_t(prev)*w+x])*0.5f;
                        } else {
                            blended=cur*0.5f+y[std::size_t(prev)*w+x]*0.25f+y[std::size_t(next)*w+x]*0.25f;
                        }
                        decodedY[base+x]=blended;
                        const float chroma=blended-cur;
                        demodulateChroma(chroma,x,xi,i.data()+base,q.data()+base,w);
                    }
                }
                if(p.palMode&&p.palSwitch&&(row&1))
                    for(int x=0;x<w;++x) q[base+x]=-q[base+x];
            }
    
            decoded.resize(count*4);
            for(int row=0;row<h;++row) for(int x=0;x<w;++x) {
                const std::size_t idx=std::size_t(row)*w+x;
                const float alpha=input.rgba[row*input.rowStrideFloats+x*4+3];
                RGB rgb=yiqToRgb({decodedY[idx],i[idx],q[idx]});
                const RGB original=readStraight(input,x,row,p.premultipliedAlpha);
                rgb.r=original.r+(rgb.r-original.r)*compositeStrength;
                rgb.g=original.g+(rgb.g-original.g)*compositeStrength;
                rgb.b=original.b+(rgb.b-original.b)*compositeStrength;
                if(p.premultipliedAlpha) { rgb.r*=alpha; rgb.g*=alpha; rgb.b*=alpha; }
                float* out=decoded.data()+idx*4;
                out[0]=rgb.r; out[1]=rgb.g; out[2]=rgb.b; out[3]=alpha;
            }
    
        } else {
            decoded.resize(count*4);
            for(int y=0;y<h;++y)
                std::copy_n(input.rgba+y*input.rowStrideFloats,w*4,decoded.data()+std::size_t(y)*w*4);
        }

        ConstImageView decodedView{decoded.data(),w,h,w*4};
        std::vector<float> multipathProcessed;
        if(p.multipathEchoEnabled) {
            multipathProcessed.resize(count*4);
            ImageView multipathView{multipathProcessed.data(),w,h,w*4};
            MultipathEchoParams mp=p.multipathEcho;
            mp.premultipliedAlpha=p.premultipliedAlpha;
            if(!processMultipathEcho(decodedView,multipathView,mp)) return false;
            decodedView={multipathProcessed.data(),w,h,w*4};
        }
        std::vector<float> impulseProcessed;
        if(p.impulseNoiseEnabled) {
            impulseProcessed.resize(count*4);
            ImageView impulseView{impulseProcessed.data(),w,h,w*4};
            ImpulseNoiseParams ip=p.impulseNoise;
            ip.premultipliedAlpha=p.premultipliedAlpha;
            if(!processImpulseNoise(decodedView,impulseView,ip)) return false;
            decodedView={impulseProcessed.data(),w,h,w*4};
        }
        std::vector<float> analogNoisy;
        if(p.analogNoiseEnabled) {
            analogNoisy.resize(count*4);
            ImageView noiseView{analogNoisy.data(),w,h,w*4};
            AnalogNoiseParams np=p.analogNoise;
            np.premultipliedAlpha=p.premultipliedAlpha;
            if(!processAnalogNoise(decodedView,noiseView,np)) return false;
            decodedView={analogNoisy.data(),w,h,w*4};
        }
        std::vector<float> rfProcessed;
        if(p.rfInstabilityEnabled) {
            rfProcessed.resize(count*4);
            ImageView rfView{rfProcessed.data(),w,h,w*4};
            RfInstabilityParams rp=p.rfInstability;
            rp.premultipliedAlpha=p.premultipliedAlpha;
            if(!processRfInstability(decodedView,rfView,rp)) return false;
            decodedView={rfProcessed.data(),w,h,w*4};
        }
        std::vector<float> verticalBlended;
        std::vector<float> chromaSubsampled;
        std::vector<float> smeared;
        if(p.verticalChromaBlendEnabled && p.verticalChromaBlendAmount>0.0f) {
            verticalBlended.resize(count*4);
            ImageView verticalView{verticalBlended.data(),w,h,w*4};
            VerticalChromaBlendParams vp;
            vp.amount=p.verticalChromaBlendAmount;
            vp.premultipliedAlpha=p.premultipliedAlpha;
            if(!processVerticalChromaBlend(decodedView,verticalView,vp)) return false;
            decodedView={verticalBlended.data(),w,h,w*4};
        }
        if(p.chromaSubsampleEnabled && p.chromaSubsample.amount>0.0f) {
            chromaSubsampled.resize(count*4);
            ImageView chromaView{chromaSubsampled.data(),w,h,w*4};
            ChromaSubsampleParams cp=p.chromaSubsample;
            cp.premultipliedAlpha=p.premultipliedAlpha;
            if(!processChromaSubsample(decodedView,chromaView,cp)) return false;
            decodedView={chromaSubsampled.data(),w,h,w*4};
        }
        if(p.lumaSmearEnabled && p.lumaSmear.strength>0.0f) {
            smeared.resize(count*4);
            ImageView smearView{smeared.data(),w,h,w*4};
            LumaSmearParams lp=p.lumaSmear;
            lp.premultipliedAlpha=p.premultipliedAlpha;
            if(!processLumaSmear(decodedView,smearView,lp)) return false;
            decodedView={smeared.data(),w,h,w*4};
        }
        std::vector<float> filtered;
        std::vector<float> bloomed;
        std::vector<float> scanlined;
        if(p.outputBandwidthEnabled && (compositeStrength>0.0f || p.outputBandwidth.verticalLumaRadiusRows>0)) {
            filtered.resize(count*4);
            ImageView filteredView{filtered.data(),w,h,w*4};
            YiqBandwidthParams bp=p.outputBandwidth; bp.premultipliedAlpha=p.premultipliedAlpha;
            if(!processYiqBandwidth(decodedView,filteredView,bp)) return false;
            decodedView={filtered.data(),w,h,w*4};
        }
        if(p.bloomEnabled && p.bloom.intensity>0.0f) {
            bloomed.resize(count*4);
            ImageView bloomView{bloomed.data(),w,h,w*4};
            BloomParams bp=p.bloom;
            bp.premultipliedAlpha=p.premultipliedAlpha;
            if(!processBloom(decodedView,bloomView,bp)) return false;
            decodedView={bloomed.data(),w,h,w*4};
        }
        if(p.scanlinesEnabled && p.scanlines.amount>0.0f) {
            scanlined.resize(count*4);
            ImageView scanlineView{scanlined.data(),w,h,w*4};
            ScanlineParams sp=p.scanlines;
            sp.premultipliedAlpha=p.premultipliedAlpha;
            if(!processScanlines(decodedView,scanlineView,sp)) return false;
            decodedView={scanlined.data(),w,h,w*4};
        }

        for(int row=0;row<h;++row) {
            const float* srcRow=src.rgba+row*src.rowStrideFloats;
            float* dstRow=dst.rgba+row*dst.rowStrideFloats;
            const float* procRow=decodedView.rgba+row*decodedView.rowStrideFloats;
            for(int x=0;x<w;++x) {
                const float* a=srcRow+x*4; const float* b=procRow+x*4; float* d=dstRow+x*4;
                d[0]=a[0]+(b[0]-a[0])*strength;
                d[1]=a[1]+(b[1]-a[1])*strength;
                d[2]=a[2]+(b[2]-a[2])*strength;
                d[3]=a[3]+(b[3]-a[3])*strength;
            }
        }
        return true;
    } catch(...) {
        return false;
    }
}

} // namespace vhs90
