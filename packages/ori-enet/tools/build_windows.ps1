# Build vendored ENet + ori_enet_shim for MSVC → lib\x86_64-pc-windows-msvc\
param([string]$Triple = "x86_64-pc-windows-msvc")
$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent $scriptDir
$enet = Join-Path $root "vendor\enet"
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

if (-not (Test-Path (Join-Path $enet "include\enet\enet.h"))) {
    throw "missing vendor/enet — git clone https://github.com/lsalzman/enet.git vendor/enet"
}

$incs = "/I$(Join-Path $enet 'include')"
$tmp = Join-Path $env:TEMP ("ori_enet_build_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    $srcs = @("callbacks.c", "compress.c", "host.c", "list.c", "packet.c", "peer.c", "protocol.c", "win32.c")
    $objs = @()
    foreach ($s in $srcs) {
        $o = Join-Path $tmp ($s -replace '\.c$', '.obj')
        & cl.exe /nologo /c /O2 /MD /DWIN32 /D_WIN32 $incs /Fo"$o" (Join-Path $enet $s)
        if ($LASTEXITCODE -ne 0) { throw "compile $s failed" }
        $objs += $o
    }
    & lib.exe /nologo /OUT:(Join-Path $out "enet.lib") @objs
    if ($LASTEXITCODE -ne 0) { throw "enet.lib failed" }

    $shimO = Join-Path $tmp "ori_enet_shim.obj"
    & cl.exe /nologo /c /O2 /MD /DWIN32 /D_WIN32 $incs /Fo"$shimO" (Join-Path $root "native\ori_enet_shim.c")
    if ($LASTEXITCODE -ne 0) { throw "shim compile failed" }
    & lib.exe /nologo /OUT:(Join-Path $out "ori_enet_shim.lib") $shimO
    if ($LASTEXITCODE -ne 0) { throw "shim lib failed" }
}
finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}
Write-Host "built enet → $out"
Get-ChildItem $out | Format-Table Name, Length
