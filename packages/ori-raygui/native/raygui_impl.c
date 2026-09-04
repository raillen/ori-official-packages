/* Raygui implementation + scalar FFI helpers for Ori. */
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TEXTBOX_CAP 256
static char g_textbox[TEXTBOX_CAP];
static char g_dropdown_edit = 0; /* not used externally; edit mode is Ori-side */

bool ori_gui_button(float x, float y, float w, float h, const char *text) {
    return GuiButton((Rectangle){ x, y, w, h }, text) != 0;
}

void ori_gui_label(float x, float y, float w, float h, const char *text) {
    GuiLabel((Rectangle){ x, y, w, h }, text);
}

bool ori_gui_checkbox(float x, float y, float w, float h, const char *text, bool checked) {
    bool value = checked;
    GuiCheckBox((Rectangle){ x, y, w, h }, text, &value);
    return value;
}

float ori_gui_slider(float x, float y, float w, float h, const char *textLeft, const char *textRight,
                     float value, float minValue, float maxValue) {
    float v = value;
    GuiSlider((Rectangle){ x, y, w, h }, textLeft, textRight, &v, minValue, maxValue);
    return v;
}

int ori_gui_window_box(float x, float y, float w, float h, const char *title) {
    return GuiWindowBox((Rectangle){ x, y, w, h }, title);
}

void ori_gui_group_box(float x, float y, float w, float h, const char *text) {
    GuiGroupBox((Rectangle){ x, y, w, h }, text);
}

void ori_gui_status_bar(float x, float y, float w, float h, const char *text) {
    GuiStatusBar((Rectangle){ x, y, w, h }, text);
}

void ori_gui_panel(float x, float y, float w, float h, const char *text) {
    GuiPanel((Rectangle){ x, y, w, h }, text);
}

void ori_gui_line(float x, float y, float w, float h, const char *text) {
    GuiLine((Rectangle){ x, y, w, h }, text);
}

bool ori_gui_toggle(float x, float y, float w, float h, const char *text, bool active) {
    bool v = active;
    GuiToggle((Rectangle){ x, y, w, h }, text, &v);
    return v;
}

float ori_gui_progress_bar(float x, float y, float w, float h, const char *textLeft, const char *textRight,
                           float value, float minValue, float maxValue) {
    float v = value;
    GuiProgressBar((Rectangle){ x, y, w, h }, textLeft, textRight, &v, minValue, maxValue);
    return v;
}

/* Spinner: returns updated int value. edit_mode non-zero enables keyboard edit. */
int ori_gui_spinner(float x, float y, float w, float h, const char *text,
                    int value, int minValue, int maxValue, int edit_mode) {
    int v = value;
    GuiSpinner((Rectangle){ x, y, w, h }, text, &v, minValue, maxValue, edit_mode != 0);
    return v;
}

/* Value box: returns updated int. */
int ori_gui_value_box(float x, float y, float w, float h, const char *text,
                      int value, int minValue, int maxValue, int edit_mode) {
    int v = value;
    GuiValueBox((Rectangle){ x, y, w, h }, text, &v, minValue, maxValue, edit_mode != 0);
    return v;
}

/* Text box using host buffer. Returns 1 if Enter pressed (edit finished). */
int ori_gui_textbox(float x, float y, float w, float h, int edit_mode) {
    return GuiTextBox((Rectangle){ x, y, w, h }, g_textbox, TEXTBOX_CAP, edit_mode != 0);
}

void ori_gui_textbox_set(const char *text) {
    if (!text) {
        g_textbox[0] = '\0';
        return;
    }
    strncpy(g_textbox, text, TEXTBOX_CAP - 1);
    g_textbox[TEXTBOX_CAP - 1] = '\0';
}

int ori_gui_textbox_to_path(const char *path) {
    if (!path) return 1;
    FILE *f = fopen(path, "wb");
    if (!f) return 1;
    fputs(g_textbox, f);
    fclose(f);
    return 0;
}

/* Dropdown: items are semicolon-separated (raygui convention). Returns active index.
 * edit_mode: pass 1 while open. Returns -1 if just closed (clicked); else active. */
int ori_gui_dropdown(float x, float y, float w, float h, const char *items,
                     int active, int edit_mode) {
    int a = active;
    int result = GuiDropdownBox((Rectangle){ x, y, w, h }, items, &a, edit_mode != 0);
    (void)result;
    return a;
}

/* ListView: items semicolon-separated. Returns active index; scroll via in/out not exposed (fixed 0). */
int ori_gui_list_view(float x, float y, float w, float h, const char *items, int active) {
    int scroll = 0;
    int a = active;
    GuiListView((Rectangle){ x, y, w, h }, items, &scroll, &a);
    return a;
}

void ori_gui_set_style(int control, int property, int value) {
    GuiSetStyle(control, property, value);
}

int ori_gui_get_style(int control, int property) {
    return GuiGetStyle(control, property);
}

void ori_gui_enable(void) { GuiEnable(); }
void ori_gui_disable(void) { GuiDisable(); }
void ori_gui_lock(void) { GuiLock(); }
void ori_gui_unlock(void) { GuiUnlock(); }
