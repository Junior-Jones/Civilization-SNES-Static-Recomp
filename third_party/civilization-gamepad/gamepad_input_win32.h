#ifndef CIVILIZATION_GAMEPAD_INPUT_WIN32_H
#define CIVILIZATION_GAMEPAD_INPUT_WIN32_H

#include <stddef.h>
#include <stdint.h>
#include <windows.h>

#include <SDL3/SDL_gamepad.h>

#define CIVILIZATION_GAMEPAD_BINDING_COUNT 12

typedef enum CivilizationGamepadControl {
    SC_GAMEPAD_DPAD_UP = 1,
    SC_GAMEPAD_DPAD_DOWN,
    SC_GAMEPAD_DPAD_LEFT,
    SC_GAMEPAD_DPAD_RIGHT,
    SC_GAMEPAD_LEFT_STICK_UP,
    SC_GAMEPAD_LEFT_STICK_DOWN,
    SC_GAMEPAD_LEFT_STICK_LEFT,
    SC_GAMEPAD_LEFT_STICK_RIGHT,
    SC_GAMEPAD_FACE_SOUTH,
    SC_GAMEPAD_FACE_EAST,
    SC_GAMEPAD_FACE_WEST,
    SC_GAMEPAD_FACE_NORTH,
    SC_GAMEPAD_LEFT_SHOULDER,
    SC_GAMEPAD_RIGHT_SHOULDER,
    SC_GAMEPAD_LEFT_TRIGGER,
    SC_GAMEPAD_RIGHT_TRIGGER,
    SC_GAMEPAD_START,
    SC_GAMEPAD_BACK,
    SC_GAMEPAD_LEFT_STICK_BUTTON,
    SC_GAMEPAD_RIGHT_STICK_BUTTON,
    SC_GAMEPAD_CONTROL_LAST = SC_GAMEPAD_RIGHT_STICK_BUTTON
} CivilizationGamepadControl;

typedef struct CivilizationGamepadInputWin32 {
    SDL_Gamepad *handle;
    int initialized;
    int startup_gamepad_found;
    unsigned refresh_countdown;
    wchar_t name[160];
} CivilizationGamepadInputWin32;

int civilization_gamepad_win32_initialize(CivilizationGamepadInputWin32 *input,
                                     const wchar_t *mapping_path);
void civilization_gamepad_win32_shutdown(CivilizationGamepadInputWin32 *input);
uint16_t civilization_gamepad_win32_poll(
    CivilizationGamepadInputWin32 *input,
    const int bindings[CIVILIZATION_GAMEPAD_BINDING_COUNT]);
int civilization_gamepad_win32_connected(const CivilizationGamepadInputWin32 *input);
const wchar_t *civilization_gamepad_win32_name(const CivilizationGamepadInputWin32 *input);
void civilization_gamepad_win32_default_bindings(
    int bindings[CIVILIZATION_GAMEPAD_BINDING_COUNT]);
const wchar_t *civilization_gamepad_win32_control_name(int control);
int civilization_gamepad_win32_capture_control(CivilizationGamepadInputWin32 *input);
void civilization_gamepad_win32_control_display_name(
    const CivilizationGamepadInputWin32 *input, int control,
    wchar_t *text, size_t capacity);

#endif
