// ============================================================
// OsdRenderer.cpp — camcorder OSD bitmap rasterizer.
//
// Hand-rolled glyph data (5×7 blocky font + 7-segment digits).
// No third-party fonts; ~2 KB total. Designed to look "camcorder
// authentic" — chunky, aliased, intentionally low-res.
// ============================================================

#include "OsdRenderer.h"
#include <cstring>
#include <cstdio>
#include <algorithm>

// ============================================================
// 5×7 blocky bitmap font — uppercase A-Z, digits 0-9, : space . / ▶ ‖ ●
// Each glyph is 5 wide × 7 tall = 35 bits packed into a uint64_t.
// Bit layout: row 0 = bits 0..4 (LSB = leftmost), row 6 = bits 30..34.
// ============================================================
struct Glyph5x7 {
    char     ch;
    uint64_t bits;  // 35 bits used
};

#define G(ch, r0, r1, r2, r3, r4, r5, r6) \
    { ch, ((uint64_t)(r0) <<  0) | ((uint64_t)(r1) <<  5) | \
          ((uint64_t)(r2) << 10) | ((uint64_t)(r3) << 15) | \
          ((uint64_t)(r4) << 20) | ((uint64_t)(r5) << 25) | \
          ((uint64_t)(r6) << 30) }
//                  rows are 5 bits, bit0 = rightmost.
// 0b11111 = 0x1F

static const Glyph5x7 kBlocky[] = {
    G(' ', 0, 0, 0, 0, 0, 0, 0),
    G('0', 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E),
    G('1', 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E),
    G('2', 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F),
    G('3', 0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E),
    G('4', 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02),
    G('5', 0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E),
    G('6', 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E),
    G('7', 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08),
    G('8', 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E),
    G('9', 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C),
    G(':', 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00),
    G('.', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04),
    G('/', 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00),
    G('-', 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00),
    G('A', 0x0E, 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11),
    G('B', 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E),
    G('C', 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E),
    G('D', 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E),
    G('E', 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F),
    G('F', 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10),
    G('L', 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F),
    G('M', 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11),
    G('N', 0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11),
    G('O', 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E),
    G('P', 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10),
    G('R', 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11),
    G('S', 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E),
    G('T', 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04),
    G('U', 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E),
    G('V', 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04),
    G('W', 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A),
    G('Y', 0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04),
    // Special icons:
    G('*', 0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0E),  // REC dot (filled circle approx)
    G('>', 0x10, 0x18, 0x1C, 0x1E, 0x1C, 0x18, 0x10),  // PLAY ▶
    G('=', 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B),  // PAUSE ‖ (two bars)
    G('}', 0x14, 0x16, 0x17, 0x17, 0x17, 0x16, 0x14),  // FF ▶▶ (compressed)
    G('{', 0x05, 0x0D, 0x1D, 0x1D, 0x1D, 0x0D, 0x05),  // REW ◀◀
    G('#', 0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F),  // STOP square
};

static const Glyph5x7* findGlyph(char c) {
    if (c >= 'a' && c <= 'z') c = char(c - 'a' + 'A');
    for (const auto& g : kBlocky) if (g.ch == c) return &g;
    return &kBlocky[0]; // space fallback
}

// ============================================================
// Stamp a single glyph at (px, py) into the mask, scaled by `scale`
// pixels per font cell. Optional shadow offsets +1 px right/down.
// ============================================================
static void stampGlyph(uint8_t* mask, int W, int H,
                       const Glyph5x7& g, int px, int py,
                       int scale, bool shadow)
{
    if (scale < 1) scale = 1;
    for (int row = 0; row < 7; ++row) {
        uint64_t rowBits = (g.bits >> (row * 5)) & 0x1F;
        for (int col = 0; col < 5; ++col) {
            if (!(rowBits & (1u << col))) continue;
            int x0 = px + (4 - col) * scale;  // bit0 = rightmost column
            int y0 = py + row * scale;
            for (int sy = 0; sy < scale; ++sy) {
                int yy = y0 + sy;
                if (yy < 0 || yy >= H) continue;
                for (int sx = 0; sx < scale; ++sx) {
                    int xx = x0 + sx;
                    if (xx < 0 || xx >= W) continue;
                    if (shadow) {
                        if (mask[yy * W + xx] == 0) mask[yy * W + xx] = 128;
                    } else {
                        mask[yy * W + xx] = 255;
                    }
                }
            }
        }
    }
}

static void stampString(uint8_t* mask, int W, int H, const char* str,
                        int px, int py, int scale, bool shadow)
{
    int advance = (5 + 1) * scale; // 5 wide + 1 gap
    int x = px;
    for (const char* s = str; *s; ++s) {
        if (shadow) {
            const Glyph5x7* g = findGlyph(*s);
            stampGlyph(mask, W, H, *g, x + 1, py + 1, scale, true);
        }
        const Glyph5x7* g = findGlyph(*s);
        stampGlyph(mask, W, H, *g, x, py, scale, false);
        x += advance;
    }
}

// ============================================================
// 9-corner anchor → top-left pixel for a string of width strW × strH.
// ============================================================
static void anchor9(int pos, int W, int H, int strW, int strH,
                    int& outX, int& outY)
{
    int margin = std::max(std::min(W, H) / 32, 8);
    int col = pos % 3;
    int row = pos / 3;
    if (col == 0)      outX = margin;
    else if (col == 1) outX = W / 2 - strW / 2;
    else               outX = W - strW - margin;
    if (row == 0)      outY = margin;
    else if (row == 1) outY = H / 2 - strH / 2;
    else               outY = H - strH - margin;
}

// ============================================================
// Date / time arithmetic: advance the free-running camcorder clock by
// `frameOffset` frames at `fps`.
// ============================================================
static int daysInMonth(int y, int m) {
    static const int dim[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int d = dim[(m - 1) % 12];
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))) d = 29;
    return d;
}

static void advanceClock(int& Y, int& M, int& D, int& h, int& m, int& s,
                         long long frameOffset, double fps)
{
    if (fps < 1e-3) fps = 30.0;
    long long totalSec = (long long)((double)frameOffset / fps);
    s += (int)(totalSec % 60); totalSec /= 60;
    m += (int)(totalSec % 60); totalSec /= 60;
    h += (int)(totalSec % 24); totalSec /= 24;
    long long days = totalSec;

    // Normalise seconds → minutes
    while (s >= 60) { s -= 60; m++; }
    while (s < 0)   { s += 60; m--; }
    while (m >= 60) { m -= 60; h++; }
    while (m < 0)   { m += 60; h--; }
    while (h >= 24) { h -= 24; days++; }
    while (h < 0)   { h += 24; days--; }
    while (days > 0) {
        int dim_ = daysInMonth(Y, M);
        if (D + days > dim_) {
            days -= (dim_ - D + 1);
            D = 1; M++;
            if (M > 12) { M = 1; Y++; }
        } else { D += (int)days; days = 0; }
    }
    while (days < 0) {
        if (D + days < 1) {
            days += D;
            M--;
            if (M < 1) { M = 12; Y--; }
            D = daysInMonth(Y, M);
        } else { D += (int)days; days = 0; }
    }
}

// ============================================================
// Public entry: build the full-frame mask
// ============================================================
std::vector<uint8_t> renderOsdMask(const VhsParams& p, int frameInTimeline,
                                   double fps)
{
    if (!p.osdEnable || p.osdElements == 0) return {};

    int W = p.width;
    int H = p.height;
    if (W <= 0 || H <= 0) return {};

    std::vector<uint8_t> mask((size_t)W * H, 0);
    bool shadow = p.osdShadow != 0;

    // Glyph scale: scale slider × resolution-aware base.
    // Base scale: ~3 px per font cell at 1080p.
    int baseScale = std::max(1, (int)std::round(p.osdScale * (H / 360.0)));

    // Advance free-running clock from start time.
    int Y = p.osdStartYear, M = p.osdStartMonth, D = p.osdStartDay;
    int h = p.osdStartHour, m = p.osdStartMinute, s = p.osdStartSecond;
    advanceClock(Y, M, D, h, m, s, (long long)frameInTimeline, fps);

    char buf[64];

    auto place = [&](const char* str, int posCode) {
        int len = (int)strlen(str);
        int strW = len * 6 * baseScale;
        int strH = 7 * baseScale;
        int px, py;
        anchor9(posCode, W, H, strW, strH, px, py);
        stampString(mask.data(), W, H, str, px, py, baseScale, shadow);
    };

    if (p.osdElements & OSD_BIT_DATE) {
        switch (p.osdDateFormat) {
            case 0: snprintf(buf, sizeof(buf), "%04d %02d %02d", Y, M, D); break;
            case 1: snprintf(buf, sizeof(buf), "%02d %02d %04d", M, D, Y); break;
            default: snprintf(buf, sizeof(buf), "%02d %02d %04d", D, M, Y); break;
        }
        place(buf, p.osdPositionDate);
    }

    if (p.osdElements & OSD_BIT_TIME) {
        const char* ampm = (h < 12) ? "AM" : "PM";
        int h12 = h % 12; if (h12 == 0) h12 = 12;
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d %s", h12, m, s, ampm);
        place(buf, p.osdPositionTime);
    }

    if (p.osdElements & OSD_BIT_REC) {
        bool show = true;
        if (p.osdRecBlink) {
            // Blink at 1 Hz (assume 30 fps if timeline rate unknown).
            int blinkFrames = (int)std::round(fps);
            show = ((frameInTimeline / std::max(blinkFrames, 1)) & 1) == 0;
        }
        if (show) {
            snprintf(buf, sizeof(buf), "* REC");
            place(buf, p.osdPositionRec);
        }
    }

    if (p.osdElements & OSD_BIT_MODE) {
        const char* glyph;
        switch (p.osdPlayMode) {
            case OSD_PLAY:  glyph = "> PLAY";  break;
            case OSD_PAUSE: glyph = "= PAUSE"; break;
            case OSD_FF:    glyph = "} FF";    break;
            case OSD_REW:   glyph = "{ REW";   break;
            default:        glyph = "# STOP";  break;
        }
        place(glyph, p.osdPositionMode);
    }

    if (p.osdElements & OSD_BIT_COUNTER) {
        long long totalSec = (long long)((double)frameInTimeline / std::max(fps, 1.0));
        int hh = (int)((totalSec / 3600) % 100);
        int mm = (int)((totalSec / 60) % 60);
        int ss = (int)(totalSec % 60);
        snprintf(buf, sizeof(buf), "%01d:%02d:%02d", hh % 10, mm, ss);
        place(buf, p.osdPositionCounter);
    }

    if (p.osdElements & OSD_BIT_SPEED) {
        const char* lbl = (p.osdSpeedLabel == OSD_SPEED_SP) ? "SP"
                        : (p.osdSpeedLabel == OSD_SPEED_LP) ? "LP" : "EP";
        place(lbl, p.osdPositionSpeed);
    }

    // Flip vertically: OFX buffer y=0 is at the bottom of the frame.
    for (int y = 0; y < H / 2; ++y) {
        uint8_t* rowA = mask.data() + (size_t)y * W;
        uint8_t* rowB = mask.data() + (size_t)(H - 1 - y) * W;
        for (int x = 0; x < W; ++x) std::swap(rowA[x], rowB[x]);
    }

    return mask;
}
