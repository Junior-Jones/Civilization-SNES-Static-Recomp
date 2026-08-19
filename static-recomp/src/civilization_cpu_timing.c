#include "civilization_internal.h"

#include <stdint.h>

enum CivAddressingMode {
    CIV_AM_IMM8=0, CIV_AM_DP_IND_X, CIV_AM_SR, CIV_AM_DP,
    CIV_AM_DP_LONG_IND, CIV_AM_IMP, CIV_AM_IMM_M, CIV_AM_ACC,
    CIV_AM_ABS, CIV_AM_LONG, CIV_AM_REL8, CIV_AM_DP_IND_Y,
    CIV_AM_DP_IND, CIV_AM_SR_IND_Y, CIV_AM_DP_X, CIV_AM_DP_LONG_IND_Y,
    CIV_AM_ABS_Y, CIV_AM_ABS_X, CIV_AM_LONG_X, CIV_AM_BLOCK,
    CIV_AM_REL16, CIV_AM_ABS_IND, CIV_AM_ABS_IND_X, CIV_AM_DP_Y,
    CIV_AM_IMM_X, CIV_AM_ABS_LONG_IND
};

/* Build-time opcode metadata expressed as constants in the static core.  This
   table only selects timing rules; instruction semantics remain the generated
   exact-PC dispatch and no runtime opcode fetch or interpreter is introduced. */
static const uint8_t civ_opcode_mode[256] = {
    0u,1u,0u,2u,3u,3u,3u,4u,5u,6u,7u,5u,8u,8u,8u,9u,
    10u,11u,12u,13u,3u,14u,14u,15u,5u,16u,7u,5u,8u,17u,17u,18u,
    8u,1u,9u,2u,3u,3u,3u,4u,5u,6u,7u,5u,8u,8u,8u,9u,
    10u,11u,12u,13u,14u,14u,14u,15u,5u,16u,7u,5u,17u,17u,17u,18u,
    5u,1u,0u,2u,19u,3u,3u,4u,5u,6u,7u,5u,8u,8u,8u,9u,
    10u,11u,12u,13u,19u,14u,14u,15u,5u,16u,5u,5u,9u,17u,17u,18u,
    5u,1u,20u,2u,3u,3u,3u,4u,5u,6u,7u,5u,21u,8u,8u,9u,
    10u,11u,12u,13u,14u,14u,14u,15u,5u,16u,5u,5u,22u,17u,17u,18u,
    10u,1u,20u,2u,3u,3u,3u,4u,5u,6u,5u,5u,8u,8u,8u,9u,
    10u,11u,12u,13u,14u,14u,23u,15u,5u,16u,5u,5u,8u,17u,17u,18u,
    24u,1u,24u,2u,3u,3u,3u,4u,5u,6u,5u,5u,8u,8u,8u,9u,
    10u,11u,12u,13u,14u,14u,23u,15u,5u,16u,5u,5u,17u,17u,16u,18u,
    24u,1u,0u,2u,3u,3u,3u,4u,5u,6u,5u,5u,8u,8u,8u,9u,
    10u,11u,12u,13u,3u,14u,14u,15u,5u,16u,5u,5u,25u,17u,17u,18u,
    24u,1u,0u,2u,3u,3u,3u,4u,5u,6u,5u,5u,8u,8u,8u,9u,
    10u,11u,12u,13u,8u,14u,14u,15u,5u,16u,5u,5u,22u,17u,17u,18u
};

static void civ_cpu_sample_interrupts(CivRecomp *i, uint8_t irq_lock)
{
    if(!i)return;
    if(i->nmi_flag_counter) {
        i->nmi_flag_counter--;
        if(i->nmi_flag_counter==0u) {
            if(!irq_lock) {
                i->nmi_pending=1u;
            } else {
                /* A DMA/HDMA transfer that consumed this CPU cycle defers the
                   sampled edge by one additional CPU cycle. */
                i->nmi_flag_counter=1u;
                i->nmi_pending=0u;
            }
        }
    }
}

uint8_t civ_cpu_bus_speed(const CivRecomp *i, uint32_t address)
{
    uint8_t bank=(uint8_t)(address>>16);
    uint16_t local=(uint16_t)address;
    uint8_t fast=(uint8_t)(i && (i->reg_420d&1u));
    if(bank>=0x40u && bank<=0x7Fu)return 8u;
    if(bank>=0xC0u)return fast?6u:8u;
    if(local<0x2000u)return 8u;
    if(local<0x4000u)return 6u;
    if(local<0x4200u)return 12u;
    if(local<0x6000u)return 6u;
    if(local<0x8000u)return 8u;
    return (uint8_t)((fast && bank>=0x80u)?6u:8u);
}

void civ_cpu_data_access(CivRecomp *i, uint32_t address)
{
    uint8_t speed;
    if(!i || !i->natural_timing_enabled || i->cpu_bus_timing_suppressed)return;
    speed=civ_cpu_bus_speed(i,address&0xFFFFFFu);
    { uint8_t irq_lock=civ_dma_process_cpu_cycle(i,speed);
      if(i->failed)return;
      civ_cpu_sample_interrupts(i,irq_lock); }
    i->cpu_data_access_count++;
    civ_timing_advance_master(i,speed);
}

void civ_cpu_internal_cycles(CivRecomp *i, unsigned count)
{
    if(!i || !i->natural_timing_enabled)return;
    i->cpu_internal_cycle_count+=(uint64_t)count;
    while(count--) {
        { uint8_t irq_lock=civ_dma_process_cpu_cycle(i,6u);
          if(i->failed)return;
          civ_cpu_sample_interrupts(i,irq_lock); }
        civ_timing_advance_master(i,6u);
    }
}

static unsigned civ_branch_taken(const CivRecomp *i, uint8_t opcode)
{
    switch(opcode) {
        case 0x10u:return (i->cpu.p&CIV_P_N)==0u;
        case 0x30u:return (i->cpu.p&CIV_P_N)!=0u;
        case 0x50u:return (i->cpu.p&CIV_P_V)==0u;
        case 0x70u:return (i->cpu.p&CIV_P_V)!=0u;
        case 0x80u:return 1u;
        case 0x90u:return (i->cpu.p&CIV_P_C)==0u;
        case 0xB0u:return (i->cpu.p&CIV_P_C)!=0u;
        case 0xD0u:return (i->cpu.p&CIV_P_Z)==0u;
        case 0xF0u:return (i->cpu.p&CIV_P_Z)!=0u;
        default:return 0u;
    }
}

static unsigned civ_uses_idle_or_read(uint8_t opcode)
{
    /* Exact S-CPU opcodes whose addressing semantics use the implied or
       accumulator IdleOrRead cycle. Special one-byte operations such as RTI/RTS/RTL,
       pushes and pulls have their own fixed idle sequences and must not be
       treated as IdleOrRead. */
    switch(opcode) {
        case 0x0Au:case 0x18u:case 0x1Au:case 0x1Bu:
        case 0x2Au:case 0x38u:case 0x3Au:case 0x3Bu:
        case 0x4Au:case 0x58u:case 0x5Bu:case 0x6Au:
        case 0x78u:case 0x7Bu:case 0x88u:case 0x8Au:
        case 0x98u:case 0x9Au:case 0x9Bu:case 0xA8u:
        case 0xAAu:case 0xB8u:case 0xBAu:case 0xBBu:
        case 0xC8u:case 0xCAu:case 0xD8u:case 0xDBu:
        case 0xE8u:case 0xEAu:case 0xEBu:case 0xF8u:
        case 0xFBu:return 1u;
        default:return 0u;
    }
}

static unsigned civ_is_rmw(uint8_t opcode)
{
    switch(opcode) {
        case 0x04u:case 0x06u:case 0x0Cu:case 0x0Eu:
        case 0x14u:case 0x16u:case 0x1Cu:case 0x1Eu:
        case 0x26u:case 0x2Eu:case 0x36u:case 0x3Eu:
        case 0x46u:case 0x4Eu:case 0x56u:case 0x5Eu:
        case 0x66u:case 0x6Eu:case 0x76u:case 0x7Eu:
        case 0xC6u:case 0xCEu:case 0xD6u:case 0xDEu:
        case 0xE6u:case 0xEEu:case 0xF6u:case 0xFEu:return 1u;
        default:return 0u;
    }
}

static unsigned civ_indexed_write(uint8_t opcode)
{
    switch(opcode) {
        case 0x91u:case 0x99u:case 0x9Du:case 0x9Eu:return 1u;
        default:return civ_is_rmw(opcode);
    }
}

static unsigned civ_instruction_internal_cycles(const CivRecomp *i,
                                                uint8_t opcode,
                                                uint32_t operand)
{
    uint8_t mode=civ_opcode_mode[opcode];
    unsigned cycles=0u;
    unsigned index16=(i->cpu.p&CIV_P_X)==0u;
    uint16_t base=(uint16_t)operand;
    uint16_t index=(mode==CIV_AM_ABS_Y)?i->cpu.y:i->cpu.x;

    switch(mode) {
        case CIV_AM_IMP:case CIV_AM_ACC:cycles=1u;break;
        case CIV_AM_DP:case CIV_AM_DP_IND:case CIV_AM_DP_LONG_IND:
            cycles=(i->cpu.d&0x00FFu)?1u:0u;break;
        case CIV_AM_DP_X:case CIV_AM_DP_Y:case CIV_AM_DP_IND_X:
            cycles=((i->cpu.d&0x00FFu)?1u:0u)+1u;break;
        case CIV_AM_DP_IND_Y:
            cycles=(i->cpu.d&0x00FFu)?1u:0u;
            if(civ_indexed_write(opcode)||index16||
               (((uint16_t)(base+i->cpu.y)&0xFF00u)!=(base&0xFF00u)))cycles++;
            break;
        case CIV_AM_DP_LONG_IND_Y:
            cycles=(i->cpu.d&0x00FFu)?1u:0u;break;
        case CIV_AM_SR:cycles=1u;break;
        case CIV_AM_SR_IND_Y:cycles=2u;break;
        case CIV_AM_ABS_X:case CIV_AM_ABS_Y:
            if(civ_indexed_write(opcode)||index16||
               (((uint16_t)(base+index)&0xFF00u)!=(base&0xFF00u)))cycles=1u;
            break;
        case CIV_AM_BLOCK:cycles=2u;break;
        case CIV_AM_REL16:cycles=1u;break;
        case CIV_AM_ABS_IND_X:cycles=1u;break;
        default:break;
    }

    if(mode==CIV_AM_REL8 && civ_branch_taken(i,opcode)) {
        int16_t displacement=(int8_t)(operand&0xFFu);
        uint16_t fall=(uint16_t)(i->cpu.pc+2u);
        cycles++;
        if(i->cpu.e && (((uint16_t)(fall+displacement)&0xFF00u)!=(fall&0xFF00u)))cycles++;
    }
    if(civ_is_rmw(opcode))cycles++;

    switch(opcode) {
        case 0x20u:case 0x22u:cycles++;break;
        case 0x28u:case 0x2Bu:case 0x68u:case 0x7Au:
        case 0xABu:case 0xFAu:cycles=2u;break;
        case 0x40u:cycles=2u;break;
        case 0x60u:cycles=3u;break;
        case 0x6Bu:cycles=2u;break;
        case 0xC2u:case 0xE2u:cycles++;break;
        case 0xEBu:cycles=2u;break;
        default:break;
    }
    return cycles;
}

void civ_cpu_begin_static_instruction(CivRecomp *i, uint8_t opcode,
                                      unsigned length, uint32_t operand)
{
    uint32_t address;
    unsigned n;
    if(!i || !i->natural_timing_enabled)return;
    address=((uint32_t)i->cpu.pbr<<16)|i->cpu.pc;
    for(n=0u;n<length;n++) {
        uint8_t speed=civ_cpu_bus_speed(i,(address+n)&0xFFFFFFu);
        { uint8_t irq_lock=civ_dma_process_cpu_cycle(i,speed);
          if(i->failed)return;
          civ_cpu_sample_interrupts(i,irq_lock); }
        i->cpu_code_access_count++;
        civ_timing_advance_master(i,speed);
    }
    {
        unsigned internal_cycles=civ_instruction_internal_cycles(i,opcode,operand);
        /* Implied/accumulator addressing uses IdleOrRead for its
           addressing-mode cycle. If the sampled NMI edge will be
           recognized on this CPU cycle, that nominal 6-master-clock idle
           becomes a dummy ReadCode at the already-advanced program PC.
           This matters at scanline boundaries: on Civilization's slow-ROM
           C1:B696 INX immediately before NMI the dummy read is 8 clocks,
           not 6.  Use only the proved interrupt-lookahead latch; this does
           not fetch/decode an opcode or alter generated control flow. */
        if(internal_cycles && civ_uses_idle_or_read(opcode)) {
            if(i->nmi_flag_counter==1u || i->nmi_pending) {
                civ_cpu_data_access(i,(address+length)&0xFFFFFFu);
            } else {
                civ_cpu_internal_cycles(i,1u);
            }
            if(i->failed)return;
            internal_cycles--;
        }
        civ_cpu_internal_cycles(i,internal_cycles);
    }
    if(i->failed)return;

    /* W65C816 JSR (abs,X), opcode $FC, performs two additional reads from
       the program-bank indirect table after its stack pushes and idle cycle.
       Generated Civilization authority proves the finite target set offline,
       so these accesses are timing/bus observations only; they do not decode
       or learn a runtime target.  The former timing layer omitted both reads, making every
       reached $FC up to 16 master clocks too short on slow ROM. */
    if(opcode==0xFCu) {
        uint32_t table=((uint32_t)i->cpu.pbr<<16) |
                       (uint16_t)((uint16_t)operand+i->cpu.x);
        civ_cpu_data_access(i,table);
        if(i->failed)return;
        civ_cpu_data_access(i,(table&0xFF0000u)|((table+1u)&0xFFFFu));
    }
}
