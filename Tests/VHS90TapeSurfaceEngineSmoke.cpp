#include "VHS90TapeSurfaceEngine.h"
#include <cassert>
#include <cmath>
#include <vector>
int main(){using namespace vhs90;constexpr int w=48,h=32;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;s[i]=.25f+.5f*float(x)/float(w-1);s[i+1]=.35f;s[i+2]=.65f-.3f*float(y)/float(h-1);s[i+3]=1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4};
 TapeSurfaceParams p;p.seed=17;p.frame=12;p.defectDensity=.9f;p.edgeLift=.2f;p.pinholeAmount=.5f;p.flakeAmount=.6f;p.moldAmount=.7f;p.vaselineAmount=.4f;assert(processTapeSurface(si,ai,p));assert(processTapeSurface(si,bi,p));assert(a==b);
 p.frame=13;assert(processTapeSurface(si,ci,p));assert(a!=c);
 TapeSurfaceParams off;off.defectDensity=0;off.oxideLoss=0;off.scratchDensity=0;off.creaseStrength=0;off.debrisAmount=0;off.edgeLift=0;off.surfaceRoughness=0;off.moldAmount=0;off.vaselineAmount=0;off.pinholeAmount=0;off.flakeAmount=0;std::fill(b.begin(),b.end(),0);assert(processTapeSurface(si,bi,off));assert(b==s);
 std::vector<float>semi=s,out(s.size());for(size_t i=0;i<semi.size();i+=4){semi[i]*=.4f;semi[i+1]*=.4f;semi[i+2]*=.4f;semi[i+3]=.4f;}
 ConstImageView ssi{semi.data(),w,h,w*4};ImageView so{out.data(),w,h,w*4};p.frame=12;p.premultipliedAlpha=true;assert(processTapeSurface(ssi,so,p));
 for(size_t i=3;i<out.size();i+=4)assert(std::fabs(out[i]-.4f)<1e-6f);
 for(size_t i=0;i<out.size();i+=4)assert(std::isfinite(out[i])&&std::isfinite(out[i+1])&&std::isfinite(out[i+2]));
 constexpr int sw=32,sh=8;std::vector<float>stripe(sw*sh*4),soft(sw*sh*4);
 for(int yy=0;yy<sh;++yy)for(int xx=0;xx<sw;++xx){size_t k=(yy*sw+xx)*4;float v=(xx&1)?0.9f:0.1f;stripe[k]=stripe[k+1]=stripe[k+2]=v;stripe[k+3]=1.0f;}
 ConstImageView sti{stripe.data(),sw,sh,sw*4};ImageView sto{soft.data(),sw,sh,sw*4};
 TapeSurfaceParams spacing;spacing.defectDensity=0;spacing.surfaceRoughness=0;spacing.edgeLift=1;assert(processTapeSurface(sti,sto,spacing));
 float before=0,after=0;for(int xx=0;xx<sw-1;++xx){before+=std::fabs(stripe[xx*4]-stripe[(xx+1)*4]);after+=std::fabs(soft[xx*4]-soft[(xx+1)*4]);}
 assert(after<before*0.5f);
 const auto x=sampleTapeSurface(.31f,.42f,p),y=sampleTapeSurface(.31f,.42f,p);assert(x.magnetization==y.magnetization&&x.spacingLift==y.spacingLift);
 const auto physicalA=sampleTapeSurfaceAtMm(128.375,4.25f,p),physicalB=sampleTapeSurfaceAtMm(128.375,4.25f,p),physicalC=sampleTapeSurfaceAtMm(128.475,4.25f,p);
 assert(physicalA.magnetization==physicalB.magnetization&&physicalA.spacingLift==physicalB.spacingLift&&physicalA.shed==physicalB.shed);
 assert(std::isfinite(physicalA.magnetization)&&std::isfinite(physicalA.spacingLift)&&std::isfinite(physicalA.film)&&std::isfinite(physicalA.shed));
 assert(physicalA.magnetization!=physicalC.magnetization||physicalA.spacingLift!=physicalC.spacingLift||physicalA.film!=physicalC.film);
 const auto spSample=sampleTapeSurface(.31f,.42f,p);TapeSurfaceParams ep=p;ep.tapeSpeed=2;const auto epSample=sampleTapeSurface(.31f,.42f,ep);
 assert(spSample.magnetization!=epSample.magnetization||spSample.spacingLift!=epSample.spacingLift||spSample.film!=epSample.film);
 TapeSurfaceParams detailParams=p;detailParams.pinholeAmount=1.0f;detailParams.scratchDensity=4.0f;detailParams.flakeAmount=1.0f;
 bool detailChanges=false,flakeChanges=false;
 for(int n=0;n<400&&!detailChanges;++n){const double xx=double(n)*0.37;const auto full=sampleTapeSurfaceAtMm(xx,4.25f,detailParams,true,true),coarse=sampleTapeSurfaceAtMm(xx,4.25f,detailParams,false,true);detailChanges=full.magnetization!=coarse.magnetization||full.spacingLift!=coarse.spacingLift;}
 for(int n=0;n<400&&!flakeChanges;++n){const double xx=double(n)*0.37;const auto full=sampleTapeSurfaceAtMm(xx,4.25f,detailParams,false,true),noFlakes=sampleTapeSurfaceAtMm(xx,4.25f,detailParams,false,false);flakeChanges=full.shed!=noFlakes.shed||full.magnetization!=noFlakes.magnetization;}
 assert(detailChanges&&flakeChanges);
 assert(std::fabs(ntscrtTapeWearAmount(0.0f))<1e-6f&&std::fabs(ntscrtTapeWearAmount(1.0f)-1.85f)<1e-5f);
 assert(std::fabs(ntscrtTapeDropoutAmount(1.0f)-3.0f)<1e-5f&&std::fabs(ntscrtTapeScratchAmount(1.0f)-4.0f)<1e-5f);
 assert(std::fabs(ntscrtTapeEdgeAmount(1.0f)-1.45f)<1e-5f&&std::fabs(ntscrtTapeCreaseAmount(1.0f)-5.0f)<1e-5f);
 assert(ntscrtTapeSeed(1u)==7936u);
 bool trackingChanges=false;TapeSurfaceParams tracked=p;tracked.trackingOffsetTracks=1.0f;
 for(int n=0;n<120&&!trackingChanges;++n){const float u=float(n)/120.0f;const auto centered=sampleTapeSurface(u,.42f,p),shifted=sampleTapeSurface(u,.42f,tracked);trackingChanges=centered.magnetization!=shifted.magnetization||centered.spacingLift!=shifted.spacingLift||centered.film!=shifted.film;}
 assert(trackingChanges);
 assert(std::fabs(tapeWidthMm()-12.65)<1e-6&&std::fabs(tapeTrackStartMm()-1.0)<0.02&&std::fabs(tapeTrackLengthMm()-96.7)<0.1);
 assert(std::fabs(tapeTrackPitchUm(0)-57.9)<0.5&&std::fabs(tapeTrackPitchUm(1)-28.95)<0.5&&std::fabs(tapeTrackPitchUm(2)-19.3)<0.5);
 assert(std::fabs(tapeTrackOffsetLines(0)-1.5)<0.02&&std::fabs(tapeTrackOffsetLines(1)-0.75)<0.02&&std::fabs(tapeTrackOffsetLines(2)-0.5)<0.02);
 const auto top=tapeTrackPoint(0,tapeTrackLengthMm(),0.0,0);assert(std::fabs(top.y-tapeTrackStartMm()-10.1)<0.1);
 const auto sameY0=tapeTrackPoint(10,40.0,0.0,0),sameY1=tapeTrackPoint(400,40.0,0.0,0);assert(std::fabs(sameY0.y-sameY1.y)<1e-9);
 const TapeDamageControl controls[]={TapeDamageControl::Wear,TapeDamageControl::Dropouts,TapeDamageControl::Scratches,TapeDamageControl::EdgeDamage,TapeDamageControl::Creases,TapeDamageControl::TransportFlutter};
 for(const auto control:controls)for(int k=0;k<=20;++k){const float knob=float(k)/20.0f;const float amount=ntscrtTapeAmount(control,knob);assert(std::fabs(ntscrtTapeKnobForAmount(control,amount)-knob)<1e-5f);}
 assert(std::fabs(ntscrtTapeAmountIn027(TapeDamageControl::Wear,1.0f)-1.85f)<1e-5f);
 assert(std::fabs(ntscrtTapeAmountIn027(TapeDamageControl::Dropouts,1.0f)-3.0f)<1e-5f);
 assert(std::fabs(ntscrtTapeAmountIn027(TapeDamageControl::Scratches,1.0f)-4.0f)<1e-5f);
 assert(std::fabs(ntscrtTapeAmountIn027(TapeDamageControl::EdgeDamage,1.0f)-1.45f)<1e-5f);
 assert(std::fabs(ntscrtTapeAmountIn027(TapeDamageControl::Creases,1.0f)-1.0f)<1e-5f);
 assert(std::fabs(ntscrtTransportFlutterAmount(1.0f)-5.0f)<1e-5f);
 TapeMaterialControls material;material.master=100.0f;material.frame=23;material.cassetteSeed=1;material.tapeSpeed=2;material.trackingOffsetTracks=1.25f;
 const auto mapped=makeTapeSurfaceParams(material);
 assert(mapped.seed==7936u&&mapped.frame==23&&mapped.tapeSpeed==2u&&std::fabs(mapped.trackingOffsetTracks-1.25f)<1e-6f);
 assert(std::fabs(mapped.oxideLoss-1.85f)<1e-5f&&std::fabs(mapped.defectDensity-3.0f)<1e-5f);
 assert(std::fabs(mapped.scratchDensity-4.0f)<1e-5f&&std::fabs(mapped.creaseStrength-5.0f)<1e-5f&&std::fabs(mapped.edgeLift-1.45f)<1e-5f);
 assert(std::fabs(mapped.moldAmount-1.0f)<1e-6f&&std::fabs(mapped.vaselineAmount-1.0f)<1e-6f&&std::fabs(mapped.debrisAmount-1.0f)<1e-6f);
 material.wear=0.0f;material.dropouts=0.0f;material.scratches=0.0f;material.creases=0.0f;material.edgeDamage=0.0f;material.mold=0.0f;material.vaseline=0.0f;material.sticky=0.0f;
 const auto clean=makeTapeSurfaceParams(material);assert(clean.oxideLoss==0.0f&&clean.defectDensity==0.0f&&clean.scratchDensity==0.0f&&clean.creaseStrength==0.0f&&clean.edgeLift==0.0f&&clean.moldAmount==0.0f&&clean.vaselineAmount==0.0f&&clean.debrisAmount==0.0f);
 material.master=50.0f;material.wear=100.0f;material.dropouts=0.0f;material.cassetteSeed=7;material.tapeSpeed=99;material.trackingOffsetTracks=-99.0f;
 const auto independent=makeTapeSurfaceParams(material);assert(std::fabs(independent.oxideLoss-ntscrtTapeWearAmount(.5f))<1e-5f&&independent.defectDensity==0.0f&&independent.seed==55450u&&independent.tapeSpeed==2u&&independent.trackingOffsetTracks==-8.0f);
 return 0;}
