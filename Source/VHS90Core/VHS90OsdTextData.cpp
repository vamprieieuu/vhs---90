#include "VHS90OsdTextData.h"
#include <algorithm>
#include <new>

namespace vhs90 {
namespace {
constexpr std::uint8_t kMagic[4] = {'V','9','0','T'};
constexpr std::uint16_t kVersion = 1;
constexpr std::size_t kHeaderSize = 12;
std::uint32_t readU32(const std::uint8_t* p) noexcept {
    return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) |
           (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
}
void writeU16(std::uint8_t* p, std::uint16_t v) noexcept {
    p[0] = static_cast<std::uint8_t>(v);
    p[1] = static_cast<std::uint8_t>(v >> 8);
}
void writeU32(std::uint8_t* p, std::uint32_t v) noexcept {
    for (unsigned i=0; i<4; ++i) p[i] = static_cast<std::uint8_t>(v >> (i*8));
}
}
bool isValidOsdUtf8(std::string_view s) noexcept {
    std::size_t i=0;
    while (i<s.size()) {
        const auto c=static_cast<std::uint8_t>(s[i]);
        if (c<=0x7f) { ++i; continue; }
        std::uint32_t cp=0; std::size_t n=0;
        if (c>=0xc2 && c<=0xdf) { cp=c&0x1f; n=2; }
        else if (c>=0xe0 && c<=0xef) { cp=c&0x0f; n=3; }
        else if (c>=0xf0 && c<=0xf4) { cp=c&0x07; n=4; }
        else return false;
        if (i+n>s.size()) return false;
        for (std::size_t j=1; j<n; ++j) {
            const auto d=static_cast<std::uint8_t>(s[i+j]);
            if ((d&0xc0)!=0x80) return false;
            cp=(cp<<6)|(d&0x3f);
        }
        if ((n==3 && cp<0x800) || (n==4 && cp<0x10000) ||
            (cp>=0xd800 && cp<=0xdfff) || cp>0x10ffff) return false;
        i+=n;
    }
    return true;
}
bool setOsdText(OsdTextData& data, std::string_view utf8) noexcept {
    if (utf8.size()>kOsdTextMaxUtf8Bytes || !isValidOsdUtf8(utf8)) return false;
    try { if (utf8.empty()) data.utf8.clear(); else data.utf8.assign(utf8.data(),utf8.size()); return true; }
    catch (...) { return false; }
}
bool flattenOsdText(const OsdTextData& data, std::vector<std::uint8_t>& out) noexcept {
    if (data.utf8.size()>kOsdTextMaxUtf8Bytes || !isValidOsdUtf8(data.utf8)) return false;
    try {
        std::vector<std::uint8_t> tmp(kHeaderSize+data.utf8.size());
        std::copy(kMagic,kMagic+4,tmp.begin());
        writeU16(tmp.data()+4,kVersion); writeU16(tmp.data()+6,0);
        writeU32(tmp.data()+8,static_cast<std::uint32_t>(data.utf8.size()));
        std::copy(data.utf8.begin(),data.utf8.end(),tmp.begin()+kHeaderSize);
        out.swap(tmp); return true;
    } catch (...) { return false; }
}
bool unflattenOsdText(const void* raw, std::size_t size, OsdTextData& out) noexcept {
    if (!raw || size<kHeaderSize) return false;
    const auto* p=static_cast<const std::uint8_t*>(raw);
    if (!std::equal(kMagic,kMagic+4,p) || p[4]!=1 || p[5]!=0 || p[6]!=0 || p[7]!=0) return false;
    const std::uint32_t length=readU32(p+8);
    if (length>kOsdTextMaxUtf8Bytes || size!=kHeaderSize+length) return false;
    const std::string_view text(reinterpret_cast<const char*>(p+kHeaderSize),length);
    if (!isValidOsdUtf8(text)) return false;
    return setOsdText(out,text);
}
} // namespace vhs90
