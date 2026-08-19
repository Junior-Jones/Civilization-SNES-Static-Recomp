#ifndef CIVILIZATION_AUDIO_OUTPUT_WINMM_H
#define CIVILIZATION_AUDIO_OUTPUT_WINMM_H

#if !defined(_WIN32)
#error This audio backend is for Windows only.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>

#include "civilization_app_core.h"
#include "audio_recorder_win32.h"

#include <stddef.h>
#include <stdint.h>

#define CIVILIZATION_AUDIO_DEVICE_NAME_CAPACITY 128u
#define CIVILIZATION_AUDIO_BUFFER_COUNT 4u

typedef struct CivilizationAudioSettings {
    int enabled;
    int volume_percent;
    int latency_ms;
    wchar_t device_name[CIVILIZATION_AUDIO_DEVICE_NAME_CAPACITY];
} CivilizationAudioSettings;

typedef struct CivilizationAudioOutput {
    HWAVEOUT handle;
    WAVEHDR headers[CIVILIZATION_AUDIO_BUFFER_COUNT];
    int16_t *sample_storage;
    int16_t *pending_storage;
    uint32_t buffer_frames;
    uint32_t pending_frames;
    uint32_t queued_buffers;
    uint32_t fade_frames_remaining;
    int priming;
    int paused;
    int volume_percent;
    wchar_t opened_device_name[CIVILIZATION_AUDIO_DEVICE_NAME_CAPACITY];
} CivilizationAudioOutput;

void civilization_audio_settings_defaults(CivilizationAudioSettings *settings);
void civilization_audio_settings_load(CivilizationAudioSettings *settings,
                                 const wchar_t *ini_path);
void civilization_audio_settings_save(const CivilizationAudioSettings *settings,
                                 const wchar_t *ini_path);

UINT civilization_audio_device_count(void);
int civilization_audio_device_name(UINT device_index,
                              wchar_t *name,
                              size_t name_capacity);

void civilization_audio_output_initialize(CivilizationAudioOutput *output);
int civilization_audio_output_open(CivilizationAudioOutput *output,
                              const CivilizationAudioSettings *settings,
                              wchar_t *error,
                              size_t error_capacity);
void civilization_audio_output_close(CivilizationAudioOutput *output);
void civilization_audio_output_pause(CivilizationAudioOutput *output);
void civilization_audio_output_resume(CivilizationAudioOutput *output);
void civilization_audio_output_flush(CivilizationAudioOutput *output);
int civilization_audio_output_is_open(const CivilizationAudioOutput *output);

/* Pulls only native PCM from the isolated static-recompilation core. */
void civilization_audio_output_pump(CivilizationAudioOutput *output,
                               CivilizationAudioRecorderWin32 *recorder,
                               CivilizationRecomp *game);

#endif
