param([string]$Triple = "x86_64-pc-windows-msvc")
$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent $scriptDir
$ori = $env:ORI_BIN
if (-not $ori) { $c = Get-Command ori -EA SilentlyContinue; if ($c) { $ori = $c.Source } }
if (-not $ori) { throw "Set ORI_BIN" }
$env:ORI_USE_SYSTEM_LINKER = if ($env:ORI_USE_SYSTEM_LINKER) { $env:ORI_USE_SYSTEM_LINKER } else { "1" }

& (Join-Path $scriptDir "build_windows.ps1") -Triple $Triple
& $ori check (Join-Path $root "rres\pack.orl")
if ($LASTEXITCODE -ne 0) { throw "check failed" }
$src = Join-Path $root "examples\pack_roundtrip.orl"
if (Test-Path $src) {
    $out = Join-Path $env:TEMP "rres_roundtrip.exe"
    & $ori compile $src --out $out
    if ($LASTEXITCODE -ne 0) { throw "compile failed" }
    & $out
    if ($LASTEXITCODE -ne 0) { throw "run failed" }
}
Write-Host "smoke_windows (ori-rres): all green"
