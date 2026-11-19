#include "VHS90CompositeEngine.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    using namespace vhs90;
    constexpr int w=96,h=8;
    std::vector<float> src(w*h*4), boxOut(src.size()), boxAgain(src.size()), zeroOut(src.size());
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        size_t i=(y*w+x)*4;
        const bool red=((x/2+y)&1)==0;
        src[i]=red?0.9f:0.05f;
        src[i+1]=0.15f+0.05f*float(y);
        src[i+2]=red?0.08f:0.85f;
        src[i+3]=1.0f;
    }
    ConstImageView si{src.data(),w,h,w*4};
    ImageView bo{boxOut.data(),w,h,w*4}, ba{boxAgain.data(),w,h,w*4}, zo{zeroOut.data(),w,h,w*4};

    CompositeParams p; p.strength=1.0f; p.outputBandwidthEnabled=false; p.demodulator=CompositeDemodulator::Box;
    p.phase=CompositePhase::Quadrature90; p.frame=3;
    assert(processCompositeRoundTrip(si,bo,p));
    assert(processCompositeRoundTrip(si,ba,p));
    assert(boxOut==boxAgain);
    bool changed=false;
    for(size_t i=0;i<boxOut.size();i+=4) if(std::fabs(boxOut[i]-src[i])>1e-4f) { changed=true; break; }
    assert(changed);

    CompositeParams pZero=p; pZero.strength=0.0f;
    assert(processCompositeRoundTrip(si,zo,pZero));
    assert(zeroOut==src);

    std::vector<float> phase0(src.size()), phase180(src.size());
    ImageView d0{phase0.data(),w,h,w*4}, d180{phase180.data(),w,h,w*4};
    CompositeParams pp=p; pp.phase=CompositePhase::Degrees0;
    assert(processCompositeRoundTrip(si,d0,pp));
    pp.phase=CompositePhase::Degrees180;
    assert(processCompositeRoundTrip(si,d180,pp));
    assert(phase0!=phase180);

    std::vector<float> one(src.size()), two(src.size());
    ImageView d1{one.data(),w,h,w*4}, d2{two.data(),w,h,w*4};
    pp=p; pp.demodulator=CompositeDemodulator::OneLineComb;
    assert(processCompositeRoundTrip(si,d1,pp));
    pp.demodulator=CompositeDemodulator::TwoLineComb;
    assert(processCompositeRoundTrip(si,d2,pp));
    assert(one!=two);

    // RF instability is an independent post-decode module.
    std::vector<float> rfOnly(src.size());
    ImageView rfOnlyView{rfOnly.data(),w,h,w*4};
    CompositeParams rf=p;rf.compositeSignalStrength=0.0f;rf.inputBandwidthEnabled=false;rf.outputBandwidthEnabled=false;
    rf.rfInstabilityEnabled=true;rf.rfInstability.drift=.65f;rf.rfInstability.hum=.4f;rf.rfInstability.amNonlinearity=.7f;
    assert(processCompositeRoundTrip(si,rfOnlyView,rf));assert(rfOnly!=src);

    // Barash-style independent Cr/Cb displacement is applied before composite encoding.
    std::vector<float> chromaShiftOnly(src.size());
    ImageView chromaShiftOnlyView{chromaShiftOnly.data(),w,h,w*4};
    CompositeParams chromaShift=p;chromaShift.compositeSignalStrength=0.0f;chromaShift.inputBandwidthEnabled=false;chromaShift.outputBandwidthEnabled=false;
    chromaShift.chromaChannelShiftEnabled=true;chromaShift.chromaChannelShift.shiftPixels=3.0f;
    assert(processCompositeRoundTrip(si,chromaShiftOnlyView,chromaShift));assert(chromaShiftOnly!=src);

    // ComfyUI RetroTape ringing is integrated before composite modulation.
    std::vector<float> ringOnly(src.size());
    ImageView ringOnlyView{ringOnly.data(),w,h,w*4};
    CompositeParams ring=p;ring.compositeSignalStrength=0.0f;ring.inputBandwidthEnabled=false;ring.outputBandwidthEnabled=false;
    ring.ringingEnabled=true;ring.ringing.strength=.7f;ring.ringing.power=2.0f;
    assert(processCompositeRoundTrip(si,ringOnlyView,ring));assert(ringOnly!=src);

    // Barash horizontal luma downsample/upsample stage runs in the shared pipeline.
    std::vector<float> detailOnly(src.size());
    ImageView detailOnlyView{detailOnly.data(),w,h,w*4};
    CompositeParams detail=p;detail.compositeSignalStrength=0.0f;detail.inputBandwidthEnabled=false;detail.outputBandwidthEnabled=false;
    detail.tapeDetailEnabled=true;detail.tapeDetail.detail=.35f;detail.tapeDetail.minimumWidth=1;
    assert(processCompositeRoundTrip(si,detailOnlyView,detail));assert(detailOnly!=src);

    // Barash-derived edge/fade/grade/vignette/flicker stage is wired to the shared pipeline.
    std::vector<float> gradeOnly(src.size());
    ImageView gradeOnlyView{gradeOnly.data(),w,h,w*4};
    CompositeParams grade=p;grade.compositeSignalStrength=0.0f;grade.inputBandwidthEnabled=false;grade.outputBandwidthEnabled=false;
    grade.imageGradeEnabled=true;grade.imageGrade.edgeEnhancement=.5f;grade.imageGrade.fade=.3f;
    grade.imageGrade.contrast=1.15f;grade.imageGrade.saturation=.85f;grade.imageGrade.vignette=.4f;grade.imageGrade.flicker=.2f;
    assert(processCompositeRoundTrip(si,gradeOnlyView,grade));assert(gradeOnly!=src);

    // ComfyUI delayed luma echoes and I/Q chroma delay are connected to the shared pipeline.
    std::vector<float> signalGhostOnly(src.size());
    ImageView signalGhostOnlyView{signalGhostOnly.data(),w,h,w*4};
    CompositeParams signalGhost=p;signalGhost.compositeSignalStrength=0.0f;signalGhost.inputBandwidthEnabled=false;signalGhost.outputBandwidthEnabled=false;
    signalGhost.signalGhostEnabled=true;signalGhost.signalGhost.amount=.65f;signalGhost.signalGhost.chromaDelayPixels=5.0f;
    assert(processCompositeRoundTrip(si,signalGhostOnlyView,signalGhost));assert(signalGhostOnly!=src);

    // ComfyUI dropout bands and sparse luma speckles are integrated after receiver decode.
    std::vector<float> dropoutOnly(src.size());
    ImageView dropoutOnlyView{dropoutOnly.data(),w,h,w*4};
    CompositeParams dropout=p;dropout.compositeSignalStrength=0.0f;dropout.inputBandwidthEnabled=false;dropout.outputBandwidthEnabled=false;
    dropout.dropoutEnabled=true;dropout.dropouts.amount=.8f;dropout.dropouts.seed=77;dropout.dropouts.frame=5;
    assert(processCompositeRoundTrip(si,dropoutOnlyView,dropout));assert(dropoutOnly!=src);

    // Full waveform composite model uses the 525x910 signal path when explicitly selected.
    std::vector<float> waveformOnly(src.size()),waveformRepeat(src.size());
    ImageView waveformOnlyView{waveformOnly.data(),w,h,w*4},waveformRepeatView{waveformRepeat.data(),w,h,w*4};
    CompositeParams waveform=p;waveform.compositeSignalStrength=1.0f;waveform.waveformCompositeEnabled=true;
    waveform.inputBandwidthEnabled=false;waveform.outputBandwidthEnabled=false;waveform.frame=11;
    assert(processCompositeRoundTrip(si,waveformOnlyView,waveform));assert(processCompositeRoundTrip(si,waveformRepeatView,waveform));
    assert(waveformOnly==waveformRepeat&&waveformOnly!=src);

    // Temporal ghost consumes an explicitly supplied, already-processed previous frame.
    std::vector<float> previous(src.size()),temporalOnly(src.size());
    for(size_t i=0;i<previous.size();i+=4){previous[i]=.1f;previous[i+1]=.8f;previous[i+2]=.2f;previous[i+3]=1.0f;}
    ConstImageView previousView{previous.data(),w,h,w*4};ImageView temporalView{temporalOnly.data(),w,h,w*4};
    CompositeParams temporal=p;temporal.compositeSignalStrength=0.0f;temporal.inputBandwidthEnabled=false;temporal.outputBandwidthEnabled=false;
    temporal.temporalGhostEnabled=true;temporal.previousFrame=previousView;temporal.previousFrameIndex=10;temporal.temporalGhost.amount=.5f;
    assert(processCompositeRoundTrip(si,temporalView,temporal));assert(temporalOnly!=src);

    // Receiver AFC hunt, chroma flutter and Y/C crosstalk run as an independent stage.
    std::vector<float> receiverOnly(src.size());
    ImageView receiverOnlyView{receiverOnly.data(),w,h,w*4};
    CompositeParams receiver=p;receiver.compositeSignalStrength=0.0f;receiver.inputBandwidthEnabled=false;receiver.outputBandwidthEnabled=false;
    receiver.receiverInstabilityEnabled=true;receiver.receiverInstability.afcHunt=.45f;
    receiver.receiverInstability.chromaFlutter=.6f;receiver.receiverInstability.ycCrosstalk=.5f;
    assert(processCompositeRoundTrip(si,receiverOnlyView,receiver));assert(receiverOnly!=src);

    // Delayed RF multipath echoes are separate from previous-frame ghosting.
    std::vector<float> multipathOnly(src.size());
    ImageView multipathOnlyView{multipathOnly.data(),w,h,w*4};
    CompositeParams multipath=p;multipath.compositeSignalStrength=0.0f;multipath.inputBandwidthEnabled=false;multipath.outputBandwidthEnabled=false;
    multipath.multipathEchoEnabled=true;multipath.multipathEcho.gain=.35f;multipath.multipathEcho.delayPixels=12.0f;multipath.multipathEcho.ensemble=.7f;
    assert(processCompositeRoundTrip(si,multipathOnlyView,multipath));assert(multipathOnly!=src);

    // Sparse impulse bursts are a separate signal-damage stage.
    std::vector<float> impulseOnly(src.size());
    ImageView impulseOnlyView{impulseOnly.data(),w,h,w*4};
    CompositeParams impulse=p;impulse.compositeSignalStrength=0.0f;impulse.inputBandwidthEnabled=false;impulse.outputBandwidthEnabled=false;
    impulse.impulseNoiseEnabled=true;impulse.impulseNoise.amount=.8f;impulse.impulseNoise.seed=7;impulse.impulseNoise.frame=11;
    assert(processCompositeRoundTrip(si,impulseOnlyView,impulse));assert(impulseOnly!=src);

    // Post-signal modules can run without forcing the composite encoder/decoder.
    std::vector<float> postOnly(src.size()),noiseOnly(src.size());
    ImageView postOnlyView{postOnly.data(),w,h,w*4},noiseOnlyView{noiseOnly.data(),w,h,w*4};
    CompositeParams post=p;post.compositeSignalStrength=0.0f;post.inputBandwidthEnabled=false;post.outputBandwidthEnabled=false;
    post.chromaSubsampleEnabled=true;post.chromaSubsample.amount=.8f;
    assert(processCompositeRoundTrip(si,postOnlyView,post));assert(postOnly!=src);
    post=p;post.compositeSignalStrength=0.0f;post.inputBandwidthEnabled=false;post.outputBandwidthEnabled=false;
    post.analogNoiseEnabled=true;post.analogNoise.lumaAmount=.5f;post.analogNoise.chromaAmount=.4f;
    assert(processCompositeRoundTrip(si,noiseOnlyView,post));assert(noiseOnly!=src);

    std::vector<float> verticalLumaComposite(src.size());
    ImageView verticalLumaCompositeView{verticalLumaComposite.data(),w,h,w*4};
    CompositeParams verticalLumaStage=p;verticalLumaStage.outputBandwidthEnabled=true;
    verticalLumaStage.outputBandwidth.iCutoffHz=0;verticalLumaStage.outputBandwidth.qCutoffHz=0;
    verticalLumaStage.outputBandwidth.lumaCutoffHz=0;verticalLumaStage.outputBandwidth.verticalLumaRadiusRows=1;
    verticalLumaStage.outputBandwidth.verticalLumaSigmaRows=.8f;
    assert(processCompositeRoundTrip(si,verticalLumaCompositeView,verticalLumaStage));
    assert(verticalLumaComposite!=boxOut);

    std::vector<float> geometryComposite(src.size()),scanlineComposite(src.size());
    ImageView geometryCompositeView{geometryComposite.data(),w,h,w*4},scanlineCompositeView{scanlineComposite.data(),w,h,w*4};
    CompositeParams geometryStage=p;geometryStage.geometryEnabled=true;geometryStage.geometry.lineJitterPixels=1.0f;
    geometryStage.geometry.topSkewPixels=2.0f;geometryStage.geometry.frame=5;
    assert(processCompositeRoundTrip(si,geometryCompositeView,geometryStage));
    CompositeParams scanStage=p;scanStage.scanlinesEnabled=true;scanStage.scanlines.amount=.5f;
    scanStage.scanlines.frequency=.5f;
    assert(processCompositeRoundTrip(si,scanlineCompositeView,scanStage));
    assert(geometryComposite!=boxOut && scanlineComposite!=boxOut);

    std::vector<float> bloomComposite(src.size());
    ImageView bloomCompositeView{bloomComposite.data(),w,h,w*4};
    CompositeParams bloomStage=p;bloomStage.bloomEnabled=true;bloomStage.bloom.threshold=.1f;
    bloomStage.bloom.intensity=.8f;bloomStage.bloom.spread=1.5f;bloomStage.bloom.levels=2;
    assert(processCompositeRoundTrip(si,bloomCompositeView,bloomStage));
    assert(bloomComposite!=boxOut);

    std::vector<float> chromaComposite(src.size());
    ImageView chromaCompositeView{chromaComposite.data(),w,h,w*4};
    CompositeParams chromaStage=p;chromaStage.chromaSubsampleEnabled=true;
    chromaStage.chromaSubsample.horizontalFactor=2;chromaStage.chromaSubsample.verticalFactor=2;
    chromaStage.chromaSubsample.amount=1.0f;chromaStage.chromaSubsample.blurRadius=1.5f;
    assert(processCompositeRoundTrip(si,chromaCompositeView,chromaStage));
    assert(chromaComposite!=boxOut);
    for(size_t i=3;i<chromaComposite.size();i+=4)assert(std::fabs(chromaComposite[i]-1.0f)<1e-6f);

    std::vector<float> transportComposite(src.size()),surfaceComposite(src.size()),bothComposite(src.size());
    ImageView tcv{transportComposite.data(),w,h,w*4}, scv{surfaceComposite.data(),w,h,w*4}, bcv{bothComposite.data(),w,h,w*4};
    CompositeParams physical=p; physical.tapeTransportEnabled=true; physical.tapeTransport.frame=4;
    physical.tapeTransport.drumFlutterPixels=1.2f; physical.tapeTransport.flutterPixels=.4f;
    assert(processCompositeRoundTrip(si,tcv,physical));
    physical=p; physical.tapeSurfaceEnabled=true; physical.tapeSurface.frame=4;
    physical.tapeSurface.defectDensity=.8f; physical.tapeSurface.edgeLift=.15f;
    assert(processCompositeRoundTrip(si,scv,physical));
    physical.tapeTransportEnabled=true; physical.tapeTransport.drumFlutterPixels=.8f;
    assert(processCompositeRoundTrip(si,bcv,physical));
    assert(transportComposite!=boxOut && surfaceComposite!=boxOut && bothComposite!=boxOut);
    for(size_t i=3;i<bothComposite.size();i+=4) assert(std::fabs(bothComposite[i]-1.0f)<1e-6f);

    std::vector<float> palOutput(src.size()),palNoSwitch(src.size());
    ImageView palView{palOutput.data(),w,h,w*4},palNoView{palNoSwitch.data(),w,h,w*4};
    CompositeParams pal=p; pal.palMode=true; pal.palSwitch=true;
    assert(processCompositeRoundTrip(si,palView,pal));
    pal.palSwitch=false;
    assert(processCompositeRoundTrip(si,palNoView,pal));
    assert(palOutput!=boxOut);
    assert(palNoSwitch==boxOut);

    std::vector<float> vhsVertical(src.size()),vhsSmear(src.size()),vhsCombined(src.size());
    ImageView vv{vhsVertical.data(),w,h,w*4}, sv{vhsSmear.data(),w,h,w*4}, cv{vhsCombined.data(),w,h,w*4};
    CompositeParams vhs=p; vhs.verticalChromaBlendEnabled=true; vhs.verticalChromaBlendAmount=1.0f;
    assert(processCompositeRoundTrip(si,vv,vhs));
    vhs=p; vhs.lumaSmearEnabled=true; vhs.lumaSmear.strength=1.0f;
    vhs.lumaSmear.tapCount=16; vhs.lumaSmear.highlightStrength=0.25f;
    assert(processCompositeRoundTrip(si,sv,vhs));
    vhs.verticalChromaBlendEnabled=true; vhs.verticalChromaBlendAmount=0.75f;
    assert(processCompositeRoundTrip(si,cv,vhs));
    assert(vhsVertical!=boxOut && vhsSmear!=boxOut && vhsCombined!=boxOut);
    for(size_t i=3;i<vhsCombined.size();i+=4) assert(std::fabs(vhsCombined[i]-1.0f)<1e-6f);

    std::vector<float> semi=src, semiOut(src.size());
    for(size_t i=0;i<semi.size();i+=4) { semi[i]*=0.35f; semi[i+1]*=0.35f; semi[i+2]*=0.35f; semi[i+3]=0.35f; }
    ConstImageView ssi{semi.data(),w,h,w*4}; ImageView sso{semiOut.data(),w,h,w*4};
    pp=p; pp.premultipliedAlpha=true; pp.outputBandwidthEnabled=true;
    assert(processCompositeRoundTrip(ssi,sso,pp));
    for(size_t i=3;i<semiOut.size();i+=4) assert(std::fabs(semiOut[i]-0.35f)<1e-6f);

    std::vector<float> gray(w*h*4), grayOut(gray.size());
    for(size_t i=0;i<gray.size();i+=4) { gray[i]=gray[i+1]=gray[i+2]=0.45f; gray[i+3]=1.0f; }
    ConstImageView gi{gray.data(),w,h,w*4}; ImageView go{grayOut.data(),w,h,w*4};
    pp=p; pp.phase=CompositePhase::Degrees0;
    assert(processCompositeRoundTrip(gi,go,pp));
    for(int y=0;y<h;++y) for(int x=12;x<w;++x) {
        size_t i=(y*w+x)*4;
        assert(std::fabs(grayOut[i]-grayOut[i+1])<0.003f);
        assert(std::fabs(grayOut[i+1]-grayOut[i+2])<0.003f);
    }
    return 0;
}
