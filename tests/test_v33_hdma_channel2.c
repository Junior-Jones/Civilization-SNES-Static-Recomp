#include "civilization_internal.h"
#include <stdio.h>
#include <stdlib.h>

static int load_file(const char *path,uint8_t **data,size_t *size)
{
    FILE *f=fopen(path,"rb");
    long n;
    if(!f)return 0;
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return 0;}
    n=ftell(f);
    if(n<0){fclose(f);return 0;}
    if(fseek(f,0,SEEK_SET)!=0){fclose(f);return 0;}
    *data=(uint8_t*)malloc((size_t)n);
    if(!*data){fclose(f);return 0;}
    if(fread(*data,1,(size_t)n,f)!=(size_t)n){free(*data);fclose(f);return 0;}
    fclose(f);*size=(size_t)n;return 1;
}
static int service(CivRecomp *c)
{
    civ_cpu_internal_cycles(c,1u); if(c->failed)return 0; /* one-cycle start delay */
    civ_cpu_internal_cycles(c,1u); return !c->failed;
}
int main(int argc,char **argv)
{
    CivRecomp c = {0}; uint8_t *rom=NULL;size_t rom_size=0;char err[256];unsigned line;
    if(argc!=2||!load_file(argv[1],&rom,&rom_size))return 2;
    civ_reset(&c); if(!civ_attach_verified_rom(&c,rom,rom_size,err,sizeof(err))){free(rom);return 3;}
    c.natural_timing_enabled=1u;
    /* Keep this unit test in VBlank so only the explicit HDMA scheduling
       calls below can create transfer events. */
    c.vcounter=225u;
    c.dma2.dmap=0x42u;c.dma2.bbad=0x0Du;c.dma2.source_address=0x4AE3u;c.dma2.source_bank=0xC1u;c.dma2.indirect_bank=0u;
    c.cpu_io[0x0Cu]=0x04u;
    c.wram[0x1B85u]=0x34u;c.wram[0x1B86u]=0x12u;
    c.wram[0x1B81u]=0x78u;c.wram[0x1B82u]=0x56u;
    civ_hdma_schedule_init(&c); if(!service(&c)){free(rom);return 4;}
    if(c.hdma2.line_counter_repeat!=0x37u||c.dma2.transfer_size!=0x1B85u||c.hdma2.table_address!=0x4AE6u||!c.hdma2.do_transfer){free(rom);return 5;}
    civ_hdma_schedule_scanline(&c); if(!service(&c)){free(rom);return 6;}
    if(c.hdma_transfer_byte_count!=2u||c.dma2.transfer_size!=0x1B87u||c.hdma2.line_counter_repeat!=0x36u||c.hdma2.do_transfer){free(rom);return 7;}
    /* Finish the first 55-line descriptor.  Only its first line transfers. */
    for(line=1u;line<55u;line++){civ_hdma_schedule_scanline(&c);if(!service(&c)){free(rom);return 8;}}
    if(c.hdma2.line_counter_repeat!=0x50u||c.dma2.transfer_size!=0x1B81u||c.hdma2.table_address!=0x4AE9u||!c.hdma2.do_transfer){free(rom);return 9;}
    civ_hdma_schedule_scanline(&c);if(!service(&c)){free(rom);return 10;}
    if(c.hdma_transfer_byte_count!=4u||c.dma2.transfer_size!=0x1B83u||c.hdma2.line_counter_repeat!=0x4Fu||c.hdma2.do_transfer){free(rom);return 11;}
    if(c.ppu.bg_hscroll[0]!=0x0278u){free(rom);return 12;}
    free(rom);puts("v33 channel-2 HDMA PASS");return 0;
}
