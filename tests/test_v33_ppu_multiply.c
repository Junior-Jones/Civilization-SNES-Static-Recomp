#include "civilization_internal.h"
#include <stdio.h>

#define W(a,v) do { if(!civ_bus_write8(&c,(a),(v))) return __LINE__; } while(0)
#define R(a,p) do { if(!civ_bus_read8(&c,(a),(p))) return __LINE__; } while(0)

static int expect_product(CivRecomp *c,uint8_t lo,uint8_t mid,uint8_t hi)
{
    uint8_t v;
    if(!civ_bus_read8(c,0x002134u,&v)||v!=lo||c->ppu.ppu1_open_bus!=lo)return 0;
    if(!civ_bus_read8(c,0x002135u,&v)||v!=mid||c->ppu.ppu1_open_bus!=mid)return 0;
    if(!civ_bus_read8(c,0x002136u,&v)||v!=hi||c->ppu.ppu1_open_bus!=hi)return 0;
    return 1;
}

int main(void)
{
    CivRecomp c = {0};
    uint8_t v;

    civ_reset(&c);
    /* Shared $211B-$2120 write latch: A becomes $1234, then one B write
       consumes the prior $12 as B's low byte and installs signed high $FE. */
    W(0x00211Bu,0x34u); W(0x00211Bu,0x12u); W(0x00211Cu,0xFEu);
    if(c.ppu.mode7_matrix[0]!=0x1234u||c.ppu.mode7_matrix[1]!=0xFE12u||c.ppu.mode7_value_latch!=0xFEu)return 10;
    /* $1234 * -2 = -9320 = 24-bit $FFDB98. */
    if(!expect_product(&c,0x98u,0xDBu,0xFFu))return 11;
    if(c.ppu_multiply_result_read_count[0]!=1u||c.ppu_multiply_result_read_count[1]!=1u||c.ppu_multiply_result_read_count[2]!=1u)return 12;

    civ_reset(&c);
    /* Explicitly prove the single shared write latch crosses registers. */
    W(0x00211Bu,0xABu); W(0x00211Cu,0x7Fu);
    if(c.ppu.mode7_matrix[0]!=0xAB00u||c.ppu.mode7_matrix[1]!=0x7FABu||c.ppu.mode7_value_latch!=0x7Fu)return 20;
    W(0x00211Bu,0x34u);
    if(c.ppu.mode7_matrix[0]!=0x347Fu||c.ppu.mode7_value_latch!=0x34u)return 21;

    civ_reset(&c);
    /* Signed A and signed high byte of B.  A=-128, B.high=+127. */
    W(0x00211Bu,0x80u); W(0x00211Bu,0xFFu); W(0x00211Cu,0x7Fu);
    if(c.ppu.mode7_matrix[0]!=0xFF80u||!expect_product(&c,0x80u,0xC0u,0xFFu))return 30;

    civ_reset(&c);
    /* Extreme sign combination: -32768 * -128 = $400000. */
    W(0x00211Bu,0x00u); W(0x00211Bu,0x80u); W(0x00211Cu,0x80u);
    if(c.ppu.mode7_matrix[0]!=0x8000u||!expect_product(&c,0x00u,0x00u,0x40u))return 40;

    /* I/O mirrors must expose the same reached PPU register behavior. */
    if(!civ_bus_read8(&c,0x802134u,&v)||v!=0x00u||c.ppu.ppu1_open_bus!=0x00u)return 50;

    puts("v33 PPU multiply/read-latch PASS");
    return 0;
}
