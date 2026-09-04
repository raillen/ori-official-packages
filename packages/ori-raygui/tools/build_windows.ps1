# Build raygui static lib for Windows MSVC (implementation unit + raylib).
# Prefer prebuilt raylib.lib next to output; otherwise stub raylib dummy.
param(
    [switch]$StubRaylib,
    [string]$Triple = "x86_64-pc-windows-msvc"
)

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

$tmp = Join-Path $env:TEMP ("ori_raygui_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    $impl = Join-Path $root "native\raygui_impl.c"
    $obj = Join-Path $tmp "raygui_impl.obj"
    $inc = @()
    if (Test-Path (Join-Path $root "vendor\raygui.h")) { $inc += "/I$(Join-Path $root 'vendor')" }
    if (Test-Path (Join-Path $root "vendor\raylib\raylib.h")) { $inc += "/I$(Join-Path $root 'vendor\raylib')" }
    & cl.exe /nologo /c /O2 /TC @inc $impl /Fo"$obj"
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "raygui_impl.c failed (missing headers?) — writing empty raygui.lib"
        $dummy = Join-Path $tmp "d.c"
        Set-Content $dummy "void ori_raygui_dummy(void) {}`n"
        $dobj = Join-Path $tmp "d.obj"
        & cl.exe /nologo /c /O2 /TC $dummy /Fo"$dobj"
        & lib.exe /nologo /OUT:(Join-Path $out "raygui.lib") $dobj
    } else {
        & lib.exe /nologo /OUT:(Join-Path $out "raygui.lib") $obj
    }

    $ray = Join-Path $out "raylib.lib"
    if (-not (Test-Path $ray) -or $StubRaylib) {
        $dummy = Join-Path $tmp "ray.c"
        Set-Content $dummy "void ori_raylib_dummy(void) {}`n"
        $dobj = Join-Path $tmp "ray.obj"
        & cl.exe /nologo /c /O2 /TC $dummy /Fo"$dobj"
        & lib.exe /nologo /OUT:$ray $dobj
        Write-Host "staged stub raylib.lib (replace with real raylib for UI demos)"
    }
}
finally { Remove-Item -Recurse -Force $tmp -EA SilentlyContinue }
Write-Host "built raygui → $out"
