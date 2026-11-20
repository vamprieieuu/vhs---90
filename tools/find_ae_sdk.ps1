[CmdletBinding()]
param(
    [string]$CandidatePath = "",
    [switch]$Quiet
)
$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$candidates = [System.Collections.Generic.List[string]]::new()
foreach($p in @($CandidatePath,$env:AE_SDK_ROOT,$env:AE_SDK_PATH,(Join-Path $repoRoot "extern\AfterEffectsSDK"),(Join-Path $repoRoot "AfterEffectsSDK"))) {
    if(-not [string]::IsNullOrWhiteSpace($p)) { $candidates.Add($p) }
}
$checked = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
$result = $null
foreach($candidate in $candidates) {
    if(-not (Test-Path $candidate)) { continue }
    $root = (Resolve-Path $candidate).Path
    if(-not $checked.Add($root)) { continue }
    $effect = $null
    $direct = Join-Path $root "Headers\AE_Effect.h"
    $examples = Join-Path $root "Examples\Headers\AE_Effect.h"
    if(Test-Path $direct) { $effect = Get-Item $direct }
    elseif(Test-Path $examples) { $effect = Get-Item $examples }
    else { $effect = Get-ChildItem -Path $root -Filter "AE_Effect.h" -File -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1 }
    if(-not $effect) { continue }
    $headers = $effect.Directory.FullName
    $includeRoot = $effect.Directory.Parent.FullName
    if($effect.Directory.Parent.Name -eq "Examples") { $sdkRoot = $effect.Directory.Parent.Parent.FullName }
    else { $sdkRoot = $includeRoot }
    $utilCandidates = @((Join-Path $includeRoot "Util"),(Join-Path $sdkRoot "Util"))
    $util = $utilCandidates | Where-Object { Test-Path (Join-Path $_ "Param_Utils.h") } | Select-Object -First 1
    $required = @("AE_Effect.h","AE_EffectCB.h","AE_EffectCBSuites.h","AE_EffectUI.h","AE_Macros.h")
    $missing = @($required | Where-Object { -not (Test-Path (Join-Path $headers $_)) })
    if(-not $util) { $missing += "Util/Param_Utils.h" }
    $pipl = @((Join-Path $sdkRoot "Resources\PiPLtool.exe"),(Join-Path $includeRoot "Resources\PiPLtool.exe"),(Join-Path $sdkRoot "Examples\Resources\PiPLtool.exe")) | Where-Object { Test-Path $_ } | Select-Object -First 1
    if(-not $pipl) { $pipl = (Get-ChildItem -Path $sdkRoot -Filter "PiPLtool.exe" -File -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1).FullName }
    $result = [PSCustomObject]@{
        IsValid = ($missing.Count -eq 0 -and -not [string]::IsNullOrWhiteSpace($pipl))
        SdkRoot = $sdkRoot
        IncludeRoot = $includeRoot
        HeadersDir = $headers
        UtilDir = $util
        PiPLtool = $pipl
        MissingHeaders = $missing
        MissingPiPLtool = [string]::IsNullOrWhiteSpace($pipl)
    }
    if($result.IsValid) { break }
}
if($result -and $result.IsValid) {
    $env:AE_SDK_ROOT = $result.SdkRoot
    $env:AE_INCLUDE_ROOT = $result.IncludeRoot
    $env:AE_HEADERS = $result.HeadersDir
    $env:PIPL_TOOL = $result.PiPLtool
    if(-not $Quiet) {
        Write-Host "Adobe After Effects SDK validated." -ForegroundColor Green
        Write-Host "SDK Root: $($result.SdkRoot)"
        Write-Host "Headers:  $($result.HeadersDir)"
        Write-Host "Util:     $($result.UtilDir)"
        Write-Host "PiPLtool: $($result.PiPLtool)"
    }
    return $result
}
if(-not $Quiet) {
    Write-Host "Adobe After Effects SDK missing or incomplete." -ForegroundColor Red
    Write-Host "Checked: $($checked -join '; ')"
    if($result) { Write-Host "Missing headers: $($result.MissingHeaders -join ', ')" }
    Write-Host "Set AE_SDK_ROOT or AE_SDK_PATH, or pass -CandidatePath."
    Write-Host "Supported layouts: Headers/, Examples/Headers/, and nested SDK extraction folders."
}
return [PSCustomObject]@{ IsValid=$false; Checked=@($checked); Result=$result }
