# Windows MSVC smoke for ori-game (check + compile + short run).
# Usage (x64 Native Tools or after setup_raylib_windows):
#   $env:ORI_BIN = "C:\path\to\ori.exe"
#   $env:ORI_USE_SYSTEM_LINKER = "1"
#   .\tools\smoke_windows.ps1
#   .\tools\smoke_windows.ps1 -Stub

param(
    [switch]$Stub,
    [string]$Triple = "x86_64-pc-windows-msvc"
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir

$ori = $env:ORI_BIN
if (-not $ori) {
    $cmd = Get-Command ori -ErrorAction SilentlyContinue
    if ($cmd) { $ori = $cmd.Source }
}
if (-not $ori) { throw "Set ORI_BIN to ori.exe or put ori on PATH" }

$env:ORI_USE_SYSTEM_LINKER = if ($env:ORI_USE_SYSTEM_LINKER) { $env:ORI_USE_SYSTEM_LINKER } else { "1" }

Write-Host "== stage native ($Triple) =="
$setup = Join-Path $scriptDir "setup_raylib_windows.ps1"
if ($Stub) {
    & $setup -Stub -Triple $Triple
} else {
    & $setup -Triple $Triple
}

function Invoke-OriCheck([string]$file) {
    Write-Host "-- check $file"
    & $ori check $file
    if ($LASTEXITCODE -ne 0) { throw "check failed: $file" }
}

Write-Host "== ori check (core + canonic demos) =="
$checkList = @(
    "raylib.orl", "color.orl", "shape.orl", "collision.orl",
    "examples\hello_game.orl", "examples\simple_game.orl",
    "examples\app_smoke.orl", "examples\gamepad_demo.orl",
    "examples\platformer.orl"
)
foreach ($rel in $checkList) {
    $path = Join-Path $repoRoot $rel
    if (Test-Path $path) { Invoke-OriCheck $path }
}
Get-ChildItem (Join-Path $repoRoot "game\*.orl") | ForEach-Object { Invoke-OriCheck $_.FullName }
Get-ChildItem (Join-Path $repoRoot "game\mechanics\*.orl") | ForEach-Object { Invoke-OriCheck $_.FullName }

Write-Host "== ori test =="
Get-ChildItem (Join-Path $repoRoot "tests\*.orl") | ForEach-Object {
    Write-Host "-- test $($_.Name)"
    & $ori test $_.FullName
    if ($LASTEXITCODE -ne 0) { throw "test failed: $($_.Name)" }
}

Write-Host "== compile + run demos =="
$tmp = Join-Path $env:TEMP ("ori_game_smoke_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    $simple = Join-Path $tmp "simple_game.exe"
    & $ori compile (Join-Path $repoRoot "examples\simple_game.orl") --out $simple
    if ($LASTEXITCODE -ne 0) { throw "compile simple_game failed" }
    & $simple
    if ($LASTEXITCODE -ne 0) { throw "run simple_game failed" }

    $demos = @("app_smoke", "hello_game", "platformer", "gamepad_demo")
    foreach ($demo in $demos) {
        $src = Join-Path $repoRoot "examples\$demo.orl"
        if (-not (Test-Path $src)) { continue }
        $out = Join-Path $tmp "$demo.exe"
        Write-Host "-- compile $demo"
        & $ori compile $src --out $out
        if ($LASTEXITCODE -ne 0) { throw "compile $demo failed" }
        Write-Host "-- run $demo (3s)"
        $p = Start-Process -FilePath $out -PassThru -NoNewWindow
        if (-not $p.WaitForExit(3000)) {
            Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
            Write-Host "OK $demo (timeout — window still running)"
        } else {
            Write-Host "OK $demo (exit $($p.ExitCode))"
            # stub exits 0; allow non-zero only if not crash-ish
            if ($p.ExitCode -gt 1) { throw "run $demo exit $($p.ExitCode)" }
        }
    }
}
finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}

Write-Host "smoke_windows (ori-game): all green"
