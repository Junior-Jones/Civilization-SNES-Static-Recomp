#ifndef CIVILIZATION_DIAGNOSTICS_H
#define CIVILIZATION_DIAGNOSTICS_H

#include "civilization_static_recomp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CivV20AudioStatus {
    uint64_t synchronized_master_clock;
    uint64_t smp_cycles;
    uint64_t smp_instructions;
    uint64_t aot_validated_instructions;
    uint64_t pcm_frames;
    uint64_t pcm_known_frames;
    uint64_t pcm_unknown_frames;
    uint64_t pcm_overflows;
    size_t pcm_available;
    uint64_t pcm_fnv1a64;
    uint64_t nonzero_pcm_frames;
    uint64_t first_nonzero_pcm_frame;
    uint32_t capture_frames;
    uint64_t capture_fnv1a64;
    uint16_t smp_pc;
    uint16_t aot_fail_pc;
    uint8_t aot_failed;
    uint8_t expected_opcode;
    uint8_t actual_opcode;
    uint32_t code_write_barriers;
    uint64_t sdsp_primitive_steps;
    uint64_t sdsp_brr_steps;
    uint8_t dsp_phase;
    uint8_t timer_enable_mask;
    uint32_t dsp_write_count;
    uint64_t dsp_write_fnv1a64;
    uint32_t aram_write_count;
    uint64_t aram_write_fnv1a64;
    uint32_t smp_port_event_count;
    uint64_t smp_port_event_fnv1a64;
} CivV20AudioStatus;

typedef struct CivVideoCheckpoint {
    uint64_t frame_count;
    uint64_t master_clock;
    uint8_t forced_blank;
    uint8_t brightness;
    uint8_t bg_mode;
    uint8_t main_screen_layers;
    uint8_t sub_screen_layers;
    uint64_t vram_fnv1a64;
    uint64_t cgram_fnv1a64;
    uint64_t oam_fnv1a64;
    uint64_t dma_run_count;
    uint64_t dma_transfer_byte_count;
    uint64_t vram_data_write_count;
    uint64_t cgram_data_write_count;
    uint64_t oam_data_write_count;
} CivVideoCheckpoint;

typedef struct CivDiagnosticCpuState {
    uint16_t a, x, y, s, d, pc;
    uint8_t pbr, dbr, p, e;
} CivDiagnosticCpuState;

typedef struct CivDiagnosticPpuState {
    uint8_t forced_blank, brightness, bg_mode, mode1_bg3_priority;
    uint8_t bg_large_tiles[4];
    uint16_t bg_tilemap_address[4];
    uint8_t bg_double_width[4], bg_double_height[4];
    uint16_t bg_chr_address[4], bg_hscroll[4], bg_vscroll[4];
    uint8_t main_screen_layers, sub_screen_layers;
    uint8_t window_mask_main, window_mask_sub, color_math_enabled;
    uint16_t fixed_color, oam_base_address;
    uint8_t oam_mode, oam_priority_rotation;
} CivDiagnosticPpuState;

typedef struct CivDiagnosticState {
    CivDiagnosticCpuState cpu;
    CivDiagnosticPpuState ppu;
    uint64_t instruction_count, frame_count, master_clock;
    uint16_t hcounter, vcounter;
    uint8_t field, failed, natural_timing_enabled;
    uint64_t cpu_code_access_count, cpu_data_access_count;
    uint64_t cpu_internal_cycle_count, dram_refresh_count;
    uint64_t dram_refresh_master_clocks;
    uint8_t ppu_regs[CIV_PPU_WRITE_REG_COUNT];
    uint16_t controller_live[2], controller_latched[2], auto_joypad_data[4];
    uint8_t controller_device[2], controller_shift_index[2];
    uint32_t controller_serial_read_count[2];
    uint32_t auto_joypad_serial_read_count[2];
    uint32_t auto_joypad_poll_count, mouse_bios_probe_count;
    uint8_t nmi_pending, nmi_depth, irq_line, irq_depth;
    uint32_t nmi_accept_count, nmi_return_count;
    uint32_t irq_accept_count, irq_return_count;
    uint8_t framebuffer_ready;
    uint64_t framebuffer_fnv1a64;
    uint64_t wram_fnv1a64, sram_fnv1a64, guest_state_fnv1a64;
    char frontier_address[16], frontier_reason[192];
} CivDiagnosticState;

typedef struct CivTerrainRenderEntry {
    uint64_t frame;
    CivDiagnosticCpuState cpu;
    uint16_t map_x, map_y, ring_x, ring_y, tile_number, world_index;
} CivTerrainRenderEntry;

int civ_diagnostics_capture(const CivRecomp *instance, CivDiagnosticState *state);
int civ_diagnostics_read_wram(const CivRecomp *instance, size_t offset,
                              void *destination, size_t size);
int civ_diagnostics_read_oam(const CivRecomp *instance, size_t offset,
                             void *destination, size_t size);
/* Temp-only widescreen/fog discovery helper. Never used by the production UI. */
int civ_diagnostics_write_wram(CivRecomp *instance, size_t offset,
                               const void *source, size_t size);
size_t civ_widescreen_probe_terrain_entries(CivTerrainRenderEntry *out,
                                            size_t capacity);
int civ_widescreen_probe_generate_terrain_tile(const CivRecomp *instance,
                                                unsigned world_x,
                                                unsigned world_y,
                                                uint8_t graphics[128],
                                                uint16_t *tile_attributes,
                                                unsigned *steps,
                                                char *error,
                                                size_t error_capacity);
void civ_capture_video_checkpoint(const CivRecomp *instance,
                                  CivVideoCheckpoint *checkpoint);
int civ_v20_get_audio_status(const CivRecomp *instance,CivV20AudioStatus *status);
int civ_v20_write_wav(const CivRecomp *instance,const char *path);
int civ_v20_copy_pcm_capture(const CivRecomp *instance,int16_t *interleaved_stereo,
                             size_t frame_capacity,size_t *frames_written);
int civ_v20_audio_peek_dsp(const CivRecomp *instance,uint8_t address,uint8_t *value);
int civ_v20_audio_peek_aram(const CivRecomp *instance,uint16_t address,uint8_t *value);
size_t civ_v20_audio_state_size(void);
int civ_v20_audio_state_save(const CivRecomp *instance,void *data,size_t capacity);
int civ_v20_audio_state_load(CivRecomp *instance,const void *data,size_t size);

#ifdef __cplusplus
}
#endif
#endif
