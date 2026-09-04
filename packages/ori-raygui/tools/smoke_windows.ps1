param([string]$Triple = "x86_64-pc-windows-msvc")
$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent $scriptDir
$ori = $env:ORI_BIN
if (-not $ori) { $c = Get-Command ori -EA SilentlyContinue; if ($c) { $ori = $c.Source } }
if (-not $ori) { throw "Set ORI_BIN" }
$env:ORI_USE_SYSTEM_LINKER = if ($env:ORI_USE_SYSTEM_LINKER) { $env:ORI_USE_SYSTEM_LINKER } else { "1" }

& (Join-Path $scriptDir "build_windows.ps1") -Triple $Triple -StubRaylib
& $ori check (Join-Path $root "raygui\ui.orl")
if ($LASTEXITCODE -ne 0) { throw "check failed" }
Write-Host "smoke_windows (ori-raygui): check OK (UI demos need real raylib + display)"
