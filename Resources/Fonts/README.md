# Bundled Arabic fonts

The Windows AEX build embeds the two original font files from the supplied
`vhs.zip` archive as RCDATA resources inside `VHS-90.aex`:

- `Rooyin_Free_Bold.ttf` — RooyinFree family; full original file retained.
- `Sponge_Arabic_Regular.otf` — YR Sponge Arabic family; full original file retained.

The build script extracts only these two assets from `vhs/fonts/`. The other
supplied fonts remain excluded from redistribution because their license or
embedding permission is absent, restrictive, or unverified.

At runtime the plug-in writes the embedded font resources to a per-user
temporary folder and registers them with `FR_PRIVATE`. This does not install
fonts machine-wide. The DirectWrite smoke test checks that both family names
are discoverable and can render Arabic text without falling back.

The original files are not modified or subsetted. License text and attribution
are included in `OFL-1.1.txt`.
