#include "VHS90MultipathEchoEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace vhs90 {
namespace {
inline bool valid(ConstImageView s,ImageView d) noexcept {
 return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&
 s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba;
}
inline float sample(const float*row,int w,float x,int c) noexcept {
 x=std::clamp(x,0.0f,float(w-1));const int x0=int(x),x1=std::min(x0+1,w-1);const float t=x-x0;
 return row[x0*4+c]+(row[x1*4+c]-row[x0*4+c])*t;
}
inline RGB straight(const float*row,int w,float x,bool premul,float&alpha) noexcept {
 alpha=sample(row,w,x,3);RGB c{sample(row,w,x,0),sample(row,w,x,1),sample(row,w,x,2)};
 if(premul&&alpha>1.0e-6f){c.r/=alpha;c.g/=alpha;c.b/=alpha;}return c;
}
}
bool processMultipathEcho(ConstImageView src,ImageView dst,const MultipathEchoParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.gain)||!std::isfinite(p.delayPixels)||!std::isfinite(p.ensemble))return false;
 const float gain=std::clamp(p.gain,0.0f,1.0f),ensemble=std::clamp(p.ensemble,0.0f,1.0f);
 if(gain<=0.0f&&ensemble<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 const float phase=0.0039f*float(p.seed)+0.0017f*float(p.frame);
 const float base=std::max(1.0f,p.delayPixels);
 const float d1=base+3.0f+9.0f*ensemble,d2=base+11.0f+23.0f*ensemble,d3=base+27.0f+41.0f*ensemble;
 const float gb=0.14f*ensemble+0.07f*gain;
 const float g1=gain*0.42f+gb*(0.9f+0.4f*std::sin(phase*0.71f));
 const float g2=-0.75f*gb*(0.9f+0.4f*std::sin(phase*0.49f+1.3f));
 const float g3=0.52f*gb*(0.9f+0.4f*std::sin(phase*0.31f+2.1f));
 for(int y=0;y<src.height;++y){const float*sr=src.rgba+y*src.rowStrideFloats;float*dr=dst.rgba+y*dst.rowStrideFloats;
  for(int x=0;x<src.width;++x){const float*px=sr+x*4;float*out=dr+x*4;const float a=px[3];
   if(a<=1.0e-6f){std::copy_n(px,4,out);continue;}
   RGB cur{px[0],px[1],px[2]};if(p.premultipliedAlpha){cur.r/=a;cur.g/=a;cur.b/=a;}
   YIQ yiq=rgbToYiq(cur);
   const float delays[3]={d1,d2,d3},gains[3]={g1,g2,g3};
   for(int k=0;k<3;++k){float ea=0;const RGB echo=straight(sr,src.width,float(x)-delays[k],p.premultipliedAlpha,ea);
    const YIQ ey=rgbToYiq(echo);const float weight=gains[k]*std::clamp(ea,0.0f,1.0f);
    yiq.y+=ey.y*weight;yiq.i+=ey.i*weight;yiq.q+=ey.q*weight;
   }
   RGB result=yiqToRgb(yiq);if(p.premultipliedAlpha){result.r*=a;result.g*=a;result.b*=a;}
   out[0]=result.r;out[1]=result.g;out[2]=result.b;out[3]=a;
  }
 }
 return true;
}
} // namespace vhs90
