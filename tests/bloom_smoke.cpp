#include "../src/engines/shared/vhs90_bloom.hpp"
#include <cmath>
#include <iostream>
#include <vector>
int main() {
    const int w=96,h=64;
    std::vector<float>a(size_t(w)*h*4,0.02f);
    for(int y=28;y<36;++y)for(int x=42;x<54;++x){const size_t i=(size_t(y)*w+x)*4;a[i]=a[i+1]=a[i+2]=1.0f;}
    for(size_t i=3;i<a.size();i+=4)a[i]=0.8f;
    vhs90::FloatRgbaView view{a.data(),w,h,w*4};
    vhs90::BloomParams p;p.enabled=true;p.threshold=0.6f;p.intensity=0.8f;p.spread=2.2f;p.levels=3;
    vhs90::applyBloom(view,p);
    float halo=0.0f;
    for(int y=20;y<44;++y)for(int x=34;x<62;++x){if(x>=42&&x<54&&y>=28&&y<36)continue;halo+=a[(size_t(y)*w+x)*4];}
    if(halo<=0)return 1;
    for(size_t i=3;i<a.size();i+=4)if(a[i]!=0.8f)return 2;
    std::cout<<"PASS multiscale bright-pass bloom; alpha preserved\n";
    return 0;
}
