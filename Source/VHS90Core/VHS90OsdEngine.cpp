#include "VHS90OsdEngine.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdint>
namespace vhs90 { namespace {
constexpr float TR=232.0f/255.0f,TG=1.0f,TB=232.0f/255.0f;
inline float c01(float x) noexcept{return std::max(0.0f,std::min(1.0f,x));}
inline int ci(int x,int a,int b) noexcept{return std::max(a,std::min(b,x));}
inline std::uint32_t mix(std::uint32_t x) noexcept{x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;return x;}
inline float noise(std::uint32_t seed,std::int64_t frame,std::uint32_t lane) noexcept{
 const std::uint32_t f=static_cast<std::uint32_t>(frame)^static_cast<std::uint32_t>(static_cast<std::uint64_t>(frame)>>32);
 const std::uint32_t h=mix(seed^mix(f+lane*0x9e3779b9u));
 return float(h&0x00ffffffu)/8388607.5f-1.0f;
}
std::array<std::uint8_t,7> glyph(char c) noexcept {
 switch(c){
 case '0':return{14,17,19,21,25,17,14}; case '1':return{4,12,4,4,4,4,14};
 case '2':return{14,17,1,2,4,8,31}; case '3':return{30,1,1,14,1,1,30};
 case '4':return{2,6,10,18,31,2,2}; case '5':return{31,16,16,30,1,1,30};
 case '6':return{6,8,16,30,17,17,14}; case '7':return{31,1,2,4,8,8,8};
 case '8':return{14,17,17,14,17,17,14}; case '9':return{14,17,17,15,1,2,12};
 case 'A':return{14,17,17,31,17,17,17}; case 'B':return{30,17,17,30,17,17,30};
 case 'C':return{14,17,16,16,16,17,14}; case 'D':return{30,17,17,17,17,17,30};
 case 'E':return{31,16,16,30,16,16,31}; case 'F':return{31,16,16,30,16,16,16};
 case 'G':return{14,17,16,23,17,17,15}; case 'H':return{17,17,17,31,17,17,17};
 case 'I':return{14,4,4,4,4,4,14}; case 'J':return{7,2,2,2,18,18,12};
 case 'K':return{17,18,20,24,20,18,17}; case 'L':return{16,16,16,16,16,16,31};
 case 'M':return{17,27,21,21,17,17,17}; case 'N':return{17,25,21,19,17,17,17};
 case 'O':return{14,17,17,17,17,17,14}; case 'P':return{30,17,17,30,16,16,16};
 case 'Q':return{14,17,17,17,21,18,13}; case 'R':return{30,17,17,30,20,18,17};
 case 'S':return{15,16,16,14,1,1,30}; case 'T':return{31,4,4,4,4,4,4};
 case 'U':return{17,17,17,17,17,17,14}; case 'V':return{17,17,17,17,17,10,4};
 case 'W':return{17,17,17,21,21,21,10}; case 'X':return{17,17,10,4,10,17,17};
 case 'Y':return{17,17,10,4,4,4,4}; case 'Z':return{31,1,2,4,8,16,31};
 case ':':return{0,4,4,0,4,4,0}; case '-':return{0,0,0,31,0,0,0};
 case '/':return{1,2,2,4,8,8,16}; case '.':return{0,0,0,0,0,6,6};
 default:return{0,0,0,0,0,0,0};
 }}
inline void pixel(ImageView im,int x,int y,float r,float g,float b,float cov,bool premul) noexcept{
 if(x<0||x>=im.width||y<0||y>=im.height||cov<=0.0001f)return; cov=c01(cov);
 float* q=im.rgba+static_cast<std::ptrdiff_t>(y)*im.rowStrideFloats+x*4;
 const float a=c01(q[3]),oa=cov+a*(1.0f-cov);
 if(premul){q[0]=r*cov+q[0]*(1-cov);q[1]=g*cov+q[1]*(1-cov);q[2]=b*cov+q[2]*(1-cov);}
 else{const float inv=oa>1e-8f?1.0f/oa:0.0f;q[0]=(r*cov+q[0]*a*(1-cov))*inv;q[1]=(g*cov+q[1]*a*(1-cov))*inv;q[2]=(b*cov+q[2]*a*(1-cov))*inv;}
 q[3]=oa;
}
void charDraw(ImageView im,char ch,int x,int y,int s,float r,float g,float b,float a,bool pm) noexcept{
 const auto rows=glyph(ch);for(int yy=0;yy<7;++yy)for(int xx=0;xx<5;++xx)if(rows[yy]&(1u<<(4-xx)))
 for(int py=0;py<s;++py)for(int px=0;px<s;++px)pixel(im,x+xx*s+px,y+yy*s+py,r,g,b,a,pm);
}
int width(const char* t,int s) noexcept{int n=0;while(t[n])++n;return n?(n*6-1)*s:0;}
void textDraw(ImageView im,const char* t,int x,int y,int s,float a,bool pm,int dx=0) noexcept{
 for(int i=0;t[i];++i)charDraw(im,t[i],x+i*6*s+dx,y,s,TR,TG,TB,a,pm);
}
void dotDraw(ImageView im,int cx,int cy,int r,float a,bool pm,float cr,float cg,float cb) noexcept{
 for(int y=-r;y<=r;++y)for(int x=-r;x<=r;++x)if(x*x+y*y<=r*r)pixel(im,cx+x,cy+y,cr,cg,cb,a,pm);
}
void triangle(ImageView im,int x,int y,int s,float a,bool pm) noexcept{
 constexpr int rows[7]={1,2,3,3,3,2,1};
 for(int yy=0;yy<7;++yy)for(int xx=0;xx<rows[yy];++xx)
 for(int py=0;py<s;++py)for(int px=0;px<s;++px)pixel(im,x+xx*s+px,y+yy*s+py,TR,TG,TB,a,pm);
}
std::int64_t days(int y,unsigned m,unsigned d) noexcept{
 y-=m<=2;const int era=(y>=0?y:y-399)/400;const unsigned yo=static_cast<unsigned>(y-era*400);
 const unsigned am=m>2?m-3:m+9;const unsigned doy=(153*am+2)/5+d-1;
 const unsigned doe=yo*365+yo/4-yo/100+doy;return std::int64_t(era)*146097+doe-719468;
}
void civil(std::int64_t z,int& y,unsigned& m,unsigned& d) noexcept{
 z+=719468;const std::int64_t era=(z>=0?z:z-146096)/146097;const unsigned doe=static_cast<unsigned>(z-era*146097);
 const unsigned yo=(doe-doe/1460+doe/36524-doe/146096)/365;y=int(yo)+int(era*400);
 const unsigned doy=doe-(365*yo+yo/4-yo/100),mp=(5*doy+2)/153;d=doy-(153*mp+2)/5+1;m=mp<10?mp+3:mp-9;y+=(m<=2);
}
void tapeCounter(const VhsOsdParams& p,char* out,std::size_t cap) noexcept {
 const double fps=std::isfinite(p.framesPerSecond)?std::max(1.0f,std::min(240.0f,p.framesPerSecond)):30.0;
 const std::int64_t frame=std::max<std::int64_t>(0,p.frame);
 const std::int64_t wholeSeconds=static_cast<std::int64_t>(std::floor(double(frame)/fps));
 const int ff=static_cast<int>(frame-static_cast<std::int64_t>(std::floor(double(frame)/fps)*fps));
 const int ss=static_cast<int>(wholeSeconds%60), mm=static_cast<int>((wholeSeconds/60)%60);
 const std::int64_t hh=wholeSeconds/3600;
 switch(p.tapeCounterFormat) {
 case VhsTapeCounterFormat::HoursMinutesSeconds: std::snprintf(out,cap,"%02lld:%02d:%02d",static_cast<long long>(hh),mm,ss); break;
 case VhsTapeCounterFormat::HoursMinutesSecondsFrames: std::snprintf(out,cap,"%02lld:%02d:%02d:%02d",static_cast<long long>(hh),mm,ss,ff); break;
 default: std::snprintf(out,cap,"%06lld",static_cast<long long>(std::min<std::int64_t>(999999,frame))); break;
 }
}
void stamp(const VhsOsdParams& p,double elapsed,char* out,std::size_t cap) noexcept{
 const int y=ci(p.startYear,1900,2199),mo=ci(p.startMonth,1,12),d=ci(p.startDay,1,31);
 const int hh=ci(p.startHour,0,23),mm=ci(p.startMinute,0,59),ss=ci(p.startSecond,0,59);
 const double total=double(hh*3600+mm*60+ss)+elapsed,dayd=std::floor(total/86400.0);
 double rem=total-dayd*86400.0;if(rem<0)rem+=86400.0;
 const int oh=int(rem/3600.0),om=int((rem-oh*3600.0)/60.0),os=int(rem-oh*3600.0-om*60.0);
 int oy=y;unsigned omth=unsigned(mo),od=unsigned(d);civil(days(y,omth,od)+static_cast<std::int64_t>(dayd),oy,omth,od);
 switch(p.timestampFormat) {
 case VhsTimestampFormat::MonthFirst: std::snprintf(out,cap,"%02u/%02u/%04d  %02d:%02d:%02d",omth,od,oy,oh,om,os); break;
 case VhsTimestampFormat::DayFirst: std::snprintf(out,cap,"%02u/%02u/%04d  %02d:%02d:%02d",od,omth,oy,oh,om,os); break;
 case VhsTimestampFormat::DateOnly: std::snprintf(out,cap,"%04d-%02u-%02u",oy,omth,od); break;
 case VhsTimestampFormat::TimeOnly: std::snprintf(out,cap,"%02d:%02d:%02d",oh,om,os); break;
 default: std::snprintf(out,cap,"%04d-%02u-%02u  %02d:%02d:%02d",oy,omth,od,oh,om,os); break;
 }
}
} // namespace

bool applyVhsOsd(ImageView im,const VhsOsdParams& p) noexcept{
 if(!im.rgba||im.width<=0||im.height<=0||im.rowStrideFloats<std::ptrdiff_t(im.width)*4)return false;
 if(!std::isfinite(p.amount)||!std::isfinite(p.opacity)||!std::isfinite(p.scale)||!std::isfinite(p.glitch)||
 !std::isfinite(p.ghost)||!std::isfinite(p.recAmount)||!std::isfinite(p.playAmount)||!std::isfinite(p.timestampAmount)||!std::isfinite(p.batteryAmount)||
 !std::isfinite(p.recBlinkHz)||!std::isfinite(p.tapeCounterAmount)||!std::isfinite(p.tapeCounterX)||!std::isfinite(p.tapeCounterY)||!std::isfinite(p.recX)||!std::isfinite(p.recY)||!std::isfinite(p.statusX)||!std::isfinite(p.statusY)||!std::isfinite(p.timestampX)||!std::isfinite(p.timestampY)||!std::isfinite(p.recColorR)||!std::isfinite(p.recColorG)||!std::isfinite(p.recColorB)||!std::isfinite(p.timestampSpeed)||!std::isfinite(p.timeSeconds)||!std::isfinite(p.framesPerSecond))return false;
 const float amount=c01(p.amount),opacity=c01(p.opacity);
 if(amount<=0||opacity<=0||(!p.showRec&&!p.showPlay&&!p.showTimestamp&&!p.showBattery&&!p.showTapeCounter))return true;
 const int w=im.width,h=im.height;
 const int cell=std::max(1,int(std::lround((float(h)/294.0f)*std::max(0.35f,p.scale))));
 const int small=std::max(1,int(std::lround(cell*0.75f)));
 const int recX=ci(int(std::lround(w*c01(p.recX/100.0f))),0,w-1),recY=ci(int(std::lround(h*c01(p.recY/100.0f))),0,h-1);
 const int statusX=ci(int(std::lround(w*c01(p.statusX/100.0f))),0,w-1),statusY=ci(int(std::lround(h*c01(p.statusY/100.0f))),0,h-1);
 const int timestampX=ci(int(std::lround(w*c01(p.timestampX/100.0f))),0,w-1),timestampY=ci(int(std::lround(h*c01(p.timestampY/100.0f))),0,h-1);
 const float gl=std::max(0.0f,std::min(2.0f,p.glitch)),gh=c01(p.ghost);
 const int dx=int(std::lround(noise(p.seed,p.frame,1)*gl*cell*0.45f)),dy=int(std::lround(noise(p.seed,p.frame,2)*gl*cell*0.25f));
 const int gx=gh>0.002f?std::max(1,int(std::lround(cell*(0.35f+gh*0.5f)))):0;const bool pm=p.premultipliedAlpha;
 if(p.showRec&&p.recAmount>0){
  float blink=1.0f;if(p.recBlinkHz>0&&p.framesPerSecond>0){const double ph=std::fmod(std::max(0.0,p.timeSeconds)*p.recBlinkHz,1.0);if(ph>=0.65){const double u=(ph-0.65)/0.35;blink=float(0.5-0.5*std::cos(2.0*3.14159265358979323846*u));}}
  const float a=amount*opacity*c01(p.recAmount)*blink;if(a>0.001f){const int r=std::max(2,cell*2),cx=recX+r+dx,cy=recY+r/2+dy;
   dotDraw(im,cx,cy,r,a,pm,c01(p.recColorR),c01(p.recColorG),c01(p.recColorB));if(gx&&gh>0.002f)dotDraw(im,cx+gx,cy,r,a*gh,pm,c01(p.recColorR),c01(p.recColorG),c01(p.recColorB));
   if(p.drawRecText){textDraw(im,"REC",recX+2*r+cell+dx,recY+dy,cell,a,pm);if(gx&&gh>0.002f)textDraw(im,"REC",recX+2*r+cell+dx,recY+dy,cell,a*gh,pm,gx);}
  }}
 if(p.showPlay&&p.playAmount>0){
  const float a=amount*opacity*c01(p.playAmount);
  const char* label="PLAY"; const char* speed=p.tapeSpeed==3?"EP":(p.tapeSpeed==2?"LP":"SP");
  switch(p.deckMode){case VhsDeckMode::Pause:label="PAUSE";break;case VhsDeckMode::Stop:label="STOP";break;case VhsDeckMode::FastForward:label="FF";break;case VhsDeckMode::Rewind:label="REW";break;default:break;}
  const int labelWidth=width(label,cell),speedWidth=width(speed,small);
  const int x=ci(statusX-labelWidth-5*cell,0,w-1)+dx;
  if(p.deckMode==VhsDeckMode::Play) triangle(im,x,statusY+dy,cell,a,pm);
  else if(p.deckMode==VhsDeckMode::Pause) {
   for(int yy=0;yy<7*cell;++yy)for(int xx=0;xx<2*cell;++xx){pixel(im,x+cell+xx,statusY+yy,TR,TG,TB,a,pm);pixel(im,x+4*cell+xx,statusY+yy,TR,TG,TB,a,pm);}
  } else if(p.deckMode==VhsDeckMode::Stop) {
   for(int yy=0;yy<5*cell;++yy)for(int xx=0;xx<5*cell;++xx)pixel(im,x+cell+xx,statusY+cell+yy,TR,TG,TB,a,pm);
  } else if(p.deckMode==VhsDeckMode::FastForward) {
   triangle(im,x,statusY+dy,cell,a,pm); triangle(im,x+4*cell,statusY+dy,cell,a,pm);
  } else {
   for(int yy=0;yy<7*cell;++yy)for(int xx=0;xx<3*cell;++xx)pixel(im,x+xx,statusY+yy,TR,TG,TB,a,pm);
   triangle(im,x+3*cell,statusY+dy,cell,a,pm);
  }
  textDraw(im,label,x+5*cell,statusY+dy,cell,a,pm);
  const int sx=ci(statusX-speedWidth,0,w-1)+dx;
  textDraw(im,speed,sx,statusY+8*cell+small+dy,small,a,pm);
  if(gx&&gh>0.002f){textDraw(im,label,x+5*cell,statusY+dy,cell,a*gh,pm,gx);textDraw(im,speed,sx,statusY+8*cell+small+dy,small,a*gh,pm,gx);}
 }
 if(p.showTimestamp&&p.drawTimestampText&&p.timestampAmount>0){char t[40]{};stamp(p,std::max(0.0,p.timeSeconds)*std::max(0.0f,p.timestampSpeed),t,sizeof(t));
  const float a=amount*opacity*c01(p.timestampAmount);const int x=ci(timestampX-width(t,small)+dx,0,w-1),y=ci(timestampY-7*small+dy,0,h-1);
  textDraw(im,t,x,y,small,a,pm);if(gx&&gh>0.002f)textDraw(im,t,x,y,small,a*gh,pm,gx);
 }
 if(p.showTapeCounter&&p.drawTapeCounterText&&p.tapeCounterAmount>0.0f){
  char counter[40]{}; tapeCounter(p,counter,sizeof(counter));
  const float a=amount*opacity*c01(p.tapeCounterAmount);
  const int x=ci(int(std::lround(w*c01(p.tapeCounterX/100.0f)))+dx,0,w-1);
  const int y=ci(int(std::lround(h*c01(p.tapeCounterY/100.0f)))-7*small+dy,0,h-1);
  textDraw(im,counter,x,y,small,a,pm);
  if(gx&&gh>0.002f) textDraw(im,counter,x,y,small,a*gh,pm,gx);
 }
 if(p.showBattery&&p.batteryAmount>0.001f){
  const float a=amount*opacity*c01(p.batteryAmount);
  const int bw=14*small,bh=7*small,margin=3*small,x=w-margin-bw+dx,y=margin+dy,th=std::max(1,small);
  for(int xx=0;xx<bw;++xx){pixel(im,x+xx,y,TR,TG,TB,a,pm);pixel(im,x+xx,y+bh-1,TR,TG,TB,a,pm);}
  for(int yy=0;yy<bh;++yy){pixel(im,x,y+yy,TR,TG,TB,a,pm);pixel(im,x+bw-1,y+yy,TR,TG,TB,a,pm);}
  for(int yy=bh/3;yy<(2*bh)/3;++yy)pixel(im,x+bw,y+yy,TR,TG,TB,a,pm);
  const int innerW=bw-4*th,innerH=bh-2*th,segments=3,gap=std::max(1,th/2);
  const int segmentW=std::max(1,(innerW-2*gap)/segments),filled=int(std::lround(float(segments)*c01(p.batteryAmount)));
  for(int n=0;n<filled;++n)for(int yy=0;yy<innerH;++yy)for(int xx=0;xx<segmentW;++xx)
   pixel(im,x+2*th+n*(segmentW+gap)+xx,y+th+yy,TR,TG,TB,a,pm);
 }
 return true;
}
} // namespace vhs90
