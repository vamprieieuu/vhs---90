#include "../src/engines/shared/vhs90_tape_surface.hpp"
#include <cmath>
#include <iostream>
#include <vector>
int main() {
    vhs90::TapeSurfaceParams p;
    p.seed=777; p.defectDensity=0.9f; p.oxideLoss=0.7f;
    p.scratchDensity=0.8f; p.creaseStrength=0.8f; p.debrisAmount=0.7f;
    p.surfaceRoughness=0.2f;
    const auto a=vhs90::sampleTapeSurface(0.321f,0.412f,p);
    const auto b=vhs90::sampleTapeSurface(0.321f,0.412f,p);
    if(a.magnetization!=b.magnetization || a.spacingLift!=b.spacingLift ||
       a.roughness!=b.roughness || a.creaseLoss!=b.creaseLoss) return 1;
    std::vector<float> image(128*96*4,0.5f);
    for(size_t i=3;i<image.size();i+=4) image[i]=0.42f;
    const auto before=image;
    vhs90::FloatRgbaView view{image.data(),128,96,128*4};
    p.frameIndex=12;
    vhs90::applyTapeSurface(view,p);
    bool changed=false;
    for(size_t i=0;i<image.size();i+=4) {
        if(std::abs(image[i]-before[i])>1e-6f) changed=true;
        if(image[i+3]!=0.42f) return 2;
    }
    if(!changed) return 3;
    std::cout<<"PASS persistent tape map deterministic; signal altered; alpha preserved\n";
    return 0;
}
