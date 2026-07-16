# Windows MSVC smoke for ori-enet (build + check + loopback examples).
param(
    [string]$Triple = "x86_64-pc-windows-msvc",
    [string]$OriBin = $(if ($env:ORI_BIN) { $env:ORI_BIN } else { "ori" })
)
$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent $scriptDir

$ori = $OriBin
if (-not (Get-Command $ori -ErrorAction SilentlyContinue) -and -not (Test-Path $ori)) {
    $c = Get-Command ori -ErrorAction SilentlyContinue
    if ($c) { $ori = $c.Source } else { throw "Set ORI_BIN to ori.exe" }
}
$env:ORI_USE_SYSTEM_LINKER = if ($env:ORI_USE_SYSTEM_LINKER) { $env:ORI_USE_SYSTEM_LINKER } else { "1" }

& (Join-Path $scriptDir "build_windows.ps1") -Triple $Triple
& $ori check (Join-Path $root "enet\host.orl")
if ($LASTEXITCODE -ne 0) { throw "check enet.host failed" }

$tmp = Join-Path $env:TEMP ("ori_enet_smoke_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    Push-Location $root
    try {
        foreach ($ex in @("loopback_ping", "multi_message", "channels_demo", "broadcast_demo", "protocol_demo")) {
            $src = Join-Path $root "examples\$ex.orl"
            if (-not (Test-Path $src)) { continue }
            $out = Join-Path $tmp "$ex.exe"
            Write-Host "-- compile $ex"
            & $ori compile $src --out $out
            if ($LASTEXITCODE -ne 0) { throw "compile $ex failed" }
            Write-Host "-- run $ex"
            $log = Join-Path $tmp "$ex.txt"
            & $out | Tee-Object -FilePath $log
            $line = (Get-Content $log -Raw).Trim()
            if ($line -ne "ok") { throw "$ex expected ok, got: $line" }
        }
    }
    finally {
        Pop-Location
        Remove-Item -Force (Join-Path $root "ori_enet_*.tmp") -ErrorAction SilentlyContinue
    }
}
finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}
Write-Host "smoke_enet_windows: all green"
