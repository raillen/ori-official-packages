# Build Box2D + ori_box2d_shim for Windows MSVC → lib/x86_64-pc-windows-msvc/
# Usage:
#   .\tools\build_windows.ps1
# Requires: cmake, cl, lib (VS Build Tools)

param(
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
    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        throw "cl.exe not found — open x64 Native Tools or install VS Build Tools"
    }
}

Ensure-Cl
if (-not (Get-Command cmake.exe -ErrorAction SilentlyContinue)) {
    throw "cmake.exe not found on PATH"
}

$vendor = Join-Path $root "vendor\box2d"
$build = Join-Path $vendor "build-msvc"
if (-not (Test-Path (Join-Path $build "src\Release\box2d.lib")) -and
    -not (Test-Path (Join-Path $build "src\box2d.lib"))) {
    Write-Host "Configuring Box2D (MSVC)..."
    & cmake -S $vendor -B $build `
        -DCMAKE_BUILD_TYPE=Release `
        -DBOX2D_SAMPLES=OFF -DBOX2D_UNIT_TESTS=OFF -DBOX2D_BENCHMARKS=OFF `
        -G "Visual Studio 17 2022" -A x64
    if ($LASTEXITCODE -ne 0) {
        # Fallback generator
        & cmake -S $vendor -B $build `
            -DCMAKE_BUILD_TYPE=Release `
            -DBOX2D_SAMPLES=OFF -DBOX2D_UNIT_TESTS=OFF -DBOX2D_BENCHMARKS=OFF
    }
    & cmake --build $build --config Release -j 4
    if ($LASTEXITCODE -ne 0) { throw "Box2D cmake build failed" }
}

$boxCandidates = @(
    (Join-Path $build "src\Release\box2d.lib"),
    (Join-Path $build "Release\box2d.lib"),
    (Join-Path $build "src\box2d.lib"),
    (Join-Path $build "box2d.lib")
)
$boxSrc = $boxCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $boxSrc) { throw "box2d.lib not found under $build" }
Copy-Item -Force $boxSrc (Join-Path $out "box2d.lib")

Write-Host "Building ori_box2d_shim.lib..."
$tmp = Join-Path $env:TEMP ("ori_b2_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    $obj = Join-Path $tmp "ori_box2d_shim.obj"
    $inc = Join-Path $vendor "include"
    $src = Join-Path $root "native\ori_box2d_shim.c"
    & cl.exe /nologo /c /O2 /TC /I"$inc" $src /Fo"$obj"
    if ($LASTEXITCODE -ne 0) { throw "cl shim failed" }
    $shimOut = Join-Path $out "ori_box2d_shim.lib"
    & lib.exe /nologo /OUT:"$shimOut" $obj
    if ($LASTEXITCODE -ne 0) { throw "lib shim failed" }
}
finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}

Write-Host "built box2d + shim → $out"
Get-ChildItem $out | Format-Table Name, Length
