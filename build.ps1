param(
    [string]$AeSdkRoot = $env:AE_SDK_ROOT,
    [string]$BuildDir = "build",
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($AeSdkRoot)) {
    throw "Set AE_SDK_ROOT to the extracted Adobe After Effects SDK directory."
}
if (-not (Test-Path (Join-Path $AeSdkRoot "Headers/AE_Effect.h"))) {
    throw "AE SDK headers not found under: $AeSdkRoot"
}
if (-not (Test-Path (Join-Path $AeSdkRoot "Resources/PiPLtool.exe"))) {
    throw "PiPLtool.exe not found under: $AeSdkRoot"
}

$resolvedSdk = (Resolve-Path $AeSdkRoot).Path
cmake -S . -B $BuildDir -G "Visual Studio 17 2022" -A x64 -DVHS90_BUILD_AE_PLUGIN=ON "-DAE_SDK_ROOT=$resolvedSdk"
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }

cmake --build $BuildDir --config $Configuration --target VHS90_AEX --parallel
if ($LASTEXITCODE -ne 0) { throw "VHS-90 native build failed." }

ctest --test-dir $BuildDir -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "VHS-90 CPU tests failed." }

$artifact = Join-Path $BuildDir "dist/VHS-90.aex"
if (-not (Test-Path $artifact)) { throw "Build completed but AEX artifact is missing: $artifact" }
Write-Host "BUILT: $artifact"
