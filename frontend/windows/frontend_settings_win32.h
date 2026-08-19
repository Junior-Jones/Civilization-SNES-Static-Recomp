#ifndef CIVILIZATION_FRONTEND_SETTINGS_WIN32_H
#define CIVILIZATION_FRONTEND_SETTINGS_WIN32_H

#include <windows.h>
#include <stdint.h>

#include "gamepad_input_win32.h"

#define CIVILIZATION_WIN_BINDING_COUNT 12
#define CIVILIZATION_INPUT_SOURCE_KEYBOARD 0
#define CIVILIZATION_INPUT_SOURCE_GAMEPAD 1

typedef enum CivilizationWinBindingAction {
    SC_WIN_BIND_UP = 0, SC_WIN_BIND_DOWN, SC_WIN_BIND_LEFT, SC_WIN_BIND_RIGHT,
    SC_WIN_BIND_SNES_B, SC_WIN_BIND_SNES_A, SC_WIN_BIND_SNES_Y,
    SC_WIN_BIND_SNES_X, SC_WIN_BIND_SNES_L, SC_WIN_BIND_SNES_R,
    SC_WIN_BIND_START, SC_WIN_BIND_SELECT
} CivilizationWinBindingAction;

typedef struct CivilizationFrontendSettingsWin32 {
    int integer_scale;
    int pause_on_focus_loss;
    int auto_run_on_load;
    int fullscreen_on_play;
    int show_status_text;
    int snapshot_slot;
    int input_source;
    int input_source_saved;
    int getting_started_shown;
    UINT bindings[CIVILIZATION_WIN_BINDING_COUNT];
    int gamepad_bindings[CIVILIZATION_WIN_BINDING_COUNT];
} CivilizationFrontendSettingsWin32;

void civilization_frontend_settings_win32_defaults(CivilizationFrontendSettingsWin32 *s);
void civilization_frontend_settings_win32_classic(CivilizationFrontendSettingsWin32 *s);
void civilization_frontend_settings_win32_load(CivilizationFrontendSettingsWin32 *s,
                                          const wchar_t *path);
int civilization_frontend_settings_win32_save(const CivilizationFrontendSettingsWin32 *s,
                                         const wchar_t *path);
uint16_t civilization_frontend_settings_win32_input(
    const CivilizationFrontendSettingsWin32 *s, UINT virtual_key);
const wchar_t *civilization_frontend_settings_win32_action_name(int action);
void civilization_frontend_settings_win32_key_name(UINT virtual_key,
                                               wchar_t *text, size_t capacity);
int civilization_frontend_settings_win32_dialog(HWND parent, HINSTANCE instance,
                                           CivilizationFrontendSettingsWin32 *s);
int civilization_frontend_controls_win32_dialog(HWND parent, HINSTANCE instance,
                                           CivilizationFrontendSettingsWin32 *s,
                                           CivilizationGamepadInputWin32 *gamepad);

#endif
