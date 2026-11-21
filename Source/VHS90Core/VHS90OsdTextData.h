#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace vhs90 {
constexpr std::size_t kOsdTextMaxUtf8Bytes = 4096;
struct OsdTextData {
    std::string utf8;
};

// Strict UTF-8 validation: rejects overlong encodings, surrogates, truncated
// sequences, invalid continuation bytes, and code points above U+10FFFF.
bool isValidOsdUtf8(std::string_view text) noexcept;
bool setOsdText(OsdTextData& data, std::string_view utf8) noexcept;

// Stable serialized representation: "V90T", uint16 version (LE), uint16
// reserved (LE), uint32 byte length (LE), followed by validated UTF-8 bytes.
bool flattenOsdText(const OsdTextData& data, std::vector<std::uint8_t>& output) noexcept;
bool unflattenOsdText(const void* bytes, std::size_t size, OsdTextData& output) noexcept;
} // namespace vhs90
