#ifndef CIVILIZATION_AUDIO_RECORDER_WIN32_H
#define CIVILIZATION_AUDIO_RECORDER_WIN32_H
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

typedef struct CivilizationAudioRecorderWin32 {
    FILE *file;
    uint64_t frames_written;
    int write_failed;
    wchar_t path[4096];
    wchar_t last_error[256];
} CivilizationAudioRecorderWin32;
void civilization_audio_recorder_win32_init(CivilizationAudioRecorderWin32 *r);
int civilization_audio_recorder_win32_start(CivilizationAudioRecorderWin32 *r,
                                      const wchar_t *audio_directory);
int civilization_audio_recorder_win32_write(CivilizationAudioRecorderWin32 *r,
                                      const int16_t *samples,size_t frames);
int civilization_audio_recorder_win32_stop(CivilizationAudioRecorderWin32 *r);
int civilization_audio_recorder_win32_active(const CivilizationAudioRecorderWin32 *r);
#endif
