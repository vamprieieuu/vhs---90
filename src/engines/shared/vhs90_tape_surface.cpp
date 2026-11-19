#include "vhs90_tape_surface.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace vhs90 {
namespace {
constexpr float kPi=3.14159265358979323846f;
inline uint64_t mix64(uint64_t x) noexcept { x^=x>>30;x*=0xbf58476d1ce4e5b9ULL;x^=x>>27;x*=0x94d049bb133111ebULL;x^=x>>31;return x; }
inline uint64_t hashCell(uint64_t seed,int64_t x,int64_t y,uint32_t lane=0) noexcept { uint64_t h=seed^(uint64_t(x)*0x9e3779b97f4a7c15ULL)^(uint64_t(y)*0xd1b54a32d192ed03ULL)^(uint64_t(lane)*0x94d049bb133111ebULL);return mix64(h); }
inline float unit(uint64_t h) noexcept { return float((h>>40)&0xffffffu)*(1.0f/16777216.0f); }
inline float smooth(float x) noexcept { x=std::clamp(x,0.0f,1.0f);return x*x*(3.0f-2.0f*x); }
inline float falloffEllipse(float dx,float dy,float rx,float ry) noexcept { if(rx<=0||ry<=0)return 0;float q=(dx*dx)/(rx*rx)+(dy*dy)/(ry*ry);return q>=1?0:1.0f-smooth(q); }
inline void rgbToYcc(float r,float g,float b,float& y,float& cb,float& cr) noexcept { y=.299f*r+.587f*g+.114f*b;cb=-.168736f*r-.331264f*g+.5f*b;cr=.5f*r-.418688f*g-.081312f*b; }
inline void yccToRgb(float y,float cb,float cr,float& r,float& g,float& b) noexcept { r=y+1.402f*cr;g=y-.344136f*cb-.714136f*cr;b=y+1.772f*cb; }
}

TapeSurfaceSample sampleTapeSurface(float u,float v,const TapeSurfaceParams&p) noexcept {
    const float cu=std::max(p.alongCellSize,1e-4f),cv=std::max(p.acrossCellSize,1e-4f);
    const int64_t ix=int64_t(std::floor(u/cu)),iy=int64_t(std::floor(v/cv));
    TapeSurfaceSample out;
    const float density=std::clamp(p.defectDensity,0.0f,1.0f);
    for(int oy=-1;oy<=1;++oy)for(int ox=-1;ox<=1;++ox){
        const int64_t cx=ix+ox,cy=iy+oy;
        const uint64_t h=hashCell(p.seed,cx,cy);
        if(unit(h)>density*0.42f)continue;
        const float centerU=(float(cx)+unit(hashCell(p.seed,cx,cy,1)))*cu;
        const float centerV=(float(cy)+unit(hashCell(p.seed,cx,cy,2)))*cv;
        const float du=u-centerU,dv=v-centerV;
        const float type=unit(hashCell(p.seed,cx,cy,3));
        const float scale=0.25f+2.5f*std::pow(unit(hashCell(p.seed,cx,cy,4)),2.2f);
        const float radiusU=cu*scale;
        const float radiusV=cv*(0.06f+0.42f*unit(hashCell(p.seed,cx,cy,5)));
        if(type<0.42f){
            const float f=falloffEllipse(du,dv,radiusU,radiusV);
            out.magnetization*=1.0f-std::clamp(p.oxideLoss*f,0.0f,0.95f);
        }else if(type<0.68f){
            const float f=falloffEllipse(du,dv,radiusU*2.2f,radiusV*0.12f);
            out.magnetization*=1.0f-std::clamp(p.scratchDensity*f,0.0f,0.9f);
            out.roughness+=f*p.surfaceRoughness;
        }else if(type<0.86f){
            const float f=falloffEllipse(du,dv,radiusU*1.5f,radiusV*0.28f);
            out.creaseLoss=std::max(out.creaseLoss,std::clamp(p.creaseStrength*f,0.0f,1.0f));
            out.spacingLift+=f*p.debrisAmount*0.8f;
        }else{
            const float f=falloffEllipse(du,dv,radiusU*0.45f,radiusV*0.65f);
            out.spacingLift+=f*p.debrisAmount;
            out.roughness+=f*p.surfaceRoughness*1.5f;
        }
    }
    const float edge=std::min(v,1.0f-v);
    const float edgeProfile=1.0f-smooth(std::clamp(edge/0.06f,0.0f,1.0f));
    out.spacingLift+=edgeProfile*std::max(0.0f,p.edgeLift);
    out.roughness+=std::abs(std::sin((u*173.0f+v*29.0f)*kPi))*p.surfaceRoughness*0.15f;
    out.magnetization=std::clamp(out.magnetization,0.0f,1.0f);
    out.spacingLift=std::clamp(out.spacingLift,0.0f,1.0f);
    out.roughness=std::clamp(out.roughness,0.0f,0.25f);
    out.creaseLoss=std::clamp(out.creaseLoss,0.0f,1.0f);
    return out;
}

void applyTapeSurface(FloatRgbaView im,const TapeSurfaceParams&p){
    if(!im.pixels||im.width<=0||im.height<=0||im.rowStrideFloats<im.width*4)return;
    for(int y=0;y<im.height;++y){
        float* row=im.pixels+size_t(y)*im.rowStrideFloats;
        const float v=(float(y)+0.5f)/float(im.height);
        for(int x=0;x<im.width;++x){
            const float u=(float(x)+0.5f)/float(im.width)+float(p.frameIndex)*p.tapeAdvancePerFrame;
            const TapeSurfaceSample s=sampleTapeSurface(u,v,p);
            float Y,Cb,Cr;rgbToYcc(row[x*4],row[x*4+1],row[x*4+2],Y,Cb,Cr);
            const float signalLoss=1.0f-s.creaseLoss;
            const float mag=s.magnetization*signalLoss;
            Y*=mag*(1.0f-0.22f*s.spacingLift);
            const float chromaAtten=mag*(1.0f-0.65f*s.spacingLift);
            Cb*=chromaAtten;Cr*=chromaAtten;
            const float grain=(unit(hashCell(p.seed^0xA511E9B3u,int64_t(std::floor(u*4096.0f)),int64_t(std::floor(v*1024.0f)),uint32_t(p.frameIndex)))*2.0f-1.0f)*s.roughness;
            Y+=grain;
            yccToRgb(Y,Cb,Cr,row[x*4],row[x*4+1],row[x*4+2]);
            // Alpha is not touched.
        }
    }
}
} // namespace vhs90
