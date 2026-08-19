#include "civilization_internal.h"
#include "civilization_diagnostics.h"

#include <string.h>

static uint64_t diagnostic_hash(uint64_t hash, const void *data, size_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    size_t index;
    for (index = 0u; index < size; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int civ_diagnostics_capture(const CivRecomp *i, CivDiagnosticState *out)
{
    uint64_t state_hash = UINT64_C(14695981039346656037);
    if (!i || !out) return 0;
    memset(out, 0, sizeof(*out));
    out->cpu.a=i->cpu.a;out->cpu.x=i->cpu.x;out->cpu.y=i->cpu.y;
    out->cpu.s=i->cpu.s;out->cpu.d=i->cpu.d;out->cpu.pc=i->cpu.pc;
    out->cpu.pbr=i->cpu.pbr;out->cpu.dbr=i->cpu.dbr;
    out->cpu.p=i->cpu.p;out->cpu.e=i->cpu.e;
    out->instruction_count=i->instruction_count;out->frame_count=i->frame_count;
    out->master_clock=i->master_clock;out->hcounter=i->hcounter;
    out->vcounter=i->vcounter;out->field=i->field;out->failed=(uint8_t)(i->failed!=0);
    out->natural_timing_enabled=i->natural_timing_enabled;
    out->cpu_code_access_count=i->cpu_code_access_count;
    out->cpu_data_access_count=i->cpu_data_access_count;
    out->cpu_internal_cycle_count=i->cpu_internal_cycle_count;
    out->dram_refresh_count=i->dram_refresh_count;
    out->dram_refresh_master_clocks=i->dram_refresh_master_clocks;
    memcpy(out->ppu_regs,i->ppu_regs,sizeof(out->ppu_regs));
    out->ppu.forced_blank=i->ppu.forced_blank;out->ppu.brightness=i->ppu.brightness;
    out->ppu.bg_mode=i->ppu.bg_mode;out->ppu.mode1_bg3_priority=i->ppu.mode1_bg3_priority;
    memcpy(out->ppu.bg_large_tiles,i->ppu.bg_large_tiles,sizeof(out->ppu.bg_large_tiles));
    memcpy(out->ppu.bg_tilemap_address,i->ppu.bg_tilemap_address,sizeof(out->ppu.bg_tilemap_address));
    memcpy(out->ppu.bg_double_width,i->ppu.bg_double_width,sizeof(out->ppu.bg_double_width));
    memcpy(out->ppu.bg_double_height,i->ppu.bg_double_height,sizeof(out->ppu.bg_double_height));
    memcpy(out->ppu.bg_chr_address,i->ppu.bg_chr_address,sizeof(out->ppu.bg_chr_address));
    memcpy(out->ppu.bg_hscroll,i->ppu.bg_hscroll,sizeof(out->ppu.bg_hscroll));
    memcpy(out->ppu.bg_vscroll,i->ppu.bg_vscroll,sizeof(out->ppu.bg_vscroll));
    out->ppu.main_screen_layers=i->ppu.main_screen_layers;
    out->ppu.sub_screen_layers=i->ppu.sub_screen_layers;
    out->ppu.window_mask_main=i->ppu.window_mask_main;
    out->ppu.window_mask_sub=i->ppu.window_mask_sub;
    out->ppu.color_math_enabled=i->ppu.color_math_enabled;
    out->ppu.fixed_color=i->ppu.fixed_color;out->ppu.oam_mode=i->ppu.oam_mode;
    out->ppu.oam_base_address=i->ppu.oam_base_address;
    out->ppu.oam_priority_rotation=i->ppu.oam_priority_rotation;
    memcpy(out->controller_live,i->controller_live,sizeof(out->controller_live));
    memcpy(out->controller_latched,i->controller_latched,sizeof(out->controller_latched));
    memcpy(out->auto_joypad_data,i->auto_joypad_data,sizeof(out->auto_joypad_data));
    memcpy(out->controller_device,i->controller_device,sizeof(out->controller_device));
    memcpy(out->controller_shift_index,i->controller_shift_index,sizeof(out->controller_shift_index));
    memcpy(out->controller_serial_read_count,i->controller_serial_read_count,sizeof(out->controller_serial_read_count));
    memcpy(out->auto_joypad_serial_read_count,i->auto_joypad_serial_read_count,sizeof(out->auto_joypad_serial_read_count));
    out->auto_joypad_poll_count=i->auto_joypad_poll_count;
    out->mouse_bios_probe_count=i->v18_mouse_bios_probe_count;
    out->nmi_pending=i->nmi_pending;out->nmi_depth=i->nmi_depth;
    out->nmi_accept_count=i->nmi_accept_count;out->nmi_return_count=i->nmi_return_count;
    out->irq_line=i->irq_line;out->irq_depth=i->irq_depth;
    out->irq_accept_count=i->irq_accept_count;out->irq_return_count=i->irq_return_count;
    out->framebuffer_ready=i->v19_framebuffer_ready;
    out->framebuffer_fnv1a64=i->v19_framebuffer_fnv1a64;
    out->wram_fnv1a64=diagnostic_hash(UINT64_C(14695981039346656037),i->wram,sizeof(i->wram));
    out->sram_fnv1a64=diagnostic_hash(UINT64_C(14695981039346656037),i->sram,sizeof(i->sram));
    state_hash=diagnostic_hash(state_hash,&i->cpu,sizeof(i->cpu));
    state_hash=diagnostic_hash(state_hash,i->wram,sizeof(i->wram));
    state_hash=diagnostic_hash(state_hash,i->sram,sizeof(i->sram));
    state_hash=diagnostic_hash(state_hash,i->vram,sizeof(i->vram));
    state_hash=diagnostic_hash(state_hash,i->cgram,sizeof(i->cgram));
    state_hash=diagnostic_hash(state_hash,i->oam,sizeof(i->oam));
    state_hash=diagnostic_hash(state_hash,&i->master_clock,sizeof(i->master_clock));
    out->guest_state_fnv1a64=diagnostic_hash(state_hash,&i->instruction_count,sizeof(i->instruction_count));
    memcpy(out->frontier_address,i->frontier_address,sizeof(out->frontier_address));
    memcpy(out->frontier_reason,i->frontier_reason,sizeof(out->frontier_reason));
    return 1;
}

int civ_diagnostics_read_wram(const CivRecomp *instance, size_t offset,
                              void *destination, size_t size)
{
    if (!instance || !destination || offset > CIV_WRAM_SIZE ||
        size > CIV_WRAM_SIZE - offset) return 0;
    memcpy(destination, instance->wram + offset, size);
    return 1;
}
