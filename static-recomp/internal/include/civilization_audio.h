#ifndef CIVILIZATION_AUDIO_V20_H
#define CIVILIZATION_AUDIO_V20_H
#include <stddef.h>
#include <stdint.h>
#include "civilization_internal.h"
#ifdef __cplusplus
extern "C" {
#endif
int civ_v20_audio_begin(CivRecomp *instance);
int civ_v20_audio_sync_internal(CivRecomp *instance);
int civ_v20_audio_cpu_write(CivRecomp *instance,unsigned port,uint8_t value);
int civ_v20_audio_cpu_read(CivRecomp *instance,unsigned port,uint8_t *value);
void civ_v20_audio_release_internal(CivRecomp *instance);
size_t civ_v20_audio_state_size_internal(void);
int civ_v20_audio_state_save_internal(const CivRecomp *instance,void *data,size_t capacity);
int civ_v20_audio_state_load_internal(CivRecomp *instance,const void *data,size_t size);
size_t civ_v20_audio_available_internal(const CivRecomp *instance);
size_t civ_v20_audio_read_internal(CivRecomp *instance,int16_t *interleaved_stereo,
                                   uint8_t *frame_known,size_t capacity_frames);
uint64_t civ_v20_audio_overflow_internal(const CivRecomp *instance);
uint8_t civ_v20_audio_peek_aram_internal(uint16_t address);
uint8_t civ_v20_audio_peek_dsp_internal(uint8_t address);
#ifdef __cplusplus
}
#endif
#endif
