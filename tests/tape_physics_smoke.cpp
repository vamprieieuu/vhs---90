#include "../src/engines/shared/vhs90_tape_physics.hpp"
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    vhs90::HeadReadbackParams hp;
    const float clean=vhs90::headReadbackGain(2.0e6f,hp);
    hp.headToTapeSpacingMicrons=0.8f;
    const float worn=vhs90::headReadbackGain(2.0e6f,hp);
    if(!(clean>worn&&clean<=1.0f))return 1;

    vhs90::MagneticTransition transitions[2]={{10.0f,1.0f,1.0f},{20.0f,-1.0f,0.7f}};
    float rf[64];
    vhs90::renderLorentzianHeadReadback(transitions,2,10.0e6f,0.0f,64,hp,rf);
    bool positive=false,negative=false;
    for(float v:rf){positive|=v>0;negative|=v<0;}
    if(!positive||!negative)return 2;

    const int w=80,h=60;
    std::vector<float>a(size_t(w)*h*4,0.3f),b;
    for(size_t i=3;i<a.size();i+=4)a[i]=0.5f;
    b=a;
    vhs90::FloatRgbaView va{a.data(),w,h,w*4},vb{b.data(),w,h,w*4};
    vhs90::TapeTransportParams tp;tp.seed=44;tp.frameIndex=10;tp.lineJitterPixels=1.2f;tp.flutterPixels=0.5f;
    vhs90::applyTapeTransportWarp(va,tp);vhs90::applyTapeTransportWarp(vb,tp);
    if(a!=b)return 3;
    for(size_t i=3;i<a.size();i+=4)if(a[i]!=0.5f)return 4;
    std::cout<<"PASS head gap/spacing response, Lorentzian pulses, deterministic transport warp\n";
    return 0;
}
