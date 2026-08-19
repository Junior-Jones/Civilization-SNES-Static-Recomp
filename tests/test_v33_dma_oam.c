#include "civilization_internal.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    CivRecomp c = {0};
    unsigned n;
    civ_reset(&c);
    if(!civ_bus_write8(&c,0x002100u,0x80u)) return 10;
    c.natural_timing_enabled=1u;
    for(n=0;n<CIV_OAM_SIZE;n++) c.wram[0x2000u+n]=(uint8_t)((n*37u+11u)&0xFFu);
    if(!civ_bus_write8(&c,0x002102u,0x00u) || !civ_bus_write8(&c,0x002103u,0x00u)) return 11;
    if(!civ_bus_write8(&c,0x004310u,0x02u) || !civ_bus_write8(&c,0x004311u,0x04u) ||
       !civ_bus_write8(&c,0x004312u,0x00u) || !civ_bus_write8(&c,0x004313u,0x20u) ||
       !civ_bus_write8(&c,0x004314u,0x7Eu) || !civ_bus_write8(&c,0x004315u,0x20u) ||
       !civ_bus_write8(&c,0x004316u,0x02u) || !civ_bus_write8(&c,0x00420Bu,0x02u)) return 12;
    /* MDMAEN is scheduled, not executed inline.  The first CPU cycle clears
       the hardware start delay; the following cycle performs arbitration and
       the transfer. */
    if(c.dma_channel_run_count[1]!=0u || c.dma_pending_mask!=0x02u || c.dma_start_delay!=1u) return 16;
    (void)civ_dma_process_cpu_cycle(&c,8u);
    if(c.dma_channel_run_count[1]!=0u || c.dma_pending_mask!=0x02u || c.dma_start_delay!=0u) return 17;
    (void)civ_dma_process_cpu_cycle(&c,8u);
    if(c.failed || memcmp(c.oam,&c.wram[0x2000u],CIV_OAM_SIZE)!=0 ||
       c.oam_data_write_count!=CIV_OAM_SIZE || c.ppu.oam_internal_address!=0x0220u ||
       c.dma_channel_run_count[1]!=1u || c.dma_channel_transfer_byte_count[1]!=CIV_OAM_SIZE) {
        fprintf(stderr,"OAM DMA mismatch failed=%u writes=%llu addr=%04X runs=%llu bytes=%llu\n",
            c.failed,(unsigned long long)c.oam_data_write_count,c.ppu.oam_internal_address,
            (unsigned long long)c.dma_channel_run_count[1],(unsigned long long)c.dma_channel_transfer_byte_count[1]);
        return 13;
    }

    /* Prove the reached mode-1 B-bus pattern independently: $2118,$2119,$2118,$2119. */
    civ_reset(&c);
    c.natural_timing_enabled=1u;
    c.wram[0x1000u]=0x11u;c.wram[0x1001u]=0x22u;c.wram[0x1002u]=0x33u;c.wram[0x1003u]=0x44u;
    if(!civ_bus_write8(&c,0x002115u,0x80u) || !civ_bus_write8(&c,0x002116u,0x00u) || !civ_bus_write8(&c,0x002117u,0x00u) ||
       !civ_bus_write8(&c,0x004310u,0x01u) || !civ_bus_write8(&c,0x004311u,0x18u) ||
       !civ_bus_write8(&c,0x004312u,0x00u) || !civ_bus_write8(&c,0x004313u,0x10u) ||
       !civ_bus_write8(&c,0x004314u,0x7Eu) || !civ_bus_write8(&c,0x004315u,0x04u) ||
       !civ_bus_write8(&c,0x004316u,0x00u) || !civ_bus_write8(&c,0x00420Bu,0x02u)) return 14;
    (void)civ_dma_process_cpu_cycle(&c,8u);
    (void)civ_dma_process_cpu_cycle(&c,8u);
    if(c.vram[0]!=0x11u || c.vram[1]!=0x22u || c.vram[2]!=0x33u || c.vram[3]!=0x44u || c.vram_address!=2u) return 15;
    puts("PASS current scheduled DMA channel-1 modes 1/2 and hardware-style OAMDATA writes");
    return 0;
}
