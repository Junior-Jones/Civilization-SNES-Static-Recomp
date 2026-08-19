#include "civilization_app_core.h"

#include "civilization_diagnostics.h"
#include "civilization_frontend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APP_PCM_CAPACITY_FRAMES 262144u

struct CivilizationRecomp {
    CivFrontend frontend;
    uint8_t *rom;
    int16_t *pcm;
    size_t pcm_read;
    size_t pcm_count;
    int audio_overflow;
    char error[192];
};

static void copy_error(CivilizationRecomp *instance, char *error,
                       size_t error_capacity, const char *text)
{
    if (!text) text = "";
    if (instance) (void)snprintf(instance->error, sizeof(instance->error),
                                 "%s", text);
    if (error && error_capacity)
        (void)snprintf(error, error_capacity, "%s", text);
}

static uint16_t snes_mask_to_serial(uint16_t snes_mask)
{
    uint16_t serial = 0u;
    unsigned bit;
    for (bit = 0u; bit < 16u; ++bit)
        serial |= (uint16_t)(((snes_mask >> bit) & 1u) << (15u - bit));
    return serial;
}

static void pcm_sink(void *context, int16_t left, int16_t right)
{
    CivilizationRecomp *instance = (CivilizationRecomp *)context;
    size_t write;
    if (!instance || !instance->pcm) return;
    if (instance->pcm_count == APP_PCM_CAPACITY_FRAMES) {
        instance->pcm_read =
            (instance->pcm_read + 1u) % APP_PCM_CAPACITY_FRAMES;
        instance->pcm_count--;
        instance->audio_overflow = 1;
    }
    write = (instance->pcm_read + instance->pcm_count) %
            APP_PCM_CAPACITY_FRAMES;
    instance->pcm[write * 2u] = left;
    instance->pcm[write * 2u + 1u] = right;
    instance->pcm_count++;
}

static void install_host_hooks(CivilizationRecomp *instance)
{
    CivHostHooks hooks;
    memset(&hooks,0,sizeof(hooks));
    hooks.context=instance;
    hooks.pcm=pcm_sink;
    civ_set_host_hooks(instance->frontend.core,&hooks);
}

static uint64_t frame_budget(uint32_t frames)
{
    uint64_t proportional = (uint64_t)frames * UINT64_C(20000);
    return proportional > UINT64_C(2000000) ? proportional :
           UINT64_C(2000000);
}

static int advance_frames(CivilizationRecomp *instance, uint16_t input_mask,
                          uint32_t frame_count,
                          CivilizationRecompFrameResult *result,
                          int render)
{
    uint32_t completed;
    CivFrameResult frame;
    int ok;
    if (!instance || !result) return 0;
    memset(result, 0, sizeof(*result));
    memset(&frame, 0, sizeof(frame));
    result->input_mask = input_mask;
    result->start_frame = (uint32_t)civ_frame_count(instance->frontend.core);
    instance->frontend.paused = 0;
    instance->frontend.controller1 = snes_mask_to_serial(input_mask);
    ok=1;
    for(completed=0u;completed<frame_count;++completed){
        if(!civ_run_frame(instance->frontend.core,instance->frontend.controller1,
                          frame_budget(1u),render,&frame)){ok=0;break;}
    }
    result->route_continued = (uint8_t)(ok != 0);
    result->end_frame = (uint32_t)civ_frame_count(instance->frontend.core);
    if (!ok) {
        const char *reason = civ_frontier_reason(instance->frontend.core);
        copy_error(instance, NULL, 0u,
                   reason && *reason ? reason : "Static route did not reach the requested frame.");
        return 0;
    }
    if (render) {
        result->frame_rendered = frame_count==0u?0u:frame.frame_rendered;
        if (!result->frame_rendered) {
            (void)snprintf(result->renderer_error,
                           sizeof(result->renderer_error),
                           "The current Civilization frame could not be rendered.");
        }
    }
    copy_error(instance, NULL, 0u, "");
    return 1;
}

int civilization_recomp_create(CivilizationRecomp **out_instance,
                                const uint8_t *rom, size_t rom_size,
                                char *error, size_t error_capacity)
{
    CivilizationRecomp *instance;
    if (!out_instance || !rom || rom_size != CIVILIZATION_RECOMP_ROM_SIZE) {
        if (error && error_capacity)
            (void)snprintf(error, error_capacity,
                           "The exact 1,572,864-byte Civilization (USA) ROM is required.");
        return 0;
    }
    *out_instance = NULL;
    instance = (CivilizationRecomp *)calloc(1u, sizeof(*instance));
    if (!instance) return 0;
    instance->rom = (uint8_t *)malloc(rom_size);
    instance->pcm = (int16_t *)malloc(APP_PCM_CAPACITY_FRAMES * 2u *
                                      sizeof(*instance->pcm));
    if (!instance->rom || !instance->pcm) {
        civilization_recomp_destroy(instance);
        return 0;
    }
    memcpy(instance->rom, rom, rom_size);
    civ_frontend_init_empty(&instance->frontend);
    if (!civ_frontend_load_rom(&instance->frontend, instance->rom, rom_size,
                               error, error_capacity)) {
        civilization_recomp_destroy(instance);
        return 0;
    }
    install_host_hooks(instance);
    copy_error(instance, error, error_capacity, "");
    *out_instance = instance;
    return 1;
}

void civilization_recomp_destroy(CivilizationRecomp *instance)
{
    if (!instance) return;
    if (instance->frontend.loaded)
        civ_set_host_hooks(instance->frontend.core, NULL);
    civ_frontend_shutdown(&instance->frontend);
    free(instance->pcm);
    free(instance->rom);
    free(instance);
}

int civilization_recomp_reset(CivilizationRecomp *instance,
                               char *error, size_t error_capacity)
{
    if (!instance || !instance->frontend.loaded) return 0;
    civ_reset(instance->frontend.core);
    if (!civ_attach_verified_rom(instance->frontend.core, instance->rom,
                                 CIVILIZATION_RECOMP_ROM_SIZE, error,
                                 error_capacity)) return 0;
    instance->frontend.paused = 1;
    instance->frontend.controller1 = 0u;
    instance->pcm_read = 0u;
    instance->pcm_count = 0u;
    instance->audio_overflow = 0;
    install_host_hooks(instance);
    copy_error(instance, error, error_capacity, "");
    return 1;
}

int civilization_recomp_advance(CivilizationRecomp *instance,
                                 uint16_t input_mask, uint32_t frame_count,
                                 CivilizationRecompFrameResult *result)
{
    return advance_frames(instance, input_mask, frame_count, result, 1);
}

int civilization_recomp_advance_headless(CivilizationRecomp *instance,
                                          uint16_t input_mask,
                                          uint32_t frame_count,
                                          CivilizationRecompFrameResult *result)
{
    return advance_frames(instance, input_mask, frame_count, result, 0);
}

const uint32_t *civilization_recomp_frame_bgra(
    const CivilizationRecomp *instance)
{
    return instance ? civ_get_framebuffer_rgba(instance->frontend.core) : NULL;
}

uint32_t civilization_recomp_current_frame(const CivilizationRecomp *instance)
{
    return instance ? (uint32_t)civ_frame_count(instance->frontend.core) : 0u;
}

const char *civilization_recomp_last_error(const CivilizationRecomp *instance)
{
    return instance ? instance->error : "No Civilization instance.";
}

size_t civilization_recomp_audio_available(const CivilizationRecomp *instance)
{
    return instance ? instance->pcm_count : 0u;
}

size_t civilization_recomp_audio_read(CivilizationRecomp *instance,
                                       int16_t *interleaved_stereo,
                                       size_t frame_capacity)
{
    size_t frames;
    size_t index;
    if (!instance || !interleaved_stereo) return 0u;
    frames = frame_capacity < instance->pcm_count ? frame_capacity :
             instance->pcm_count;
    for (index = 0u; index < frames; ++index) {
        size_t read = (instance->pcm_read + index) % APP_PCM_CAPACITY_FRAMES;
        interleaved_stereo[index * 2u] = instance->pcm[read * 2u];
        interleaved_stereo[index * 2u + 1u] = instance->pcm[read * 2u + 1u];
    }
    instance->pcm_read = (instance->pcm_read + frames) %
                         APP_PCM_CAPACITY_FRAMES;
    instance->pcm_count -= frames;
    return frames;
}

size_t civilization_recomp_audio_discard(CivilizationRecomp *instance)
{
    size_t discarded;
    if (!instance) return 0u;
    discarded = instance->pcm_count;
    instance->pcm_read = 0u;
    instance->pcm_count = 0u;
    return discarded;
}

int civilization_recomp_audio_overflowed(
    const CivilizationRecomp *instance)
{
    return instance && instance->audio_overflow;
}

void civilization_recomp_audio_clear_overflow(CivilizationRecomp *instance)
{
    if (instance) instance->audio_overflow = 0;
}

int civilization_recomp_snapshot_save(const CivilizationRecomp *instance,
                                       const char *path, char *error,
                                       size_t error_capacity)
{
    if (!instance) return 0;
    return civ_snapshot_save(instance->frontend.core, path, error,
                             error_capacity);
}

int civilization_recomp_snapshot_load(CivilizationRecomp *instance,
                                       const char *path, char *error,
                                       size_t error_capacity)
{
    int ok;
    if (!instance) return 0;
    ok = civ_snapshot_load(instance->frontend.core, instance->rom,
                           CIVILIZATION_RECOMP_ROM_SIZE, path, error,
                           error_capacity);
    if (ok) {
        instance->pcm_read = 0u;
        instance->pcm_count = 0u;
        instance->audio_overflow = 0;
        install_host_hooks(instance);
        (void)civ_render_current_frame(instance->frontend.core);
    }
    return ok;
}

int civilization_recomp_sram_copy(const CivilizationRecomp *instance,
                                   void *destination, size_t capacity)
{
    return instance && civ_sram_copy(instance->frontend.core, destination,
                                     capacity);
}

int civilization_recomp_sram_load(CivilizationRecomp *instance,
                                   const void *source, size_t size,
                                   char *error, size_t error_capacity)
{
    return instance && civ_sram_load(instance->frontend.core, source, size,
                                     error, error_capacity);
}

int civilization_recomp_sram_dirty(const CivilizationRecomp *instance)
{
    return instance && civ_sram_dirty(instance->frontend.core);
}

void civilization_recomp_sram_mark_clean(CivilizationRecomp *instance)
{
    if (instance) civ_sram_mark_clean(instance->frontend.core);
}

int civilization_recomp_write_diagnostic_log(
    const CivilizationRecomp *instance, const char *path,
    const char *screenshot_path, char *error, size_t error_capacity)
{
    const CivRecomp *core;
    CivDiagnosticState diagnostic;
    CivVideoCheckpoint video;
    CivV20AudioStatus audio;
    FILE *file;
    uint32_t context_key;
    int audio_ok;
    if (!instance || !path || !path[0]) {
        if (error && error_capacity)
            (void)snprintf(error, error_capacity,
                           "A loaded static core and diagnostic path are required.");
        return 0;
    }
    core = instance->frontend.core;
    if (!civ_diagnostics_capture(core, &diagnostic)) {
        if (error && error_capacity)
            (void)snprintf(error, error_capacity,
                           "The static-core diagnostic state is unavailable.");
        return 0;
    }
    civ_capture_video_checkpoint(core, &video);
    memset(&audio, 0, sizeof(audio));
    audio_ok = civ_v20_get_audio_status(core, &audio);
    context_key = ((uint32_t)diagnostic.cpu.pbr << 16) | diagnostic.cpu.pc |
                  ((uint32_t)(diagnostic.cpu.e & 1u) << 24) |
                  ((uint32_t)((diagnostic.cpu.p & 0x20u) != 0u) << 25) |
                  ((uint32_t)((diagnostic.cpu.p & 0x10u) != 0u) << 26);
    file = fopen(path, "wb");
    if (!file) {
        if (error && error_capacity)
            (void)snprintf(error, error_capacity,
                           "The screenshot diagnostic log could not be opened.");
        return 0;
    }
    (void)fprintf(file,
        "Civilization Static Recomp 1.1.0 - Screenshot Static-Core Log\r\n"
        "format=civilization-v35-screenshot-static-log-v1\r\n"
        "screenshot=%s\r\n"
        "authority=closed-exact-ROM-static-PBR-PC-E-M-X\r\n"
        "authority_contexts=103584\r\n"
        "runtime_decoder=0\r\n"
        "runtime_learning=0\r\n"
        "runtime_fallback=0\r\n"
        "frame=%llu\r\n"
        "instructions=%llu\r\n"
        "master_clock=%llu\r\n"
        "beam_h=%u\r\nbeam_v=%u\r\nfield=%u\r\n"
        "cpu_context=%02X:%04X/E%uM%uX%u\r\n"
        "cpu_context_key=%08X\r\n"
        "cpu_a=%04X\r\ncpu_x=%04X\r\ncpu_y=%04X\r\n"
        "cpu_s=%04X\r\ncpu_d=%04X\r\ncpu_dbr=%02X\r\ncpu_p=%02X\r\n"
        "controller1_live=%04X\r\nautojoy1=%04X\r\n"
        "nmi_pending=%u\r\nnmi_depth=%u\r\nnmi_accepts=%u\r\nnmi_returns=%u\r\n"
        "irq_line=%u\r\nirq_depth=%u\r\nirq_accepts=%u\r\nirq_returns=%u\r\n"
        "ppu_forced_blank=%u\r\nppu_brightness=%u\r\nppu_bg_mode=%u\r\n"
        "ppu_main_layers=%02X\r\nppu_sub_layers=%02X\r\n"
        "framebuffer_ready=%u\r\nframebuffer_fnv1a64=%016llX\r\n"
        "wram_fnv1a64=%016llX\r\nsram_fnv1a64=%016llX\r\n"
        "vram_fnv1a64=%016llX\r\ncgram_fnv1a64=%016llX\r\noam_fnv1a64=%016llX\r\n"
        "guest_state_fnv1a64=%016llX\r\n"
        "dma_runs=%llu\r\ndma_bytes=%llu\r\n"
        "audio_status_available=%d\r\naudio_smp_pc=%04X\r\n"
        "audio_smp_instructions=%llu\r\naudio_aot_instructions=%llu\r\n"
        "audio_pcm_frames=%llu\r\naudio_pcm_fnv1a64=%016llX\r\n"
        "audio_aot_failed=%u\r\naudio_code_write_barriers=%u\r\n"
        "failed=%d\r\nfrontier_address=%s\r\nfrontier_reason=%s\r\n",
        screenshot_path ? screenshot_path : "",
        (unsigned long long)diagnostic.frame_count,
        (unsigned long long)diagnostic.instruction_count,
        (unsigned long long)diagnostic.master_clock,
        (unsigned)diagnostic.hcounter, (unsigned)diagnostic.vcounter,
        (unsigned)diagnostic.field, (unsigned)diagnostic.cpu.pbr,
        (unsigned)diagnostic.cpu.pc, (unsigned)diagnostic.cpu.e,
        (unsigned)((diagnostic.cpu.p & 0x20u) != 0u),
        (unsigned)((diagnostic.cpu.p & 0x10u) != 0u),
        (unsigned)context_key, (unsigned)diagnostic.cpu.a,
        (unsigned)diagnostic.cpu.x, (unsigned)diagnostic.cpu.y,
        (unsigned)diagnostic.cpu.s, (unsigned)diagnostic.cpu.d,
        (unsigned)diagnostic.cpu.dbr, (unsigned)diagnostic.cpu.p,
        (unsigned)diagnostic.controller_live[0],
        (unsigned)diagnostic.auto_joypad_data[0],
        (unsigned)diagnostic.nmi_pending, (unsigned)diagnostic.nmi_depth,
        (unsigned)diagnostic.nmi_accept_count, (unsigned)diagnostic.nmi_return_count,
        (unsigned)diagnostic.irq_line, (unsigned)diagnostic.irq_depth,
        (unsigned)diagnostic.irq_accept_count, (unsigned)diagnostic.irq_return_count,
        (unsigned)video.forced_blank, (unsigned)video.brightness,
        (unsigned)video.bg_mode, (unsigned)video.main_screen_layers,
        (unsigned)video.sub_screen_layers,
        (unsigned)diagnostic.framebuffer_ready,
        (unsigned long long)diagnostic.framebuffer_fnv1a64,
        (unsigned long long)diagnostic.wram_fnv1a64, (unsigned long long)diagnostic.sram_fnv1a64,
        (unsigned long long)video.vram_fnv1a64,
        (unsigned long long)video.cgram_fnv1a64,
        (unsigned long long)video.oam_fnv1a64,
        (unsigned long long)diagnostic.guest_state_fnv1a64,
        (unsigned long long)video.dma_run_count,
        (unsigned long long)video.dma_transfer_byte_count,
        audio_ok, (unsigned)audio.smp_pc,
        (unsigned long long)audio.smp_instructions,
        (unsigned long long)audio.aot_validated_instructions,
        (unsigned long long)audio.pcm_frames,
        (unsigned long long)audio.pcm_fnv1a64,
        (unsigned)audio.aot_failed, (unsigned)audio.code_write_barriers,
        diagnostic.failed, diagnostic.frontier_address, diagnostic.frontier_reason);
    if (fclose(file) != 0) {
        if (error && error_capacity)
            (void)snprintf(error, error_capacity,
                           "The screenshot diagnostic log could not be finalized.");
        return 0;
    }
    if (error && error_capacity) error[0] = '\0';
    return 1;
}
