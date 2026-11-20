param(
    [string]$AeSdkRoot = $env:AE_SDK_ROOT,
    [string]$BuildDir = "build",
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$FindSdkScript = Join-Path $PSScriptRoot "tools/find_ae_sdk.ps1"
$SdkInfo = & $FindSdkScript -CandidatePath $AeSdkRoot -Quiet
if (-not $SdkInfo -or -not $SdkInfo.IsValid) {
    throw "Adobe After Effects SDK discovery failed. Run tools/find_ae_sdk.ps1 for diagnostics."
}
$resolvedSdk = $SdkInfo.SdkRoot
Write-Host "AE SDK: $resolvedSdk"
Write-Host "AE Headers: $($SdkInfo.HeadersDir)"
Write-Host "PiPLtool: $($SdkInfo.PiPLtool)"

cmake -S . -B $BuildDir -G "Visual Studio 17 2022" -A x64 -DVHS90_BUILD_AE_PLUGIN=ON "-DAE_SDK_ROOT=$resolvedSdk"
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }

cmake --build $BuildDir --config $Configuration --target VHS90_AEX --parallel
if ($LASTEXITCODE -ne 0) { throw "VHS-90 native build failed." }

ctest --test-dir $BuildDir -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "VHS-90 CPU tests failed." }

$artifact = Join-Path $BuildDir "VHS-90.aex"
if (-not (Test-Path $artifact)) { throw "Build completed but AEX artifact is missing: $artifact" }
Write-Host "BUILT: $artifact"
