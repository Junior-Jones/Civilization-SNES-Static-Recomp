#include "civilization_internal.h"
#include <stdio.h>

int main(void)
{
    CivRecomp c = {0};
    uint16_t got0=0u,got1=0u;
    unsigned bit;
    uint8_t v;
    const uint16_t mask0=0x0A55u,mask1=0x05AAu;
    civ_reset(&c);
    civ_set_controller_input(&c,0u,mask0);
    civ_set_controller_input(&c,1u,mask1);
    if(!civ_bus_write8(&c,0x004016u,1u) || !civ_bus_write8(&c,0x004016u,0u)) return 10;
    for(bit=0;bit<16u;bit++) {
        if(!civ_bus_read8(&c,0x004016u,&v)) return 11;
        got0|=(uint16_t)((v&1u)<<bit);
        if(!civ_bus_read8(&c,0x004017u,&v)) return 12;
        got1|=(uint16_t)((v&1u)<<bit);
    }
    if(got0!=mask0 || got1!=mask1 || c.controller_strobe_write_count!=2u ||
       c.controller_serial_read_count[0]!=16u || c.controller_serial_read_count[1]!=16u) {
        fprintf(stderr,"controller mismatch got=%04X/%04X strobe=%u reads=%u/%u\n",got0,got1,c.controller_strobe_write_count,c.controller_serial_read_count[0],c.controller_serial_read_count[1]);
        return 13;
    }
    puts("PASS Version 16 manual SNES controller latch/shift order");
    return 0;
}
