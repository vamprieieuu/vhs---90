# Windows AEX build and delivery

## Canonical inputs

Use the original user-supplied `vhs.zip`. The build script extracts:

- `vhs/open source native plugin for ae/VHS6RIFT/extern/AfterEffectsSDK/`
- `vhs/fonts/Rooyin_Free_Bold.ttf`
- `vhs/fonts/Sponge_Arabic_Regular.otf`

The build script verifies both font files against their audited SHA-256 hashes and preserves the complete files.

## Local Windows build

Run from the repository root in PowerShell on Windows with Visual Studio 2022 C++ tools installed:

```powershell
.\build.ps1 -VhsArchivePath "D:\path\to\vhs.zip" -BuildDir build -Configuration Release
```

The only plugin deliverable is written directly to:

```
build\VHS-90.aex
```

There is no `dist\`, `Release\`, or nested artifact folder. The script stops on SDK, PiPL, compile, test, or missing-output errors.

## GitHub-hosted build

Open Actions → **VHS-90 Windows AEX Build** → Run workflow. Supply a downloadable URL for the original archive if `vhs.zip` is not already in the checkout. The hosted Windows 2022 runner uses the VHS6RIFT SDK extracted by `build.ps1`.

After a successful build and CTest run, the workflow:
- uploads an Actions artifact containing only `VHS-90.aex`;
- creates a GitHub Release with the raw `.aex` file as a direct download.

The private ChatGPT Library file is not automatically available to GitHub-hosted runners. Do not publish the Adobe SDK archive publicly merely to make CI pass; use an authorized private/signed download source or perform the local Windows build.

## Release gates

A successful native build and CPU smoke suite still do not prove the effect loads or renders correctly in After Effects. The final host check must be performed in AE 2023 / Windows 11: load the AEX, apply it to footage, render, save/reopen the project, duplicate the effect, test MFR, and inspect Arabic/English OSD output.
