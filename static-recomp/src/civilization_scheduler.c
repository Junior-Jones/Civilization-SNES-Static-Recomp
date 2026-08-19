#include "civilization_internal.h"

#include <string.h>

static uint16_t civ_scanline_period(const CivRecomp *i) {
    if (i && i->field && i->vcounter==240u) return 1360u;
    return 1364u;
}
int civ_headless_frame_stop(CivRecomp *i) {
    if(!i || !i->headless_frame_stop_enabled ||
       i->frame_count<i->headless_frame_stop_target)return 0;
    i->headless_frame_stop_reached=1u;
    return 1;
}
static void civ_latch_timer_irq(CivRecomp *i) {
    if (!i || i->irq_line) return;
    i->irq_line=1u; i->irq_transition=1u; i->irq_hold_master_clocks=4u; i->irq_event_count++;
}
static void civ_timing_raw_master(CivRecomp *i) {
    uint16_t period;
    if (!i) return;
    period=civ_scanline_period(i);
    if (i->irq_hold_master_clocks>0u) i->irq_hold_master_clocks--;
    i->master_clock++;
    i->hcounter++;
    if (i->hcounter>=period) {
        i->hcounter=0u;
        i->vcounter++;
        i->dram_refresh_done=0u;
        i->dram_refresh_hcounter=(uint16_t)(538u-(i->master_clock&7u));
        if (i->vcounter>=CIV_NTSC_SCANLINES) {
            i->vcounter=0u;
            i->field^=1u;
            i->frame_count++;
            /* SNES HDMA frame initialization is scheduled at H=12 plus the
               current master-clock phase.  Execution itself is deferred to
               the CPU/DMA arbitration path, including the one-cycle delay. */
            i->hdma_init_hcounter=(uint16_t)(12u+(i->master_clock&7u));
            if(i->v19_visible_capture_started && !i->v19_first_visible_frame_captured) {
                civ_video_freeze_visible_frame(i);
                i->v19_visible_capture_started=0u;
                i->v19_visible_capture_armed=0u;
            } else if(i->v19_visible_capture_armed && !i->v19_first_visible_frame_captured) {
                civ_video_begin_visible_frame_capture(i);
            }
        }
        if(i->vcounter<CIV_FRAME_HEIGHT) {
            i->scanline_ppu[i->vcounter]=i->ppu;
            memcpy(i->scanline_ppu_regs[i->vcounter],i->ppu_regs,CIV_PPU_WRITE_REG_COUNT);
            i->scanline_state_valid[i->vcounter]=1u;
        }
        if (i->virq_enable && !i->hirq_enable && i->vcounter==i->vtime_raw) civ_latch_timer_irq(i);
    }
    {
        uint16_t vblank_start=(uint16_t)((i->ppu_regs[0x33u]&0x04u)?240u:225u);
        if(i->vcounter==0u && i->hcounter==i->hdma_init_hcounter)
            civ_hdma_schedule_init(i);
        /* MesenCE/SNES timing schedules visible-line HDMA at H=276*4=1104.
           The transfer remains pending until the CPU arbitration point. */
        if(i->vcounter<vblank_start && i->hcounter==1104u)
            civ_hdma_schedule_scanline(i);
    }
    {
        uint16_t nmi_scanline=(uint16_t)((i->ppu_regs[0x33u]&0x04u)?240u:225u);
        if(i->vcounter==nmi_scanline && i->hcounter==2u) {
            i->nmi_flag=1u;
            if(i->auto_joypad_enable)(void)civ_auto_joypad_poll(i);
        } else if(i->vcounter==nmi_scanline && i->hcounter==6u) {
            /* The external NMI line changes here, but the S-CPU does not
               accept it immediately.  The edge detector samples this latch
               on the following CPU cycle (see civilization_cpu_timing.c). */
            if(i->nmi_enable)i->nmi_flag_counter=1u;
        } else if(i->vcounter==0u && i->hcounter==2u) {
            i->nmi_flag=0u;
        }
    }
    if (i->hirq_enable) {
        uint16_t hmatch=(uint16_t)((i->htime_raw+1u)*4u);
        if (i->hcounter==hmatch && (!i->virq_enable || i->vcounter==i->vtime_raw)) civ_latch_timer_irq(i);
    }
}
void civ_timing_advance_master(CivRecomp *i, uint32_t master_clocks) {
    uint32_t n;
    if (!i) return;
    for (n=0;n<master_clocks;n++) {
        uint32_t refresh_clock;
        civ_timing_raw_master(i);
        if(!i->natural_timing_enabled || i->dram_refresh_active ||
           i->dram_refresh_done || i->hcounter!=i->dram_refresh_hcounter)continue;
        i->dram_refresh_done=1u;
        i->dram_refresh_active=1u;
        i->dram_refresh_count++;
        for(refresh_clock=0u;refresh_clock<40u;refresh_clock++)
            civ_timing_raw_master(i);
        i->dram_refresh_master_clocks+=40u;
        i->dram_refresh_active=0u;
    }
}
