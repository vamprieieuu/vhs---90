#include "VHS90TemporalEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 {
namespace {
inline bool valid(ConstImageView v) noexcept { return v.rgba&&v.width>0&&v.height>0&&v.rowStrideFloats>=v.width*4; }
inline void sample(const std::vector<float>&src,int w,int h,float x,float y,float out[4]) noexcept {
 x=std::clamp(x,0.0f,float(w-1));y=std::clamp(y,0.0f,float(h-1));
 const int x0=int(x),y0=int(y),x1=std::min(x0+1,w-1),y1=std::min(y0+1,h-1);const float fx=x-x0,fy=y-y0;
 const size_t a=(size_t(y0)*w+x0)*4,b=(size_t(y0)*w+x1)*4,c=(size_t(y1)*w+x0)*4,d=(size_t(y1)*w+x1)*4;
 for(int k=0;k<4;++k){const float ab=src[a+k]+(src[b+k]-src[a+k])*fx,cd=src[c+k]+(src[d+k]-src[c+k])*fx;out[k]=ab+(cd-ab)*fy;}
}
inline RGB straight(const float*p,bool premul) noexcept {
 RGB c{p[0],p[1],p[2]};const float a=p[3];
 if(premul&&a>1.0e-6f){c.r/=a;c.g/=a;c.b/=a;}return c;
}
}
bool processTemporalGhost(ConstImageView cur,ConstImageView prev,ImageView dst,const TemporalGhostParams&p) noexcept {
 if(!valid(cur)||!valid(prev)||!dst.rgba||cur.width!=prev.width||cur.height!=prev.height||
    dst.width!=cur.width||dst.height!=cur.height||dst.rowStrideFloats<dst.width*4||
    dst.rgba==cur.rgba||dst.rgba==prev.rgba||
    !std::isfinite(p.amount)||!std::isfinite(p.lumaAmount)||!std::isfinite(p.chromaAmount)||
    !std::isfinite(p.offsetXPixels)||!std::isfinite(p.offsetYPixels)||
    !std::isfinite(p.redFringePixels)||!std::isfinite(p.blueFringePixels)||!std::isfinite(p.fieldWeave))
  return false;
 const float amount=std::clamp(p.amount,0.0f,1.0f),luma=std::clamp(p.lumaAmount,0.0f,2.0f);
 const float chroma=std::clamp(p.chromaAmount,0.0f,2.0f),weave=std::clamp(p.fieldWeave,0.0f,1.0f);
 if(amount<=0.0f) { for(int y=0;y<cur.height;++y)std::copy_n(cur.rgba+y*cur.rowStrideFloats,cur.width*4,dst.rgba+y*dst.rowStrideFloats);return true; }
 try {
  const int w=cur.width,h=cur.height;std::vector<float>old(size_t(w)*h*4);
  for(int y=0;y<h;++y)std::copy_n(prev.rgba+y*prev.rowStrideFloats,w*4,old.data()+size_t(y)*w*4);
  for(int y=0;y<h;++y){const float*cr=cur.rgba+y*cur.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
   for(int x=0;x<w;++x){const float*px=cr+x*4;float*out=dr+x*4;const float alpha=px[3];
    if(alpha<=1.0e-6f){std::copy_n(px,4,out);continue;}
    float g[4],r[4],b[4];sample(old,w,h,float(x)-p.offsetXPixels,float(y)-p.offsetYPixels,g);
    sample(old,w,h,float(x)-p.offsetXPixels-p.redFringePixels,float(y)-p.offsetYPixels,r);
    sample(old,w,h,float(x)-p.offsetXPixels-p.blueFringePixels,float(y)-p.offsetYPixels,b);
    const RGB currentRgb=straight(px,p.premultipliedAlpha),ghostRgb=straight(g,p.premultipliedAlpha);
    const float ghostMix=amount*std::clamp(g[3],0.0f,1.0f);
    const YIQ cy=rgbToYiq(currentRgb),gy=rgbToYiq(ghostRgb);
    const float outY=cy.y+(gy.y-cy.y)*ghostMix*luma;
    const float outI=cy.i+(gy.i-cy.i)*ghostMix*chroma;
    const float outQ=cy.q+(gy.q-cy.q)*ghostMix*chroma;
    RGB result=yiqToRgb({outY,outI,outQ});
    const float fieldMix=((y&1)==(p.fieldParity&1))?ghostMix*weave:0.0f;
    const RGB red=straight(r,p.premultipliedAlpha),blue=straight(b,p.premultipliedAlpha);
    result.r+=(red.r-result.r)*fieldMix;
    result.g+=(ghostRgb.g-result.g)*fieldMix;
    result.b+=(blue.b-result.b)*fieldMix;
    if(p.premultipliedAlpha){result.r*=alpha;result.g*=alpha;result.b*=alpha;}
    out[0]=result.r;out[1]=result.g;out[2]=result.b;out[3]=alpha;
   }
  }
  return true;
 }catch(...){return false;}
}
} // namespace vhs90
