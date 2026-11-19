# VHS-90 — Supplied Font Pack Audit

Date: 2026-11-19
Input: `vhs/fonts/` from the supplied `vhs.zip`
Inspection: all 15 font files parsed with fontTools; family/full-name metadata, Unicode cmap coverage, Arabic digit coverage and OpenType GSUB feature tags recorded.

## Exact supplied filenames and metadata

| Supplied filename | Internal family / full name | Glyph count | Arabic shaping features |
|---|---|---:|---|
| `A Massir Ballpoint.ttf` | A Massir Ballpoint | 441 | fina, init, isol, liga, medi, rlig |
| `A Naskh Tahrir.ttf` | A Naskh Tahrir | 295 | fina, init, liga, medi, mset |
| `a-bad-khat.ttf` | A-bad kh@t | 300 | fina, init, liga, medi, mset |
| `alfont_com_Guesswhat-Exceptional.otf` | Guesswhat Exceptional | 80 | aalt, ccmp, dlig, fina, init, isol, liga, medi, rlig, ss01–ss03 |
| `Arabic_Childhood_Regular.ttf` | Arabic Childhood ver1 / خط طفولة (الإِصدَارُ الأوّلُ) | 92 | fina, init, medi, rlig |
| `arialnarrow_bold.ttf` | Arial Narrow Bold | 697 | no GSUB features |
| `Evogria.otf` | Evogria | 537 | no GSUB features |
| `k-kamran.ttf` | K Kamran | 500 | fina, init, isol, liga, medi, mset |
| `Maqroo_Arabick_Regular.ttf` | Maqroo Regular | 230 | aalt, case, ccmp, dlig, fina, frac, init, isol, lnum, locl, medi, onum, pnum, rlig, ss01, ss02, sups, tnum |
| `Milan_Titles_Black.otf` | Milan Display Black | 467 | alef, fina, init, liga, medi, rlig, ss01, ss02, swsh, yeh |
| `nasma.ttf` | Nasma | 283 | ccmp, fina, init, isol, liga, medi, rlig |
| `RealVhsFontRegular-WyV0z.ttf` | Real Vhs Font Regular | 55 | no GSUB features |
| `Rooyin_Free_Bold.ttf` | RooyinFree Bold | 725 | ccmp, fina, init, locl, medi, rlig, ss01–ss06, subs, sups, swsh, tnum |
| `Sponge_Arabic_Regular.otf` | YR Sponge Arabic | 1,245 | aalt, ccmp, dlig, fina, frac, init, liga, locl, medi, ordn, rlig |
| `ZIAN_Black.ttf` | ZIAN Black | 277 | KASH, aalt, calt, ccmp, dlig, fina, init, isol, liga, medi, ornm, rlig, ss01–ss06, swsh, titl, warq |

## Coverage findings

- 15 font files are present; 10 have Arabic codepoints in U+0600–U+06FF and OpenType joining/shaping features. The other five are Latin/graphic display faces.
- All 10 Arabic-capable fonts include Arabic-Indic digits U+0660–U+0669.
- Six Arabic-capable fonts include Persian digits U+06F0–U+06F9: A Massir Ballpoint, A Naskh Tahrir, a-bad-khat, K Kamran, Maqroo, and Rooyin.
- Arabic Childhood does not contain Latin digits; Guesswhat Exceptional also lacks Latin digits and common OSD punctuation. These need a documented fallback within the supplied font pack if used for date/time or mixed-language labels.
- Latin digits are present in 13 of 15 fonts; Evogria, Arial Narrow Bold and Real VHS Font are Latin/display-oriented and have no Arabic glyphs.
- Arabic fonts expose joining features such as `init`, `medi`, `fina`, `isol` and `rlig`; this confirms the font files contain shaping tables, but does not by itself prove that the plugin's text renderer will shape Arabic correctly.

## VHS-90 Text / OSD integration requirements

1. Bundle these exact 15 files under the plugin's internal font resources. Do not silently substitute system fonts or download additional fonts.
2. Keep the original supplied filenames intact in the font selector and resource manifest. Internal family names are metadata and should not overwrite the user's requested displayed filenames.
3. Use a real shaping/layout engine for Arabic joining, ligatures, bidi ordering and diacritics; glyph cmap presence alone is not enough. On Windows, evaluate DirectWrite/Uniscribe integration in the native AE environment.
4. For Arabic locale, format digits as U+0660–U+0669. Where a chosen font lacks the requested digit set or Latin digits, choose a compatible fallback from the same supplied pack without changing the selected text's script or direction.
5. Date/month localization must be handled by VHS-90's locale/date formatter, not baked into font assets.
6. Text/OSD should be rendered procedurally from font glyphs; do not convert text to static PNG overlays.

## Validation limits

- All 15 font binaries were parsed successfully for metadata and cmap/GSUB inspection.
- No actual DirectWrite/Uniscribe shaping or raster preview was executed in this environment.
- This audit does not yet claim that the font binaries have been copied into the VHS-90 repository or packaged into an AEX.
