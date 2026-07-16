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

$tmp = Join-Path $env:TEMP ("ori_rres_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    $src = Join-Path $root "native\ori_rres_shim.c"
    $obj = Join-Path $tmp "ori_rres_shim.obj"
    & cl.exe /nologo /c /O2 /TC $src /Fo"$obj"
    if ($LASTEXITCODE -ne 0) { throw "cl failed" }
    # Package native_libs = ["rres"] — produce rres.lib from shim object
    & lib.exe /nologo /OUT:(Join-Path $out "rres.lib") $obj
    if ($LASTEXITCODE -ne 0) { throw "lib failed" }
}
finally { Remove-Item -Recurse -Force $tmp -EA SilentlyContinue }
Write-Host "built rres → $out"
