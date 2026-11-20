#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "VHS90DirectWriteText.h"
#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>
#include <string>
namespace vhs90::win {
namespace {
using Microsoft::WRL::ComPtr;
struct Factories {
    ComPtr<ID2D1Factory> d2d;
    ComPtr<IDWriteFactory> dwrite;
    HRESULT status = E_FAIL;
    Factories() noexcept {
        status = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, d2d.GetAddressOf());
        if (FAILED(status)) return;
        status = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                                     reinterpret_cast<IUnknown**>(dwrite.GetAddressOf()));
    }
};
Factories& factories() noexcept { static Factories instance; return instance; }

bool ensureBundledFonts() noexcept {
    static const bool loaded = []() noexcept {
        HMODULE module = nullptr;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                reinterpret_cast<LPCWSTR>(&ensureBundledFonts), &module) || !module)
            return false;
        wchar_t temp[MAX_PATH]{};
        const DWORD n = GetTempPathW(MAX_PATH, temp);
        if (n == 0 || n >= MAX_PATH) return false;
        std::wstring root(temp, n);
        root += L"VHS90\\Fonts";
        const auto slash = root.find_last_of(L"\\/");
        if (slash != std::wstring::npos) {
            const std::wstring parent = root.substr(0, slash);
            if (!CreateDirectoryW(parent.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
                return false;
        }
        if (!CreateDirectoryW(root.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
            return false;

        struct FontResource { WORD id; const wchar_t* filename; };
        constexpr FontResource fonts[] = {
            {501, L"Rooyin_Free_Bold.ttf"},
            {502, L"Sponge_Arabic_Regular.otf"}
        };
        bool anyLoaded = false;
        for (const auto& font : fonts) {
            HRSRC resource = FindResourceW(module, MAKEINTRESOURCEW(font.id), RT_RCDATA);
            if (!resource) continue;
            HGLOBAL handle = LoadResource(module, resource);
            const DWORD bytes = SizeofResource(module, resource);
            const void* data = handle ? LockResource(handle) : nullptr;
            if (!data || bytes == 0) continue;
            const std::wstring path = root + L"\\" + font.filename;
            HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (file == INVALID_HANDLE_VALUE) continue;
            DWORD written = 0;
            const BOOL ok = WriteFile(file, data, bytes, &written, nullptr);
            CloseHandle(file);
            if (ok && written == bytes && AddFontResourceExW(path.c_str(), FR_PRIVATE, nullptr) != 0)
                anyLoaded = true;
        }
        if (anyLoaded) {
            SendMessageW(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
            auto& f = factories();
            if (SUCCEEDED(f.status) && f.dwrite) {
                ComPtr<IDWriteFontCollection> refreshed;
                f.dwrite->GetSystemFontCollection(&refreshed, TRUE);
            }
        }
        return anyLoaded;
    }();
    return loaded;
}
class ComApartment {
public:
    ComApartment() noexcept {
        const HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        initialized_ = SUCCEEDED(hr);
        valid_ = initialized_ || hr == RPC_E_CHANGED_MODE;
    }
    ~ComApartment() { if (initialized_) CoUninitialize(); }
    bool valid() const noexcept { return valid_; }
private:
    bool initialized_ = false, valid_ = false;
};
bool utf8ToWide(const char* input, std::wstring& output) {
    output.clear();
    if (!input) return false;
    std::size_t boundedLength = 0;
    while (boundedLength < (1u << 20) && input[boundedLength] != '\0') ++boundedLength;
    if (boundedLength == (1u << 20)) return false;
    const int bytes = static_cast<int>(boundedLength);
    if (bytes <= 0) return false;
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input, bytes, nullptr, 0);
    if (count <= 0) return false;
    output.resize(static_cast<std::size_t>(count));
    return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input, bytes, output.data(), count) == count;
}
bool findFont(IDWriteFactory* factory, const wchar_t* familyName) noexcept {
    ensureBundledFonts();
    if (!factory || !familyName || !*familyName) return false;
    ComPtr<IDWriteFontCollection> collection;
    if (FAILED(factory->GetSystemFontCollection(&collection, FALSE))) return false;
    UINT32 index = 0; BOOL exists = FALSE;
    return SUCCEEDED(collection->FindFamilyName(familyName, &index, &exists)) && exists;
}
float finiteClamp(float v, float lo, float hi, float fallback) noexcept {
    return std::isfinite(v) ? std::max(lo, std::min(hi, v)) : fallback;
}
DWRITE_READING_DIRECTION inferReadingDirection(const std::wstring& text) noexcept {
    for (wchar_t ch : text) {
        const unsigned int c = static_cast<unsigned int>(ch);
        if ((c >= 0x0590u && c <= 0x08FFu) || (c >= 0xFB1Du && c <= 0xFDFFu) ||
            (c >= 0xFE70u && c <= 0xFEFFu) || (c >= 0x10800u && c <= 0x10FFFu) ||
            (c >= 0x1E800u && c <= 0x1EEFFu))
            return DWRITE_READING_DIRECTION_RIGHT_TO_LEFT;
        if ((c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z') ||
            (c >= 0x00C0u && c <= 0x02AFu) || (c >= 0x0370u && c <= 0x058Fu))
            return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
    }
    return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
}
void compositePremultipliedRect(ImageView dst, const BYTE* bgra, UINT stride,
                                  UINT width, UINT height, int dstX, int dstY, bool dstPremul) noexcept {
    for (UINT y = 0; y < height; ++y) {
        const int targetY = dstY + static_cast<int>(y);
        if (targetY < 0 || targetY >= dst.height) continue;
        const BYTE* row = bgra + static_cast<std::size_t>(y) * stride;
        float* out = dst.rgba + static_cast<std::ptrdiff_t>(targetY) * dst.rowStrideFloats;
        for (UINT x = 0; x < width; ++x) {
            const int targetX = dstX + static_cast<int>(x);
            if (targetX < 0 || targetX >= dst.width) continue;
            const BYTE* p = row + static_cast<std::size_t>(x) * 4;
            const float a = p[3] / 255.0f;
            if (a <= 0.00001f) continue;
            const float sr = p[2] / 255.0f, sg = p[1] / 255.0f, sb = p[0] / 255.0f;
            float* q = out + static_cast<std::ptrdiff_t>(targetX) * 4;
            const float da = std::max(0.0f, std::min(1.0f, q[3]));
            const float oa = a + da * (1.0f - a);
            if (dstPremul) {
                q[0] = sr + q[0] * (1.0f - a);
                q[1] = sg + q[1] * (1.0f - a);
                q[2] = sb + q[2] * (1.0f - a);
            } else {
                const float inv = oa > 1.0e-8f ? 1.0f / oa : 0.0f;
                q[0] = (sr + q[0] * da * (1.0f - a)) * inv;
                q[1] = (sg + q[1] * da * (1.0f - a)) * inv;
                q[2] = (sb + q[2] * da * (1.0f - a)) * inv;
            }
            q[3] = oa;
        }
    }
}
} // namespace
bool isDirectWriteAvailable() noexcept {
    const auto& f = factories(); return SUCCEEDED(f.status) && f.d2d && f.dwrite;
}
std::vector<std::wstring> enumerateSystemFontFamilies() {
    ensureBundledFonts();
    std::vector<std::wstring> names;
    ComApartment apartment; if (!apartment.valid()) return names;
    auto& f = factories(); if (FAILED(f.status) || !f.dwrite) return names;
    ComPtr<IDWriteFontCollection> collection;
    if (FAILED(f.dwrite->GetSystemFontCollection(&collection, FALSE))) return names;
    const UINT32 count = collection->GetFontFamilyCount(); names.reserve(count);
    for (UINT32 i = 0; i < count; ++i) {
        ComPtr<IDWriteFontFamily> family; ComPtr<IDWriteLocalizedStrings> strings;
        if (FAILED(collection->GetFontFamily(i, &family)) ||
            FAILED(family->GetFamilyNames(&strings)) || strings->GetCount() == 0) continue;
        UINT32 index = 0; BOOL exists = FALSE;
        if (FAILED(strings->FindLocaleName(L"en-us", &index, &exists)) || !exists) index = 0;
        UINT32 length = 0; if (FAILED(strings->GetStringLength(index, &length))) continue;
        std::wstring name(static_cast<std::size_t>(length) + 1, L'\0');
        if (SUCCEEDED(strings->GetString(index, name.data(), length + 1))) {
            name.resize(length); names.push_back(std::move(name));
        }
    }
    return names;
}
DirectWriteTextResult drawDirectWriteText(ImageView dst, const DirectWriteTextOptions& o) noexcept {
    DirectWriteTextResult result{};
    if (!dst.rgba || dst.width <= 0 || dst.height <= 0 ||
        dst.rowStrideFloats < static_cast<std::ptrdiff_t>(dst.width) * 4 || !o.utf8Text || !*o.utf8Text)
        return result;
    try {
        ComApartment apartment; if (!apartment.valid()) return result;
        auto& f = factories(); if (FAILED(f.status) || !f.d2d || !f.dwrite) return result;
        std::wstring text, family;
        if (!utf8ToWide(o.utf8Text, text)) return result;
        if (!utf8ToWide(o.utf8FontFamily ? o.utf8FontFamily : "Segoe UI", family)) family = L"Segoe UI";
        result.requestedFontFound = findFont(f.dwrite.Get(), family.c_str());
        result.usedFallbackFont = !result.requestedFontFound;
        ComPtr<IWICImagingFactory> wic;
        HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wic));
        if (FAILED(hr)) return result;
        const float x = finiteClamp(o.x, -static_cast<float>(dst.width), static_cast<float>(dst.width), 0.0f);
        const float y = finiteClamp(o.y, -static_cast<float>(dst.height), static_cast<float>(dst.height), 0.0f);
        const int originX = std::max(0, std::min(dst.width - 1, static_cast<int>(std::floor(x))));
        const int originY = std::max(0, std::min(dst.height - 1, static_cast<int>(std::floor(y))));
        const float mw = finiteClamp(o.maxWidth, 0.0f, static_cast<float>(dst.width - originX), 0.0f);
        const float mh = finiteClamp(o.maxHeight, 0.0f, static_cast<float>(dst.height - originY), 0.0f);
        const UINT bitmapWidth = static_cast<UINT>(std::max(1.0, std::ceil(mw > 0.0f ? static_cast<double>(mw) : static_cast<double>(dst.width - originX))));
        const UINT bitmapHeight = static_cast<UINT>(std::max(1.0, std::ceil(mh > 0.0f ? static_cast<double>(mh) : static_cast<double>(dst.height - originY))));
        ComPtr<IWICBitmap> bitmap;
        hr = wic->CreateBitmap(bitmapWidth, bitmapHeight, GUID_WICPixelFormat32bppPBGRA,
                               WICBitmapCacheOnLoad, &bitmap);
        if (FAILED(hr)) return result;
        ComPtr<ID2D1RenderTarget> target;
        const auto props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
        hr = f.d2d->CreateWicBitmapRenderTarget(bitmap.Get(), props, &target);
        if (FAILED(hr)) return result;
        const float localX = x - static_cast<float>(originX);
        const float localY = y - static_cast<float>(originY);
        const float lw = static_cast<float>(bitmapWidth);
        const float lh = static_cast<float>(bitmapHeight);
        const float fs = finiteClamp(o.fontSize, 1.0f, 512.0f, 28.0f);
        ComPtr<IDWriteTextFormat> format;
        const auto weight=static_cast<DWRITE_FONT_WEIGHT>(std::clamp<std::uint16_t>(o.fontWeight,100,900));
        hr = f.dwrite->CreateTextFormat(family.c_str(), nullptr, weight,
            DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fs, L"en-us", &format);
        if (FAILED(hr)) {
            hr = f.dwrite->CreateTextFormat(L"Segoe UI", nullptr, weight,
                DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fs, L"en-us", &format);
            if (FAILED(hr)) return result;
            result.usedFallbackFont = true;
        }
        if (o.direction == TextDirection::RightToLeft) format->SetReadingDirection(DWRITE_READING_DIRECTION_RIGHT_TO_LEFT);
        else if (o.direction == TextDirection::LeftToRight) format->SetReadingDirection(DWRITE_READING_DIRECTION_LEFT_TO_RIGHT);
        else format->SetReadingDirection(inferReadingDirection(text));
        if (o.alignment == TextAlignment::Center) format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        else if (o.alignment == TextAlignment::Trailing) format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
        else format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        ComPtr<IDWriteTextLayout> layout;
        hr = f.dwrite->CreateTextLayout(text.c_str(), static_cast<UINT32>(text.size()), format.Get(), lw, lh, &layout);
        if (FAILED(hr)) return result;
        ComPtr<ID2D1SolidColorBrush> brush;
        const auto color = D2D1::ColorF(finiteClamp(o.red,0,1,1), finiteClamp(o.green,0,1,1),
                                        finiteClamp(o.blue,0,1,1), finiteClamp(o.opacity,0,1,1));
        hr = target->CreateSolidColorBrush(color, &brush); if (FAILED(hr)) return result;
        target->BeginDraw(); target->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
        target->DrawTextLayout(D2D1::Point2F(localX,localY), layout.Get(), brush.Get(), D2D1_DRAW_TEXT_OPTIONS_CLIP);
        hr = target->EndDraw(); if (FAILED(hr)) return result;
        ComPtr<IWICBitmapLock> lock; WICRect rect{0,0,static_cast<INT>(bitmapWidth),static_cast<INT>(bitmapHeight)};
        hr = bitmap->Lock(&rect, WICBitmapLockRead, &lock); if (FAILED(hr)) return result;
        UINT size = 0, stride = 0; BYTE* pixels = nullptr;
        hr = lock->GetStride(&stride); if (FAILED(hr)) return result;
        hr = lock->GetDataPointer(&size, &pixels);
        if (FAILED(hr) || !pixels || stride < bitmapWidth*4 ||
            size < stride*bitmapHeight) return result;
        compositePremultipliedRect(dst, pixels, stride, bitmapWidth, bitmapHeight, originX, originY, o.premultipliedDestination);
        result.rendered = true; return result;
    } catch (...) { return result; }
}
} // namespace vhs90::win
