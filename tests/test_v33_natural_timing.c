#include "civilization_internal.h"

#include <stdio.h>

static int require(int condition,const char *message)
{
    if(condition)return 1;
    fprintf(stderr,"FAIL natural timing: %s\n",message);
    return 0;
}

int main(void)
{
    CivRecomp core = {0};
    civ_reset(&core);

    if(!require(civ_cpu_bus_speed(&core,0x001FFFu)==8u,"WRAM must use 8 master clocks"))return 1;
    if(!require(civ_cpu_bus_speed(&core,0x002000u)==6u,"low-bank PPU aperture must use 6 master clocks"))return 1;
    if(!require(civ_cpu_bus_speed(&core,0x004000u)==12u,"controller aperture must use 12 master clocks"))return 1;
    if(!require(civ_cpu_bus_speed(&core,0x004200u)==6u,"CPU I/O aperture must use 6 master clocks"))return 1;
    if(!require(civ_cpu_bus_speed(&core,0x008000u)==8u,"SlowROM code must use 8 master clocks"))return 1;
    core.reg_420d=1u;
    if(!require(civ_cpu_bus_speed(&core,0x808000u)==6u,"FastROM code must use 6 master clocks"))return 1;
    if(!require(civ_cpu_bus_speed(&core,0x408000u)==8u,"banks $40-$7F must remain 8 clocks"))return 1;

    core.natural_timing_enabled=1u;
    core.cpu.pbr=0x80u;
    core.cpu.pc=0x8000u;
    civ_cpu_begin_static_instruction(&core,0xEAu,1u,0u); /* NOP: one fetch + one internal cycle. */
    if(!require(core.master_clock==12u,"FastROM NOP must consume 6+6 master clocks"))return 1;
    if(!require(core.cpu_code_access_count==1u,"static code fetch counter mismatch"))return 1;
    if(!require(core.cpu_internal_cycle_count==1u,"static internal-cycle counter mismatch"))return 1;

    civ_reset(&core);
    core.natural_timing_enabled=1u;
    civ_timing_advance_master(&core,538u);
    if(!require(core.master_clock==578u,"DRAM refresh must insert 40 master clocks"))return 1;
    if(!require(core.hcounter==578u && core.vcounter==0u,"refresh must advance the shared PPU beam"))return 1;
    if(!require(core.dram_refresh_count==1u && core.dram_refresh_master_clocks==40u,
                "DRAM refresh accounting mismatch"))return 1;

    puts("PASS current core uses one natural NTSC master clock for static CPU fetches, internal cycles, bus speeds, and DRAM refresh");
    return 0;
}
