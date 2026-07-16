# Build Jolt stub + dummy Jolt.lib for Windows MSVC (CI-friendly default).
# Real Jolt: -Real (requires cmake + long C++ build).
param(
    [switch]$Real,
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
        throw "cl.exe not found"
    }
}

Ensure-Cl
$tmp = Join-Path $env:TEMP ("ori_jolt_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    if ($Real) {
        Write-Warning "Real Jolt Windows build: use CMake on vendor/JoltPhysics (not fully automated yet). Building stub ABI instead."
    }
    Write-Host "Building ori_jolt_shim (stub) + dummy Jolt.lib..."
    $obj = Join-Path $tmp "ori_jolt_stub.obj"
    $stub = Join-Path $root "native\ori_jolt_stub.c"
    & cl.exe /nologo /c /O2 /TC $stub /Fo"$obj"
    if ($LASTEXITCODE -ne 0) { throw "cl stub failed" }
    & lib.exe /nologo /OUT:(Join-Path $out "ori_jolt_shim.lib") $obj
    if ($LASTEXITCODE -ne 0) { throw "lib shim failed" }

    $dummyC = Join-Path $tmp "jolt_dummy.c"
    Set-Content $dummyC "void ori_jolt_lib_dummy(void) {}`n"
    $dobj = Join-Path $tmp "jolt_dummy.obj"
    & cl.exe /nologo /c /O2 /TC $dummyC /Fo"$dobj"
    & lib.exe /nologo /OUT:(Join-Path $out "Jolt.lib") $dobj
    if ($LASTEXITCODE -ne 0) { throw "lib Jolt dummy failed" }
    Set-Content (Join-Path $out "ORI_JOLT_MODE.txt") "stub"
}
finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}
Write-Host "built jolt stub → $out"
