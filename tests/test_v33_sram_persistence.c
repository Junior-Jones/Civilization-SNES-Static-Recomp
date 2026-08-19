#include "civilization_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fail(const char *message)
{
    fprintf(stderr,"v33 SRAM persistence regression failed: %s\n",message);
    return 1;
}

int main(void)
{
    CivRecomp *core=(CivRecomp *)calloc(1u,sizeof(*core));
    uint8_t *image=(uint8_t *)malloc(CIV_SRAM_SIZE);
    size_t offset=0u;
    char error[192]={0};
    if(!core||!image){free(image);free(core);return fail("allocation");}

    civ_reset(core);
    if(civ_sram_size()!=CIV_SRAM_SIZE){free(image);free(core);return fail("SRAM size");}
    if(!civ_hirom_sram_offset(0x20u,0x6000u,&offset)||offset!=0u){free(image);free(core);return fail("primary SRAM mapping");}
    if(!civ_hirom_sram_offset(0xA0u,0x6000u,&offset)||offset!=0u){free(image);free(core);return fail("mirror SRAM mapping");}
    if(civ_sram_dirty(core)){free(image);free(core);return fail("fresh SRAM unexpectedly dirty");}

    if(!civ_bus_write8(core,0x206000u,0x5Au)){free(image);free(core);return fail("mapped SRAM write");}
    if(core->sram[0]!=0x5Au||!civ_sram_dirty(core)){free(image);free(core);return fail("SRAM write/dirty tracking");}

    /* Console reset must preserve the battery-backed cartridge image. */
    civ_reset(core);
    if(core->sram[0]!=0x5Au){free(image);free(core);return fail("reset erased battery SRAM");}

    if(!civ_sram_copy(core,image,CIV_SRAM_SIZE)){free(image);free(core);return fail("SRAM copy");}
    if(image[0]!=0x5Au){free(image);free(core);return fail("SRAM copy contents");}
    civ_sram_mark_clean(core);
    if(civ_sram_dirty(core)){free(image);free(core);return fail("mark-clean");}

    memset(image,0xA5,CIV_SRAM_SIZE);
    if(!civ_sram_load(core,image,CIV_SRAM_SIZE,error,sizeof(error))){free(image);free(core);return fail("SRAM load");}
    if(core->sram[0]!=0xA5u||core->sram[CIV_SRAM_SIZE-1u]!=0xA5u){free(image);free(core);return fail("SRAM loaded contents");}
    if(civ_sram_load(core,image,CIV_SRAM_SIZE-1u,error,sizeof(error))){free(image);free(core);return fail("wrong-size SRAM accepted");}

    /* A second reset must preserve the host-loaded battery image too. */
    civ_reset(core);
    if(core->sram[0]!=0xA5u||core->sram[CIV_SRAM_SIZE-1u]!=0xA5u){free(image);free(core);return fail("reset erased loaded SRAM");}

    puts("v33 SRAM persistence regression PASS");
    free(image);free(core);
    return 0;
}
