# script to clone and build cimgui
param (
    [string]$TargetOS = "windows"
)

Write-Host "Cloning cimgui..."
git clone https://github.com/cimgui/cimgui.git --recursive

cd cimgui
mkdir build
cd build

if ($TargetOS -eq "windows") {
    cmake ..
    cmake --build . --config Release
    Copy-Item "Release\cimgui.dll" "..\..\lib\win-x64\"
} elseif ($TargetOS -eq "linux") {
    cmake ..
    make
    Copy-Item "cimgui.so" "..\..\lib\linux-x64\libcimgui.so"
} elseif ($TargetOS -eq "macos") {
    cmake ..
    make
    Copy-Item "cimgui.dylib" "..\..\lib\macos-arm64\libcimgui.dylib"
}

cd ..\..
Write-Host "Build complete! DLLs placed in lib/ directory."

