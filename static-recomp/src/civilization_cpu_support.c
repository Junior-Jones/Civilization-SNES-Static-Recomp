#include "civilization_internal.h"

#include <stdio.h>

static uint8_t civ_status_without(uint8_t status, uint8_t flag) {
    return (uint8_t)(status & (uint8_t)(0xFFu ^ (unsigned)flag));
}

void civ_set_nz8(CivRecomp *i, uint8_t value) {
    if (value == 0u) i->cpu.p |= CIV_P_Z; else i->cpu.p = civ_status_without(i->cpu.p,CIV_P_Z);
    if (value & 0x80u) i->cpu.p |= CIV_P_N; else i->cpu.p = civ_status_without(i->cpu.p,CIV_P_N);
}
void civ_set_nz16(CivRecomp *i, uint16_t value) {
    if (value == 0u) i->cpu.p |= CIV_P_Z; else i->cpu.p = civ_status_without(i->cpu.p,CIV_P_Z);
    if (value & 0x8000u) i->cpu.p |= CIV_P_N; else i->cpu.p = civ_status_without(i->cpu.p,CIV_P_N);
}
void civ_cmp8(CivRecomp *i, uint8_t left, uint8_t right) {
    uint8_t result=(uint8_t)(left-right);
    if (left >= right) i->cpu.p |= CIV_P_C; else i->cpu.p = civ_status_without(i->cpu.p,CIV_P_C);
    civ_set_nz8(i,result);
}
void civ_cmp16(CivRecomp *i, uint16_t left, uint16_t right) {
    uint16_t result=(uint16_t)(left-right);
    if (left >= right) i->cpu.p |= CIV_P_C; else i->cpu.p = civ_status_without(i->cpu.p,CIV_P_C);
    civ_set_nz16(i,result);
}
int civ_adc8_binary(CivRecomp *i, uint8_t value, const char *address) {
    uint8_t left, result;
    uint16_t sum;
    unsigned carry;
    if (!i) return 0;
    if (i->cpu.p & CIV_P_D) return civ_fail_frontier(i,"Decimal ADC reached before target-specific decimal arithmetic implementation.",address);
    left=(uint8_t)i->cpu.a; carry=(i->cpu.p&CIV_P_C)?1u:0u; sum=(uint16_t)left+(uint16_t)value+(uint16_t)carry; result=(uint8_t)sum;
    if (sum>0xFFu) i->cpu.p|=CIV_P_C; else i->cpu.p=civ_status_without(i->cpu.p,CIV_P_C);
    if (((~(left^value))&(left^result)&0x80u)!=0u) i->cpu.p|=CIV_P_V; else i->cpu.p=civ_status_without(i->cpu.p,CIV_P_V);
    i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|result); civ_set_nz8(i,result); return 1;
}
int civ_adc16_binary(CivRecomp *i, uint16_t value, const char *address) {
    uint16_t left, result;
    uint32_t sum;
    unsigned carry;
    if (!i) return 0;
    if (i->cpu.p & CIV_P_D) return civ_fail_frontier(i,"Decimal ADC reached before target-specific decimal arithmetic implementation.",address);
    left=i->cpu.a; carry=(i->cpu.p&CIV_P_C)?1u:0u; sum=(uint32_t)left+(uint32_t)value+(uint32_t)carry; result=(uint16_t)sum;
    if (sum>0xFFFFu) i->cpu.p|=CIV_P_C; else i->cpu.p=civ_status_without(i->cpu.p,CIV_P_C);
    if (((~(left^value))&(left^result)&0x8000u)!=0u) i->cpu.p|=CIV_P_V; else i->cpu.p=civ_status_without(i->cpu.p,CIV_P_V);
    i->cpu.a=result; civ_set_nz16(i,result); return 1;
}
int civ_sbc8_binary(CivRecomp *i, uint8_t value, const char *address) {
    uint8_t left, result; uint16_t sub; unsigned borrow;
    if (!i) return 0;
    if (i->cpu.p & CIV_P_D) return civ_fail_frontier(i,"Decimal SBC reached before target-specific decimal arithmetic implementation.",address);
    left=(uint8_t)i->cpu.a; borrow=(i->cpu.p&CIV_P_C)?0u:1u; sub=(uint16_t)value+(uint16_t)borrow; result=(uint8_t)((uint16_t)left-sub);
    if ((uint16_t)left>=sub) i->cpu.p|=CIV_P_C; else i->cpu.p=civ_status_without(i->cpu.p,CIV_P_C);
    if (((left^value)&(left^result)&0x80u)!=0u) i->cpu.p|=CIV_P_V; else i->cpu.p=civ_status_without(i->cpu.p,CIV_P_V);
    i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|result); civ_set_nz8(i,result); return 1;
}
int civ_sbc16_binary(CivRecomp *i, uint16_t value, const char *address) {
    uint16_t left, result; uint32_t sub; unsigned borrow;
    if (!i) return 0;
    if (i->cpu.p & CIV_P_D) return civ_fail_frontier(i,"Decimal SBC reached before target-specific decimal arithmetic implementation.",address);
    left=i->cpu.a; borrow=(i->cpu.p&CIV_P_C)?0u:1u; sub=(uint32_t)value+(uint32_t)borrow; result=(uint16_t)((uint32_t)left-sub);
    if ((uint32_t)left>=sub) i->cpu.p|=CIV_P_C; else i->cpu.p=civ_status_without(i->cpu.p,CIV_P_C);
    if (((left^value)&(left^result)&0x8000u)!=0u) i->cpu.p|=CIV_P_V; else i->cpu.p=civ_status_without(i->cpu.p,CIV_P_V);
    i->cpu.a=result; civ_set_nz16(i,result); return 1;
}


int civ_irq_enter_native(CivRecomp *i) {
    uint8_t old_p;
    uint16_t vector;
    if (!i) return 0;
    if (i->cpu.e) return civ_fail_frontier(i,"Current static IRQ authority is native-mode only.",NULL);
    if (i->irq_depth!=0u) return civ_fail_frontier(i,"Nested native IRQ reached before finite nested-interrupt authority exists.",NULL);
    /* Hardware interrupt entry wastes two CPU cycles: first a real code read
       at the interrupted PC's bus speed, then one six-master-clock idle. */
    civ_cpu_data_access(i,((uint32_t)i->cpu.pbr<<16)|i->cpu.pc);
    if(i->failed)return 0;
    civ_cpu_internal_cycles(i,1u);
    old_p=i->cpu.p;
    if (!civ_push8(i,i->cpu.pbr)) return 0;
    if (!civ_push8(i,(uint8_t)(i->cpu.pc>>8))) return 0;
    if (!civ_push8(i,(uint8_t)i->cpu.pc)) return 0;
    if (!civ_push8(i,old_p)) return 0;
    i->cpu.p=civ_status_without((uint8_t)(old_p|CIV_P_I),CIV_P_D);
    if (!civ_bus_read16(i,0x00FFEEu,&vector)) return 0;
    i->cpu.pbr=0u;
    i->cpu.pc=vector;
    i->last_irq_vector=vector;
    i->irq_depth=1u;
    i->irq_accept_count++;
    i->irq_transition=0u;
    return 1;
}

int civ_nmi_enter_native(CivRecomp *i) {
    uint8_t old_p;
    uint16_t vector;
    if(!i)return 0;
    if(i->cpu.e)return civ_fail_frontier(i,"Current static NMI authority is native-mode only.",NULL);
    if(i->nmi_depth!=0u || i->irq_depth!=0u)return civ_fail_frontier(i,"Nested/overlapping interrupt reached outside current finite interrupt authority.",NULL);
    /* Same two wasted cycles as the real W65C816: code read at the current
       PC bus speed, then a six-master-clock idle cycle. */
    civ_cpu_data_access(i,((uint32_t)i->cpu.pbr<<16)|i->cpu.pc);
    if(i->failed)return 0;
    civ_cpu_internal_cycles(i,1u);
    old_p=i->cpu.p;
    if(!civ_push8(i,i->cpu.pbr))return 0;
    if(!civ_push8(i,(uint8_t)(i->cpu.pc>>8)))return 0;
    if(!civ_push8(i,(uint8_t)i->cpu.pc))return 0;
    if(!civ_push8(i,old_p))return 0;
    i->cpu.p=civ_status_without((uint8_t)(old_p|CIV_P_I),CIV_P_D);
    if(!civ_bus_read16(i,0x00FFEAu,&vector))return 0;
    i->cpu.pbr=0u;
    i->cpu.pc=vector;
    i->last_nmi_vector=vector;
    i->nmi_depth=1u;
    i->nmi_pending=0u;
    i->nmi_accept_count++;
    return 1;
}

int civ_rti_native(CivRecomp *i) {
    uint8_t p,lo,hi,bank;
    uint8_t returning_irq,returning_nmi;
    if (!i) return 0;
    if (i->cpu.e) return civ_fail_frontier(i,"Native RTI authority is native-mode only.",NULL);
    returning_irq=(uint8_t)(i->irq_depth!=0u);
    returning_nmi=(uint8_t)(i->nmi_depth!=0u);
    if((unsigned)returning_irq+(unsigned)returning_nmi!=1u)
        return civ_fail_frontier(i,"RTI reached without exactly one statically accepted native interrupt frame.",NULL);
    if (!civ_pull8(i,&p) || !civ_pull8(i,&lo) || !civ_pull8(i,&hi) || !civ_pull8(i,&bank)) return 0;
    i->cpu.p=p;
    if (i->cpu.p&CIV_P_X) { i->cpu.x&=0x00FFu; i->cpu.y&=0x00FFu; }
    i->cpu.pc=(uint16_t)(lo|((uint16_t)hi<<8));
    i->cpu.pbr=bank;
    if(returning_irq) { i->irq_depth--; i->irq_return_count++; }
    else { i->nmi_depth--; i->nmi_return_count++; }
    return 1;
}

int civ_push8(CivRecomp *i, uint8_t value) {
    uint32_t address;
    if (!i) return 0;
    address=(uint32_t)i->cpu.s;
    if (!civ_bus_write8(i,address,value)) return 0;
    if (i->cpu.e) i->cpu.s=(uint16_t)(0x0100u|((i->cpu.s-1u)&0x00FFu));
    else i->cpu.s=(uint16_t)(i->cpu.s-1u);
    return 1;
}
int civ_pull8(CivRecomp *i, uint8_t *value) {
    uint32_t address;
    if (!i || !value) return 0;
    if (i->cpu.e) i->cpu.s=(uint16_t)(0x0100u|((i->cpu.s+1u)&0x00FFu));
    else i->cpu.s=(uint16_t)(i->cpu.s+1u);
    address=(uint32_t)i->cpu.s;
    return civ_bus_read8(i,address,value);
}

int civ_block_move_step(CivRecomp *i, uint8_t destination_bank, uint8_t source_bank, int adjust, const char *address) {
    uint8_t data;
    uint16_t old_a;
    if (!i || (adjust!=1 && adjust!=-1)) return 0;
    i->cpu.dbr=destination_bank;
    if (!civ_bus_read8(i,((uint32_t)source_bank<<16)|i->cpu.x,&data)) return 0;
    if (!civ_bus_write8(i,((uint32_t)destination_bank<<16)|i->cpu.y,data)) return 0;
    if (i->cpu.p & CIV_P_X) {
        uint8_t x=(uint8_t)i->cpu.x, y=(uint8_t)i->cpu.y;
        x=(uint8_t)(x+adjust); y=(uint8_t)(y+adjust);
        i->cpu.x=(uint16_t)x;
        i->cpu.y=(uint16_t)y;
    } else {
        i->cpu.x=(uint16_t)(i->cpu.x+adjust);
        i->cpu.y=(uint16_t)(i->cpu.y+adjust);
    }
    old_a=i->cpu.a;
    i->cpu.a=(uint16_t)(i->cpu.a-1u);
    i->block_move_byte_count++;
    i->instruction_count++;
    if (old_a==0u) i->cpu.pc=(uint16_t)(i->cpu.pc+3u);
    /* otherwise PC remains on the fixed MVN/MVP instruction for the next byte */
    (void)address;
    return 1;
}

int civ_require_width(CivRecomp *i, unsigned e, unsigned m, unsigned x, const char *address) {
    unsigned actual_m=i->cpu.e?1u:(unsigned)((i->cpu.p&CIV_P_M)!=0u);
    unsigned actual_x=i->cpu.e?1u:(unsigned)((i->cpu.p&CIV_P_X)!=0u);
    if ((unsigned)i->cpu.e!=e || actual_m!=m || actual_x!=x) {
        char msg[192]; (void)snprintf(msg,sizeof(msg),"Generated context width mismatch at %s: expected E%uM%uX%u, got E%uM%uX%u.",address?address:"unknown",e,m,x,(unsigned)i->cpu.e,actual_m,actual_x);
        return civ_fail_frontier(i,msg,address);
    }
    return 1;
}
