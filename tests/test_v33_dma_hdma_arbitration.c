#include "civilization_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int load_file(const char *path,uint8_t **data,size_t *size)
{
    FILE *f=fopen(path,"rb"); long n;
    if(!f)return 0;
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return 0;}
    n=ftell(f); if(n<0){fclose(f);return 0;}
    if(fseek(f,0,SEEK_SET)!=0){fclose(f);return 0;}
    *data=(uint8_t*)malloc((size_t)n); if(!*data){fclose(f);return 0;}
    if(fread(*data,1,(size_t)n,f)!=(size_t)n){free(*data);fclose(f);return 0;}
    fclose(f); *size=(size_t)n; return 1;
}

static int prepare(CivRecomp *c,const uint8_t *rom,size_t rom_size,char *err,size_t err_size)
{
    civ_reset(c);
    if(!civ_attach_verified_rom(c,rom,rom_size,err,err_size))return 0;
    c->natural_timing_enabled=1u;
    /* Isolate DMA/HDMA arbitration from DRAM-refresh stretching.  Beam/NMI/HDMA
       scheduling still advances normally while this latch suppresses only the
       separate refresh insertion used by the full natural run. */
    c->dram_refresh_active=1u;
    c->vcounter=200u;
    c->hcounter=1000u;
    c->field=0u;
    c->dma0.dmap=0x00u;
    c->dma0.bbad=0x00u;       /* $2100; zero-filled WRAM keeps forced blank. */
    c->dma0.source_address=0u;
    c->dma0.source_bank=0x7Eu;
    c->dma0.transfer_size=0u; /* Hardware convention: 0 means 65536 bytes. */
    memset(c->wram,0,sizeof(c->wram));
    return 1;
}

static int run_disabled_hdma_case(CivRecomp *c)
{
    uint64_t start_master=c->master_clock;
    if(!civ_bus_write8(c,0x00420Bu,0x01u))return 0;
    civ_cpu_internal_cycles(c,2u); /* clear MDMA start delay, then run DMA */
    if(c->failed)return 0;
    if(c->dma_channel_transfer_byte_count[0]!=65536u || c->dma_channel_run_count[0]!=1u)return 0;
    if(c->frame_count!=2u || c->hdma_init_pending || c->hdma_transfer_pending)return 0;
    /* With refresh explicitly suppressed, the exact Mesen-style arbitration
       clock receipt is deterministic: MDMAEN bus write plus 2 CPU cycles (18), start sync (4),
       global+channel overhead (16), 65536*8 transfer clocks, end sync (2). */
    if(c->master_clock-start_master!=UINT64_C(524328))return 0;
    return 1;
}

static int run_enabled_hdma_case(CivRecomp *c)
{
    /* Start in VBlank so the first active HDMA event is frame initialization. */
    c->vcounter=225u;
    c->hcounter=1000u;
    c->frame_count=0u;
    c->field=0u;
    c->master_clock=0u;
    c->dma0.source_address=0u;
    c->dma0.transfer_size=0u;
    c->dma2.dmap=0x42u;
    c->dma2.bbad=0x0Du;
    c->dma2.source_address=0x4AE3u;
    c->dma2.source_bank=0xC1u;
    c->dma2.indirect_bank=0x00u;
    c->cpu_io[0x0Cu]=0x04u;
    c->wram[0x1B81u]=0x78u;c->wram[0x1B82u]=0x56u;
    c->wram[0x1B85u]=0x34u;c->wram[0x1B86u]=0x12u;
    if(!civ_bus_write8(c,0x00420Bu,0x01u))return 0;
    civ_cpu_internal_cycles(c,2u);
    if(c->failed)return 0;
    if(c->dma_channel_transfer_byte_count[0]!=65536u || c->dma_channel_run_count[0]!=1u)return 0;
    /* The long manual DMA spans two frame starts. Both channel-2 HDMA
       initializations and many visible-line transfers must have been serviced
       inside the active manual DMA rather than accumulating as duplicate
       pending requests. */
    if(c->frame_count!=2u || c->hdma_init_count!=2u || c->hdma_scanline_count<225u)return 0;
    if(c->dma_channel_run_count[2]==0u || c->hdma_table_read_count==0u)return 0;
    if(c->hdma_init_pending || c->hdma_transfer_pending || c->dma_pending_mask)return 0;
    return 1;
}

int main(int argc,char **argv)
{
    CivRecomp c = {0}; uint8_t *rom=NULL; size_t rom_size=0; char err[256];
    if(argc!=2 || !load_file(argv[1],&rom,&rom_size))return 2;
    if(!prepare(&c,rom,rom_size,err,sizeof(err))){free(rom);return 3;}
    if(!run_disabled_hdma_case(&c)){
        fprintf(stderr,"disabled-HDMA arbitration failed: frame=%llu master=%llu pending=%u/%u reason=%s\n",
                (unsigned long long)c.frame_count,(unsigned long long)c.master_clock,
                c.hdma_init_pending,c.hdma_transfer_pending,civ_frontier_reason(&c));
        free(rom);return 4;
    }
    if(!prepare(&c,rom,rom_size,err,sizeof(err))){free(rom);return 5;}
    if(!run_enabled_hdma_case(&c)){
        fprintf(stderr,"enabled-HDMA arbitration failed: frame=%llu master=%llu init=%llu scan=%llu pending=%u/%u reason=%s\n",
                (unsigned long long)c.frame_count,(unsigned long long)c.master_clock,
                (unsigned long long)c.hdma_init_count,(unsigned long long)c.hdma_scanline_count,
                c.hdma_init_pending,c.hdma_transfer_pending,civ_frontier_reason(&c));
        free(rom);return 6;
    }
    printf("v33 DMA/HDMA arbitration PASS: frame=%llu master=%llu init=%llu scan=%llu hdma-bytes=%llu\n",
           (unsigned long long)c.frame_count,(unsigned long long)c.master_clock,
           (unsigned long long)c.hdma_init_count,(unsigned long long)c.hdma_scanline_count,
           (unsigned long long)c.hdma_transfer_byte_count);
    free(rom);return 0;
}
