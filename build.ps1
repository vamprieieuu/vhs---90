param(
    [string]$AeSdkRoot = $env:AE_SDK_ROOT,
    [string]$VhsArchivePath = $env:VHS_ARCHIVE_PATH,
    [string]$BuildDir = "build",
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$repoRoot = $PSScriptRoot
$checkedInVhsRoot = Join-Path $repoRoot "VHS"
$checkedInSdk = Join-Path $checkedInVhsRoot "open source native plugin for ae/VHS6RIFT/extern/AfterEffectsSDK"
$checkedInFonts = Join-Path $checkedInVhsRoot "fonts"
$fontDirectory = Join-Path $repoRoot "Resources/Fonts"
New-Item -ItemType Directory -Force -Path $fontDirectory | Out-Null

# Prefer the VHS6RIFT SDK and original fonts already checked into this repository.
# This removes the need to expose a private ChatGPT Library archive to GitHub Actions.
if (([string]::IsNullOrWhiteSpace($AeSdkRoot) -or -not (Test-Path (Join-Path $AeSdkRoot "Headers/AE_Effect.h"))) -and
    (Test-Path (Join-Path $checkedInSdk "Headers/AE_Effect.h"))) {
    $AeSdkRoot = $checkedInSdk
    Write-Host "Using checked-in VHS6RIFT SDK: $AeSdkRoot"
}

$fontEntries = @{
    "Rooyin_Free_Bold.ttf" = "Rooyin_Free_Bold.ttf"
    "Sponge_Arabic_Regular.otf" = "Sponge_Arabic_Regular.otf"
}
foreach ($fontName in $fontEntries.Keys) {
    $source = Join-Path $checkedInFonts $fontName
    $destination = Join-Path $fontDirectory $fontName
    if ((Test-Path $source) -and -not (Test-Path $destination)) {
        Copy-Item -LiteralPath $source -Destination $destination
        Write-Host "Copied bundled font from repository: $fontName"
    }
}

if ([string]::IsNullOrWhiteSpace($VhsArchivePath)) {
    $candidateArchive = Join-Path $repoRoot "vhs.zip"
    if (Test-Path $candidateArchive) { $VhsArchivePath = $candidateArchive }
}

# Optional compatibility fallback: extract SDK/fonts from the original archive
# only when the checked-in repository assets are not available.
$hasCheckedInFonts = (Test-Path (Join-Path $fontDirectory "Rooyin_Free_Bold.ttf")) -and
                     (Test-Path (Join-Path $fontDirectory "Sponge_Arabic_Regular.otf"))
$hasCheckedInSdk = Test-Path (Join-Path $AeSdkRoot "Headers/AE_Effect.h")
if (-not [string]::IsNullOrWhiteSpace($VhsArchivePath) -and
    (-not $hasCheckedInSdk -or -not $hasCheckedInFonts)) {
    if (-not (Test-Path $VhsArchivePath)) { throw "VHS archive not found: $VhsArchivePath" }
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $archive = [System.IO.Compression.ZipFile]::OpenRead((Resolve-Path $VhsArchivePath).Path)
    try {
        if (-not $hasCheckedInSdk) {
            $sdkPrefix = "vhs/open source native plugin for ae/VHS6RIFT/extern/AfterEffectsSDK/"
            $sdkTarget = Join-Path (Join-Path $repoRoot $BuildDir) "_sdk/AfterEffectsSDK"
            foreach ($entry in $archive.Entries) {
                if (-not $entry.FullName.StartsWith($sdkPrefix, [StringComparison]::Ordinal)) { continue }
                $relative = $entry.FullName.Substring($sdkPrefix.Length).Replace('/', [IO.Path]::DirectorySeparatorChar)
                if ([string]::IsNullOrWhiteSpace($relative)) { continue }
                $destination = Join-Path $sdkTarget $relative
                $fullDestination = [IO.Path]::GetFullPath($destination)
                $sdkRootPrefix = [IO.Path]::GetFullPath($sdkTarget).TrimEnd([IO.Path]::DirectorySeparatorChar, [IO.Path]::AltDirectorySeparatorChar) + [IO.Path]::DirectorySeparatorChar
                if (-not $fullDestination.StartsWith($sdkRootPrefix, [StringComparison]::OrdinalIgnoreCase)) {
                    throw "Unsafe SDK archive entry: $($entry.FullName)"
                }
                if ($entry.FullName.EndsWith('/')) {
                    New-Item -ItemType Directory -Force -Path $destination | Out-Null
                } else {
                    New-Item -ItemType Directory -Force -Path (Split-Path $destination) | Out-Null
                    [System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $destination, $true)
                }
            }
            $AeSdkRoot = $sdkTarget
        }
        if (-not $hasCheckedInFonts) {
            $archiveFontEntries = @{
                "vhs/fonts/Rooyin_Free_Bold.ttf" = "Rooyin_Free_Bold.ttf"
                "vhs/fonts/Sponge_Arabic_Regular.otf" = "Sponge_Arabic_Regular.otf"
            }
            foreach ($entry in $archive.Entries) {
                if ($archiveFontEntries.ContainsKey($entry.FullName)) {
                    $destination = Join-Path $fontDirectory $archiveFontEntries[$entry.FullName]
                    [System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $destination, $true)
                }
            }
        }
    } finally {
        $archive.Dispose()
    }
}

$fontFiles = @(
    (Join-Path $fontDirectory "Rooyin_Free_Bold.ttf"),
    (Join-Path $fontDirectory "Sponge_Arabic_Regular.otf")
)
$missingFonts = @($fontFiles | Where-Object { -not (Test-Path $_) })
if ($missingFonts.Count -gt 0) {
    throw "Bundled OFL font files are missing. Commit the original licensed files under VHS/fonts or supply vhs.zip. Missing: $($missingFonts -join ', ')"
}

$expectedFontHashes = @{
    "Rooyin_Free_Bold.ttf" = "b74d6379ee2c97049e2e144ac2bda09cc0f30771d8c5697c901fd39fe32b90f1"
    "Sponge_Arabic_Regular.otf" = "15bed437224230c115ad8349f200211615eba5a7d3ddf007862199cf1ae90c5c"
}
foreach ($font in $fontFiles) {
    $name = Split-Path $font -Leaf
    $actualHash = (Get-FileHash -Algorithm SHA256 $font).Hash.ToLowerInvariant()
    if ($actualHash -ne $expectedFontHashes[$name]) {
        throw "Font checksum mismatch for $name. Use the unmodified original licensed font."
    }
}

$FindSdkScript = Join-Path $PSScriptRoot "tools/find_ae_sdk.ps1"
$SdkInfo = & $FindSdkScript -CandidatePath $AeSdkRoot -Quiet
if (-not $SdkInfo -or -not $SdkInfo.IsValid) {
    throw "Adobe After Effects SDK discovery failed. Checked VHS6RIFT in-repository SDK and optional archive. Run tools/find_ae_sdk.ps1 for diagnostics."
}
$resolvedSdk = $SdkInfo.SdkRoot
Write-Host "AE SDK: $resolvedSdk"
Write-Host "AE Headers: $($SdkInfo.HeadersDir)"
Write-Host "PiPLtool: $($SdkInfo.PiPLtool)"
Write-Host "Bundled fonts: $($fontFiles -join '; ')"

cmake -S $repoRoot -B $BuildDir -G "Visual Studio 17 2022" -A x64 -DVHS90_BUILD_AE_PLUGIN=ON "-DAE_SDK_ROOT=$resolvedSdk"
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }

# Match VHS6RIFT's default all-target build: compile the AEX plus the CPU smoke-test executables.
# Only build/VHS-90.aex is published; test binaries remain internal to the build workspace.
cmake --build $BuildDir --config $Configuration --parallel
if ($LASTEXITCODE -ne 0) { throw "VHS-90 native build failed." }

ctest --test-dir $BuildDir -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "VHS-90 CPU tests failed." }

$artifact = Join-Path $BuildDir "VHS-90.aex"
if (-not (Test-Path $artifact)) { throw "Build completed but AEX artifact is missing: $artifact" }
if ((Get-Item $artifact).Length -lt 65536) { throw "AEX output is unexpectedly small." }
Write-Host "BUILT: $artifact"
