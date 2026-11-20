# VHS-90 — DirectWrite renderer implementation

Date: 2026-11-21
Branch: work/osd-directwrite-integration-map

## Implemented
- Added a Windows-only DirectWrite/Direct2D/WIC text overlay adapter under Source/VHS90Windows.
- UTF-8 conversion is strict (MultiByteToWideChar with MB_ERR_INVALID_CHARS); malformed input fails safely.
- IDWriteTextLayout handles shaping, bidi, Arabic joining, ligatures and combining-mark placement. The API exposes Auto/LTR/RTL reading direction.
- Text renders to WIC 32bpp premultiplied BGRA, then composites into float ImageView with row-stride support and premultiplied/straight destination alpha.
- Added system font enumeration, requested-family detection, and DirectWrite's normal fallback behavior.
- Added a Windows-only smoke test for mixed Arabic/Latin text, system font enumeration, output coverage and alpha preservation.
- Portable VHS90Core remains independent of Windows headers and COM.

## Not yet integrated / not claimed
- The adapter is not yet called from VHS90Effect.cpp. Existing bitmap OSD and its pre-Composite position are unchanged.
- No AE free-text parameter has been added. AE SDK headers are external via AE_SDK_ROOT; the repository does not vendor those headers and contains no existing string-parameter example to safely copy. The exact target SDK API and persistence/disk-ID policy must be confirmed before changing parameter layout.
- No font binaries were bundled. System font enumeration and fallback do not require redistribution. Any bundled font must include the complete approved binary and license notice.
- No Windows/MSVC build, DirectWrite visual test, AEX build or in-host After Effects validation is claimed.

## Next
1. Confirm the target SDK string parameter API and persistence behavior from the actual SDK headers.
2. Add a version-safe text control and transport settings through both legacy and Smart Render paths.
3. Invoke this renderer inside the existing OSD stage immediately before Composite, preserving bitmap HUD fallback.
4. Capture visual acceptance tests for Arabic shaping, mixed direction, font fallback and downstream VHS signal damage.
