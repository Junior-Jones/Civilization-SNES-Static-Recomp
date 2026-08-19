#ifndef CIVILIZATION_FRONTEND_H
#define CIVILIZATION_FRONTEND_H
#include "civilization_static_recomp.h"
#include <stddef.h>
#include <stdint.h>
#define CIV_FRONTEND_SNAPSHOT_SLOTS 5u
#define CIV_FRONTEND_PATH_CAPACITY 4096u
#define CIV_FRONTEND_ERROR_CAPACITY 192u
typedef struct CivFrontend {
    CivRecomp *core;
    const uint8_t *rom;
    size_t rom_size;
    int loaded;
    int paused;
    uint16_t controller1;
    char state_directory[CIV_FRONTEND_PATH_CAPACITY];
    char last_error[CIV_FRONTEND_ERROR_CAPACITY];
    uint64_t last_sram_flush_frame;
} CivFrontend;
void civ_frontend_init_empty(CivFrontend *f);
void civ_frontend_shutdown(CivFrontend *f);
int civ_frontend_load_rom(CivFrontend *f,const uint8_t *rom,size_t size,char *error,size_t error_cap);
int civ_frontend_reset(CivFrontend *f,char *error,size_t error_cap);
int civ_frontend_set_state_directory(CivFrontend *f,const char *directory,char *error,size_t error_cap);
const char *civ_frontend_state_directory(const CivFrontend *f);
const char *civ_frontend_last_error(const CivFrontend *f);
int civ_frontend_load_persistent_sram(CivFrontend *f);
int civ_frontend_flush_persistent_sram(CivFrontend *f,int force);
int civ_frontend_sram_path(const CivFrontend *f,char *path,size_t capacity);
void civ_frontend_pause(CivFrontend *f);
void civ_frontend_play(CivFrontend *f);
void civ_frontend_set_controller1(CivFrontend *f,uint16_t mask);
int civ_frontend_run(CivFrontend *f,uint64_t instructions);
int civ_frontend_run_to_frame(CivFrontend *f,uint64_t target_frame,uint64_t instruction_budget);
/* Persistent slots are intentionally 1-based, matching the SimCity launcher. */
int civ_frontend_snapshot_save(CivFrontend *f,unsigned slot);
int civ_frontend_snapshot_load(CivFrontend *f,unsigned slot);
int civ_frontend_snapshot_exists(const CivFrontend *f,unsigned slot);
int civ_frontend_snapshot_path(const CivFrontend *f,unsigned slot,char *path,size_t capacity);
int civ_frontend_frame_counter_ready(const CivFrontend *f);
int civ_frontend_frame_presenter_ready(const CivFrontend *f);
int civ_frontend_audio_pcm_ready(const CivFrontend *f);
#endif
