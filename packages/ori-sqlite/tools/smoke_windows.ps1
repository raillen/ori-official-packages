param([string]$Triple = "x86_64-pc-windows-msvc")
$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent $scriptDir
$ori = $env:ORI_BIN
if (-not $ori) { $c = Get-Command ori -EA SilentlyContinue; if ($c) { $ori = $c.Source } }
if (-not $ori) { throw "Set ORI_BIN" }
$env:ORI_USE_SYSTEM_LINKER = if ($env:ORI_USE_SYSTEM_LINKER) { $env:ORI_USE_SYSTEM_LINKER } else { "1" }

& (Join-Path $scriptDir "build_windows.ps1") -Triple $Triple
& $ori check (Join-Path $root "sqlite\db.orl")
if ($LASTEXITCODE -ne 0) { throw "check failed" }

$tmp = Join-Path $env:TEMP ("sqlite_smoke_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    foreach ($demo in @("kv_store", "tx_rollback", "prepared_demo")) {
        $src = Join-Path $root "examples\$demo.orl"
        if (-not (Test-Path $src)) { continue }
        $out = Join-Path $tmp "$demo.exe"
        & $ori compile $src --out $out
        if ($LASTEXITCODE -ne 0) { throw "compile $demo" }
        & $out
        if ($LASTEXITCODE -ne 0) { throw "run $demo" }
    }
}
finally { Remove-Item -Recurse -Force $tmp -EA SilentlyContinue }
Write-Host "smoke_windows (ori-sqlite): all green"
