#include "VHS90RegionEngine.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

#define VHS90_CHECK_MSG(expr) do { if (!(expr)) { std::fprintf(stderr, "VHS90RegionEngineSmoke failed at line %d: %s\\n", __LINE__, #expr); std::abort(); } } while (0)
#include <cmath>
#include <vector>

static float pixel(const std::vector<float>& v, int w, int x, int y, int c) {
    return v[(y*w+x)*4+c];
}

int main() {
    using namespace vhs90;
    constexpr int w=64, h=48;
    std::vector<float> src(w*h*4, 0.25f), a(src.size(),0.0f), b(src.size(),0.0f), c(src.size(),0.0f);
    for (size_t i=3;i<src.size();i+=4) src[i]=1.0f;
    ConstImageView si{src.data(),w,h,w*4};
    ImageView da{a.data(),w,h,w*4}, db{b.data(),w,h,w*4}, dc{c.data(),w,h,w*4};

    DamageRegion top;
    top.kind=RegionKind::TrackingNoise; top.shape=RegionShape::HorizontalBand;
    top.centerX=0.5f; top.centerY=0.25f; top.width=1.0f; top.height=0.20f;
    top.intensity=1.0f; top.maxShiftPx=8.0f; top.noiseAmount=0.8f; top.noiseDensity=1.0f;
    top.seedOffset=10;

    DamageRegion right;
    right.kind=RegionKind::HeadSwitch; right.shape=RegionShape::VerticalBand;
    right.centerX=0.90f; right.centerY=0.5f; right.width=0.20f; right.height=1.0f;
    right.intensity=1.0f; right.maxShiftPx=12.0f; right.dropoutProbability=1.0f;
    right.dropoutAmount=0.5f; right.seedOffset=20;

    DamageRegion regions[2]{top,right};
    RegionEngineParams p; p.regions=regions; p.regionCount=2; p.seed=123; p.frame=7; p.timeSeconds=0.25;
    VHS90_CHECK_MSG(processDamageRegions(si,da,p));
    VHS90_CHECK_MSG(processDamageRegions(si,db,p));
    VHS90_CHECK_MSG(a==b);
    RegionEngineParams pNext=p; pNext.frame=8;
    std::vector<float> next(src.size(),0.0f); ImageView dnext{next.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(si,dnext,pNext));
    VHS90_CHECK_MSG(next!=a);

    VHS90_CHECK_MSG(std::fabs(pixel(a,w,10,12,0)-0.25f)>1e-5f);
    VHS90_CHECK_MSG(std::fabs(pixel(a,w,10,40,0)-0.25f)<1e-5f);
    VHS90_CHECK_MSG(std::fabs(pixel(a,w,60,40,0)-0.25f)>1e-5f);

    DamageRegion full; full.kind=RegionKind::HeadSwitch; full.shape=RegionShape::HorizontalBand;
    full.centerX=0.5f; full.centerY=0.5f; full.width=1.0f; full.height=1.0f;
    full.intensity=1.0f; full.maxShiftPx=4.0f; full.dropoutProbability=1.0f; full.dropoutAmount=0.3f;
    RegionEngineParams pFull; pFull.regions=&full; pFull.regionCount=1; pFull.seed=4; pFull.frame=1;
    VHS90_CHECK_MSG(processDamageRegions(si,dc,pFull));
    VHS90_CHECK_MSG(std::fabs(pixel(c,w,32,24,0)-0.25f)>1e-5f);

    std::vector<float> alphaSrc=src, alphaDst(src.size(),0.0f);
    for (size_t i=3;i<alphaSrc.size();i+=4) alphaSrc[i]=0.0f;
    ConstImageView asi{alphaSrc.data(),w,h,w*4}; ImageView adi{alphaDst.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(asi,adi,pFull));
    for (size_t i=0;i<alphaDst.size();i+=4) {
        VHS90_CHECK_MSG(alphaDst[i]==alphaSrc[i]);
        VHS90_CHECK_MSG(alphaDst[i+1]==alphaSrc[i+1]);
        VHS90_CHECK_MSG(alphaDst[i+2]==alphaSrc[i+2]);
        VHS90_CHECK_MSG(alphaDst[i+3]==0.0f);
    }

    std::vector<float> zero(src.size(),0.0f); ImageView dzi{zero.data(),w,h,w*4};
    RegionEngineParams pZero=p; pZero.globalIntensity=0.0f;
    VHS90_CHECK_MSG(processDamageRegions(si,dzi,pZero));
    VHS90_CHECK_MSG(zero==src);

    // A region whose bottom edge is exactly at the top of the frame stays
    // outside the raster; it must not be clamped back into the visible image.
    DamageRegion offFrame=full;
    offFrame.centerY=-0.10f; offFrame.height=0.20f;
    offFrame.maxShiftPx=0.0f; offFrame.waveAmplitudePx=0.0f;
    offFrame.servoDriftPx=0.0f; offFrame.dropoutProbability=0.0f;
    offFrame.noiseAmount=1.0f; offFrame.noiseDensity=1.0f; offFrame.chromaShiftPx=0.0f;
    RegionEngineParams pOff; pOff.regions=&offFrame; pOff.regionCount=1; pOff.seed=5; pOff.frame=1;
    std::vector<float> offOutput(src.size(),0.0f); ImageView doff{offOutput.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(si,doff,pOff));
    VHS90_CHECK_MSG(offOutput==src);

    // Region geometry supports arbitrary rotation without changing its normalized center/size.
    DamageRegion rotated=full;rotated.kind=RegionKind::TrackingNoise;rotated.shape=RegionShape::Rectangle;
    rotated.centerX=.5f;rotated.centerY=.5f;rotated.width=.65f;rotated.height=.18f;
    rotated.angleRadians=0.78539816339f;rotated.maxShiftPx=0;rotated.servoDriftPx=0;
    rotated.waveAmplitudePx=0;rotated.tearProbability=0;rotated.dropoutProbability=0;
    rotated.noiseDensity=1;rotated.noiseAmount=.5f;rotated.intensity=1;
    DamageRegion unrotated=rotated;unrotated.angleRadians=0;
    RegionEngineParams rotP;rotP.regions=&rotated;rotP.regionCount=1;rotP.seed=71;rotP.frame=9;
    RegionEngineParams unrotP=rotP;unrotP.regions=&unrotated;
    std::vector<float> rotOut(src.size()),unrotOut(src.size());
    ImageView rotView{rotOut.data(),w,h,w*4},unrotView{unrotOut.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(si,rotView,rotP));VHS90_CHECK_MSG(processDamageRegions(si,unrotView,unrotP));
    VHS90_CHECK_MSG(rotOut!=unrotOut);

    // Servo drift profiles crossfade between deterministic epochs instead of
    // re-randomizing the entire row profile on every frame.
    std::vector<float> servoSource(src.size(),0.0f);
    for(int yy=0;yy<h;++yy) for(int xx=0;xx<w;++xx) {
        const size_t ii=(yy*w+xx)*4;
        servoSource[ii]=float(xx)/float(w-1); servoSource[ii+1]=0.25f;
        servoSource[ii+2]=1.0f-float(xx)/float(w-1); servoSource[ii+3]=1.0f;
    }
    ConstImageView servoInput{servoSource.data(),w,h,w*4};
    DamageRegion servo=full; servo.kind=RegionKind::TrackingNoise;
    servo.maxShiftPx=0.0f; servo.waveAmplitudePx=0.0f; servo.servoDriftPx=18.0f;
    servo.servoSmoothnessPx=10.0f; servo.servoEpochFrames=8;
    servo.tearProbability=0.0f; servo.dropoutProbability=0.0f;
    servo.noiseAmount=0.0f; servo.noiseDensity=0.0f; servo.chromaShiftPx=0.0f;
    RegionEngineParams servoA; servoA.regions=&servo; servoA.regionCount=1;
    servoA.seed=55; servoA.frame=20;
    RegionEngineParams servoB=servoA; servoB.frame=21;
    std::vector<float> servoOutA(src.size(),0.0f),servoOutB(src.size(),0.0f);
    ImageView servoViewA{servoOutA.data(),w,h,w*4},servoViewB{servoOutB.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(servoInput,servoViewA,servoA));
    VHS90_CHECK_MSG(processDamageRegions(servoInput,servoViewB,servoB));
    float servoDelta=0.0f;
    for(size_t i=0;i<servoOutA.size();i+=4) servoDelta+=std::fabs(servoOutA[i]-servoOutB[i]);
    VHS90_CHECK_MSG(servoDelta>0.0f);
    VHS90_CHECK_MSG(servoDelta<static_cast<float>(w*h)*0.20f);

    // Tracking-noise texture advects smoothly instead of being independently
    // reseeded each frame (which reads as random flicker).
    DamageRegion advected = full;
    advected.kind = RegionKind::TrackingNoise;
    advected.shape = RegionShape::Rectangle;
    advected.centerX = 0.5f; advected.centerY = 0.5f;
    advected.width = 1.0f; advected.height = 1.0f;
    advected.intensity = 1.0f; advected.maxShiftPx = 0.0f;
    advected.waveAmplitudePx = 0.0f; advected.servoDriftPx = 0.0f;
    advected.tearProbability = 0.0f; advected.dropoutProbability = 0.0f;
    advected.noiseAmount = 0.8f; advected.noiseDensity = 1.0f;
    advected.chromaShiftPx = 0.0f; advected.cellWidthPx = 4.0f; advected.cellHeightPx = 2.0f;
    RegionEngineParams advectP; advectP.regions = &advected; advectP.regionCount = 1;
    advectP.seed = 0xA55Au; advectP.frame = 100; advectP.timeSeconds = 0.0;
    std::vector<float> advect0(src.size()), advectNear(src.size()), advectFar(src.size()), advectRepeat(src.size());
    ImageView advect0View{advect0.data(),w,h,w*4};
    ImageView advectNearView{advectNear.data(),w,h,w*4};
    ImageView advectFarView{advectFar.data(),w,h,w*4};
    ImageView advectRepeatView{advectRepeat.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(si,advect0View,advectP));
    advectP.timeSeconds = 0.02;
    VHS90_CHECK_MSG(processDamageRegions(si,advectNearView,advectP));
    advectP.timeSeconds = 1.0;
    VHS90_CHECK_MSG(processDamageRegions(si,advectFarView,advectP));
    VHS90_CHECK_MSG(processDamageRegions(si,advectRepeatView,advectP));
    VHS90_CHECK_MSG(advectNear != advect0);
    VHS90_CHECK_MSG(advectFar != advect0);
    VHS90_CHECK_MSG(advectRepeat == advectFar);
    double nearDelta = 0.0, farDelta = 0.0;
    for (size_t i=0; i<advect0.size(); i+=4) {
        nearDelta += std::fabs(advectNear[i] - advect0[i]);
        farDelta += std::fabs(advectFar[i] - advect0[i]);
    }
    VHS90_CHECK_MSG(nearDelta > 0.0);
    VHS90_CHECK_MSG(farDelta > nearDelta);

    // Each selectable tracking profile must produce a deterministic, genuinely distinct render.
    std::vector<float> profileSource(src.size(),0.0f);
    for(int py=0;py<h;++py) for(int px=0;px<w;++px) { const size_t pi=(py*w+px)*4; profileSource[pi]=float(px)/float(w-1); profileSource[pi+1]=float(py)/float(h-1); profileSource[pi+2]=1.0f-float(px)/float(w-1); profileSource[pi+3]=1.0f; }
    ConstImageView profileInput{profileSource.data(),w,h,w*4};
    std::vector<float> profileOutputs[8];
    for(int profile=0;profile<8;++profile) {
        profileOutputs[profile].resize(src.size(),0.0f);
        DamageRegion variant=top;
        variant.kind=RegionKind::TrackingNoise; variant.shape=RegionShape::HorizontalBand;
        variant.trackingProfile=static_cast<TrackingProfile>(profile);
        variant.intensity=1.0f; variant.maxShiftPx=0.0f; variant.waveAmplitudePx=8.0f;
        variant.servoDriftPx=14.0f; variant.servoSmoothnessPx=7.0f;
        variant.tearProbability=0.35f; variant.tearShiftPx=24.0f;
        variant.noiseAmount=0.35f; variant.noiseDensity=0.8f; variant.chromaShiftPx=2.0f;
        RegionEngineParams vp; vp.regions=&variant; vp.regionCount=1; vp.seed=991; vp.frame=13; vp.timeSeconds=0.73;
        ImageView vv{profileOutputs[profile].data(),w,h,w*4};
        VHS90_CHECK_MSG(processDamageRegions(profileInput,vv,vp));
        std::vector<float> repeat(src.size(),0.0f); ImageView rv{repeat.data(),w,h,w*4};
        VHS90_CHECK_MSG(processDamageRegions(profileInput,rv,vp)); VHS90_CHECK_MSG(repeat==profileOutputs[profile]);
    }
    for(int aidx=0;aidx<8;++aidx) for(int bidx=aidx+1;bidx<8;++bidx)
        VHS90_CHECK_MSG(profileOutputs[aidx]!=profileOutputs[bidx]);

    // VHS-main tracking noise must move as a continuous spatial field, not
    // flicker from a frame-seeded random replacement.
    DamageRegion movingVhs=top;
    movingVhs.trackingProfile=TrackingProfile::VhsMainTracking;
    movingVhs.centerY=0.5f; movingVhs.height=1.0f;
    movingVhs.maxShiftPx=9.0f; movingVhs.waveAmplitudePx=0.0f;
    movingVhs.servoDriftPx=0.0f; movingVhs.tearProbability=0.0f;
    movingVhs.noiseAmount=0.35f; movingVhs.noiseDensity=0.0f;
    movingVhs.chromaShiftPx=0.0f;
    RegionEngineParams movingVhsP; movingVhsP.regions=&movingVhs; movingVhsP.regionCount=1;
    movingVhsP.seed=0x90A11E5Eu; movingVhsP.frame=13; movingVhsP.timeSeconds=0.20;
    std::vector<float> vhsT0(src.size()),vhsT1(src.size()),vhsT2(src.size()),vhsRepeat(src.size());
    ImageView vhsV0{vhsT0.data(),w,h,w*4},vhsV1{vhsT1.data(),w,h,w*4};
    ImageView vhsV2{vhsT2.data(),w,h,w*4},vhsVR{vhsRepeat.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(profileInput,vhsV0,movingVhsP));
    movingVhsP.timeSeconds=0.22;
    VHS90_CHECK_MSG(processDamageRegions(profileInput,vhsV1,movingVhsP));
    movingVhsP.timeSeconds=0.90;
    VHS90_CHECK_MSG(processDamageRegions(profileInput,vhsV2,movingVhsP));
    VHS90_CHECK_MSG(processDamageRegions(profileInput,vhsVR,movingVhsP));
    VHS90_CHECK_MSG(vhsT0!=vhsT1 && vhsT1!=vhsT2 && vhsT2==vhsRepeat);
    double vhsNearDelta=0.0,vhsFarDelta=0.0;
    for(size_t i=0;i<vhsT0.size();i+=4) {
        vhsNearDelta+=std::fabs(vhsT1[i]-vhsT0[i]);
        vhsFarDelta+=std::fabs(vhsT2[i]-vhsT0[i]);
    }
    VHS90_CHECK_MSG(vhsNearDelta>0.0 && vhsFarDelta>vhsNearDelta);

    // Source-backed head-switch presets use the same editable region geometry.
    DamageRegion headRift=full;
    headRift.kind=RegionKind::HeadSwitch; headRift.shape=RegionShape::HorizontalBand;
    headRift.headSwitchProfile=HeadSwitchProfile::Vhs6Rift;
    headRift.centerX=0.5f; headRift.centerY=0.5f; headRift.width=1.0f; headRift.height=1.0f;
    headRift.intensity=1.0f; headRift.maxShiftPx=18.0f; headRift.waveAmplitudePx=0.0f;
    headRift.servoDriftPx=0.0f; headRift.tearProbability=0.0f;
    headRift.dropoutProbability=0.0f; headRift.noiseAmount=0.0f;
    headRift.noiseDensity=0.0f; headRift.chromaShiftPx=0.0f;
    RegionEngineParams headRiftP; headRiftP.regions=&headRift; headRiftP.regionCount=1;
    headRiftP.seed=0x6A6Au; headRiftP.frame=11; headRiftP.timeSeconds=0.20;
    std::vector<float> headRiftA(src.size()),headRiftB(src.size()),headRiftRepeat(src.size());
    ImageView headRiftAV{headRiftA.data(),w,h,w*4},headRiftBV{headRiftB.data(),w,h,w*4};
    ImageView headRiftRepeatV{headRiftRepeat.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(profileInput,headRiftAV,headRiftP));
    headRiftP.timeSeconds=0.25;
    VHS90_CHECK_MSG(processDamageRegions(profileInput,headRiftBV,headRiftP));
    VHS90_CHECK_MSG(processDamageRegions(profileInput,headRiftRepeatV,headRiftP));
    VHS90_CHECK_MSG(headRiftA!=headRiftB && headRiftB==headRiftRepeat);

    DamageRegion headNtsc=headRift;
    headNtsc.headSwitchProfile=HeadSwitchProfile::NtscR;
    headNtsc.maxShiftPx=14.0f; headNtsc.waveAmplitudePx=6.0f;
    headNtsc.servoSmoothnessPx=12.0f; headNtsc.noiseAmount=0.30f;
    headNtsc.snowIntensity=0.8f; headNtsc.snowAnisotropy=0.4f;
    RegionEngineParams headNtscP=headRiftP; headNtscP.regions=&headNtsc;
    std::vector<float> headNtscOut(src.size());
    ImageView headNtscView{headNtscOut.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(profileInput,headNtscView,headNtscP));
    VHS90_CHECK_MSG(headNtscOut!=headRiftB);

    DamageRegion headMoving=headRift;
    headMoving.motionYPerSecond=0.25f;
    RegionEngineParams headMovingP=headRiftP; headMovingP.regions=&headMoving;
    headMovingP.timeSeconds=0.25;
    std::vector<float> headMovingOut(src.size());
    ImageView headMovingView{headMovingOut.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(profileInput,headMovingView,headMovingP));
    VHS90_CHECK_MSG(headMovingOut!=headRiftB);

    // NTSC-R snow intensity and anisotropy are independent controls.
    DamageRegion ntscSnow = full;
    ntscSnow.kind = RegionKind::TrackingNoise;
    ntscSnow.trackingProfile = TrackingProfile::NtscRTracking;
    ntscSnow.shape = RegionShape::HorizontalBand;
    ntscSnow.centerX = 0.5f; ntscSnow.centerY = 0.5f;
    ntscSnow.width = 1.0f; ntscSnow.height = 1.0f;
    ntscSnow.intensity = 1.0f; ntscSnow.maxShiftPx = 0.0f;
    ntscSnow.waveAmplitudePx = 0.0f; ntscSnow.servoDriftPx = 0.0f;
    ntscSnow.tearProbability = 0.0f; ntscSnow.dropoutProbability = 0.0f;
    ntscSnow.noiseAmount = 0.0f; ntscSnow.snowIntensity = 0.8f;
    ntscSnow.snowAnisotropy = 0.0f; ntscSnow.noiseDriftXCellsPerSecond = 1.35f;
    ntscSnow.noiseDriftYCellsPerSecond = 0.42f;
    RegionEngineParams snowP; snowP.regions = &ntscSnow; snowP.regionCount = 1;
    snowP.seed = 0xC0FFEEu; snowP.frame = 8; snowP.timeSeconds = 0.4;
    std::vector<float> snowIndependent(src.size()), snowClumped(src.size()), snowOff(src.size());
    ImageView snowIndependentView{snowIndependent.data(),w,h,w*4};
    ImageView snowClumpedView{snowClumped.data(),w,h,w*4};
    ImageView snowOffView{snowOff.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(profileInput,snowIndependentView,snowP));
    ntscSnow.snowAnisotropy = 0.95f;
    VHS90_CHECK_MSG(processDamageRegions(profileInput,snowClumpedView,snowP));
    ntscSnow.snowIntensity = 0.0f;
    VHS90_CHECK_MSG(processDamageRegions(profileInput,snowOffView,snowP));
    VHS90_CHECK_MSG(snowIndependent != snowOff);
    VHS90_CHECK_MSG(snowClumped != snowIndependent);
    std::vector<float> snowMoving(src.size(),0.0f); ImageView snowMovingView{snowMoving.data(),w,h,w*4};
    RegionEngineParams snowMoveP=snowP; snowMoveP.timeSeconds=0.9;
    VHS90_CHECK_MSG(processDamageRegions(profileInput,snowMovingView,snowMoveP));
    VHS90_CHECK_MSG(snowMoving != snowIndependent);

    // Four differently typed tracking regions can coexist in one render stack.
    DamageRegion trackingStack[4];
    for(int li=0;li<4;++li) {
        DamageRegion& layer=trackingStack[li]; layer.kind=RegionKind::TrackingNoise;
        layer.shape=li==3?RegionShape::VerticalBand:RegionShape::HorizontalBand;
        layer.trackingProfile=static_cast<TrackingProfile>(li);
        layer.centerX=li==3?.82f:.5f; layer.centerY=.18f+float(li)*.2f;
        layer.width=li==3?.18f:1.0f; layer.height=li==3?1.0f:.16f;
        layer.intensity=.7f; layer.maxShiftPx=8.0f+float(li)*3.0f;
        layer.servoDriftPx=6.0f+float(li)*2.0f; layer.waveAmplitudePx=5.0f+float(li)*2.0f;
        layer.tearProbability=.2f; layer.tearShiftPx=18.0f; layer.noiseAmount=.2f; layer.noiseDensity=.35f;
        layer.seedOffset=std::uint32_t(0x1000+li*0x100);
    }
    std::vector<float> stackA(src.size()),stackB(src.size());
    ImageView stackAv{stackA.data(),w,h,w*4},stackBv{stackB.data(),w,h,w*4};
    RegionEngineParams stackParams; stackParams.regions=trackingStack; stackParams.regionCount=4;
    stackParams.seed=0x9911u; stackParams.frame=24; stackParams.timeSeconds=.9;
    VHS90_CHECK_MSG(processDamageRegions(profileInput,stackAv,stackParams));
    VHS90_CHECK_MSG(processDamageRegions(profileInput,stackBv,stackParams));
    VHS90_CHECK_MSG(stackA==stackB && stackA!=profileSource);

    // Signed chroma displacement must work in both directions.
    std::vector<float> chromaSrc(src.size()),chromaOut(src.size(),0.0f);
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        size_t i=(y*w+x)*4;
        chromaSrc[i]=float(x)/float(w-1); chromaSrc[i+1]=0.25f;
        chromaSrc[i+2]=1.0f-float(x)/float(w-1); chromaSrc[i+3]=1.0f;
    }
    ConstImageView csi{chromaSrc.data(),w,h,w*4}; ImageView cdo{chromaOut.data(),w,h,w*4};
    DamageRegion signedChroma=full;
    signedChroma.maxShiftPx=0.0f; signedChroma.waveAmplitudePx=0.0f;
    signedChroma.servoDriftPx=0.0f; signedChroma.tearProbability=0.0f;
    signedChroma.noiseAmount=0.0f; signedChroma.noiseDensity=0.0f;
    signedChroma.dropoutProbability=0.0f; signedChroma.chromaShiftPx=-2.0f;
    RegionEngineParams pChroma; pChroma.regions=&signedChroma; pChroma.regionCount=1;
    pChroma.seed=8; pChroma.frame=2;
    VHS90_CHECK_MSG(processDamageRegions(csi,cdo,pChroma));
    const size_t mid=(24*w+32)*4;
    VHS90_CHECK_MSG(chromaOut[mid]>chromaSrc[mid]);
    VHS90_CHECK_MSG(chromaOut[mid+2]>chromaSrc[mid+2]);
    std::vector<float> positiveChroma(src.size(),0.0f);
    ImageView positiveView{positiveChroma.data(),w,h,w*4};
    signedChroma.chromaShiftPx=2.0f;
    VHS90_CHECK_MSG(processDamageRegions(csi,positiveView,pChroma));
    VHS90_CHECK_MSG(positiveChroma[mid]<chromaSrc[mid]);
    VHS90_CHECK_MSG(positiveChroma[mid+2]<chromaSrc[mid+2]);
    // Tracking bands must travel through the frame and wrap back to their
    // starting position. Hold frame/seed fixed to isolate spatial motion.
    DamageRegion moving=top;
    moving.centerX=.5f; moving.centerY=.5f; moving.width=1.0f; moving.height=.20f;
    moving.motionYPerSecond=.25f; moving.maxShiftPx=0.0f;
    moving.waveAmplitudePx=0.0f; moving.servoDriftPx=0.0f;
    moving.tearProbability=0.0f; moving.dropoutProbability=0.0f;
    moving.noiseAmount=.8f; moving.noiseDensity=1.0f; moving.chromaShiftPx=0.0f;
    moving.noiseDriftXCellsPerSecond=0.0f; moving.noiseDriftYCellsPerSecond=0.0f;
    moving.noiseDriftXCellsPerSecond=0.0f; moving.noiseDriftYCellsPerSecond=0.0f;
    RegionEngineParams moveP; moveP.regions=&moving; moveP.regionCount=1;
    moveP.seed=404; moveP.frame=17; moveP.timeSeconds=0.0;
    std::vector<float> moveStart(src.size()),moveMiddle(src.size()),moveLoop(src.size());
    ImageView moveStartView{moveStart.data(),w,h,w*4};
    ImageView moveMiddleView{moveMiddle.data(),w,h,w*4};
    ImageView moveLoopView{moveLoop.data(),w,h,w*4};
    VHS90_CHECK_MSG(processDamageRegions(si,moveStartView,moveP));
    moveP.timeSeconds=2.0;
    VHS90_CHECK_MSG(processDamageRegions(si,moveMiddleView,moveP));
    VHS90_CHECK_MSG(moveStart!=moveMiddle);
    moveP.timeSeconds=4.8; // (1 + region height) / 0.25 seconds
    VHS90_CHECK_MSG(processDamageRegions(si,moveLoopView,moveP));
    float loopError=0.0f;
    for(size_t i=0;i<moveStart.size();++i) loopError+=std::fabs(moveStart[i]-moveLoop[i]);
    VHS90_CHECK_MSG(loopError<1.0e-3f);

    return 0;
}
