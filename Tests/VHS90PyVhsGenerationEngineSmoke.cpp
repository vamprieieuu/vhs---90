#include "VHS90PyVhsGenerationEngine.h"
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    using namespace vhs90;
    constexpr int w=64,h=24;
    std::vector<float> src(w*h*4), out(src.size()), repeat(src.size()), pass(src.size());
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        const std::size_t i=(std::size_t(y)*w+x)*4;
        src[i]=float(x)/float(w-1); src[i+1]=float(y)/float(h-1);
        src[i+2]=float((x+y)%17)/16.0f; src[i+3]=1.0f;
    }
    ConstImageView in{src.data(),w,h,w*4};
    PyVhsGenerationParams p;
    ImageView o{out.data(),w,h,w*4}, r{repeat.data(),w,h,w*4}, q{pass.data(),w,h,w*4};
    assert(processPyVhsGeneration(in,q,p)); assert(pass==src);
    p.lumaCompression=1.9f; p.chromaCompression=8.0f;
    p.lumaNoiseSigma=2.0f; p.chromaNoise=3.0f; p.chromaWave=1.0f; p.frame=7;
    assert(processPyVhsGeneration(in,o,p)); assert(processPyVhsGeneration(in,r,p));
    assert(out==repeat); assert(out!=src);
    for(std::size_t i=3;i<out.size();i+=4) assert(std::fabs(out[i]-1.0f)<1e-6f);
    return 0;
}
