#pragma once
// Windows-only DirectWrite overlay renderer. VHS90Core must not include this header.
#include "VHS90SignalKernels.h"
#include <cstdint>
#include <string>
#include <vector>
namespace vhs90::win {
enum class TextDirection : std::uint8_t { Auto, LeftToRight, RightToLeft };
enum class TextAlignment : std::uint8_t { Leading, Center, Trailing };
struct DirectWriteTextOptions {
    const char* utf8Text = nullptr;
    const char* utf8FontFamily = "Segoe UI";
    float x = 24.0f, y = 24.0f, fontSize = 28.0f;
    std::uint16_t fontWeight = 400;
    float maxWidth = 0.0f, maxHeight = 0.0f;
    float red = 1.0f, green = 1.0f, blue = 1.0f, opacity = 1.0f;
    TextDirection direction = TextDirection::Auto;
    TextAlignment alignment = TextAlignment::Leading;
    bool premultipliedDestination = true;
};
struct DirectWriteTextResult {
    bool rendered = false;
    bool requestedFontFound = false;
    bool usedFallbackFont = false;
};
bool isDirectWriteAvailable() noexcept;
std::vector<std::wstring> enumerateSystemFontFamilies();
DirectWriteTextResult drawDirectWriteText(ImageView destination, const DirectWriteTextOptions& options) noexcept;
} // namespace vhs90::win
