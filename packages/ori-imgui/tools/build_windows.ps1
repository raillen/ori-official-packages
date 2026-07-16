# Build ori_imgui host for Windows MSVC (stub-first if full GLFW build not ready).
# Full GLFW+GL3: set -Full and ensure vendor/cimgui + vendor/glfw are present.
param(
    [switch]$Full,
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

if ($Full) {
    Write-Host "Full imgui Windows build: use tools/build_cimgui.ps1 + CMake for GLFW; then compile native/ori_imgui_host.cpp"
    Write-Host "This path is not fully automated — prefer Linux smoke or complete CMake project."
}

# Minimal stub host so link lines resolve for headless CI.
$tmp = Join-Path $env:TEMP ("ori_imgui_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
try {
    $stub = Join-Path $tmp "ori_imgui_stub.c"
    @"
#include <stdint.h>
int ori_imgui_init(int w, int h, int title) { (void)w;(void)h;(void)title; return 0; }
void ori_imgui_shutdown(void) {}
int ori_imgui_should_close(void) { static int n; return ++n > 2; }
void ori_imgui_begin_frame(void) {}
void ori_imgui_end_frame(void) {}
int ori_imgui_begin(int name, int flags) { (void)name;(void)flags; return 1; }
void ori_imgui_end(void) {}
int ori_imgui_button(int label) { (void)label; return 0; }
int ori_imgui_small_button(int label) { (void)label; return 0; }
void ori_imgui_text(int text) { (void)text; }
void ori_imgui_text_colored(int r,int g,int b,int a,int text) { (void)r;(void)g;(void)b;(void)a;(void)text; }
int ori_imgui_checkbox(int label, int checked) { (void)label; return checked; }
int ori_imgui_slider_int(int label, int v, int lo, int hi) { (void)label;(void)lo;(void)hi; return v; }
int ori_imgui_drag_int(int label, int v, int lo, int hi) { (void)label;(void)lo;(void)hi; return v; }
void ori_imgui_progress_bar(int frac, int overlay) { (void)frac;(void)overlay; }
int ori_imgui_collapsing_header(int label) { (void)label; return 0; }
int ori_imgui_tree_node(int label) { (void)label; return 0; }
void ori_imgui_tree_pop(void) {}
void ori_imgui_separator(void) {}
void ori_imgui_same_line(void) {}
void ori_imgui_spacing(void) {}
void ori_imgui_bullet_text(int text) { (void)text; }
int ori_imgui_want_capture_mouse(void) { return 0; }
int ori_imgui_want_capture_keyboard(void) { return 0; }
void ori_imgui_set_next_window_size(int w, int h) { (void)w;(void)h; }
void ori_imgui_set_next_window_pos(int x, int y) { (void)x;(void)y; }
int ori_imgui_input_text(int label) { (void)label; return 0; }
int ori_imgui_input_text_to_path(int path) { (void)path; return 0; }
void ori_imgui_input_text_set(int text) { (void)text; }
int ori_imgui_begin_main_menu_bar(void) { return 0; }
void ori_imgui_end_main_menu_bar(void) {}
int ori_imgui_begin_menu(int label) { (void)label; return 0; }
void ori_imgui_end_menu(void) {}
int ori_imgui_menu_item(int label) { (void)label; return 0; }
/* Wave 5 extras — weak stubs */
int ori_imgui_slider_float_milli(int label, int v, int lo, int hi) { (void)label;(void)lo;(void)hi; return v; }
int ori_imgui_combo(int label, int idx, int i0,int i1,int i2,int i3,int i4,int i5,int i6,int i7) {
    (void)label;(void)i0;(void)i1;(void)i2;(void)i3;(void)i4;(void)i5;(void)i6;(void)i7; return idx;
}
int ori_imgui_begin_child(int id, int w, int h, int border) { (void)id;(void)w;(void)h;(void)border; return 1; }
void ori_imgui_end_child(void) {}
"@ | Set-Content -Path $stub -Encoding ASCII

    $obj = Join-Path $tmp "ori_imgui_stub.obj"
    & cl.exe /nologo /c /O2 /TC $stub /Fo"$obj"
    if ($LASTEXITCODE -ne 0) { throw "cl stub failed" }
    & lib.exe /nologo /OUT:(Join-Path $out "ori_imgui.lib") $obj
    # native_libs = ["ori_imgui", "glfw3", "sysdeps"] — placeholders
    $dummy = Join-Path $tmp "dummy.c"
    Set-Content $dummy "void ori_imgui_dep_dummy(void) {}`n"
    $dobj = Join-Path $tmp "dummy.obj"
    & cl.exe /nologo /c /O2 /TC $dummy /Fo"$dobj"
    & lib.exe /nologo /OUT:(Join-Path $out "glfw3.lib") $dobj
    & lib.exe /nologo /OUT:(Join-Path $out "sysdeps.lib") $dobj
}
finally { Remove-Item -Recurse -Force $tmp -EA SilentlyContinue }
Write-Host "built imgui stub host → $out (use -Full for real GLFW+GL)"
