# P1 — Umbrella Windows ECO smoke (run on a real MSVC host).
#
#   $env:ORI_BIN = "C:\path\to\ori.exe"
#   $env:ORI_USE_SYSTEM_LINKER = "1"
#   .\tools\smoke_eco_windows.ps1
#   .\tools\smoke_eco_windows.ps1 -Stub
#   .\tools\smoke_eco_windows.ps1 -ValidateOnly   # path inventory only (no build)
#
# Continues after individual failures; exits non-zero if any package failed.
# From Linux you can dry-check scripts with:
#   ./tools/validate_os_scripts.sh

param(
    [switch]$Stub,
    [switch]$ValidateOnly
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$gameRoot = Split-Path -Parent $scriptDir
$projRoot = Split-Path -Parent $gameRoot

$okCount = 0
$failCount = 0
$skipCount = 0

function Run-PkgSmoke(
    [string]$name,
    [string]$dir,
    [string]$script,
    [string[]]$extraArgs = @()
) {
    Write-Host ""
    Write-Host "======== ECO Windows smoke: $name ========"
    if (-not (Test-Path $dir)) {
        Write-Host "SKIP $name (missing $dir)"
        $script:skipCount++
        return
    }
    $ps1 = Join-Path $dir $script
    if (-not (Test-Path $ps1)) {
        Write-Host "SKIP $name (missing $script)"
        $script:skipCount++
        return
    }
    if ($ValidateOnly) {
        Write-Host "OK $name (script present: $script)"
        $script:okCount++
        return
    }
    Push-Location $dir
    try {
        & $ps1 @extraArgs
        if ($LASTEXITCODE -ne 0 -and $null -ne $LASTEXITCODE) {
            Write-Host "FAIL $name (exit $LASTEXITCODE)"
            $script:failCount++
            return
        }
        Write-Host "OK $name"
        $script:okCount++
    }
    catch {
        Write-Host "FAIL $name — $_"
        $script:failCount++
    }
    finally {
        Pop-Location
    }
}

Write-Host "smoke_eco_windows · projRoot=$projRoot · Stub=$Stub · ValidateOnly=$ValidateOnly"

# Core 8 (PHASE-OS matrix)
if ($Stub) {
    Run-PkgSmoke "ori-game" $gameRoot "tools\smoke_windows.ps1" @("-Stub")
}
else {
    Run-PkgSmoke "ori-game" $gameRoot "tools\smoke_windows.ps1"
}
Run-PkgSmoke "ori-box2d" (Join-Path $projRoot "ori-box2d") "tools\smoke_windows.ps1"
Run-PkgSmoke "ori-jolt" (Join-Path $projRoot "ori-jolt") "tools\smoke_windows.ps1"
Run-PkgSmoke "ori-sqlite" (Join-Path $projRoot "ori-sqlite") "tools\smoke_windows.ps1"
Run-PkgSmoke "ori-rres" (Join-Path $projRoot "ori-rres") "tools\smoke_windows.ps1"
Run-PkgSmoke "ori-imgui" (Join-Path $projRoot "ori-imgui") "tools\smoke_windows.ps1"
Run-PkgSmoke "ori-raygui" (Join-Path $projRoot "ori-raygui") "tools\smoke_windows.ps1"
Run-PkgSmoke "ori-enet" (Join-Path $projRoot "ori-enet") "tools\smoke_windows.ps1"

# Studio-critical / optional (SKIP if no smoke_windows.ps1)
Run-PkgSmoke "ori-nfd" (Join-Path $projRoot "ori-nfd") "tools\smoke_windows.ps1"
Run-PkgSmoke "ori-imguizmo" (Join-Path $projRoot "ori-imguizmo") "tools\smoke_windows.ps1"
Run-PkgSmoke "ori-imnodes" (Join-Path $projRoot "ori-imnodes") "tools\smoke_windows.ps1"
Run-PkgSmoke "ori-implot" (Join-Path $projRoot "ori-implot") "tools\smoke_windows.ps1"
Run-PkgSmoke "ori-imgui-extras" (Join-Path $projRoot "ori-imgui-extras") "tools\smoke_windows.ps1"

# Studio shell (ImGui demo package) — best-effort
$shell = Join-Path $projRoot "ori-imgui\demos\studio_shell"
if (Test-Path (Join-Path $shell "tools\smoke.sh")) {
    Write-Host ""
    Write-Host "======== ECO Windows smoke: studio_shell ========"
    if ($ValidateOnly) {
        Write-Host "OK studio_shell (tools\smoke.sh present)"
        $okCount++
    }
    else {
        Write-Host "NOTE: studio_shell smoke is sh-based; run via Git Bash / WSL or port later"
        Write-Host "SKIP studio_shell (needs sh host on Windows)"
        $skipCount++
    }
}

Write-Host ""
Write-Host "smoke_eco_windows: ok=$okCount fail=$failCount skip=$skipCount"
if ($failCount -gt 0) {
    Write-Host "smoke_eco_windows: FAILED"
    exit 1
}
if ($ValidateOnly) {
    Write-Host "smoke_eco_windows: validate-only green (no MSVC execution)"
}
else {
    Write-Host "smoke_eco_windows: all green"
}
exit 0
