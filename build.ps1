param(
    [string]$AeSdkRoot = $env:AE_SDK_ROOT,
    [string]$VhsArchivePath = $env:VHS_ARCHIVE_PATH,
    [string]$BuildDir = "build",
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$repoRoot = $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($VhsArchivePath)) {
    $candidateArchive = Join-Path $repoRoot "vhs.zip"
    if (Test-Path $candidateArchive) { $VhsArchivePath = $candidateArchive }
}

# The user-supplied vhs.zip is the canonical source for both the VHS6RIFT
# After Effects SDK and the two redistribution-cleared Arabic font files.
if (-not [string]::IsNullOrWhiteSpace($VhsArchivePath)) {
    if (-not (Test-Path $VhsArchivePath)) { throw "VHS archive not found: $VhsArchivePath" }
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $archive = [System.IO.Compression.ZipFile]::OpenRead((Resolve-Path $VhsArchivePath).Path)
    try {
        $sdkPrefix = "vhs/open source native plugin for ae/VHS6RIFT/extern/AfterEffectsSDK/"
        $sdkTarget = Join-Path (Join-Path $repoRoot $BuildDir) "_sdk/AfterEffectsSDK"
        if ([string]::IsNullOrWhiteSpace($AeSdkRoot) -or -not (Test-Path (Join-Path $AeSdkRoot "Headers/AE_Effect.h"))) {
            foreach ($entry in $archive.Entries) {
                if (-not $entry.FullName.StartsWith($sdkPrefix, [StringComparison]::Ordinal)) { continue }
                $relative = $entry.FullName.Substring($sdkPrefix.Length).Replace('/', [IO.Path]::DirectorySeparatorChar)
                if ([string]::IsNullOrWhiteSpace($relative)) { continue }
                $destination = Join-Path $sdkTarget $relative
                $fullDestination = [IO.Path]::GetFullPath($destination)
                if (-not $fullDestination.StartsWith([IO.Path]::GetFullPath($sdkTarget), [StringComparison]::OrdinalIgnoreCase)) {
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
            Write-Host "Extracted the SDK from VHS6RIFT: $AeSdkRoot"
        }

        $fontDirectory = Join-Path $repoRoot "Resources/Fonts"
        New-Item -ItemType Directory -Force -Path $fontDirectory | Out-Null
        $fontEntries = @{
            "vhs/fonts/Rooyin_Free_Bold.ttf" = "Rooyin_Free_Bold.ttf"
            "vhs/fonts/Sponge_Arabic_Regular.otf" = "Sponge_Arabic_Regular.otf"
        }
        foreach ($entry in $archive.Entries) {
            if ($fontEntries.ContainsKey($entry.FullName)) {
                $destination = Join-Path $fontDirectory $fontEntries[$entry.FullName]
                [System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $destination, $true)
            }
        }
    } finally {
        $archive.Dispose()
    }
}

$fontFiles = @(
    (Join-Path $repoRoot "Resources/Fonts/Rooyin_Free_Bold.ttf"),
    (Join-Path $repoRoot "Resources/Fonts/Sponge_Arabic_Regular.otf")
)
$missingFonts = @($fontFiles | Where-Object { -not (Test-Path $_) })
if ($missingFonts.Count -gt 0) {
    throw "Bundled OFL font files are missing. Supply the original vhs.zip using -VhsArchivePath or place both licensed font files in Resources/Fonts. Missing: $($missingFonts -join ', ')"
}

$expectedFontHashes = @{
    "Rooyin_Free_Bold.ttf" = "b74d6379ee2c97049e2e144ac2bda09cc0f30771d8c5697c901fd39fe32b90f1"
    "Sponge_Arabic_Regular.otf" = "15bed437224230c115ad8349f200211615eba5a7d3ddf007862199cf1ae90c5c"
}
foreach ($font in $fontFiles) {
    $name = Split-Path $font -Leaf
    $actualHash = (Get-FileHash -Algorithm SHA256 $font).Hash.ToLowerInvariant()
    if ($actualHash -ne $expectedFontHashes[$name]) {
        throw "Font checksum mismatch for $name. Use the unmodified original font from vhs.zip."
    }
}

$FindSdkScript = Join-Path $PSScriptRoot "tools/find_ae_sdk.ps1"
$SdkInfo = & $FindSdkScript -CandidatePath $AeSdkRoot -Quiet
if (-not $SdkInfo -or -not $SdkInfo.IsValid) {
    throw "Adobe After Effects SDK discovery failed. Supply the VHS6RIFT SDK root or the original vhs.zip. Run tools/find_ae_sdk.ps1 for diagnostics."
}
$resolvedSdk = $SdkInfo.SdkRoot
Write-Host "AE SDK: $resolvedSdk"
Write-Host "AE Headers: $($SdkInfo.HeadersDir)"
Write-Host "PiPLtool: $($SdkInfo.PiPLtool)"
Write-Host "Bundled fonts: $($fontFiles -join '; ')"

cmake -S $repoRoot -B $BuildDir -G "Visual Studio 17 2022" -A x64 -DVHS90_BUILD_AE_PLUGIN=ON "-DAE_SDK_ROOT=$resolvedSdk"
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }

cmake --build $BuildDir --config $Configuration --target VHS90_AEX --parallel
if ($LASTEXITCODE -ne 0) { throw "VHS-90 native build failed." }

ctest --test-dir $BuildDir -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "VHS-90 CPU tests failed." }

$artifact = Join-Path $BuildDir "VHS-90.aex"
if (-not (Test-Path $artifact)) { throw "Build completed but AEX artifact is missing: $artifact" }
if ((Get-Item $artifact).Length -lt 65536) { throw "AEX output is unexpectedly small." }
Write-Host "BUILT: $artifact"
