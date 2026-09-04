# Windows smoke for ori-box2d
param([string]$Triple = "x86_64-pc-windows-msvc")

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent $scriptDir

$ori = $env:ORI_BIN
if (-not $ori) {
    $cmd = Get-Command ori -ErrorAction SilentlyContinue
    if ($cmd) { $ori = $cmd.Source }
}
if (-not $ori) { throw "Set ORI_BIN to ori.exe" }
$env:ORI_USE_SYSTEM_LINKER = if ($env:ORI_USE_SYSTEM_LINKER) { $env:ORI_USE_SYSTEM_LINKER } else { "1" }

& (Join-Path $scriptDir "build_windows.ps1") -Triple $Triple

Write-Host "== check library modules =="
# Module rename: box2d.world → box2d.dynamics (Ori clash with game.world)
& $ori check (Join-Path $root "box2d\dynamics.orl")
if ($LASTEXITCODE -ne 0) { throw "check dynamics.orl failed" }
& $ori check (Join-Path $root "box2d\units.orl")
if ($LASTEXITCODE -ne 0) { throw "check units.orl failed" }
if (Test-Path (Join-Path $root "box2d\world_sync.orl")) {
    & $ori check (Join-Path $root "box2d\world_sync.orl")
    if ($LASTEXITCODE -ne 0) { throw "check world_sync.orl failed" }
}

$tmp = Join-Path $env:TEMP ("box2d_smoke_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    foreach ($demo in @("boxes_fall", "impulse_test", "joints_demo")) {
        $src = Join-Path $root "examples\$demo.orl"
        if (-not (Test-Path $src)) { continue }
        $out = Join-Path $tmp "$demo.exe"
        Write-Host "-- compile $demo"
        & $ori compile $src --out $out
        if ($LASTEXITCODE -ne 0) { throw "compile $demo failed" }
        Write-Host "-- run $demo"
        & $out
        if ($LASTEXITCODE -ne 0) { throw "run $demo failed" }
    }
}
finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}

Write-Host "smoke_windows (ori-box2d): all green"
