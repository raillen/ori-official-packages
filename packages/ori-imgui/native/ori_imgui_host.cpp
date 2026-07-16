#include "imgui.h"
#include "imgui_internal.h" /* DockBuilder (studio layout) */
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Optional raylib embed (P2-A). Define ORI_IMGUI_NO_RAYLIB to skip. */
#ifndef ORI_IMGUI_NO_RAYLIB
#include "raylib.h"
#endif

static GLFWwindow *g_window = nullptr;
static bool g_inited = false;
enum HostMode { HOST_NONE = 0, HOST_GLFW = 1, HOST_RAYLIB = 2 };
static HostMode g_mode = HOST_NONE;

/* Multi-context: slot 0 is primary (created on init). */
#define MAX_IMGUI_CTX 4
static ImGuiContext *g_ctx[MAX_IMGUI_CTX] = {};
static int g_ctx_live[MAX_IMGUI_CTX] = {};
static int g_current_ctx = 0;

/* Multiple input-text slots so demos can own several fields. */
#define INPUT_SLOTS 4
#define INPUT_CAP 1024
static char g_input_bufs[INPUT_SLOTS][INPUT_CAP];
static int g_input_active = 0;

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static char *input_slot(int slot) {
    if (slot < 0 || slot >= INPUT_SLOTS) slot = 0;
    return g_input_bufs[slot];
}

/* Defined below (Tier 2 style). Forward decl for init paths. */
static void style_colors_spectrum_impl(bool dark);

extern "C" int ori_imgui_init(int width, int height, int64_t title_ptr) {
    if (g_inited) return 0;
    const char *title = (const char *)(intptr_t)title_ptr;
    if (!title) title = "Ori ImGui";

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    g_window = glfwCreateWindow(width > 0 ? width : 640, height > 0 ? height : 480, title, nullptr, nullptr);
    if (!g_window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(g_window);
    glfwSwapInterval(1);

    ImGuiContext *primary = ImGui::CreateContext();
    g_ctx[0] = primary;
    g_ctx_live[0] = 1;
    g_current_ctx = 0;
    ImGui::SetCurrentContext(primary);
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; /* Tier 0 */
    style_colors_spectrum_impl(true);
    for (int i = 0; i < INPUT_SLOTS; ++i) g_input_bufs[i][0] = '\0';
    g_input_active = 0;

    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    g_mode = HOST_GLFW;
    g_inited = true;
    return 0;
}

extern "C" void ori_imgui_shutdown(void) {
    if (!g_inited) return;
    ImGui_ImplOpenGL3_Shutdown();
    if (g_mode == HOST_GLFW) {
        ImGui_ImplGlfw_Shutdown();
        if (g_window) {
            glfwDestroyWindow(g_window);
            g_window = nullptr;
        }
        glfwTerminate();
    }
    /* Destroy extra multi-contexts first, then primary slot 0. */
    for (int i = 1; i < MAX_IMGUI_CTX; ++i) {
        if (g_ctx_live[i] && g_ctx[i]) {
            ImGui::DestroyContext(g_ctx[i]);
            g_ctx[i] = nullptr;
            g_ctx_live[i] = 0;
        }
    }
    if (g_ctx_live[0] && g_ctx[0]) {
        ImGui::SetCurrentContext(g_ctx[0]);
        ImGui::DestroyContext(g_ctx[0]);
    } else {
        ImGui::DestroyContext(nullptr);
    }
    g_ctx[0] = nullptr;
    g_ctx_live[0] = 0;
    g_current_ctx = 0;
    g_mode = HOST_NONE;
    g_inited = false;
}

extern "C" int ori_imgui_should_close(void) {
    if (!g_window) return 1;
    return glfwWindowShouldClose(g_window) ? 1 : 0;
}

extern "C" void ori_imgui_begin_frame(void) {
    if (!g_inited) return;
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

extern "C" void ori_imgui_end_frame(void) {
    if (!g_inited || !g_window) return;
    ImGui::Render();
    int display_w = 0, display_h = 0;
    glfwGetFramebufferSize(g_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(g_window);
}

extern "C" int ori_imgui_begin(int64_t name_ptr, int flags) {
    const char *name = (const char *)(intptr_t)name_ptr;
    if (!name) name = "window";
    return ImGui::Begin(name, nullptr, flags) ? 1 : 0;
}

extern "C" void ori_imgui_end(void) {
    ImGui::End();
}

extern "C" int ori_imgui_button(int64_t label_ptr) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "button";
    return ImGui::Button(label) ? 1 : 0;
}

extern "C" int ori_imgui_small_button(int64_t label_ptr) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "btn";
    return ImGui::SmallButton(label) ? 1 : 0;
}

extern "C" void ori_imgui_text(int64_t text_ptr) {
    const char *text = (const char *)(intptr_t)text_ptr;
    if (!text) text = "";
    ImGui::TextUnformatted(text);
}

extern "C" void ori_imgui_text_colored(int r, int g, int b, int a, int64_t text_ptr) {
    const char *text = (const char *)(intptr_t)text_ptr;
    if (!text) text = "";
    ImGui::TextColored(ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f), "%s", text);
}

extern "C" int ori_imgui_checkbox(int64_t label_ptr, int checked) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "check";
    bool v = checked != 0;
    ImGui::Checkbox(label, &v);
    return v ? 1 : 0;
}

extern "C" int ori_imgui_slider_int(int64_t label_ptr, int value, int v_min, int v_max) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "slider";
    int v = value;
    ImGui::SliderInt(label, &v, v_min, v_max);
    return v;
}

/* Float slider via milli-int (value/1000) to keep FFI as int. */
extern "C" int ori_imgui_slider_float_milli(int64_t label_ptr, int value_milli, int v_min_milli, int v_max_milli) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "slider";
    float v = value_milli / 1000.0f;
    float vmin = v_min_milli / 1000.0f;
    float vmax = v_max_milli / 1000.0f;
    ImGui::SliderFloat(label, &v, vmin, vmax, "%.3f");
    return (int)(v * 1000.0f + (v >= 0.0f ? 0.5f : -0.5f));
}

extern "C" int ori_imgui_drag_int(int64_t label_ptr, int value, int v_min, int v_max) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "drag";
    int v = value;
    ImGui::DragInt(label, &v, 1.0f, v_min, v_max);
    return v;
}

extern "C" void ori_imgui_progress_bar(int fraction_milli, int64_t overlay_ptr) {
    float f = fraction_milli / 1000.0f;
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    const char *overlay = (const char *)(intptr_t)overlay_ptr;
    ImGui::ProgressBar(f, ImVec2(-1.0f, 0.0f), overlay && overlay[0] ? overlay : nullptr);
}

extern "C" int ori_imgui_collapsing_header(int64_t label_ptr) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "header";
    return ImGui::CollapsingHeader(label) ? 1 : 0;
}

extern "C" int ori_imgui_tree_node(int64_t label_ptr) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "node";
    return ImGui::TreeNode(label) ? 1 : 0;
}

extern "C" void ori_imgui_tree_pop(void) {
    ImGui::TreePop();
}

extern "C" void ori_imgui_separator(void) {
    ImGui::Separator();
}

extern "C" void ori_imgui_same_line(void) {
    ImGui::SameLine();
}

extern "C" void ori_imgui_spacing(void) {
    ImGui::Spacing();
}

extern "C" void ori_imgui_bullet_text(int64_t text_ptr) {
    const char *text = (const char *)(intptr_t)text_ptr;
    if (!text) text = "";
    ImGui::BulletText("%s", text);
}

extern "C" int ori_imgui_want_capture_mouse(void) {
    if (!g_inited) return 0;
    return ImGui::GetIO().WantCaptureMouse ? 1 : 0;
}

extern "C" int ori_imgui_want_capture_keyboard(void) {
    if (!g_inited) return 0;
    return ImGui::GetIO().WantCaptureKeyboard ? 1 : 0;
}

extern "C" void ori_imgui_set_next_window_size(int w, int h) {
    ImGui::SetNextWindowSize(ImVec2((float)w, (float)h), ImGuiCond_FirstUseEver);
}

extern "C" void ori_imgui_set_next_window_pos(int x, int y) {
    ImGui::SetNextWindowPos(ImVec2((float)x, (float)y), ImGuiCond_FirstUseEver);
}

/* ---- input_text (multi-slot) ---- */

extern "C" void ori_imgui_input_text_select(int slot) {
    if (slot < 0 || slot >= INPUT_SLOTS) slot = 0;
    g_input_active = slot;
}

extern "C" int ori_imgui_input_text(int64_t label_ptr) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "input";
    return ImGui::InputText(label, input_slot(g_input_active), INPUT_CAP) ? 1 : 0;
}

extern "C" int ori_imgui_input_text_slot(int64_t label_ptr, int slot) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "input";
    return ImGui::InputText(label, input_slot(slot), INPUT_CAP) ? 1 : 0;
}

extern "C" int ori_imgui_input_text_multiline(int64_t label_ptr, int slot, int height) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "text";
    float h = height > 0 ? (float)height : 80.0f;
    return ImGui::InputTextMultiline(label, input_slot(slot), INPUT_CAP, ImVec2(-1.0f, h)) ? 1 : 0;
}

extern "C" int ori_imgui_input_text_to_path(int64_t path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) return 1;
    FILE *f = fopen(path, "wb");
    if (!f) return 1;
    fputs(input_slot(g_input_active), f);
    fclose(f);
    return 0;
}

extern "C" int ori_imgui_input_text_slot_to_path(int slot, int64_t path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) return 1;
    FILE *f = fopen(path, "wb");
    if (!f) return 1;
    fputs(input_slot(slot), f);
    fclose(f);
    return 0;
}

extern "C" void ori_imgui_input_text_set(int64_t text_ptr) {
    const char *text = (const char *)(intptr_t)text_ptr;
    char *buf = input_slot(g_input_active);
    if (!text) {
        buf[0] = '\0';
        return;
    }
    strncpy(buf, text, INPUT_CAP - 1);
    buf[INPUT_CAP - 1] = '\0';
}

extern "C" void ori_imgui_input_text_slot_set(int slot, int64_t text_ptr) {
    const char *text = (const char *)(intptr_t)text_ptr;
    char *buf = input_slot(slot);
    if (!text) {
        buf[0] = '\0';
        return;
    }
    strncpy(buf, text, INPUT_CAP - 1);
    buf[INPUT_CAP - 1] = '\0';
}

extern "C" void ori_imgui_input_text_clear(int slot) {
    input_slot(slot)[0] = '\0';
}

extern "C" int ori_imgui_input_text_len(int slot) {
    return (int)strlen(input_slot(slot));
}

/* ---- combo: up to 8 fixed string labels ---- */

extern "C" int ori_imgui_combo(
    int64_t label_ptr, int current, int count,
    int64_t a0, int64_t a1, int64_t a2, int64_t a3,
    int64_t a4, int64_t a5, int64_t a6, int64_t a7) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "combo";
    if (count < 0) count = 0;
    if (count > 8) count = 8;
    const char *items[8];
    int64_t ptrs[8] = {a0, a1, a2, a3, a4, a5, a6, a7};
    for (int i = 0; i < count; ++i) {
        items[i] = (const char *)(intptr_t)ptrs[i];
        if (!items[i]) items[i] = "";
    }
    int cur = current;
    if (cur < 0) cur = 0;
    if (count > 0 && cur >= count) cur = count - 1;
    if (count > 0) {
        ImGui::Combo(label, &cur, items, count);
    } else {
        ImGui::Text("%s (empty)", label);
    }
    return cur;
}

/* ---- child regions ---- */

extern "C" int ori_imgui_begin_child(int64_t id_ptr, int w, int h, int border) {
    const char *id = (const char *)(intptr_t)id_ptr;
    if (!id) id = "child";
    ImGuiChildFlags flags = border ? ImGuiChildFlags_Borders : ImGuiChildFlags_None;
    return ImGui::BeginChild(id, ImVec2((float)w, (float)h), flags) ? 1 : 0;
}

extern "C" void ori_imgui_end_child(void) {
    ImGui::EndChild();
}

/* ---- menus ---- */

extern "C" int ori_imgui_begin_main_menu_bar(void) {
    return ImGui::BeginMainMenuBar() ? 1 : 0;
}

extern "C" void ori_imgui_end_main_menu_bar(void) {
    ImGui::EndMainMenuBar();
}

extern "C" int ori_imgui_begin_menu(int64_t label_ptr) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "Menu";
    return ImGui::BeginMenu(label) ? 1 : 0;
}

extern "C" void ori_imgui_end_menu(void) {
    ImGui::EndMenu();
}

extern "C" int ori_imgui_menu_item(int64_t label_ptr) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "item";
    return ImGui::MenuItem(label) ? 1 : 0;
}

/* ---- P2-A: raylib embed (window already created by game.app / raylib) ---- */

#ifndef ORI_IMGUI_NO_RAYLIB

static void raylib_feed_input(void) {
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)GetScreenWidth(), (float)GetScreenHeight());
    float dt = GetFrameTime();
    io.DeltaTime = dt > 0.0f ? dt : (1.0f / 60.0f);

    Vector2 mp = GetMousePosition();
    io.AddMousePosEvent(mp.x, mp.y);
    io.AddMouseButtonEvent(0, IsMouseButtonDown(MOUSE_BUTTON_LEFT));
    io.AddMouseButtonEvent(1, IsMouseButtonDown(MOUSE_BUTTON_RIGHT));
    io.AddMouseButtonEvent(2, IsMouseButtonDown(MOUSE_BUTTON_MIDDLE));
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) io.AddMouseWheelEvent(0.0f, wheel);

    /* Common keys → ImGui (subset sufficient for editor HUD). */
    struct Map { int rl; ImGuiKey im; };
    static const Map keymap[] = {
        { KEY_TAB, ImGuiKey_Tab },
        { KEY_LEFT, ImGuiKey_LeftArrow },
        { KEY_RIGHT, ImGuiKey_RightArrow },
        { KEY_UP, ImGuiKey_UpArrow },
        { KEY_DOWN, ImGuiKey_DownArrow },
        { KEY_PAGE_UP, ImGuiKey_PageUp },
        { KEY_PAGE_DOWN, ImGuiKey_PageDown },
        { KEY_HOME, ImGuiKey_Home },
        { KEY_END, ImGuiKey_End },
        { KEY_INSERT, ImGuiKey_Insert },
        { KEY_DELETE, ImGuiKey_Delete },
        { KEY_BACKSPACE, ImGuiKey_Backspace },
        { KEY_SPACE, ImGuiKey_Space },
        { KEY_ENTER, ImGuiKey_Enter },
        { KEY_ESCAPE, ImGuiKey_Escape },
        { KEY_LEFT_CONTROL, ImGuiKey_LeftCtrl },
        { KEY_LEFT_SHIFT, ImGuiKey_LeftShift },
        { KEY_LEFT_ALT, ImGuiKey_LeftAlt },
        { KEY_RIGHT_CONTROL, ImGuiKey_RightCtrl },
        { KEY_RIGHT_SHIFT, ImGuiKey_RightShift },
        { KEY_A, ImGuiKey_A }, { KEY_C, ImGuiKey_C }, { KEY_V, ImGuiKey_V },
        { KEY_X, ImGuiKey_X }, { KEY_Y, ImGuiKey_Y }, { KEY_Z, ImGuiKey_Z },
    };
    for (const Map &m : keymap) {
        if (IsKeyPressed(m.rl)) io.AddKeyEvent(m.im, true);
        if (IsKeyReleased(m.rl)) io.AddKeyEvent(m.im, false);
    }
    io.AddKeyEvent(ImGuiMod_Ctrl, IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL));
    io.AddKeyEvent(ImGuiMod_Shift, IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
    io.AddKeyEvent(ImGuiMod_Alt, IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));

    int ch = GetCharPressed();
    while (ch > 0) {
        io.AddInputCharacter((unsigned int)ch);
        ch = GetCharPressed();
    }
}

extern "C" int ori_imgui_init_raylib(void) {
    if (g_inited) return g_mode == HOST_RAYLIB ? 0 : 1;
    ImGuiContext *primary = ImGui::CreateContext();
    g_ctx[0] = primary;
    g_ctx_live[0] = 1;
    g_current_ctx = 0;
    ImGui::SetCurrentContext(primary);
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; /* Tier 0 */
    style_colors_spectrum_impl(true); /* Studio default: Spectrum dark */
    for (int i = 0; i < INPUT_SLOTS; ++i) g_input_bufs[i][0] = '\0';
    g_input_active = 0;
    /* raylib already owns the GL context + window */
    ImGui_ImplOpenGL3_Init("#version 130");
    g_window = nullptr;
    g_mode = HOST_RAYLIB;
    g_inited = true;
    return 0;
}

extern "C" void ori_imgui_begin_frame_raylib(void) {
    if (!g_inited || g_mode != HOST_RAYLIB) return;
    raylib_feed_input();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

extern "C" void ori_imgui_end_frame_raylib(void) {
    if (!g_inited || g_mode != HOST_RAYLIB) return;
    ImGui::Render();
    /* Do not clear or swap — game.app / raylib EndDrawing owns the framebuffer. */
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

extern "C" int ori_imgui_is_raylib_mode(void) {
    return g_mode == HOST_RAYLIB ? 1 : 0;
}

#else /* ORI_IMGUI_NO_RAYLIB */

extern "C" int ori_imgui_init_raylib(void) { return 1; }
extern "C" void ori_imgui_begin_frame_raylib(void) {}
extern "C" void ori_imgui_end_frame_raylib(void) {}
extern "C" int ori_imgui_is_raylib_mode(void) { return 0; }

#endif

/* ---- Tier 0: docking + tables + selectable ---- */

extern "C" void ori_imgui_dock_space_over_viewport(void) {
    if (!g_inited) return;
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
}

/* Stable dockspace id (not window-scoped GetID). */
static ImGuiID studio_dock_id(void) {
    return ImHashStr("OriStudioDock");
}

/* Host dockspace with passthrough central node (raylib 3D viewport visible). */
extern "C" void ori_imgui_dock_space_studio(void) {
    if (!g_inited) return;
    ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags host_flags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("##StudioDockHost", nullptr, host_flags);
    ImGui::PopStyleVar(3);
    ImGui::DockSpace(studio_dock_id(), ImVec2(0.0f, 0.0f),
                     ImGuiDockNodeFlags_PassthruCentralNode |
                         ImGuiDockNodeFlags_NoDockingInCentralNode);
    ImGui::End();
}

/*
 * True when mouse is over the empty central dock (3D viewport free zone).
 * Dock host is fullscreen; without this, WantCaptureMouse is almost always true.
 */
extern "C" int ori_imgui_is_viewport_hovered(void) {
    if (!g_inited) return 1;
    ImGuiDockNode *central = ImGui::DockBuilderGetCentralNode(studio_dock_id());
    if (central == nullptr) {
        return ImGui::GetIO().WantCaptureMouse ? 0 : 1;
    }
    const ImRect r = central->Rect();
    const ImVec2 mp = ImGui::GetMousePos();
    if (!r.Contains(mp)) {
        return 0;
    }
    if (!ImGui::GetIO().WantCaptureMouse) {
        return 1;
    }
    ImGuiContext &g = *GImGui;
    if (g.HoveredWindow == nullptr) {
        return 1;
    }
    const char *name = g.HoveredWindow->Name;
    if (name != nullptr && strstr(name, "##StudioDockHost") != nullptr) {
        return 1;
    }
    /* Any other hovered ImGui window (docked panel / floating) owns mouse. */
    return 0;
}

/*
 * ADR 2026-07-15 ImGui fino — default layout (Reset Layout):
 *
 *  ┌ Hierarchy ─┬──── Viewport (center / passthrough) ──┬─ Inspector ─┐
 *  │            │                                        │             │
 *  └────────────┴────────────────────────────────────────┴─────────────┘
 *
 * No bottom row. Project/Tools/Console/… opt-in via View menu (when shown).
 */
extern "C" void ori_imgui_apply_studio_dock_layout(void) {
    if (!g_inited) return;
    ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGuiID dock_id = studio_dock_id();

    ImGui::DockBuilderRemoveNode(dock_id);
    ImGui::DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dock_id, vp->WorkSize);

    ImGuiID main_id = dock_id;
    ImGuiID left_id = 0;
    ImGuiID right_id = 0;

    /* Left ~20% */
    ImGui::DockBuilderSplitNode(main_id, ImGuiDir_Left, 0.20f, &left_id, &main_id);
    /* Right ~22% of remaining */
    ImGui::DockBuilderSplitNode(main_id, ImGuiDir_Right, 0.22f, &right_id, &main_id);

    ImGui::DockBuilderDockWindow("Hierarchy", left_id);
    ImGui::DockBuilderDockWindow("Inspector", right_id);
    /* Optional panels dock into same columns if user enables them */
    ImGui::DockBuilderDockWindow("Project", left_id);
    ImGui::DockBuilderDockWindow("Tools", right_id);

    /* Center remains empty → PassthruCentralNode shows 3D viewport */

    ImGui::DockBuilderFinish(dock_id);
}

extern "C" int ori_imgui_dock_space(int64_t id_ptr, int w, int h) {
    if (!g_inited) return 0;
    const char *id = (const char *)(intptr_t)id_ptr;
    if (!id) id = "dockspace";
    ImGuiID dock_id = ImGui::GetID(id);
    return (int)ImGui::DockSpace(dock_id, ImVec2((float)w, (float)h));
}

extern "C" int ori_imgui_begin_table(int64_t id_ptr, int columns, int flags) {
    const char *id = (const char *)(intptr_t)id_ptr;
    if (!id) id = "table";
    if (columns < 1) columns = 1;
    return ImGui::BeginTable(id, columns, (ImGuiTableFlags)flags) ? 1 : 0;
}

extern "C" void ori_imgui_end_table(void) {
    ImGui::EndTable();
}

extern "C" void ori_imgui_table_next_row(void) {
    ImGui::TableNextRow();
}

extern "C" int ori_imgui_table_next_column(void) {
    return ImGui::TableNextColumn() ? 1 : 0;
}

extern "C" int ori_imgui_table_set_column_index(int column) {
    return ImGui::TableSetColumnIndex(column) ? 1 : 0;
}

extern "C" void ori_imgui_table_setup_column(int64_t label_ptr, int flags) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "";
    ImGui::TableSetupColumn(label, (ImGuiTableColumnFlags)flags);
}

extern "C" void ori_imgui_table_headers_row(void) {
    ImGui::TableHeadersRow();
}

extern "C" int ori_imgui_selectable(int64_t label_ptr, int selected) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "item";
    bool sel = selected != 0;
    if (ImGui::Selectable(label, sel)) {
        return 1;
    }
    return 0;
}

extern "C" int ori_imgui_is_item_clicked(void) {
    return ImGui::IsItemClicked() ? 1 : 0;
}

extern "C" void ori_imgui_columns(int count, int64_t id_ptr, int border) {
    const char *id = (const char *)(intptr_t)id_ptr;
    ImGui::Columns(count > 0 ? count : 1, id && id[0] ? id : nullptr, border != 0);
}

extern "C" void ori_imgui_next_column(void) {
    ImGui::NextColumn();
}

/* ---- Tier 1: popups, plot, draw-list helpers ---- */

#define PLOT_CAP 256
static float g_plot_vals[PLOT_CAP];
static int g_plot_count = 0;

extern "C" void ori_imgui_open_popup(int64_t id_ptr) {
    const char *id = (const char *)(intptr_t)id_ptr;
    if (!id) id = "popup";
    ImGui::OpenPopup(id);
}

extern "C" int ori_imgui_begin_popup_modal(int64_t name_ptr) {
    const char *name = (const char *)(intptr_t)name_ptr;
    if (!name) name = "Modal";
    return ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize) ? 1 : 0;
}

extern "C" int ori_imgui_begin_popup(int64_t id_ptr) {
    const char *id = (const char *)(intptr_t)id_ptr;
    if (!id) id = "popup";
    return ImGui::BeginPopup(id) ? 1 : 0;
}

extern "C" void ori_imgui_end_popup(void) {
    ImGui::EndPopup();
}

extern "C" void ori_imgui_close_current_popup(void) {
    ImGui::CloseCurrentPopup();
}

extern "C" void ori_imgui_plot_clear(void) {
    g_plot_count = 0;
}

extern "C" void ori_imgui_plot_push_milli(int value_milli) {
    if (g_plot_count >= PLOT_CAP) {
        /* shift left */
        memmove(g_plot_vals, g_plot_vals + 1, (PLOT_CAP - 1) * sizeof(float));
        g_plot_count = PLOT_CAP - 1;
    }
    g_plot_vals[g_plot_count++] = value_milli / 1000.0f;
}

extern "C" void ori_imgui_plot_lines(
    int64_t label_ptr, int scale_min_milli, int scale_max_milli, int w, int h) {
    const char *label = (const char *)(intptr_t)label_ptr;
    if (!label) label = "plot";
    float smin = scale_min_milli / 1000.0f;
    float smax = scale_max_milli / 1000.0f;
    if (g_plot_count <= 0) {
        ImGui::Text("%s (empty)", label);
        return;
    }
    ImGui::PlotLines(
        label, g_plot_vals, g_plot_count, 0, nullptr, smin, smax,
        ImVec2((float)(w > 0 ? w : -1), (float)(h > 0 ? h : 80)));
}

extern "C" int ori_imgui_plot_count(void) {
    return g_plot_count;
}

extern "C" int ori_imgui_cursor_screen_x(void) {
    return (int)ImGui::GetCursorScreenPos().x;
}

extern "C" int ori_imgui_cursor_screen_y(void) {
    return (int)ImGui::GetCursorScreenPos().y;
}

extern "C" void ori_imgui_set_cursor_screen(int x, int y) {
    ImGui::SetCursorScreenPos(ImVec2((float)x, (float)y));
}

extern "C" void ori_imgui_dummy(int w, int h) {
    ImGui::Dummy(ImVec2((float)w, (float)h));
}

extern "C" void ori_imgui_draw_line(
    int x0, int y0, int x1, int y1, int r, int g, int b, int a, int thick) {
    ImDrawList *dl = ImGui::GetWindowDrawList();
    if (!dl) return;
    ImU32 col = IM_COL32(r & 255, g & 255, b & 255, a & 255);
    float t = thick > 0 ? (float)thick : 1.0f;
    dl->AddLine(ImVec2((float)x0, (float)y0), ImVec2((float)x1, (float)y1), col, t);
}

extern "C" void ori_imgui_draw_rect(
    int x, int y, int w, int h, int r, int g, int b, int a, int filled) {
    ImDrawList *dl = ImGui::GetWindowDrawList();
    if (!dl) return;
    ImU32 col = IM_COL32(r & 255, g & 255, b & 255, a & 255);
    ImVec2 p0((float)x, (float)y);
    ImVec2 p1((float)(x + w), (float)(y + h));
    if (filled) {
        dl->AddRectFilled(p0, p1, col);
    } else {
        dl->AddRect(p0, p1, col);
    }
}

extern "C" int ori_imgui_is_mouse_clicked(int button) {
    return ImGui::IsMouseClicked(button) ? 1 : 0;
}

extern "C" int ori_imgui_is_mouse_down(int button) {
    return ImGui::IsMouseDown(button) ? 1 : 0;
}

extern "C" int ori_imgui_is_mouse_released(int button) {
    return ImGui::IsMouseReleased(button) ? 1 : 0;
}

extern "C" int ori_imgui_mouse_x(void) {
    return (int)ImGui::GetIO().MousePos.x;
}

extern "C" int ori_imgui_mouse_y(void) {
    return (int)ImGui::GetIO().MousePos.y;
}

extern "C" int ori_imgui_is_item_active(void) {
    return ImGui::IsItemActive() ? 1 : 0;
}

extern "C" int ori_imgui_is_item_hovered(void) {
    return ImGui::IsItemHovered() ? 1 : 0;
}

extern "C" void ori_imgui_text_disabled(int64_t text_ptr) {
    const char *text = (const char *)(intptr_t)text_ptr;
    if (!text) text = "";
    ImGui::TextDisabled("%s", text);
}

/* ---- Multi-context (B3 / PR5) ----
 * Slot 0 = primary (editor or game host after init).
 * Extra slots = isolated ImGui contexts (e.g. secondary editor panels).
 * Switch with set_current_context before begin_frame / widget calls.
 */
extern "C" int ori_imgui_create_context(void) {
    if (!g_inited) return -1;
    for (int i = 1; i < MAX_IMGUI_CTX; ++i) {
        if (!g_ctx_live[i]) {
            g_ctx[i] = ImGui::CreateContext();
            if (!g_ctx[i]) return -1;
            g_ctx_live[i] = 1;
            ImGui::SetCurrentContext(g_ctx[i]);
            ImGui::StyleColorsDark();
            /* Restore previous current so create is non-stealing. */
            if (g_ctx_live[g_current_ctx] && g_ctx[g_current_ctx]) {
                ImGui::SetCurrentContext(g_ctx[g_current_ctx]);
            } else {
                ImGui::SetCurrentContext(g_ctx[0]);
                g_current_ctx = 0;
            }
            return i;
        }
    }
    return -1; /* pool full (MAX_IMGUI_CTX) */
}

extern "C" int ori_imgui_set_current_context(int id) {
    if (!g_inited) return 1;
    if (id < 0 || id >= MAX_IMGUI_CTX || !g_ctx_live[id] || !g_ctx[id]) return 1;
    ImGui::SetCurrentContext(g_ctx[id]);
    g_current_ctx = id;
    return 0;
}

extern "C" int ori_imgui_current_context(void) {
    return g_current_ctx;
}

extern "C" int ori_imgui_context_is_live(int id) {
    if (id < 0 || id >= MAX_IMGUI_CTX) return 0;
    return (g_ctx_live[id] && g_ctx[id]) ? 1 : 0;
}

extern "C" int ori_imgui_max_contexts(void) {
    return MAX_IMGUI_CTX;
}

extern "C" void ori_imgui_destroy_context(int id) {
    /* Primary slot 0 is owned by init/shutdown — refuse destroy. */
    if (id <= 0 || id >= MAX_IMGUI_CTX || !g_ctx_live[id] || !g_ctx[id]) return;
    if (g_current_ctx == id) {
        ImGui::SetCurrentContext(g_ctx[0]);
        g_current_ctx = 0;
    }
    ImGui::DestroyContext(g_ctx[id]);
    g_ctx[id] = nullptr;
    g_ctx_live[id] = 0;
}

/* ---- Tier 2 style / image ---- */

/* Adobe Spectrum-inspired palette (colors from adobe/imgui Spectrum.md / spectrum.cpp).
 * Theme only — no widget API fork, no AdobeClean font. */
static ImVec4 spectrum_rgb(unsigned int rgb) {
    const float r = (float)((rgb >> 16) & 0xFF) / 255.0f;
    const float g = (float)((rgb >> 8) & 0xFF) / 255.0f;
    const float b = (float)((rgb >> 0) & 0xFF) / 255.0f;
    return ImVec4(r, g, b, 1.0f);
}

static ImVec4 spectrum_rgba(unsigned int rgb, float a) {
    ImVec4 c = spectrum_rgb(rgb);
    c.w = a;
    return c;
}

static void style_colors_spectrum_impl(bool dark) {
    /* Dark palette (GRAY50..GRAY900 inverted sense vs light) */
    const unsigned int g50  = dark ? 0x252525 : 0xFFFFFF;
    const unsigned int g75  = dark ? 0x2F2F2F : 0xFAFAFA;
    const unsigned int g100 = dark ? 0x323232 : 0xF5F5F5;
    const unsigned int g200 = dark ? 0x393939 : 0xEAEAEA;
    const unsigned int g300 = dark ? 0x3E3E3E : 0xE1E1E1;
    const unsigned int g400 = dark ? 0x4D4D4D : 0xCACACA;
    const unsigned int g500 = dark ? 0x5C5C5C : 0xB3B3B3;
    const unsigned int g600 = dark ? 0x7B7B7B : 0x8E8E8E;
    const unsigned int g700 = dark ? 0x999999 : 0x707070;
    const unsigned int g800 = dark ? 0xCDCDCD : 0x4B4B4B;
    const unsigned int g900 = dark ? 0xFFFFFF : 0x2C2C2C;
    const unsigned int blue400 = dark ? 0x2680EB : 0x2680EB;
    const unsigned int blue500 = dark ? 0x378EF0 : 0x1473E6;
    const unsigned int blue600 = dark ? 0x4B9CF5 : 0x0D66D0;
    const unsigned int blue700 = dark ? 0x5AA9FA : 0x095ABA;

    ImGuiStyle *style = &ImGui::GetStyle();
    style->WindowRounding = 4.0f;
    style->ChildRounding = 4.0f;
    style->FrameRounding = 4.0f;
    style->PopupRounding = 4.0f;
    style->ScrollbarRounding = 4.0f;
    style->GrabRounding = 4.0f;
    style->TabRounding = 4.0f;
    style->FrameBorderSize = 1.0f;
    style->WindowBorderSize = 1.0f;
    style->PopupBorderSize = 1.0f;
    style->WindowPadding = ImVec2(10.0f, 10.0f);
    style->FramePadding = ImVec2(8.0f, 4.0f);
    style->ItemSpacing = ImVec2(8.0f, 6.0f);
    style->ScrollbarSize = 12.0f;

    ImVec4 *col = style->Colors;
    col[ImGuiCol_Text] = spectrum_rgb(g800);
    col[ImGuiCol_TextDisabled] = spectrum_rgb(g500);
    col[ImGuiCol_WindowBg] = spectrum_rgb(g100);
    col[ImGuiCol_ChildBg] = spectrum_rgba(g50, 0.0f);
    col[ImGuiCol_PopupBg] = spectrum_rgb(g50);
    col[ImGuiCol_Border] = spectrum_rgb(g300);
    col[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    col[ImGuiCol_FrameBg] = spectrum_rgb(g75);
    col[ImGuiCol_FrameBgHovered] = spectrum_rgb(g50);
    col[ImGuiCol_FrameBgActive] = spectrum_rgb(g200);
    col[ImGuiCol_TitleBg] = spectrum_rgb(g300);
    col[ImGuiCol_TitleBgActive] = spectrum_rgb(g200);
    col[ImGuiCol_TitleBgCollapsed] = spectrum_rgb(g400);
    col[ImGuiCol_MenuBarBg] = spectrum_rgb(g100);
    col[ImGuiCol_ScrollbarBg] = spectrum_rgb(g100);
    col[ImGuiCol_ScrollbarGrab] = spectrum_rgb(g400);
    col[ImGuiCol_ScrollbarGrabHovered] = spectrum_rgb(g600);
    col[ImGuiCol_ScrollbarGrabActive] = spectrum_rgb(g700);
    col[ImGuiCol_CheckMark] = spectrum_rgb(blue500);
    col[ImGuiCol_SliderGrab] = spectrum_rgb(g700);
    col[ImGuiCol_SliderGrabActive] = spectrum_rgb(g800);
    col[ImGuiCol_Button] = spectrum_rgb(g75);
    col[ImGuiCol_ButtonHovered] = spectrum_rgb(g50);
    col[ImGuiCol_ButtonActive] = spectrum_rgb(g200);
    col[ImGuiCol_Header] = spectrum_rgb(blue400);
    col[ImGuiCol_HeaderHovered] = spectrum_rgb(blue500);
    col[ImGuiCol_HeaderActive] = spectrum_rgb(blue600);
    col[ImGuiCol_Separator] = spectrum_rgb(g400);
    col[ImGuiCol_SeparatorHovered] = spectrum_rgb(g600);
    col[ImGuiCol_SeparatorActive] = spectrum_rgb(g700);
    col[ImGuiCol_ResizeGrip] = spectrum_rgb(g400);
    col[ImGuiCol_ResizeGripHovered] = spectrum_rgb(g600);
    col[ImGuiCol_ResizeGripActive] = spectrum_rgb(g700);
    col[ImGuiCol_Tab] = spectrum_rgb(g300);
    col[ImGuiCol_TabHovered] = spectrum_rgb(blue700);
    col[ImGuiCol_TabSelected] = spectrum_rgb(blue500);
    col[ImGuiCol_TabSelectedOverline] = spectrum_rgb(blue600);
    col[ImGuiCol_TabDimmed] = spectrum_rgb(g400);
    col[ImGuiCol_TabDimmedSelected] = spectrum_rgb(blue700);
    col[ImGuiCol_TabDimmedSelectedOverline] = spectrum_rgb(blue600);
    col[ImGuiCol_DockingPreview] = spectrum_rgba(blue500, 0.40f);
    col[ImGuiCol_DockingEmptyBg] = spectrum_rgb(g100);
    col[ImGuiCol_PlotLines] = spectrum_rgb(blue400);
    col[ImGuiCol_PlotLinesHovered] = spectrum_rgb(blue600);
    col[ImGuiCol_PlotHistogram] = spectrum_rgb(blue400);
    col[ImGuiCol_PlotHistogramHovered] = spectrum_rgb(blue600);
    col[ImGuiCol_TableHeaderBg] = spectrum_rgb(g200);
    col[ImGuiCol_TableBorderStrong] = spectrum_rgb(g400);
    col[ImGuiCol_TableBorderLight] = spectrum_rgb(g300);
    col[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
    col[ImGuiCol_TableRowBgAlt] = spectrum_rgba(g200, 0.35f);
    col[ImGuiCol_TextSelectedBg] = spectrum_rgba(blue400, 0.35f);
    col[ImGuiCol_DragDropTarget] = spectrum_rgba(0xDFBF00, 0.90f);
    col[ImGuiCol_NavCursor] = spectrum_rgba(g900, 0.40f);
    col[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.70f);
    col[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    col[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

extern "C" void ori_imgui_style_colors_dark(void) { ImGui::StyleColorsDark(); }
extern "C" void ori_imgui_style_colors_light(void) { ImGui::StyleColorsLight(); }
extern "C" void ori_imgui_style_colors_classic(void) { ImGui::StyleColorsClassic(); }
extern "C" void ori_imgui_style_colors_spectrum(void) { style_colors_spectrum_impl(true); }
extern "C" void ori_imgui_style_colors_spectrum_light(void) { style_colors_spectrum_impl(false); }

extern "C" void ori_imgui_push_style_color(int idx, int r, int g, int b, int a) {
    ImGui::PushStyleColor(idx, ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f));
}

extern "C" void ori_imgui_pop_style_color(int count) {
    ImGui::PopStyleColor(count > 0 ? count : 1);
}

extern "C" void ori_imgui_show_style_editor(void) {
    ImGui::ShowStyleEditor(nullptr);
}

static ImTextureRef texture_ref_from_id(int tex_id) {
    /* Opaque GPU id (OpenGL GLuint / raylib Texture.id) as ImTextureID. */
    return ImTextureRef((ImTextureID)(ImU64)(intptr_t)tex_id);
}

extern "C" void ori_imgui_image(int tex_id, int w, int h) {
    int sw = w > 0 ? w : 1;
    int sh = h > 0 ? h : 1;
    ImGui::Image(texture_ref_from_id(tex_id), ImVec2((float)sw, (float)sh));
}

/* ImageButton from texture id. Returns 1 if clicked this frame. */
extern "C" int ori_imgui_image_button(int64_t id_ptr, int tex_id, int w, int h) {
    const char *str_id = (const char *)(intptr_t)id_ptr;
    if (!str_id || !str_id[0]) str_id = "##imgbtn";
    int sw = w > 0 ? w : 1;
    int sh = h > 0 ? h : 1;
    return ImGui::ImageButton(str_id, texture_ref_from_id(tex_id), ImVec2((float)sw, (float)sh)) ? 1 : 0;
}
