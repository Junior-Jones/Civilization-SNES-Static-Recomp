#ifndef CIVILIZATION_APP_CORE_H
#define CIVILIZATION_APP_CORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIVILIZATION_RECOMP_ROM_SIZE 1572864u
#define CIVILIZATION_RECOMP_FRAME_WIDTH 256u
#define CIVILIZATION_RECOMP_FRAME_HEIGHT 224u
#define CIVILIZATION_RECOMP_HOST_AUDIO_SAMPLE_RATE 32040u
#define CIVILIZATION_RECOMP_AUDIO_CHANNELS 2u
#define CIVILIZATION_RECOMP_AUDIO_BITS_PER_SAMPLE 16u
#define CIVILIZATION_RECOMP_PRESENTATION_FPS_NUMERATOR 39375000u
#define CIVILIZATION_RECOMP_PRESENTATION_FPS_DENOMINATOR 655171u

enum CivilizationRecompInput {
    CIVILIZATION_INPUT_B      = 0x8000u,
    CIVILIZATION_INPUT_Y      = 0x4000u,
    CIVILIZATION_INPUT_SELECT = 0x2000u,
    CIVILIZATION_INPUT_START  = 0x1000u,
    CIVILIZATION_INPUT_UP     = 0x0800u,
    CIVILIZATION_INPUT_DOWN   = 0x0400u,
    CIVILIZATION_INPUT_LEFT   = 0x0200u,
    CIVILIZATION_INPUT_RIGHT  = 0x0100u,
    CIVILIZATION_INPUT_A      = 0x0080u,
    CIVILIZATION_INPUT_X      = 0x0040u,
    CIVILIZATION_INPUT_L      = 0x0020u,
    CIVILIZATION_INPUT_R      = 0x0010u
};

typedef struct CivilizationRecomp CivilizationRecomp;

typedef struct CivilizationRecompFrameResult {
    uint8_t route_continued;
    uint8_t frame_rendered;
    uint16_t input_mask;
    uint32_t start_frame;
    uint32_t end_frame;
    char renderer_error[192];
} CivilizationRecompFrameResult;

int civilization_recomp_create(CivilizationRecomp **out_instance,
                                const uint8_t *rom, size_t rom_size,
                                char *error, size_t error_capacity);
void civilization_recomp_destroy(CivilizationRecomp *instance);
int civilization_recomp_reset(CivilizationRecomp *instance,
                               char *error, size_t error_capacity);
int civilization_recomp_advance(CivilizationRecomp *instance,
                                 uint16_t input_mask, uint32_t frame_count,
                                 CivilizationRecompFrameResult *result);
int civilization_recomp_advance_headless(CivilizationRecomp *instance,
                                          uint16_t input_mask,
                                          uint32_t frame_count,
                                          CivilizationRecompFrameResult *result);
const uint32_t *civilization_recomp_frame_bgra(
    const CivilizationRecomp *instance);
uint32_t civilization_recomp_current_frame(
    const CivilizationRecomp *instance);
const char *civilization_recomp_last_error(
    const CivilizationRecomp *instance);

size_t civilization_recomp_audio_available(
    const CivilizationRecomp *instance);
size_t civilization_recomp_audio_read(CivilizationRecomp *instance,
                                       int16_t *interleaved_stereo,
                                       size_t frame_capacity);
size_t civilization_recomp_audio_discard(CivilizationRecomp *instance);
int civilization_recomp_audio_overflowed(
    const CivilizationRecomp *instance);
void civilization_recomp_audio_clear_overflow(
    CivilizationRecomp *instance);

int civilization_recomp_snapshot_save(const CivilizationRecomp *instance,
                                       const char *path, char *error,
                                       size_t error_capacity);
int civilization_recomp_snapshot_load(CivilizationRecomp *instance,
                                       const char *path, char *error,
                                       size_t error_capacity);
int civilization_recomp_sram_copy(const CivilizationRecomp *instance,
                                   void *destination, size_t capacity);
int civilization_recomp_sram_load(CivilizationRecomp *instance,
                                   const void *source, size_t size,
                                   char *error, size_t error_capacity);
int civilization_recomp_sram_dirty(const CivilizationRecomp *instance);
void civilization_recomp_sram_mark_clean(CivilizationRecomp *instance);
int civilization_recomp_write_diagnostic_log(
    const CivilizationRecomp *instance, const char *path,
    const char *screenshot_path, char *error, size_t error_capacity);

#ifdef __cplusplus
}
#endif

#endif
