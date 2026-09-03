#include "civilization_internal.h"
#include "civilization_audio.h"
#include "civilization_diagnostics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *EXPECTED_SHA = "de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32";

static uint16_t read_le16(const uint8_t *p) { return (uint16_t)(p[0] | ((uint16_t)p[1] << 8)); }

static void copy_text(char *dst, size_t cap, const char *src) {
    if (!dst || cap == 0u) return;
    if (!src) src = "";
    (void)snprintf(dst, cap, "%s", src);
}

int civ_verify_rom(const uint8_t *rom, size_t size, CivRomInfo *info, char *error, size_t error_cap) {
    char sha[65];
    const uint8_t *header;
    if (!rom) { copy_text(error,error_cap,"ROM buffer is null."); return 0; }
    if (size != CIV_ROM_SIZE) { copy_text(error,error_cap,"Wrong ROM size; expected 1,572,864 unheadered bytes."); return 0; }
    civ_sha256_hex(rom,size,sha);
    if (strcmp(sha,EXPECTED_SHA) != 0) { copy_text(error,error_cap,"Wrong Civilization ROM SHA-256."); return 0; }
    header=rom+0xFFC0u;
    if (header[0x15] != 0x31u) { copy_text(error,error_cap,"Expected FastROM HiROM map mode $31."); return 0; }
    if (read_le16(rom+0xFFFCu) != 0x804Au) { copy_text(error,error_cap,"Unexpected reset vector."); return 0; }
    if (info) {
        memset(info,0,sizeof(*info)); info->file_bytes=size; info->crc32=civ_crc32(rom,size);
        memcpy(info->sha256,sha,65u); memcpy(info->title,header,21u); info->title[21]='\0';
        info->map_mode=header[0x15]; info->cartridge_type=header[0x16];
        info->rom_size_code=header[0x17]; info->sram_size_code=header[0x18]; info->region_code=header[0x19];
        info->checksum_complement=read_le16(header+0x1Cu); info->checksum=read_le16(header+0x1Eu);
        info->reset_vector=read_le16(rom+0xFFFCu);
    }
    copy_text(error,error_cap,""); return 1;
}

int civ_attach_verified_rom(CivRecomp *i, const uint8_t *rom, size_t size, char *error, size_t error_cap) {
    if (!i) { copy_text(error,error_cap,"Static core instance is null."); return 0; }
    if (!civ_verify_rom(rom,size,NULL,error,error_cap)) return 0;
    i->rom=rom; i->rom_size=size;
    copy_text(error,error_cap,"");
    return 1;
}

static uint64_t civ_fnv1a64(const uint8_t *data, size_t size) {
    uint64_t hash=UINT64_C(14695981039346656037);
    size_t n;
    if(!data)return hash;
    for(n=0;n<size;n++) {
        hash^=(uint64_t)data[n];
        hash*=UINT64_C(1099511628211);
    }
    return hash;
}

void civ_video_begin_visible_frame_capture(CivRecomp *i)
{
    if(!i || i->v19_first_visible_frame_captured)return;
    /* Certification keeps hashes and counters, not duplicate render memory. */
    i->v19_first_visible_vram_fnv1a64=civ_fnv1a64(i->vram,sizeof(i->vram));
    i->v19_first_visible_cgram_fnv1a64=civ_fnv1a64(i->cgram,sizeof(i->cgram));
    i->v19_first_visible_oam_fnv1a64=civ_fnv1a64(i->oam,sizeof(i->oam));
    i->v19_first_visible_dma_run_count=i->dma_run_count;
    i->v19_first_visible_dma_transfer_byte_count=i->dma_transfer_byte_count;
    i->v19_first_visible_vram_data_write_count=i->vram_data_write_count;
    i->v19_first_visible_cgram_data_write_count=i->cgram_data_write_count;
    i->v19_first_visible_oam_data_write_count=i->oam_data_write_count;
    i->v19_first_visible_master_clock=i->master_clock;
    i->v19_visible_capture_frame_start=i->frame_count;
    i->v19_visible_capture_started=1u;
}

void civ_video_freeze_visible_frame(CivRecomp *i)
{
    if(!i || i->v19_first_visible_frame_captured)return;
    i->v19_first_visible_scanline_fnv1a64=civ_fnv1a64((const uint8_t *)i->scanline_ppu_regs,sizeof(i->scanline_ppu_regs));
    i->v19_first_visible_frame_number=i->v19_visible_capture_frame_start;
    i->v19_first_visible_frame_end_master_clock=i->master_clock;
    i->v19_first_visible_frame_captured=1u;
}

void civ_capture_video_checkpoint(const CivRecomp *i, CivVideoCheckpoint *checkpoint) {
    if(!checkpoint)return;
    memset(checkpoint,0,sizeof(*checkpoint));
    if(!i)return;
    checkpoint->frame_count=i->frame_count;
    checkpoint->master_clock=i->master_clock;
    checkpoint->forced_blank=i->ppu.forced_blank;
    checkpoint->brightness=i->ppu.brightness;
    checkpoint->bg_mode=i->ppu.bg_mode;
    checkpoint->main_screen_layers=i->ppu.main_screen_layers;
    checkpoint->sub_screen_layers=i->ppu.sub_screen_layers;
    checkpoint->vram_fnv1a64=civ_fnv1a64(i->vram,sizeof(i->vram));
    checkpoint->cgram_fnv1a64=civ_fnv1a64(i->cgram,sizeof(i->cgram));
    checkpoint->oam_fnv1a64=civ_fnv1a64(i->oam,sizeof(i->oam));
    checkpoint->dma_run_count=i->dma_run_count;
    checkpoint->dma_transfer_byte_count=i->dma_transfer_byte_count;
    checkpoint->vram_data_write_count=i->vram_data_write_count;
    checkpoint->cgram_data_write_count=i->cgram_data_write_count;
    checkpoint->oam_data_write_count=i->oam_data_write_count;
}

int civ_fail_frontier(CivRecomp *i, const char *reason, const char *address) {
    if (!i) return 0;
    i->failed = 1;
    copy_text(i->frontier_reason, sizeof(i->frontier_reason), reason);
    if (address) copy_text(i->frontier_address, sizeof(i->frontier_address), address);
    else (void)snprintf(i->frontier_address, sizeof(i->frontier_address),"%02X:%04X", i->cpu.pbr, i->cpu.pc);
    return 0;
}

void civ_reset(CivRecomp *i) {
    static const uint8_t signature[8] = {'C','I','V','S','R','M','3','3'};
    uint8_t preserved_sram[CIV_SRAM_SIZE];
    uint8_t preserved_dirty = 0u;
    CivHostHooks preserved_hooks;
    uint8_t preserved_widescreen=0u;
    int preserve_sram;
    if (!i) return;
    /* Public callers cannot construct CivRecomp directly. A zero signature is
       retained only for internal structural tests that explicitly zero their
       machine object before first reset. */
    memset(&preserved_hooks,0,sizeof(preserved_hooks));
    preserve_sram = memcmp(i->reset_signature, signature, sizeof(signature)) == 0;
    if (preserve_sram) {
        preserved_hooks=i->host_hooks;
        preserved_widescreen=i->widescreen_enabled;
        civ_v20_audio_release_internal(i);
        memcpy(preserved_sram, i->sram, sizeof(preserved_sram));
        preserved_dirty = i->sram_dirty;
    }
    memset(i, 0, sizeof(*i));
    if (preserve_sram) {
        memcpy(i->sram, preserved_sram, sizeof(preserved_sram));
        i->sram_dirty = preserved_dirty;
        i->host_hooks=preserved_hooks;
        i->widescreen_enabled=preserved_widescreen;
    }
    memcpy(i->reset_signature, signature, sizeof(signature));
    i->cpu.s = 0x01FFu;
    i->cpu.p = CIV_P_I | CIV_P_M | CIV_P_X;
    i->cpu.e = 1u;
    i->cpu.pbr = 0u;
    i->cpu.dbr = 0u;
    i->cpu.pc = 0x804Au;
    i->dram_refresh_hcounter=538u;
    if(i->host_hooks.pcm)
        civ_v20_set_host_pcm_sink(i,i->host_hooks.pcm,i->host_hooks.context);
}

CivRecomp *civ_create(char *error, size_t error_cap) {
    CivRecomp *instance = (CivRecomp *)calloc(1u, sizeof(*instance));
    if (!instance) {
        copy_text(error, error_cap, "Unable to allocate the Civilization static core.");
        return NULL;
    }
    civ_reset(instance);
    copy_text(error, error_cap, "");
    return instance;
}

void civ_destroy(CivRecomp *instance) {
    if (!instance) return;
    civ_v20_audio_release_internal(instance);
    memset(instance, 0, sizeof(*instance));
    free(instance);
}

uint64_t civ_frame_count(const CivRecomp *instance) {
    return instance ? instance->frame_count : 0u;
}

uint64_t civ_instruction_count(const CivRecomp *instance) {
    return instance ? instance->instruction_count : 0u;
}

uint64_t civ_master_clock(const CivRecomp *instance) {
    return instance ? instance->master_clock : 0u;
}

int civ_has_failed(const CivRecomp *instance) {
    return instance && instance->failed;
}

int civ_audio_active(const CivRecomp *instance) {
    return instance && instance->v20_full_static_audio_acquired;
}

uint64_t civ_audio_pcm_frames(const CivRecomp *instance) {
    return instance ? instance->v20_pcm_frame_count : 0u;
}

size_t civ_audio_available(const CivRecomp *instance) {
    return instance && instance->v20_full_static_audio_acquired ?
           civ_v20_audio_available_internal(instance) : 0u;
}

size_t civ_audio_read(CivRecomp *instance,int16_t *out,uint8_t *known,
                      size_t capacity) {
    if(!instance||!instance->v20_full_static_audio_acquired)return 0u;
    return civ_v20_audio_read_internal(instance,out,known,capacity);
}

uint64_t civ_audio_overflow_count(const CivRecomp *instance) {
    return instance&&instance->v20_full_static_audio_acquired?
           civ_v20_audio_overflow_internal(instance):0u;
}

int civ_run_to_frame(CivRecomp *instance, uint64_t target_frame,
                     uint64_t instruction_budget) {
    int ok;
    if (!instance || target_frame < instance->frame_count) return 0;
    instance->headless_frame_stop_target = target_frame;
    instance->headless_frame_stop_enabled = 1u;
    instance->headless_frame_stop_reached = 0u;
    if (instance->frame_count < target_frame && !instance->failed)
        (void)civ_run_static(instance, instruction_budget);
    instance->headless_frame_stop_enabled = 0u;
    ok = !instance->failed && instance->frame_count >= target_frame;
    if (ok && instance->v20_full_static_audio_acquired)
        ok = civ_v20_audio_sync(instance);
    return ok;
}

void civ_set_host_hooks(CivRecomp *instance,const CivHostHooks *hooks) {
    if(!instance)return;
    if(hooks)instance->host_hooks=*hooks;
    else memset(&instance->host_hooks,0,sizeof(instance->host_hooks));
    civ_v20_set_host_pcm_sink(instance,instance->host_hooks.pcm,
                              instance->host_hooks.context);
}

void civ_set_widescreen_enabled(CivRecomp *instance,int enabled) {
    if(!instance)return;
    if(!enabled&&instance->widescreen_enabled)
        civ_set_controller_input(instance,0u,0u);
    instance->widescreen_enabled=(uint8_t)(enabled!=0);
    if(!instance->widescreen_enabled) {
        instance->widescreen_cursor_extension_x=0;
        instance->widescreen_cursor_extension_y=0;
        instance->widescreen_previous_input=0u;
        instance->widescreen_consumed_direction=0u;
        instance->widescreen_input_rebased=0u;
        instance->widescreen_clear_after_release=0u;
    }
}

int civ_widescreen_enabled(const CivRecomp *instance) {
    return instance&&instance->widescreen_enabled;
}

int civ_widescreen_live_map_state(const CivRecomp *instance) {
    uint16_t top_left;
    uint16_t top_inner;
    uint16_t top_right;
    if(!instance||instance->ppu.forced_blank||instance->ppu.bg_mode!=1u||
       instance->ppu.main_screen_layers!=0x13u||
       instance->ppu.bg_tilemap_address[0]!=0x5800u||
       instance->ppu.bg_tilemap_address[1]!=0x5C00u)return 0;
    /* The bare world-map frame has a stable three-word upper border in the
       guest's authoritative BG1 staging map. City/unit command menus reuse
       the same PPU mode and tilemap addresses but replace this border with
       their status header, so the PPU signature alone is not a safe gate. */
    top_left=(uint16_t)(instance->wram[0x2462u]|
        ((uint16_t)instance->wram[0x2463u]<<8));
    top_inner=(uint16_t)(instance->wram[0x2464u]|
        ((uint16_t)instance->wram[0x2465u]<<8));
    top_right=(uint16_t)(instance->wram[0x249Cu]|
        ((uint16_t)instance->wram[0x249Du]<<8));
    /* Palette/priority bits vary with the active terrain palette. The map
       frame identity is carried by the 10-bit tile number itself. */
    return (top_left&0x03FFu)==0x004Fu&&
           (top_inner&0x03FFu)==0x0050u&&
           (top_right&0x03FFu)==0x004Fu;
}

int civ_widescreen_frame_active(const CivRecomp *instance) {
    return instance&&instance->widescreen_enabled&&
           civ_widescreen_live_map_state(instance);
}

unsigned civ_frame_width(const CivRecomp *instance) {
    if(!instance)return 0u;
    if(instance->v19_framebuffer_ready&&instance->v19_framebuffer_width)
        return instance->v19_framebuffer_width;
    return civ_widescreen_frame_active(instance)?CIV_WIDESCREEN_WIDTH:
                                                  CIV_FRAME_WIDTH;
}

int civ_widescreen_cursor_extension_x(const CivRecomp *instance) {
    return instance?(int)instance->widescreen_cursor_extension_x:0;
}

int civ_widescreen_cursor_extension_y(const CivRecomp *instance) {
    return instance?(int)instance->widescreen_cursor_extension_y:0;
}

int civ_run_frame(CivRecomp *instance,uint16_t controller1,
                  uint64_t instruction_budget,int render,
                  CivFrameResult *result) {
    CivFrameResult local;
    uint64_t target;
    int ok;
    if(!result)result=&local;
    memset(result,0,sizeof(*result));
    if(!instance)return 0;
    result->start_frame=instance->frame_count;
    result->instructions_executed=instance->instruction_count;
    if(instance->frame_count==UINT64_MAX)return 0;
    target=instance->frame_count+1u;
    civ_set_controller_input(instance,0u,controller1);
    ok=civ_run_to_frame(instance,target,instruction_budget);
    result->end_frame=instance->frame_count;
    result->instructions_executed=instance->instruction_count-result->instructions_executed;
    result->frame_completed=(uint8_t)(ok&&instance->frame_count>=target);
    if(ok&&render)result->frame_rendered=(uint8_t)civ_render_present_frame(instance);
    if(ok&&(!render||result->frame_rendered)){
        if(instance->host_hooks.frame_complete)
            instance->host_hooks.frame_complete(instance->host_hooks.context,
                instance->frame_count,render?civ_get_framebuffer_rgba(instance):NULL);
        if(instance->host_hooks.diagnostic)
            instance->host_hooks.diagnostic(instance->host_hooks.context,"frame-complete");
        return 1;
    }
    if(instance->host_hooks.failure)
        instance->host_hooks.failure(instance->host_hooks.context,
            civ_frontier_address(instance),civ_frontier_reason(instance));
    return 0;
}

size_t civ_sram_size(void) { return CIV_SRAM_SIZE; }

int civ_sram_copy(const CivRecomp *instance, void *destination, size_t capacity) {
    if (!instance || !destination || capacity < CIV_SRAM_SIZE) return 0;
    memcpy(destination, instance->sram, CIV_SRAM_SIZE);
    return 1;
}

static void civ_copy_error(char *error, size_t error_cap, const char *text) {
    if (!error || error_cap == 0u) return;
    if (!text) text = "";
    (void)snprintf(error, error_cap, "%s", text);
}

int civ_sram_load(CivRecomp *instance, const void *source, size_t size,
                  char *error, size_t error_cap) {
    if (!instance || !source || size != CIV_SRAM_SIZE) {
        civ_copy_error(error, error_cap, "The Civilization battery SRAM image must be exactly 32 KiB.");
        return 0;
    }
    memcpy(instance->sram, source, CIV_SRAM_SIZE);
    instance->sram_dirty = 0u;
    civ_copy_error(error, error_cap, "");
    return 1;
}

int civ_sram_dirty(const CivRecomp *instance) {
    return instance && instance->sram_dirty != 0u;
}

void civ_sram_mark_clean(CivRecomp *instance) {
    if (instance) instance->sram_dirty = 0u;
}





const char *civ_frontier_reason(const CivRecomp *i){return i?i->frontier_reason:"";}
const char *civ_frontier_address(const CivRecomp *i){return i?i->frontier_address:"";}
