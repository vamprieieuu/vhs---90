#include "VHS90TapeTransportEngine.h"
#include <cassert>
#include <vector>
int main(){using namespace vhs90;constexpr int w=32,h=16;std::vector<float>s(w*h*4),a(s.size()),b(s.size()),c(s.size());
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){size_t i=(y*w+x)*4;s[i]=float(x)/float(w-1);s[i+1]=float(y)/float(h-1);s[i+2]=.4f;s[i+3]=(x<8)?0.0f:1.0f;}
 ConstImageView si{s.data(),w,h,w*4};ImageView ai{a.data(),w,h,w*4},bi{b.data(),w,h,w*4},ci{c.data(),w,h,w*4};
 TapeTransportParams p;p.seed=9;p.frame=14;p.drumFlutterPixels=1.2f;p.flutterPixels=.5f;p.stickSlipPixels=.2f;p.lineJitterPixels=.1f;
 assert(processTapeTransportWarp(si,ai,p));assert(processTapeTransportWarp(si,bi,p));assert(a==b);
 p.frame=15;assert(processTapeTransportWarp(si,ci,p));assert(a!=c);
 TapeTransportParams off;off.drumFlutterPixels=0;off.flutterPixels=0;off.stickSlipPixels=0;off.lineJitterPixels=0;std::fill(b.begin(),b.end(),0.0f);assert(processTapeTransportWarp(si,bi,off));assert(b==s);
 for(size_t i=3;i<a.size();i+=4)assert(a[i]>=0.0f&&a[i]<=1.0f);
    TapeTransportParams slow;slow.servoJitterPixels=1.8f;slow.seed=99;slow.frame=3;
    std::vector<float> slowA(s.size()),slowB(s.size());ImageView slowAView{slowA.data(),w,h,w*4},slowBView{slowB.data(),w,h,w*4};
    assert(processTapeTransportWarp(si,slowAView,slow));assert(processTapeTransportWarp(si,slowBView,slow));assert(slowA==slowB&&slowA!=s);
    TapeTransportParams pi;pi.seed=0x91u;pi.frame=180;pi.controlTrackServoAmount=1.0f;
    pi.controlTrackPulseStrength=1.0f;pi.trackingTargetPixels=2.0f;
    std::vector<float> piA(s.size()),piB(s.size());ImageView piAView{piA.data(),w,h,w*4},piBView{piB.data(),w,h,w*4};
    assert(processTapeTransportWarp(si,piAView,pi));assert(processTapeTransportWarp(si,piBView,pi));assert(piA==piB);
    pi.controlTrackPulseDropout=1.0f;std::vector<float> lost(s.size());ImageView lostView{lost.data(),w,h,w*4};
    assert(processTapeTransportWarp(si,lostView,pi));assert(lost!=piA);
    pi.controlTrackPulseDropout=0.0f;pi.pictureSearchSpeed=4.0f;std::vector<float> search(s.size());ImageView searchView{search.data(),w,h,w*4};
    assert(processTapeTransportWarp(si,searchView,pi));assert(search!=piA);
    TapeTransportParams physical=pi;physical.pictureSearchSpeed=1.0f;physical.usePhysicalControlTrackMap=true;
    physical.controlTrackTape.oxideLoss=1.85f;physical.controlTrackTape.pinholeAmount=1.0f;physical.controlTrackTape.edgeLift=1.45f;physical.controlTrackTape.moldAmount=1.0f;
    std::vector<float> physicalServo(s.size());ImageView physicalServoView{physicalServo.data(),w,h,w*4};
    assert(processTapeTransportWarp(si,physicalServoView,physical));assert(physicalServo!=piA);
    TapeTransportParams sticky;sticky.seed=0x91u;sticky.frame=14;sticky.drumFlutterPixels=0.0f;sticky.flutterPixels=0.0f;sticky.stickSlipPixels=0.0f;sticky.lineJitterPixels=0.0f;sticky.servoJitterPixels=0.0f;sticky.controlTrackTape.debrisAmount=1.0f;sticky.controlTrackTape.moldAmount=0.8f;
    std::vector<float> stickyA(s.size()),stickyB(s.size()),stickyNext(s.size());ImageView stickyAView{stickyA.data(),w,h,w*4},stickyBView{stickyB.data(),w,h,w*4},stickyNextView{stickyNext.data(),w,h,w*4};
    assert(processTapeTransportWarp(si,stickyAView,sticky));assert(processTapeTransportWarp(si,stickyBView,sticky));assert(stickyA==stickyB&&stickyA!=s);
    sticky.frame=15;assert(processTapeTransportWarp(si,stickyNextView,sticky));assert(stickyNext!=stickyA);
    sticky.controlTrackTape.debrisAmount=0.0f;sticky.controlTrackTape.moldAmount=0.0f;sticky.frame=14;std::fill(stickyB.begin(),stickyB.end(),0.0f);assert(processTapeTransportWarp(si,stickyBView,sticky));assert(stickyB==s);
    sticky.frame=14;sticky.backTension=-1.0f;std::vector<float> tensionLow(s.size()),tensionHigh(s.size());ImageView tensionLowView{tensionLow.data(),w,h,w*4},tensionHighView{tensionHigh.data(),w,h,w*4};
    assert(processTapeTransportWarp(si,tensionLowView,sticky));sticky.backTension=1.0f;assert(processTapeTransportWarp(si,tensionHighView,sticky));assert(tensionLow!=tensionHigh);
 return 0;}
