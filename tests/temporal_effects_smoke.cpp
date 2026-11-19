#include "../src/engines/shared/vhs90_temporal_effects.hpp"
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const int w=64,h=48;
    std::vector<float> current(size_t(w)*h*4,0.1f),previous(size_t(w)*h*4,0.8f);
    for(size_t i=3;i<current.size();i+=4)current[i]=0.33f;
    for(size_t i=3;i<previous.size();i+=4)previous[i]=0.9f;
    const auto original=current;
    vhs90::FloatRgbaView c{current.data(),w,h,w*4},p{previous.data(),w,h,w*4};
    vhs90::TemporalGhostParams gp;gp.enabled=true;gp.amount=0.4f;
    gp.offsetXPixels=1.5f;gp.fieldWeave=0.25f;
    vhs90::applyTemporalGhost(c,p,gp);
    bool changed=false;
    for(size_t i=0;i<current.size();i+=4) {
        if(std::abs(current[i]-original[i])>1e-5f)changed=true;
        if(current[i+3]!=0.33f)return 1;
    }
    if(!changed)return 2;
    std::cout<<"PASS host-supplied temporal ghost blend; alpha preserved\n";
    return 0;
}
