#include "VHS90DeviceConnectionEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace vhs90 { namespace {
struct Node { float x,y,phase,energy; };
inline std::uint32_t mix(std::uint32_t x) noexcept { x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;return x; }
inline float rnd(std::uint32_t seed,std::uint32_t a,std::uint32_t b=0) noexcept { return float(mix(seed^(a*0x9e3779b9u)^(b*0x85ebca6bu))&0x00ffffffu)/16777216.0f; }
inline float smooth(float a,float b,float x) noexcept { float t=std::clamp((x-a)/(b-a),0.0f,1.0f);return t*t*(3.0f-2.0f*t); }
inline bool valid(ConstImageView s,ImageView d) noexcept { return s.rgba&&d.rgba&&s.width>0&&s.height>0&&d.width==s.width&&d.height==s.height&&s.rowStrideFloats>=s.width*4&&d.rowStrideFloats>=d.width*4&&s.rgba!=d.rgba; }
inline void over(float* p,float r,float g,float b,float a,bool premul) noexcept {
 if(a<=0.0f||p[3]<=0.0f)return;
 const float oldA=p[3],inv=1.0f-a;
 if(premul){p[0]=r*a+p[0]*inv;p[1]=g*a+p[1]*inv;p[2]=b*a+p[2]*inv;}
 else {p[0]=(r*a+p[0]*oldA*inv)/(a+oldA*inv);p[1]=(g*a+p[1]*oldA*inv)/(a+oldA*inv);p[2]=(b*a+p[2]*oldA*inv)/(a+oldA*inv);}
 p[3]=a+oldA*inv;
}
}
bool processDeviceConnections(ConstImageView src,ImageView dst,const DeviceConnectionParams&p) noexcept {
 if(!valid(src,dst)||!std::isfinite(p.amount)||!std::isfinite(p.density)||!std::isfinite(p.nodeSize)||!std::isfinite(p.speed)||!std::isfinite(p.distribution)||!std::isfinite(p.maxLinkDistance)||!std::isfinite(p.framesPerSecond)||p.framesPerSecond<=0.0f)return false;
 const float amount=std::clamp(p.amount,0.0f,1.0f);
 if(amount<=0.0f){for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);return true;}
 for(int y=0;y<src.height;++y)std::copy_n(src.rgba+y*src.rowStrideFloats,src.width*4,dst.rgba+y*dst.rowStrideFloats);
 const int w=src.width,h=src.height;
 const int count=std::clamp(int(std::lround(8.0f+std::clamp(p.density,0.0f,1.0f)*120.0f)),8,128);
 const int cols=std::max(2,int(std::ceil(std::sqrt(float(count)*float(w)/float(h)))));
 const int rows=std::max(2,int(std::ceil(float(count)/float(cols))));
 const float distMix=std::clamp(p.distribution,0.0f,1.0f);
 const float t=float(p.frame)/p.framesPerSecond*p.speed;
 const float maxD=std::max(0.025f,p.maxLinkDistance)*float(std::min(w,h));
 std::vector<Node> nodes;nodes.reserve(std::size_t(cols*rows));
 for(int gy=0;gy<rows;++gy)for(int gx=0;gx<cols;++gx){
  const std::uint32_t id=std::uint32_t(gy*cols+gx);
  const float jx=rnd(p.seed,id,1u)-0.5f,jy=rnd(p.seed,id,2u)-0.5f;
  const float rx=rnd(p.seed,id,3u),ry=rnd(p.seed,id,4u);
  float x=(float(gx)+0.5f+ jx*0.82f)/float(cols)*float(w);
  float y=(float(gy)+0.5f+ jy*0.82f)/float(rows)*float(h);
  x=x*(1.0f-distMix)+rx*float(w)*distMix;
  y=y*(1.0f-distMix)+ry*float(h)*distMix;
  const float phase=rnd(p.seed,id,5u)*6.28318530718f;
  const float drift=std::sin(t*1.7f+phase)*std::min(float(w),float(h))*0.012f;
  const float driftY=std::cos(t*1.31f+phase*1.3f)*std::min(float(w),float(h))*0.012f;
  nodes.push_back({std::clamp(x+drift,0.0f,float(w-1)),std::clamp(y+driftY,0.0f,float(h-1)),phase,0.65f+0.35f*std::sin(t*2.0f+phase)});
 }
 const float r=std::clamp(p.red,0.0f,1.0f),g=std::clamp(p.green,0.0f,1.0f),b=std::clamp(p.blue,0.0f,1.0f);
 const float lineWidth=std::clamp(p.nodeSize,0.5f,8.0f)*0.55f;
 auto blendPixel=[&](int x,int y,float strength){if(x<0||x>=w||y<0||y>=h)return;float*px=dst.rgba+y*dst.rowStrideFloats+x*4;const float a=amount*std::clamp(strength,0.0f,1.0f);over(px,r,g,b,a,p.premultipliedAlpha);};
 // Sparse nearest-neighbour graph. Every node links to its two closest peers;
 // this avoids unstable all-to-all tangles as density changes.
 for(std::size_t i=0;i<nodes.size();++i){
  float best1=1.0e30f,best2=1.0e30f;int n1=-1,n2=-1;
  for(std::size_t j=0;j<nodes.size();++j){if(i==j)continue;const float dx=nodes[i].x-nodes[j].x,dy=nodes[i].y-nodes[j].y,d=dx*dx+dy*dy;if(d<best1){best2=best1;n2=n1;best1=d;n1=int(j);}else if(d<best2){best2=d;n2=int(j);}}
  const int links[2]={n1,n2};
  for(int k=0;k<2;++k){const int j=links[k];if(j<0||std::size_t(j)<=i)continue;const Node&a=nodes[i],&z=nodes[std::size_t(j)];const float dx=z.x-a.x,dy=z.y-a.y,len=std::sqrt(dx*dx+dy*dy);if(len>maxD)continue;
   const int steps=std::max(1,int(std::ceil(len*1.35f)));for(int q=0;q<=steps;++q){const float u=float(q)/float(steps),px=a.x+dx*u,py=a.y+dy*u;const int ix=int(std::lround(px)),iy=int(std::lround(py));const float pulse=0.45f+0.55f*(0.5f+0.5f*std::sin(t*4.0f+a.phase+u*6.2831853f));const float strength=0.28f*pulse*(1.0f-len/maxD*0.35f);const int rad=int(std::ceil(lineWidth));for(int oy=-rad;oy<=rad;++oy)for(int ox=-rad;ox<=rad;++ox){const float d2=float(ox*ox+oy*oy);if(d2<=lineWidth*lineWidth)blendPixel(ix+ox,iy+oy,strength*(1.0f-smooth(lineWidth*lineWidth-0.5f,lineWidth*lineWidth+0.5f,d2)));}}
  }
 }
 const float nodeRadius=std::clamp(p.nodeSize,0.5f,8.0f);
 for(const Node&n:nodes){const int rad=int(std::ceil(nodeRadius*2.0f));const float pulse=0.55f+0.45f*n.energy;for(int oy=-rad;oy<=rad;++oy)for(int ox=-rad;ox<=rad;++ox){const float d=std::sqrt(float(ox*ox+oy*oy));if(d<=nodeRadius*2.0f)blendPixel(int(std::lround(n.x))+ox,int(std::lround(n.y))+oy,0.82f*pulse*(1.0f-smooth(nodeRadius,nodeRadius*2.0f,d)));}}
 return true;
}
} // namespace vhs90
