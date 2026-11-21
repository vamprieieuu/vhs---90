#include "VHS90ImageGradeEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=64,h=32;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size()),z(s.size()),slow(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;float v=x<32?.25f:.8f;s[i]=v;s[i+1]=.45f;s[i+2]=.65f;s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4},zi{z.data(),w,h,w*4},slowi{slow.data(),w,h,w*4};
 ImageGradeParams p;p.edgeEnhancement=.7f;p.chromaEdgeEnhancement=.8f;p.fade=.3f;p.blackLift=.5f;p.contrast=1.2f;p.brightness=.02f;p.saturation=.8f;p.vignette=.6f;p.flicker=.4f;p.seed=3;p.frame=5;
 assert(processImageGrade(si,ai,p));assert(processImageGrade(si,bi,p));assert(a==b&&a!=s);
 p.frame=6;assert(processImageGrade(si,ci,p));assert(a!=c);
 ImageGradeParams chromaOnly;chromaOnly.chromaEdgeEnhancement=1.0f;
 assert(processImageGrade(si,zi,chromaOnly));assert(z!=s);
 chromaOnly.chromaSharpenCutoffHz=560000.0f;assert(processImageGrade(si,slowi,chromaOnly));assert(slow!=z);
 
 ImageGradeParams color;std::vector<float> colorA(s.size()),colorB(s.size());ImageView colorAi{colorA.data(),w,h,w*4},colorBi{colorB.data(),w,h,w*4};
 color.chromaIGain=1.8f;color.chromaQGain=.45f;color.chromaIOffset=.08f;color.chromaQOffset=-.06f;color.chromaQuantization=.025f;
 assert(processImageGrade(si,colorAi,color));assert(processImageGrade(si,colorBi,color));assert(colorA==colorB&&colorA!=s);
 color.chromaQuantization=0.0f;assert(processImageGrade(si,colorBi,color));assert(colorA!=colorB);

 // Chroma quantization must reduce chroma magnitude without randomly rotating hue.
 ImageGradeParams quant;quant.chromaQuantization=.06f;
 std::vector<float> quantized(s.size());ImageView quantizedView{quantized.data(),w,h,w*4};
 assert(processImageGrade(si,quantizedView,quant));
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){
  const size_t k=(size_t(y)*w+x)*4;
  const YIQ before=rgbToYiq(RGB{s[k],s[k+1],s[k+2]});
  const YIQ after=rgbToYiq(RGB{quantized[k],quantized[k+1],quantized[k+2]});
  if(std::sqrt(before.i*before.i+before.q*before.q)>.01f){
   const float angleBefore=std::atan2(before.q,before.i),angleAfter=std::atan2(after.q,after.i);
   const float angleError=std::fabs(std::remainder(angleAfter-angleBefore,2.0f*3.14159265358979323846f));
   assert(angleError<1.0e-4f);
  }
 }
 ImageGradeParams sat3,sat5;sat3.saturation=3.0f;sat5.saturation=5.0f;
 std::vector<float> sat3Out(s.size()),sat5Out(s.size());
 ImageView sat3View{sat3Out.data(),w,h,w*4},sat5View{sat5Out.data(),w,h,w*4};
 assert(processImageGrade(si,sat3View,sat3));assert(processImageGrade(si,sat5View,sat5));
 assert(sat3Out!=sat5Out);
 ImageGradeParams off;assert(processImageGrade(si,zi,off));assert(z==s);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.4f;semi[i+1]*=.4f;semi[i+2]*=.4f;semi[i+3]=.4f;}
 ConstImageView pi{semi.data(),w,h,w*4};ImageView po{out.data(),w,h,w*4};assert(processImageGrade(pi,po,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.4f)<1e-6f);
 return 0;}
