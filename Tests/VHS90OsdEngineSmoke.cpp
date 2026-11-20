#include "VHS90OsdEngine.h"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>
int main() {
    using namespace vhs90;
    constexpr int w=640,h=360;
    std::vector<float> source(static_cast<std::size_t>(w)*h*4,0.0f);
    for(std::size_t i=3;i<source.size();i+=4) source[i]=1.0f;
    auto first=source,repeat=source,second=source;
    VhsOsdParams p; p.timeSeconds=0.0; p.frame=0; p.glitch=0.0f;
    ImageView a{first.data(),w,h,w*4}, b{repeat.data(),w,h,w*4};
    assert(applyVhsOsd(a,p)); assert(applyVhsOsd(b,p));
    assert(first==repeat); assert(first!=source);
    for(std::size_t i=3;i<first.size();i+=4) assert(std::fabs(first[i]-1.0f)<1e-6f);
    bool foundRed=false;
    for(int y=0;y<50&&!foundRed;++y) for(int x=0;x<90;++x) {
        const std::size_t i=(static_cast<std::size_t>(y)*w+x)*4;
        if(first[i]>first[i+1]+0.2f&&first[i]>first[i+2]+0.2f){foundRed=true;break;}
    }
    assert(foundRed);
    VhsOsdParams later=p; later.timeSeconds=1.0;
    ImageView c{second.data(),w,h,w*4}; assert(applyVhsOsd(c,later));
    bool timestampChanged=false;
    for(int y=h-40;y<h;++y) for(int x=w-380;x<w;++x) {
        const std::size_t i=(static_cast<std::size_t>(y)*w+x)*4;
        if(std::fabs(first[i]-second[i])+std::fabs(first[i+1]-second[i+1])+std::fabs(first[i+2]-second[i+2])>0.01f){timestampChanged=true;break;}
    }
    assert(timestampChanged);
    auto untouched=source; VhsOsdParams off; off.amount=0.0f;
    ImageView d{untouched.data(),w,h,w*4}; assert(applyVhsOsd(d,off)); assert(untouched==source);
    std::vector<float> transparent(static_cast<std::size_t>(w)*h*4,0.0f);
    VhsOsdParams recOnly; recOnly.showPlay=false; recOnly.showTimestamp=false; recOnly.glitch=0.0f; recOnly.ghost=0.0f;
    ImageView e{transparent.data(),w,h,w*4}; assert(applyVhsOsd(e,recOnly));
    bool alphaAdded=false; for(std::size_t i=3;i<transparent.size();i+=4) if(transparent[i]>0.1f){alphaAdded=true;break;}
    assert(alphaAdded);
    std::vector<float> bluePixels(static_cast<std::size_t>(w)*h*4,0.0f);
    for(std::size_t i=3;i<bluePixels.size();i+=4) bluePixels[i]=1.0f;
    VhsOsdParams blue; blue.showPlay=false; blue.showTimestamp=false; blue.glitch=0.0f; blue.ghost=0.0f;
    blue.recColorR=0.0f; blue.recColorG=0.1f; blue.recColorB=1.0f;
    ImageView blueView{bluePixels.data(),w,h,w*4}; assert(applyVhsOsd(blueView,blue));
    bool foundBlue=false;
    for(int y=0;y<50&&!foundBlue;++y) for(int x=0;x<90;++x) {
        const std::size_t i=(static_cast<std::size_t>(y)*w+x)*4;
        if(bluePixels[i+2]>bluePixels[i]+0.5f){foundBlue=true;break;}
    }
    assert(foundBlue);
    std::vector<float> batteryPixels=source;
    VhsOsdParams battery; battery.showRec=false;battery.showPlay=false;battery.showTimestamp=false;
    battery.showBattery=true;battery.batteryAmount=.75f;battery.glitch=0.0f;battery.ghost=0.0f;
    ImageView batteryView{batteryPixels.data(),w,h,w*4};assert(applyVhsOsd(batteryView,battery));
    bool foundBattery=false;
    for(int y=20;y<55&&!foundBattery;++y)for(int x=w-40;x<w-5;++x){
        const std::size_t i=(static_cast<std::size_t>(y)*w+x)*4;
        if(batteryPixels[i]>0.2f&&batteryPixels[i+1]>0.2f&&batteryPixels[i+2]>0.2f){foundBattery=true;break;}
    }
    assert(foundBattery);
    // VHS deck transport modes and independently configurable OSD anchors.
    auto renderDeck = [&](VhsDeckMode mode, int speed, VhsTimestampFormat format,
                          float statusX, float statusY, float stampX, float stampY) {
        std::vector<float> framePixels(static_cast<std::size_t>(w)*h*4,0.0f);
        for(std::size_t i=3;i<framePixels.size();i+=4) framePixels[i]=1.0f;
        VhsOsdParams deck;
        deck.showRec=false; deck.showBattery=false; deck.glitch=0.0f; deck.ghost=0.0f;
        deck.deckMode=mode; deck.tapeSpeed=speed; deck.timestampFormat=format;
        deck.statusX=statusX; deck.statusY=statusY; deck.timestampX=stampX; deck.timestampY=stampY;
        ImageView view{framePixels.data(),w,h,w*4};
        assert(applyVhsOsd(view,deck));
        return framePixels;
    };
    const auto playDeck=renderDeck(VhsDeckMode::Play,1,VhsTimestampFormat::Iso,98,3,98,96);
    const auto pauseDeck=renderDeck(VhsDeckMode::Pause,2,VhsTimestampFormat::MonthFirst,98,3,98,96);
    const auto stopDeck=renderDeck(VhsDeckMode::Stop,3,VhsTimestampFormat::DayFirst,98,3,98,96);
    const auto ffDeck=renderDeck(VhsDeckMode::FastForward,1,VhsTimestampFormat::DateOnly,98,3,98,96);
    const auto rewindDeck=renderDeck(VhsDeckMode::Rewind,2,VhsTimestampFormat::TimeOnly,98,3,98,96);
    assert(playDeck!=pauseDeck && pauseDeck!=stopDeck && stopDeck!=ffDeck && ffDeck!=rewindDeck);
    const auto spDeck=renderDeck(VhsDeckMode::Play,1,VhsTimestampFormat::Iso,98,3,98,96);
    const auto lpDeck=renderDeck(VhsDeckMode::Play,2,VhsTimestampFormat::Iso,98,3,98,96);
    const auto epDeck=renderDeck(VhsDeckMode::Play,3,VhsTimestampFormat::Iso,98,3,98,96);
    assert(spDeck!=lpDeck && lpDeck!=epDeck);
    const auto isoDeck=renderDeck(VhsDeckMode::Play,1,VhsTimestampFormat::Iso,98,3,98,96);
    const auto usDeck=renderDeck(VhsDeckMode::Play,1,VhsTimestampFormat::MonthFirst,98,3,98,96);
    const auto euDeck=renderDeck(VhsDeckMode::Play,1,VhsTimestampFormat::DayFirst,98,3,98,96);
    assert(isoDeck!=usDeck && usDeck!=euDeck);
    const auto movedDeck=renderDeck(VhsDeckMode::Play,1,VhsTimestampFormat::Iso,70,35,50,50);
    assert(playDeck!=movedDeck);
    ImageView invalid{first.data(),w,h,w*4-1}; assert(!applyVhsOsd(invalid,p));
    return 0;
}
