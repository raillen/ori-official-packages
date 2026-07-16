# Stage native libs for ori-game on Windows MSVC:
#   lib/x86_64-pc-windows-msvc/ori_raylib_shim.lib  — ori_rl_* scalar ABI
#   lib/x86_64-pc-windows-msvc/raylib.lib           — real raylib or stub dummy
#
# Usage:
#   .\tools\setup_raylib_windows.ps1           # prefer real raylib if found
#   .\tools\setup_raylib_windows.ps1 -Stub     # headless stub (CI / no GPU)
#
# Requires: Visual Studio Build Tools (cl.exe, lib.exe) on PATH or via vswhere.

param(
    [switch]$Stub,
    [string]$Triple = "x86_64-pc-windows-msvc"
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$outDir = Join-Path $repoRoot "lib\$Triple"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$shimLib = Join-Path $outDir "ori_raylib_shim.lib"
$rayLib = Join-Path $outDir "raylib.lib"

function Find-VsDevShell {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) { return $null }
    $install = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $install) { return $null }
    $dev = Join-Path $install "Common7\Tools\Launch-VsDevShell.ps1"
    if (Test-Path $dev) { return $dev }
    return $null
}

function Ensure-Cl {
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) { return }
    $dev = Find-VsDevShell
    if ($dev) {
        Write-Host "Importing VS dev shell: $dev"
        & $dev -Arch amd64 -HostArch amd64 | Out-Null
    }
    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        throw "cl.exe not found. Install VS Build Tools (Desktop C++) or open 'x64 Native Tools Command Prompt'."
    }
    if (-not (Get-Command lib.exe -ErrorAction SilentlyContinue)) {
        throw "lib.exe not found (MSVC librarian)."
    }
}

function Build-Stub {
    Ensure-Cl
    Write-Host "Building headless ori_rl_* stub → $shimLib"
    $tmp = Join-Path $env:TEMP ("ori_raylib_stub_" + [guid]::NewGuid().ToString("N"))
    New-Item -ItemType Directory -Force -Path $tmp | Out-Null
    try {
        $obj = Join-Path $tmp "raylib_stub.obj"
        $dummyC = Join-Path $tmp "dummy.c"
        $dummyObj = Join-Path $tmp "dummy.obj"
        & cl.exe /nologo /c /O2 /TC (Join-Path $scriptDir "raylib_stub.c") /Fo"$obj"
        if ($LASTEXITCODE -ne 0) { throw "cl failed on raylib_stub.c" }
        & lib.exe /nologo /OUT:"$shimLib" $obj
        if ($LASTEXITCODE -ne 0) { throw "lib failed for ori_raylib_shim.lib" }

        Set-Content -Path $dummyC -Value "void ori_raylib_dummy(void) {}`n" -NoNewline
        & cl.exe /nologo /c /O2 /TC $dummyC /Fo"$dummyObj"
        & lib.exe /nologo /OUT:"$rayLib" $dummyObj
        if ($LASTEXITCODE -ne 0) { throw "lib failed for raylib.lib" }
        Write-Host "Stub installed (WindowShouldClose exits after a few frames)."
    }
    finally {
        Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
    }
}

function Try-CopyPrebuiltRaylib {
    $candidates = @(
        (Join-Path $outDir "raylib.lib"),  # already staged
        (Join-Path $repoRoot "vendor\raylib\lib\raylib.lib"),
        (Join-Path $repoRoot "vendor\raylib\projects\VS2022\build\raylib\bin\x64\Release\raylib.lib")
    )
    foreach ($c in $candidates) {
        if ((Test-Path $c) -and ((Get-Item $c).Length -gt 10000)) {
            if ($c -ne $rayLib) { Copy-Item -Force $c $rayLib }
            Write-Host "Using prebuilt raylib: $c"
            return $true
        }
    }
    return $false
}

function Build-RealShim {
    Ensure-Cl
    $inc = $null
    foreach ($d in @(
        (Join-Path $outDir ""),
        (Join-Path $repoRoot "vendor\raylib\src"),
        (Join-Path $repoRoot "vendor\raylib\include"),
        "C:\raylib\src"
    )) {
        if (Test-Path (Join-Path $d "raylib.h")) { $inc = $d; break }
    }
    if (-not $inc) {
        Write-Warning "raylib.h not found; cannot build real shim"
        return $false
    }
    Write-Host "Building real ori_raylib_shim → $shimLib (I$inc)"
    $tmp = Join-Path $env:TEMP ("ori_raylib_shim_" + [guid]::NewGuid().ToString("N"))
    New-Item -ItemType Directory -Force -Path $tmp | Out-Null
    try {
        $obj = Join-Path $tmp "ori_raylib_shim.obj"
        $src = Join-Path $repoRoot "native\ori_raylib_shim.c"
        & cl.exe /nologo /c /O2 /TC /I"$inc" $src /Fo"$obj"
        if ($LASTEXITCODE -ne 0) { return $false }
        & lib.exe /nologo /OUT:"$shimLib" $obj
        if ($LASTEXITCODE -ne 0) { return $false }
        return $true
    }
    finally {
        Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
    }
}

if ($Stub) {
    Build-Stub
    exit 0
}

if ((Try-CopyPrebuiltRaylib) -and (Build-RealShim)) {
    Write-Host "Real raylib + shim ready → $outDir"
    exit 0
}

Write-Host "Falling back to headless stub"
Build-Stub
