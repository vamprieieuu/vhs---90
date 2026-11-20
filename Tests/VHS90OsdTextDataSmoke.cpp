#include "VHS90OsdTextData.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
using namespace vhs90;
int main() {
    OsdTextData source, decoded;
    const std::string multilingual = "REC • PLAY • مرحباً بالعالم • ١٢٣ • VHS-90";
    assert(setOsdText(source,multilingual));
    std::vector<std::uint8_t> bytes;
    assert(flattenOsdText(source,bytes));
    assert(unflattenOsdText(bytes.data(),bytes.size(),decoded));
    assert(decoded.utf8==multilingual);
    assert(isValidOsdUtf8(""));
    assert(isValidOsdUtf8("ASCII"));
    const std::string embeddedNul("A\0B",3);
    assert(!isValidOsdUtf8(embeddedNul));
    assert(!isValidOsdUtf8("\xC0\xAF")); // overlong
    assert(!isValidOsdUtf8("\xED\xA0\x80")); // UTF-16 surrogate
    assert(!isValidOsdUtf8("\xF4\x90\x80\x80")); // > U+10FFFF
    assert(!isValidOsdUtf8("\xE2\x82")); // truncated
    assert(!setOsdText(source,std::string(kOsdTextMaxUtf8Bytes+1,'x')));
    auto corrupt=bytes; corrupt[0]='X';
    assert(!unflattenOsdText(corrupt.data(),corrupt.size(),decoded));
    corrupt=bytes; corrupt[4]=2;
    assert(!unflattenOsdText(corrupt.data(),corrupt.size(),decoded));
    corrupt=bytes; corrupt[8]=0xff;
    assert(!unflattenOsdText(corrupt.data(),corrupt.size(),decoded));
    return 0;
}
