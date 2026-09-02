#ifndef CIVILIZATION_STATIC_RECOMP_H
#define CIVILIZATION_STATIC_RECOMP_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_ROM_SIZE 1572864u
#define CIV_WRAM_SIZE 131072u
#define CIV_SRAM_SIZE 32768u
#define CIV_PPU_WRITE_REG_COUNT 0x34u
#define CIV_CPU_IO_WRITE_REG_COUNT 0x0Eu
#define CIV_NTSC_SCANLINES 262u
#define CIV_FRAME_WIDTH 256u
#define CIV_FRAME_HEIGHT 224u
#define CIV_FRAME_PIXELS (CIV_FRAME_WIDTH*CIV_FRAME_HEIGHT)
#define CIV_VRAM_SIZE 65536u
#define CIV_CGRAM_SIZE 512u
#define CIV_OAM_SIZE 544u
#define CIV_INPUT_DEVICE_JOYPAD 0u
#define CIV_INPUT_DEVICE_MOUSE 1u
#define CIV_MOUSE_BUTTON_LEFT 0x01u
#define CIV_MOUSE_BUTTON_RIGHT 0x02u

/* Public callers own only this handle. The machine layout is private to the
   static core and its explicitly opted-in diagnostic/build interfaces. */
typedef struct CivRecomp CivRecomp;

typedef void (*CivHostPcmSink)(void *context,int16_t left,int16_t right);
typedef void (*CivHostFrameSink)(void *context,uint64_t frame_number,
                                 const uint32_t *rgba_pixels);
typedef void (*CivHostFailureSink)(void *context,const char *address,
                                   const char *reason);
typedef void (*CivHostDiagnosticSink)(void *context,const char *event);

typedef struct CivHostHooks {
    void *context;
    CivHostFrameSink frame_complete;
    CivHostFailureSink failure;
    CivHostDiagnosticSink diagnostic;
    CivHostPcmSink pcm;
} CivHostHooks;

typedef struct CivFrameResult {
    uint64_t start_frame;
    uint64_t end_frame;
    uint64_t instructions_executed;
    uint8_t frame_completed;
    uint8_t frame_rendered;
} CivFrameResult;

#ifdef CIVILIZATION_CORE_INTERNAL
typedef struct CivCpuState {
    uint16_t a, x, y, s, d, pc;
    uint8_t pbr, dbr, p, e;
} CivCpuState;


typedef struct CivDmaChannel0 {
    uint8_t dmap;
    uint8_t bbad;
    uint16_t source_address;
    uint8_t source_bank;
    uint16_t transfer_size;
    /* $43x7 is the HDMA indirect bank.  Version 33 exposes it only for the
       source-proved Civilization channel-2 HDMA route; manual-DMA channels
       continue to fail closed if their unproved indirect register is used. */
    uint8_t indirect_bank;
} CivDmaChannel0;

typedef struct CivHdmaChannel2State {
    uint16_t table_address;          /* internal $4328/$4329 table cursor */
    uint8_t line_counter_repeat;     /* internal $432A line/repeat state */
    uint8_t do_transfer;
    uint8_t finished;
    uint8_t initialized;
} CivHdmaChannel2State;


typedef struct CivPpuDecodedState {
    /* Version 15 non-audio reached-register decode.  These fields model register
       configuration state only; no visible frame is claimed while forced blank remains set. */
    uint8_t forced_blank;
    uint8_t brightness;
    uint8_t oam_mode;
    uint16_t oam_base_address;
    uint16_t oam_address_offset;
    uint16_t oam_ram_address;
    uint16_t oam_internal_address;
    uint8_t oam_priority_rotation;
    uint8_t bg_mode;
    uint8_t mode1_bg3_priority;
    uint8_t bg_large_tiles[4];
    uint8_t mosaic_size;
    uint8_t mosaic_enabled_mask;
    uint16_t bg_tilemap_address[4];
    uint8_t bg_double_width[4];
    uint8_t bg_double_height[4];
    uint16_t bg_chr_address[4];
    uint16_t bg_hscroll[4];
    uint16_t bg_vscroll[4];
    uint8_t hv_scroll_latch;
    uint8_t h_scroll_latch;
    uint8_t mode7_value_latch;
    uint8_t mode7_select;
    uint16_t mode7_hscroll;
    uint16_t mode7_vscroll;
    uint16_t mode7_matrix[4];
    /* PPU1 open-bus latch.  Version 33 source proof reaches only the Mode 7
       multiply result ports $2134-$2136 on the PPU read side. */
    uint8_t ppu1_open_bus;
    uint16_t mode7_center_x;
    uint16_t mode7_center_y;
    uint8_t window_active[2][6];
    uint8_t window_inverted[2][6];
    uint8_t window_left[2];
    uint8_t window_right[2];
    uint8_t mask_logic[6];
    uint8_t main_screen_layers;
    uint8_t sub_screen_layers;
    uint8_t window_mask_main;
    uint8_t window_mask_sub;
    uint8_t color_math_clip_mode;
    uint8_t color_math_prevent_mode;
    uint8_t color_math_add_subscreen;
    uint8_t direct_color_mode;
    uint8_t color_math_enabled;
    uint8_t color_math_subtract;
    uint8_t color_math_halve;
    uint16_t fixed_color;
    uint8_t extbg_enabled;
    uint8_t pseudo_hires;
    uint8_t overscan;
    uint8_t obj_interlace;
    uint8_t screen_interlace;
} CivPpuDecodedState;

struct CivRecomp {
    CivCpuState cpu;
    uint8_t wram[CIV_WRAM_SIZE];
    uint8_t sram[CIV_SRAM_SIZE];
    /* Battery-backed SRAM is host-persisted separately from snapshots. */
    uint8_t sram_dirty;
    uint8_t reset_signature[8];

    /* Exact external ROM is attached after reset; it is never embedded. */
    const uint8_t *rom;
    size_t rom_size;

    /* Boot-visible PPU/CPU-I/O write state. */
    uint8_t ppu_regs[CIV_PPU_WRITE_REG_COUNT];       /* $2100-$2133 */
    uint16_t ppu_write_count[CIV_PPU_WRITE_REG_COUNT];

    /* Version 15 decoded state for already-reached PPU configuration writes.
       OAM data ($2104) remains fail-closed because the natural route has not reached it. */
    CivPpuDecodedState ppu;
    uint8_t cpu_io[CIV_CPU_IO_WRITE_REG_COUNT];      /* $4200-$420D */
    uint16_t cpu_io_write_count[CIV_CPU_IO_WRITE_REG_COUNT];

    /* Reached S-CPU multiplication/division registers.  The values are
       functional at an instruction boundary; result latency is scheduled in
       the shared NTSC clock domain separately. */
    uint8_t cpu_math_mul_a;
    uint8_t cpu_math_mul_b;
    uint16_t cpu_math_dividend;
    uint8_t cpu_math_divisor;
    uint16_t cpu_math_quotient;
    uint16_t cpu_math_remainder;
    uint32_t cpu_math_operation_count;

    /* Version 04 reached PPU memory ports and channel-0 DMA state. */
    uint8_t vram[CIV_VRAM_SIZE];
    uint8_t cgram[CIV_CGRAM_SIZE];
    uint16_t vram_address;
    uint16_t vram_increment_size;
    uint8_t vram_mapping;
    uint8_t vram_increment_on_high;
    uint8_t cgram_address;
    uint8_t cgram_latch_low;
    uint8_t cgram_latch_phase;
    CivDmaChannel0 dma0;
    CivDmaChannel0 dma1;
    CivDmaChannel0 dma2; /* Version 33 source-proved HDMA channel 2. */
    CivDmaChannel0 dma7;
    CivHdmaChannel2State hdma2;
    /* Version 21 exact manual-DMA scheduling.  A write to MDMAEN ($420B)
       arms DMA; the transfer starts only after the hardware one-CPU-cycle
       start delay, then uses master-clock alignment/overhead before the
       selected channel runs. */
    uint8_t dma_pending_mask;
    uint8_t dma_start_delay;
    /* Version 33 target-specific HDMA scheduling.  Exact-ROM producer proof
       restricts HDMAEN to {0,$04}; only channel 2 can be active. */
    uint8_t hdma_init_pending;
    uint8_t hdma_transfer_pending;
    uint16_t hdma_init_hcounter;
    uint64_t hdma_init_count;
    uint64_t hdma_scanline_count;
    uint64_t hdma_table_read_count;
    uint64_t hdma_transfer_byte_count;
    uint64_t dma_run_count;
    uint64_t dma_channel_run_count[8];
    uint64_t dma_channel_transfer_byte_count[8];
    uint64_t dma_transfer_byte_count;
    uint64_t vram_data_write_count;
    uint64_t cgram_data_write_count;
    uint64_t ppu_multiply_result_read_count[3]; /* $2134-$2136 */
    /* Version 16 reached OAMDATA ($2104) authority.  OAM is the 512-byte low table
       plus 32-byte high table; low-table writes commit in hardware-style pairs. */
    uint8_t oam[CIV_OAM_SIZE];
    uint8_t oam_write_buffer;
    uint64_t oam_data_write_count;

    /* Version 03 functional CPU timer/IRQ model.  The beam is in SNES master clocks.
       Exact reset-to-beam phase and interrupt pipeline latency remain separate proof gates. */
    uint64_t master_clock;
    uint16_t hcounter;
    uint16_t vcounter;
    uint8_t field;
    /* Version 20 natural NTSC clock domain.  Every S-CPU code, data, and
       internal cycle advances the same master clock used by PPU and audio. */
    uint8_t natural_timing_enabled;
    uint8_t cpu_bus_timing_suppressed;
    uint8_t dram_refresh_done;
    uint8_t dram_refresh_active;
    uint16_t dram_refresh_hcounter;
    uint64_t dram_refresh_count;
    uint64_t dram_refresh_master_clocks;
    uint64_t cpu_code_access_count;
    uint64_t cpu_data_access_count;
    uint64_t cpu_internal_cycle_count;
    /* Headless cooperative stop hook.  It yields only after a completed
       already-generated instruction and never changes guest state. */
    uint8_t headless_frame_stop_enabled;
    uint8_t headless_frame_stop_reached;
    uint8_t static_cpu_phase; /* 0=bootstrap, 1=post-audio bootstrap, 2=pre-continuation, 3=closed-graph continuation. */
    uint64_t headless_frame_stop_target;
    CivHostHooks host_hooks; /* Host-only; excluded from snapshots. */
    uint16_t htime_raw;
    uint16_t vtime_raw;
    uint8_t auto_joypad_enable;
    uint8_t hirq_enable;
    uint8_t virq_enable;
    uint8_t nmi_enable;
    /* Version 21 models the S-CPU NMI edge detector separately from the
       CPU-visible $4210 flag.  nmi_flag_counter is loaded by the H=6 NMI
       line transition and sampled on the following CPU cycle; nmi_pending
       is the sampled NeedNmi latch consumed after the current instruction. */
    uint8_t nmi_flag;
    uint8_t nmi_flag_counter;
    uint8_t nmi_pending;
    uint8_t nmi_depth;
    uint32_t nmi_accept_count;
    uint32_t nmi_return_count;
    uint16_t last_nmi_vector;
    uint8_t irq_line;
    uint8_t irq_transition;
    uint8_t irq_hold_master_clocks;
    uint32_t irq_event_count;
    uint32_t timeup_read_count;

    /* Version 05 functional native-IRQ/RTI and reached HVBJOY polling evidence.
       Exact interrupt-pipeline and DMA arbitration phase remain uncertified. */
    uint32_t hvbjoy_read_count;
    /* Version 16 reached manual SNES controller serial I/O.  Host masks use
       serial bit order B,Y,Select,Start,Up,Down,Left,Right,A,X,L,R,0,0,0,0. */
    uint16_t controller_live[2];
    uint16_t controller_latched[2];
    uint8_t controller_shift_index[2];
    uint8_t controller_strobe;
    uint32_t controller_serial_read_count[2];
    uint32_t controller_strobe_write_count;

    /* Version 18 target-specific SNES Mouse/auto-joypad support.  This models
       controller-port protocol only; it does not promote the NMI Mouse BIOS
       into the continuous cold-boot authority until that route is proved. */
    uint8_t controller_device[2];
    uint8_t mouse_sensitivity[2];
    uint8_t mouse_buttons[2];
    int16_t mouse_dx[2];
    int16_t mouse_dy[2];
    uint8_t mouse_left_flag[2];
    uint8_t mouse_up_flag[2];
    uint32_t mouse_shift_packet[2];
    uint16_t auto_joypad_data[4];
    uint8_t auto_joypad_busy;
    uint32_t auto_joypad_poll_count;
    uint32_t auto_joypad_serial_read_count[2];
    uint32_t v18_mouse_bios_probe_count;

    uint32_t irq_accept_count;
    uint32_t irq_return_count;
    uint8_t irq_depth;
    uint16_t last_irq_vector;
    uint32_t stage_dispatch_count;
    uint16_t last_stage_index;
    uint32_t descriptor_handler_8746_count;
    uint32_t descriptor_handler_8780_count;
    uint32_t descriptor_handler_8598_count;

    /* Retained generated-core diagnostic: exact main-thread WRAM clear bytes. */
    uint32_t v16_wram_clear_bytes;

    /* The sole production audio lane. The S-SMP starts
       from the hardware IPL ROM and is admitted only through generated exact
       PC/opcode authority; the S-DSP emits target-native PCM. */
    uint8_t v20_full_static_audio_enabled;
    uint8_t v20_full_static_audio_acquired;
    uint8_t v20_full_static_audio_failed;
    uint8_t v20_full_static_audio_authoritative;
    uint64_t v20_static_smp_instructions;
    uint64_t v20_static_smp_aot_instructions;
    uint16_t v20_static_smp_pc;
    uint8_t v20_static_aot_failed;
    uint16_t v20_static_aot_fail_pc;
    uint8_t v20_static_aot_expected_opcode;
    uint8_t v20_static_aot_actual_opcode;
    uint32_t v20_static_code_write_barriers;
    uint64_t v20_sdsp_primitive_steps;
    uint64_t v20_sdsp_brr_steps;
    uint64_t v20_pcm_frame_count;
    uint64_t v20_pcm_fnv1a64;
    uint64_t v20_nonzero_pcm_frame_count;
    uint64_t v20_first_nonzero_pcm_frame;
    uint32_t v20_pcm_capture_frame_count;
    uint64_t v20_pcm_capture_fnv1a64;
    int16_t v20_pcm_capture[8192u * 2u];
    uint8_t v20_pcm_capture_started;
    uint32_t v20_dsp_write_count;
    uint64_t v20_dsp_write_fnv1a64;
    uint32_t v20_aram_write_count;
    uint64_t v20_aram_write_fnv1a64;
    uint32_t v20_smp_port_event_count;
    uint64_t v20_smp_port_event_fnv1a64;

    /* Version 19 visible-frame certification.  The first frozen frame is the
       first complete NTSC frame that begins after the authentic forced-blank
       clear.  Per-scanline arrays are PPU write-latch snapshots at each line
       start; no HDMA/cycle-exact claim is implied when no reached HDMA path has
       been proved. */
    uint8_t v19_forced_blank_clear_observed;
    uint8_t v19_forced_blank_clear_value;
    uint8_t v19_visible_capture_armed;
    uint8_t v19_visible_capture_started;
    uint8_t v19_first_visible_frame_captured;
    uint64_t v19_forced_blank_clear_frame;
    uint64_t v19_forced_blank_clear_master_clock;
    uint16_t v19_forced_blank_clear_hcounter;
    uint16_t v19_forced_blank_clear_vcounter;
    uint64_t v19_visible_capture_frame_start;
    uint64_t v19_first_visible_frame_number;
    uint64_t v19_first_visible_master_clock; /* frame-start master clock */
    uint64_t v19_first_visible_frame_end_master_clock;
    /* Active presentation state, refreshed at the start of every displayed
       scanline.  Decoded state is retained because scroll registers are
       write-latched streams and cannot be reconstructed from final bytes. */
    CivPpuDecodedState scanline_ppu[CIV_FRAME_HEIGHT];
    uint8_t scanline_ppu_regs[CIV_FRAME_HEIGHT][CIV_PPU_WRITE_REG_COUNT];
    uint8_t scanline_state_valid[CIV_FRAME_HEIGHT];
    uint64_t v19_first_visible_scanline_fnv1a64;
    uint64_t v19_first_visible_vram_fnv1a64;
    uint64_t v19_first_visible_cgram_fnv1a64;
    uint64_t v19_first_visible_oam_fnv1a64;
    uint64_t v19_first_visible_dma_run_count;
    uint64_t v19_first_visible_dma_transfer_byte_count;
    uint64_t v19_first_visible_vram_data_write_count;
    uint64_t v19_first_visible_cgram_data_write_count;
    uint64_t v19_first_visible_oam_data_write_count;

    /* Version 19 first authentic Mode-1 presentation surface.  The renderer is
       target-native core code; host frontends only consume this RGBA buffer. */
    uint32_t v19_framebuffer_rgba[CIV_FRAME_PIXELS];
    uint64_t v19_framebuffer_fnv1a64;
    uint8_t v19_framebuffer_ready;

    /* Version 09 deterministic functional frame boundary.  This counts NTSC
       H/V counter wraps only; it is not by itself a rendered-frame claim. */
    uint64_t frame_count;

    /* Writes to mapped cartridge ROM are electrically ignored, but are retained
       as diagnostics because Civilization performs this during boot. */
    uint32_t ignored_rom_write_count;
    uint32_t last_ignored_rom_write_address;
    uint8_t last_ignored_rom_write_value;

    /* Compatibility aliases kept for existing diagnostics. */
    uint8_t reg_4200;
    uint8_t reg_420d;
    uint8_t reg_2100;

    uint64_t instruction_count;
    uint64_t block_move_byte_count;
    int failed;
    char frontier_address[16];
    char frontier_reason[192];
};
#endif /* CIVILIZATION_CORE_INTERNAL */


typedef struct CivRomInfo {
    size_t file_bytes;
    uint32_t crc32;
    char sha256[65];
    char title[22];
    uint8_t map_mode;
    uint8_t cartridge_type;
    uint8_t rom_size_code;
    uint8_t sram_size_code;
    uint8_t region_code;
    uint16_t checksum_complement;
    uint16_t checksum;
    uint16_t reset_vector;
} CivRomInfo;

int civ_verify_rom(const uint8_t *rom, size_t size, CivRomInfo *info, char *error, size_t error_cap);
/* Explicit lifecycle. civ_create() returns a reset, SRAM-zeroed core. */
CivRecomp *civ_create(char *error, size_t error_cap);
void civ_destroy(CivRecomp *instance);
int civ_attach_verified_rom(CivRecomp *instance, const uint8_t *rom, size_t size, char *error, size_t error_cap);
int civ_hirom_rom_offset(uint8_t bank, uint16_t address, size_t *offset);
int civ_hirom_sram_offset(uint8_t bank, uint16_t address, size_t *offset);
void civ_reset(CivRecomp *instance);
uint64_t civ_frame_count(const CivRecomp *instance);
uint64_t civ_instruction_count(const CivRecomp *instance);
uint64_t civ_master_clock(const CivRecomp *instance);
int civ_has_failed(const CivRecomp *instance);
int civ_audio_active(const CivRecomp *instance);
uint64_t civ_audio_pcm_frames(const CivRecomp *instance);
size_t civ_audio_available(const CivRecomp *instance);
size_t civ_audio_read(CivRecomp *instance,int16_t *interleaved_stereo,
                      uint8_t *frame_known,size_t capacity_frames);
uint64_t civ_audio_overflow_count(const CivRecomp *instance);
int civ_run_to_frame(CivRecomp *instance, uint64_t target_frame,
                     uint64_t instruction_budget);
void civ_set_host_hooks(CivRecomp *instance,const CivHostHooks *hooks);
int civ_run_frame(CivRecomp *instance,uint16_t controller1,
                  uint64_t instruction_budget,int render,
                  CivFrameResult *result);
size_t civ_sram_size(void);
int civ_sram_copy(const CivRecomp *instance, void *destination, size_t capacity);
int civ_sram_load(CivRecomp *instance, const void *source, size_t size, char *error, size_t error_cap);
int civ_sram_dirty(const CivRecomp *instance);
void civ_sram_mark_clean(CivRecomp *instance);
int civ_snapshot_save(const CivRecomp *instance, const char *path, char *error, size_t error_cap);
int civ_snapshot_load(CivRecomp *instance, const uint8_t *rom, size_t rom_size, const char *path, char *error, size_t error_cap);
int civ_run_static(CivRecomp *instance, uint64_t max_instructions);
/* Compatibility spelling retained for existing frontends; this is the current
   Version 22 static runner, not a selectable Version 20 runtime. */
int civ_run_v20(CivRecomp *instance, uint64_t max_instructions);
int civ_v20_audio_sync(CivRecomp *instance);
void civ_v20_set_host_pcm_sink(CivRecomp *instance,CivHostPcmSink sink,void *context);
int civ_render_current_frame(CivRecomp *instance);
/* Source-compatibility spelling for Version 20-era frontends. */
int civ_v20_render_current_frame(CivRecomp *instance);
const uint32_t *civ_get_framebuffer_rgba(const CivRecomp *instance);
void civ_set_controller_input(CivRecomp *instance,unsigned controller,uint16_t serial_mask);
void civ_set_mouse_input(CivRecomp *instance,unsigned controller,int16_t dx,int16_t dy,uint8_t buttons);
int civ_auto_joypad_poll(CivRecomp *instance);
void civ_timing_advance_master(CivRecomp *instance, uint32_t master_clocks);
const char *civ_frontier_reason(const CivRecomp *instance);
const char *civ_frontier_address(const CivRecomp *instance);

#ifdef __cplusplus
}
#endif
#endif
