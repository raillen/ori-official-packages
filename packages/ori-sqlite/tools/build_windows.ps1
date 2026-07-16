# Build sqlite amalgamation + ori_sqlite_shim for MSVC
param([string]$Triple = "x86_64-pc-windows-msvc")
$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent $scriptDir
$out = Join-Path $root "lib\$Triple"
New-Item -ItemType Directory -Force -Path $out | Out-Null

function Ensure-Cl {
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) { return }
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $install = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        $dev = Join-Path $install "Common7\Tools\Launch-VsDevShell.ps1"
        if (Test-Path $dev) { & $dev -Arch amd64 -HostArch amd64 | Out-Null }
    }
    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) { throw "cl.exe not found" }
}
Ensure-Cl

# Prefer amalgamation under vendor/
$amalg = $null
foreach ($c in @(
    (Join-Path $root "vendor\sqlite3.c"),
    (Join-Path $root "vendor\sqlite\sqlite3.c"),
    (Join-Path $root "native\sqlite3.c")
)) { if (Test-Path $c) { $amalg = $c; break } }

$tmp = Join-Path $env:TEMP ("ori_sqlite_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    if ($amalg) {
        Write-Host "Compiling sqlite3 amalgamation..."
        $obj = Join-Path $tmp "sqlite3.obj"
        $inc = Split-Path $amalg
        & cl.exe /nologo /c /O2 /TC /DSQLITE_OMIT_LOAD_EXTENSION /I"$inc" $amalg /Fo"$obj"
        if ($LASTEXITCODE -ne 0) { throw "sqlite3.c failed" }
        & lib.exe /nologo /OUT:(Join-Path $out "sqlite3.lib") $obj
    } else {
        Write-Host "No amalgamation found — building empty sqlite3.lib placeholder"
        $dummy = Join-Path $tmp "sqlite3_dummy.c"
        Set-Content $dummy "void ori_sqlite3_dummy(void) {}`n"
        $dobj = Join-Path $tmp "sqlite3_dummy.obj"
        & cl.exe /nologo /c /O2 /TC $dummy /Fo"$dobj"
        & lib.exe /nologo /OUT:(Join-Path $out "sqlite3.lib") $dobj
    }

    $shimSrc = Join-Path $root "native\ori_sqlite_shim.c"
    $shimObj = Join-Path $tmp "ori_sqlite_shim.obj"
    $incFlags = @()
    if ($amalg) { $incFlags = @("/I$(Split-Path $amalg)") }
    & cl.exe /nologo /c /O2 /TC @incFlags $shimSrc /Fo"$shimObj"
    if ($LASTEXITCODE -ne 0) { throw "shim compile failed" }
    & lib.exe /nologo /OUT:(Join-Path $out "ori_sqlite_shim.lib") $shimObj
    if ($LASTEXITCODE -ne 0) { throw "shim lib failed" }
}
finally { Remove-Item -Recurse -Force $tmp -EA SilentlyContinue }
Write-Host "built sqlite → $out"
