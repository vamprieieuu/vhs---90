# Windows AEX build and delivery

## Canonical checked-in inputs

The repository includes the original VHS6RIFT build foundation and font source files:

- SDK: `VHS/open source native plugin for ae/VHS6RIFT/extern/AfterEffectsSDK/`
- PiPL compiler: SDK `Resources/PiPLtool.exe`
- Font source: `VHS/fonts/`

The build copies only the two fonts cleared for full-file redistribution by the audited font manifest:

- `Rooyin_Free_Bold.ttf`
- `Sponge_Arabic_Regular.otf`

It verifies both files against their audited SHA-256 hashes before compiling them as RCDATA resources into the AEX. Other supplied fonts remain excluded because their license or embedding permission is absent, restrictive, or unverified.

An original `vhs.zip` is no longer required for the normal GitHub-hosted build. The optional workflow URL input remains only as a fallback for a checkout missing the checked-in source assets.

## Local Windows build

Run from the repository root in PowerShell on Windows with Visual Studio 2022 C++ tools installed:

```powershell
.\build.ps1 -BuildDir build -Configuration Release
```

The only plugin deliverable is written directly to:

```
build\VHS-90.aex
```

There is no `dist\`, `Release\`, or nested artifact folder. The script stops on SDK, PiPL, checksum, compile, test, or missing-output errors.

## GitHub-hosted build

Open Actions → **VHS-90 Windows AEX Build** → Run workflow. Leave the optional archive URL blank when the checked-in `VHS/` directory is present.

After a successful native build and CTest run, the workflow uploads an Actions artifact containing only `VHS-90.aex` and creates a GitHub Release with the raw `.aex` file as a direct download.

## Release gates

A successful native build and CPU smoke suite still do not prove the effect loads or renders correctly in After Effects. The final host check must be performed in AE 2023 / Windows 11: load the AEX, apply it to footage, render, save/reopen the project, duplicate the effect, test MFR, and inspect Arabic/English OSD output.


## Raw AEX delivery

The Build workflow publishes only `VHS-90.aex` as a raw GitHub Release asset. It intentionally does not use `actions/upload-artifact`, because GitHub Actions artifacts are downloaded as ZIP archives even when the uploaded payload is a single AEX. The build workspace contains the AEX at `build/VHS-90.aex`; the direct Release download is the user-facing deliverable.
