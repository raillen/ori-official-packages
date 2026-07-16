param([string]$Triple = "x86_64-pc-windows-msvc")
$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent $scriptDir
$ori = $env:ORI_BIN
if (-not $ori) { $c = Get-Command ori -EA SilentlyContinue; if ($c) { $ori = $c.Source } }
if (-not $ori) { throw "Set ORI_BIN" }
$env:ORI_USE_SYSTEM_LINKER = if ($env:ORI_USE_SYSTEM_LINKER) { $env:ORI_USE_SYSTEM_LINKER } else { "1" }

& (Join-Path $scriptDir "build_windows.ps1") -Triple $Triple
# Module rename: jolt.world → jolt.dynamics (Ori clash with game.world)
foreach ($mod in @("dynamics.orl", "units.orl", "character_ctrl.orl", "world_character.orl")) {
    $path = Join-Path $root "jolt\$mod"
    if (-not (Test-Path $path)) { continue }
    Write-Host "-- check jolt\$mod"
    & $ori check $path
    if ($LASTEXITCODE -ne 0) { throw "check failed: $mod" }
}

$tmp = Join-Path $env:TEMP ("jolt_smoke_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    foreach ($demo in @(
        "boxes_fall", "impulse_test", "wave4_test",
        "character_test", "character_frame_test", "character_world_test"
    )) {
        $src = Join-Path $root "examples\$demo.orl"
        if (-not (Test-Path $src)) { continue }
        $out = Join-Path $tmp "$demo.exe"
        Write-Host "-- compile $demo"
        & $ori compile $src --out $out
        if ($LASTEXITCODE -ne 0) { throw "compile $demo" }
        Write-Host "-- run $demo"
        & $out
        if ($LASTEXITCODE -ne 0) { throw "run $demo" }
    }
}
finally { Remove-Item -Recurse -Force $tmp -EA SilentlyContinue }
Write-Host "smoke_windows (ori-jolt): all green"
