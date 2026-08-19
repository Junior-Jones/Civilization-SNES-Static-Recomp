#!/usr/bin/env python3
"""Neutral offline W65C816 semantic emitter used by Civilization static generation.

This module contains instruction/addressing semantics only. It owns no graph traversal,
versioned route discovery, execution fallback, runtime opcode decoder, or ROM authority.
Generated C remains exact-address dispatched by the current closed-core generator.
"""

def dp_addr(op:int)->str:
    return f'(uint16_t)(i->cpu.d+0x{op:02X}u)'

def dp_addr_x(op:int)->str:
    return f'(uint16_t)(i->cpu.d+0x{op:02X}u+i->cpu.x)'

def db_addr(op:int)->str:
    return f'(((uint32_t)i->cpu.dbr<<16)|0x{op:04X}u)'


def db_addr_x(op:int)->str:
    return f'(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x{op:04X}u+i->cpu.x))'

def db_addr_y(op:int)->str:
    return f'(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x{op:04X}u+i->cpu.y))'


def sem(inst,c):
    m=inst.mnemonic;op=inst.operand;L=[];addr=f'{c.pbr:02X}:{c.pc:04X}'
    if m=='SEI': L.append('i->cpu.p |= CIV_P_I;')
    elif m=='CLI': L.append('i->cpu.p &= (uint8_t)~CIV_P_I;')
    elif m=='CLC': L.append('i->cpu.p &= (uint8_t)~CIV_P_C;')
    elif m=='SEC': L.append('i->cpu.p |= CIV_P_C;')
    elif m=='XCE': L += ['{ uint8_t old_e=i->cpu.e; uint8_t old_c=(uint8_t)((i->cpu.p&CIV_P_C)!=0u);','  i->cpu.e=old_c; if(old_e)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C;','  if(i->cpu.e){i->cpu.p|=(CIV_P_M|CIV_P_X);i->cpu.x&=0x00FFu;i->cpu.y&=0x00FFu;i->cpu.s=(uint16_t)(0x0100u|(i->cpu.s&0x00FFu));} }']
    elif m=='REP': L += [f'i->cpu.p &= (uint8_t)~0x{op:02X}u;','if(i->cpu.e)i->cpu.p|=(CIV_P_M|CIV_P_X);']
    elif m=='SEP': L += [f'i->cpu.p |= 0x{op:02X}u;','if(i->cpu.e)i->cpu.p|=(CIV_P_M|CIV_P_X);','if(i->cpu.p&CIV_P_X){i->cpu.x&=0x00FFu;i->cpu.y&=0x00FFu;}']
    elif m=='NOP': pass
    elif m=='LDA' and inst.mode=='dp_long_ind':
        if c.m:L += [f'{{ uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_read8(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint8_t p0,p1,p2; uint16_t v,b=(uint16_t)(i->cpu.d+0x{op:02X}u); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_read16(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='dp_long_ind_y':
        if c.m:L += [f'{{ uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint8_t p0,p1,p2; uint16_t v,b=(uint16_t)(i->cpu.d+0x{op:02X}u); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='dp_ind_y':
        if c.m:L += [f'{{ uint16_t ptr; uint8_t v; uint32_t a; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t ptr,v; uint32_t a; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='dp_ind':
        if c.m:L += [f'{{ uint16_t ptr; uint8_t v; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; if(!civ_bus_read8(i,((uint32_t)i->cpu.dbr<<16)|ptr,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t ptr,v; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; if(!civ_bus_read16(i,((uint32_t)i->cpu.dbr<<16)|ptr,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='sr':
        if c.m:L += [f'{{ uint8_t v; uint16_t a=(uint16_t)(i->cpu.s+0x{op:02X}u); if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v,a=(uint16_t)(i->cpu.s+0x{op:02X}u); if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='dp_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr_x(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr_x(op)},&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='imm_m':
        if c.m:L += [f'i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|0x{op:02X}u);','civ_set_nz8(i,(uint8_t)i->cpu.a);']
        else:L += [f'i->cpu.a=0x{op:04X}u;','civ_set_nz16(i,i->cpu.a);']
    elif m=='LDA' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='abs_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='abs_y':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_y(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_y(op)},&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='long_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDA' and inst.mode=='long':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,0x{op:06X}u,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,0x{op:06X}u,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }}']
    elif m=='LDX' and inst.mode=='imm_x':
        if c.x:L += [f'i->cpu.x=(uint16_t)0x{op:02X}u;','civ_set_nz8(i,(uint8_t)i->cpu.x);']
        else:L += [f'i->cpu.x=0x{op:04X}u;','civ_set_nz16(i,i->cpu.x);']
    elif m=='LDX' and inst.mode=='dp':
        if c.x:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; i->cpu.x=v; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; i->cpu.x=v; civ_set_nz16(i,v); }}']
    elif m=='LDX' and inst.mode=='abs':
        if c.x:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; i->cpu.x=v; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; i->cpu.x=v; civ_set_nz16(i,v); }}']
    elif m=='LDY' and inst.mode=='dp':
        if c.x:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; i->cpu.y=v; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; i->cpu.y=v; civ_set_nz16(i,v); }}']
    elif m=='LDY' and inst.mode=='abs':
        if c.x:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; i->cpu.y=v; civ_set_nz8(i,(uint8_t)i->cpu.y); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; i->cpu.y=v; civ_set_nz16(i,i->cpu.y); }}']
    elif m=='LDY' and inst.mode=='abs_x':
        if c.x:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; i->cpu.y=v; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; i->cpu.y=v; civ_set_nz16(i,v); }}']
    elif m=='LDY' and inst.mode=='imm_x':
        if c.x:L += [f'i->cpu.y=(uint16_t)0x{op:02X}u;','civ_set_nz8(i,(uint8_t)i->cpu.y);']
        else:L += [f'i->cpu.y=0x{op:04X}u;','civ_set_nz16(i,i->cpu.y);']
    elif m=='STY' and inst.mode=='dp':
        if c.x:L.append(f'if(!civ_bus_write8(i,{dp_addr(op)},(uint8_t)i->cpu.y))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{dp_addr(op)},i->cpu.y))return 0;')
    elif m=='STA' and inst.mode=='dp_long_ind':
        if c.m:L += [f'{{ uint8_t p0,p1,p2; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_write8(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),(uint8_t)i->cpu.a))return 0; }}']
        else:L += [f'{{ uint8_t p0,p1,p2; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_write16(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),i->cpu.a))return 0; }}']
    elif m=='STA' and inst.mode=='dp_long_ind_y':
        if c.m:L += [f'{{ uint8_t p0,p1,p2; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_write8(i,a,(uint8_t)i->cpu.a))return 0; }}']
        else:L += [f'{{ uint8_t p0,p1,p2; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_write16(i,a,i->cpu.a))return 0; }}']
    elif m=='STA' and inst.mode=='dp_ind_y':
        if c.m:L += [f'{{ uint16_t ptr; uint32_t a; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_write8(i,a,(uint8_t)i->cpu.a))return 0; }}']
        else:L += [f'{{ uint16_t ptr; uint32_t a; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_write16(i,a,i->cpu.a))return 0; }}']
    elif m=='STA' and inst.mode=='dp_ind':
        if c.m:L += [f'{{ uint16_t ptr; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; if(!civ_bus_write8(i,((uint32_t)i->cpu.dbr<<16)|ptr,(uint8_t)i->cpu.a))return 0; }}']
        else:L += [f'{{ uint16_t ptr; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; if(!civ_bus_write16(i,((uint32_t)i->cpu.dbr<<16)|ptr,i->cpu.a))return 0; }}']
    elif m=='STA' and inst.mode=='dp_x':
        if c.m:L.append(f'if(!civ_bus_write8(i,{dp_addr_x(op)},(uint8_t)i->cpu.a))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{dp_addr_x(op)},i->cpu.a))return 0;')
    elif m=='STA' and inst.mode=='sr':
        if c.m:L.append(f'if(!civ_bus_write8(i,(uint16_t)(i->cpu.s+0x{op:02X}u),(uint8_t)i->cpu.a))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,(uint16_t)(i->cpu.s+0x{op:02X}u),i->cpu.a))return 0;')
    elif m=='STA' and inst.mode=='dp':
        if c.m:L.append(f'if(!civ_bus_write8(i,{dp_addr(op)},(uint8_t)i->cpu.a))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{dp_addr(op)},i->cpu.a))return 0;')
    elif m=='STX' and inst.mode=='dp':
        if c.x:L.append(f'if(!civ_bus_write8(i,{dp_addr(op)},(uint8_t)i->cpu.x))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{dp_addr(op)},i->cpu.x))return 0;')
    elif m=='STX' and inst.mode=='abs':
        if c.x:L.append(f'if(!civ_bus_write8(i,{db_addr(op)},(uint8_t)i->cpu.x))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{db_addr(op)},i->cpu.x))return 0;')
    elif m=='STA' and inst.mode=='long':
        if c.m:L.append(f'if(!civ_bus_write8(i,0x{op:06X}u,(uint8_t)i->cpu.a))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,0x{op:06X}u,i->cpu.a))return 0;')
    elif m=='STA' and inst.mode=='abs':
        if c.m:L.append(f'if(!civ_bus_write8(i,{db_addr(op)},(uint8_t)i->cpu.a))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{db_addr(op)},i->cpu.a))return 0;')
    elif m=='STA' and inst.mode=='abs_x':
        if c.m:L.append(f'if(!civ_bus_write8(i,{db_addr_x(op)},(uint8_t)i->cpu.a))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{db_addr_x(op)},i->cpu.a))return 0;')
    elif m=='STA' and inst.mode=='abs_y':
        if c.m:L.append(f'if(!civ_bus_write8(i,{db_addr_y(op)},(uint8_t)i->cpu.a))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{db_addr_y(op)},i->cpu.a))return 0;')
    elif m=='STA' and inst.mode=='long_x':
        if c.m:L.append(f'if(!civ_bus_write8(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,(uint8_t)i->cpu.a))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,i->cpu.a))return 0;')
    elif m=='STY' and inst.mode=='abs':
        if c.x:L.append(f'if(!civ_bus_write8(i,{db_addr(op)},(uint8_t)i->cpu.y))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{db_addr(op)},i->cpu.y))return 0;')
    elif m=='STZ' and inst.mode=='dp':
        if c.m:L.append(f'if(!civ_bus_write8(i,{dp_addr(op)},0u))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{dp_addr(op)},0u))return 0;')
    elif m=='STZ' and inst.mode=='dp_x':
        if c.m:L.append(f'if(!civ_bus_write8(i,{dp_addr_x(op)},0u))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{dp_addr_x(op)},0u))return 0;')
    elif m=='STZ' and inst.mode=='abs':
        if c.m:L.append(f'if(!civ_bus_write8(i,{db_addr(op)},0u))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{db_addr(op)},0u))return 0;')
    elif m=='STZ' and inst.mode=='abs_x':
        if c.m:L.append(f'if(!civ_bus_write8(i,{db_addr_x(op)},0u))return 0;')
        else:L.append(f'if(!civ_bus_write16(i,{db_addr_x(op)},0u))return 0;')
    elif m=='CMP' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; civ_cmp16(i,i->cpu.a,v); }}']
    elif m=='CMP' and inst.mode=='dp_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr_x(op)},&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr_x(op)},&v))return 0; civ_cmp16(i,i->cpu.a,v); }}']
    elif m=='CMP' and inst.mode=='sr':
        if c.m:L += [f'{{ uint8_t v; uint16_t a=(uint16_t)(i->cpu.s+0x{op:02X}u); if(!civ_bus_read8(i,a,&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }}']
        else:L += [f'{{ uint16_t v,a=(uint16_t)(i->cpu.s+0x{op:02X}u); if(!civ_bus_read16(i,a,&v))return 0; civ_cmp16(i,i->cpu.a,v); }}']
    elif m=='CMP' and inst.mode=='imm_m':
        if c.m:L.append(f'civ_cmp8(i,(uint8_t)i->cpu.a,0x{op:02X}u);')
        else:L.append(f'civ_cmp16(i,i->cpu.a,0x{op:04X}u);')
    elif m=='CMP' and inst.mode=='long':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,0x{op:06X}u,&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,0x{op:06X}u,&v))return 0; civ_cmp16(i,i->cpu.a,v); }}']
    elif m=='CMP' and inst.mode=='long_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; civ_cmp16(i,i->cpu.a,v); }}']
    elif m=='CMP' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; civ_cmp16(i,i->cpu.a,v); }}']
    elif m=='CMP' and inst.mode=='abs_y':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_y(op)},&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_y(op)},&v))return 0; civ_cmp16(i,i->cpu.a,v); }}']
    elif m=='CMP' and inst.mode=='abs_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; civ_cmp16(i,i->cpu.a,v); }}']
    elif m=='CPX' and inst.mode=='imm_x':
        if c.x:L.append(f'civ_cmp8(i,(uint8_t)i->cpu.x,0x{op:02X}u);')
        else:L.append(f'civ_cmp16(i,i->cpu.x,0x{op:04X}u);')
    elif m=='CPX' and inst.mode=='dp':
        if c.x:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.x,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; civ_cmp16(i,i->cpu.x,v); }}']
    elif m=='CPX' and inst.mode=='abs':
        if c.x:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.x,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; civ_cmp16(i,i->cpu.x,v); }}']
    elif m=='CPY' and inst.mode=='imm_x':
        if c.x:L += [f'{{ uint8_t y=(uint8_t)i->cpu.y,v=0x{op:02X}u,r=(uint8_t)(y-v); if(y>=v)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; civ_set_nz8(i,r); }}']
        else:L += [f'{{ uint16_t y=i->cpu.y,v=0x{op:04X}u,r=(uint16_t)(y-v); if(y>=v)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; civ_set_nz16(i,r); }}']
    elif m=='CPY' and inst.mode=='dp':
        if c.x:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.y,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; civ_cmp16(i,i->cpu.y,v); }}']
    elif m=='CPY' and inst.mode=='abs':
        if c.x:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.y,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; civ_cmp16(i,i->cpu.y,v); }}']
    elif m=='BIT' and inst.mode=='imm_m':
        if c.m:L += [f'if(((uint8_t)i->cpu.a&0x{op:02X}u)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z;']
        else:L += [f'if((i->cpu.a&0x{op:04X}u)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z;']
    elif m=='BIT' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; if(((uint8_t)i->cpu.a&v)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; i->cpu.p=(uint8_t)((i->cpu.p&~(CIV_P_N|CIV_P_V))|(v&(CIV_P_N|CIV_P_V))); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; if((i->cpu.a&v)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; i->cpu.p=(uint8_t)((i->cpu.p&~(CIV_P_N|CIV_P_V))|((v&0x8000u)?CIV_P_N:0u)|((v&0x4000u)?CIV_P_V:0u)); }}']
    elif m=='BIT' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; if(((uint8_t)i->cpu.a&v)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; i->cpu.p=(uint8_t)((i->cpu.p&~(CIV_P_N|CIV_P_V))|(v&(CIV_P_N|CIV_P_V))); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; if((i->cpu.a&v)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; i->cpu.p=(uint8_t)((i->cpu.p&~(CIV_P_N|CIV_P_V))|((v&0x8000u)?CIV_P_N:0u)|((v&0x4000u)?CIV_P_V:0u)); }}']
    elif m=='AND' and inst.mode=='dp_long_ind_y':
        if c.m:L += [f'{{ uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint8_t p0,p1,p2; uint16_t v,b=(uint16_t)(i->cpu.d+0x{op:02X}u); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='AND' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='AND' and inst.mode=='dp_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr_x(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr_x(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='AND' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='AND' and inst.mode=='abs_y':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_y(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_y(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='AND' and inst.mode=='abs_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='AND' and inst.mode=='long':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,0x{op:06X}u,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,0x{op:06X}u,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='AND' and inst.mode=='long_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='AND' and inst.mode=='imm_m':
        if c.m:L += [f'i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&0x{op:02X}u));','civ_set_nz8(i,(uint8_t)i->cpu.a);']
        else:L += [f'i->cpu.a=(uint16_t)(i->cpu.a&0x{op:04X}u);','civ_set_nz16(i,i->cpu.a);']
    elif m=='EOR' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a^v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a^v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='EOR' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a^v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a^v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='EOR' and inst.mode=='abs_y':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_y(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a^v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_y(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a^v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='EOR' and inst.mode=='abs_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a^v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a^v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='EOR' and inst.mode=='long_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a^v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a^v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='EOR' and inst.mode=='imm_m':
        if c.m:L += [f'i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a^0x{op:02X}u));','civ_set_nz8(i,(uint8_t)i->cpu.a);']
        else:L += [f'i->cpu.a=(uint16_t)(i->cpu.a^0x{op:04X}u);','civ_set_nz16(i,i->cpu.a);']
    elif m=='TSB' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v,a=(uint8_t)i->cpu.a; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; if((uint8_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint8_t)(v|a); if(!civ_bus_write8(i,{dp_addr(op)},v))return 0; }}']
        else:L += [f'{{ uint16_t v,a=i->cpu.a; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; if((uint16_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint16_t)(v|a); if(!civ_bus_write16(i,{dp_addr(op)},v))return 0; }}']
    elif m=='TSB' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v,a=(uint8_t)i->cpu.a; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; if((uint8_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint8_t)(v|a); if(!civ_bus_write8(i,{db_addr(op)},v))return 0; }}']
        else:L += [f'{{ uint16_t v,a=i->cpu.a; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; if((uint16_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint16_t)(v|a); if(!civ_bus_write16(i,{db_addr(op)},v))return 0; }}']
    elif m=='TRB' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v,a=(uint8_t)i->cpu.a; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; if((uint8_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint8_t)(v&~a); if(!civ_bus_write8(i,{dp_addr(op)},v))return 0; }}']
        else:L += [f'{{ uint16_t v,a=i->cpu.a; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; if((uint16_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint16_t)(v&~a); if(!civ_bus_write16(i,{dp_addr(op)},v))return 0; }}']
    elif m=='TRB' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v,a=(uint8_t)i->cpu.a; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; if((uint8_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint8_t)(v&~a); if(!civ_bus_write8(i,{db_addr(op)},v))return 0; }}']
        else:L += [f'{{ uint16_t v,a=i->cpu.a; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; if((uint16_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint16_t)(v&~a); if(!civ_bus_write16(i,{db_addr(op)},v))return 0; }}']
    elif m=='ORA' and inst.mode=='dp_long_ind':
        if c.m:L += [f'{{ uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_read8(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint8_t p0,p1,p2; uint16_t v,b=(uint16_t)(i->cpu.d+0x{op:02X}u); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_read16(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='ORA' and inst.mode=='dp_long_ind_y':
        if c.m:L += [f'{{ uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint8_t p0,p1,p2; uint16_t v,b=(uint16_t)(i->cpu.d+0x{op:02X}u); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='ORA' and inst.mode=='dp_ind_y':
        if c.m:L += [f'{{ uint16_t ptr; uint8_t v; uint32_t a; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t ptr,v; uint32_t a; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='ORA' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='ORA' and inst.mode=='sr':
        if c.m:L += [f'{{ uint8_t v; uint16_t a=(uint16_t)(i->cpu.s+0x{op:02X}u); if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v,a=(uint16_t)(i->cpu.s+0x{op:02X}u); if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='ORA' and inst.mode=='imm_m':
        if c.m:L += [f'i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|0x{op:02X}u));','civ_set_nz8(i,(uint8_t)i->cpu.a);']
        else:L += [f'i->cpu.a=(uint16_t)(i->cpu.a|0x{op:04X}u);','civ_set_nz16(i,i->cpu.a);']
    elif m=='ORA' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='ORA' and inst.mode=='abs_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='ORA' and inst.mode=='abs_y':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_y(op)},&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_y(op)},&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='ORA' and inst.mode=='long':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,0x{op:06X}u,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,0x{op:06X}u,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='ORA' and inst.mode=='long_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }}']
    elif m=='ASL' and inst.mode=='acc':
        if c.m:L += ['{ uint8_t v=(uint8_t)i->cpu.a; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v<<1); i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }']
        else:L += ['{ uint16_t v=i->cpu.a; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v<<1); i->cpu.a=v; civ_set_nz16(i,v); }']
    elif m=='ASL' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v<<1); if(!civ_bus_write8(i,{dp_addr(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v<<1); if(!civ_bus_write16(i,{dp_addr(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='ASL' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v<<1); if(!civ_bus_write8(i,{db_addr(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v<<1); if(!civ_bus_write16(i,{db_addr(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='LSR' and inst.mode=='acc':
        if c.m:L += ['{ uint8_t v=(uint8_t)i->cpu.a; if(v&0x01u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v>>1); i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }']
        else:L += ['{ uint16_t v=i->cpu.a; if(v&0x0001u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v>>1); i->cpu.a=v; civ_set_nz16(i,v); }']
    elif m=='LSR' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; if(v&0x01u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v>>1); if(!civ_bus_write8(i,{db_addr(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; if(v&0x0001u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v>>1); if(!civ_bus_write16(i,{db_addr(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='LSR' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; if(v&0x01u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v>>1); if(!civ_bus_write8(i,{dp_addr(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; if(v&0x0001u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v>>1); if(!civ_bus_write16(i,{dp_addr(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='ROL' and inst.mode=='acc':
        if c.m:L += ['{ uint8_t v=(uint8_t)i->cpu.a; uint8_t cin=(uint8_t)((i->cpu.p&CIV_P_C)!=0u); if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)((v<<1)|cin); i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }']
        else:L += ['{ uint16_t v=i->cpu.a; uint16_t cin=(uint16_t)((i->cpu.p&CIV_P_C)!=0u); if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)((v<<1)|cin); i->cpu.a=v; civ_set_nz16(i,v); }']
    elif m=='ROL' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v,cin=(uint8_t)((i->cpu.p&CIV_P_C)!=0u); if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)((v<<1)|cin); if(!civ_bus_write8(i,{db_addr(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v,cin=(uint16_t)((i->cpu.p&CIV_P_C)!=0u); if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)((v<<1)|cin); if(!civ_bus_write16(i,{db_addr(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='ROL' and inst.mode=='abs_x':
        if c.m:L += [f'{{ uint8_t v,cin=(uint8_t)((i->cpu.p&CIV_P_C)!=0u); if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)((v<<1)|cin); if(!civ_bus_write8(i,{db_addr_x(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v,cin=(uint16_t)((i->cpu.p&CIV_P_C)!=0u); if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)((v<<1)|cin); if(!civ_bus_write16(i,{db_addr_x(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='ROR' and inst.mode=='acc':
        if c.m:L += ['{ uint8_t v=(uint8_t)i->cpu.a; uint8_t cin=(uint8_t)((i->cpu.p&CIV_P_C)?0x80u:0u); if(v&0x01u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)((v>>1)|cin); i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }']
        else:L += ['{ uint16_t v=i->cpu.a; uint16_t cin=(uint16_t)((i->cpu.p&CIV_P_C)?0x8000u:0u); if(v&0x0001u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)((v>>1)|cin); i->cpu.a=v; civ_set_nz16(i,v); }']
    elif m=='XBA':
        L += ['i->cpu.a=(uint16_t)((i->cpu.a<<8)|(i->cpu.a>>8));','civ_set_nz8(i,(uint8_t)i->cpu.a);']
    elif m=='DEC' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; v=(uint8_t)(v-1u); if(!civ_bus_write8(i,{dp_addr(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; v=(uint16_t)(v-1u); if(!civ_bus_write16(i,{dp_addr(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='DEC' and inst.mode=='dp_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr_x(op)},&v))return 0; v=(uint8_t)(v-1u); if(!civ_bus_write8(i,{dp_addr_x(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr_x(op)},&v))return 0; v=(uint16_t)(v-1u); if(!civ_bus_write16(i,{dp_addr_x(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='DEC' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; v=(uint8_t)(v-1u); if(!civ_bus_write8(i,{db_addr(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; v=(uint16_t)(v-1u); if(!civ_bus_write16(i,{db_addr(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='DEC' and inst.mode=='abs_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; v=(uint8_t)(v-1u); if(!civ_bus_write8(i,{db_addr_x(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; v=(uint16_t)(v-1u); if(!civ_bus_write16(i,{db_addr_x(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='DEC' and inst.mode=='acc':
        if c.m:L += ['i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|(uint8_t)((uint8_t)i->cpu.a-1u));','civ_set_nz8(i,(uint8_t)i->cpu.a);']
        else:L += ['i->cpu.a=(uint16_t)(i->cpu.a-1u);','civ_set_nz16(i,i->cpu.a);']
    elif m=='INC' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; v=(uint8_t)(v+1u); if(!civ_bus_write8(i,{dp_addr(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; v=(uint16_t)(v+1u); if(!civ_bus_write16(i,{dp_addr(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='INC' and inst.mode=='dp_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr_x(op)},&v))return 0; v=(uint8_t)(v+1u); if(!civ_bus_write8(i,{dp_addr_x(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr_x(op)},&v))return 0; v=(uint16_t)(v+1u); if(!civ_bus_write16(i,{dp_addr_x(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='INC' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; v=(uint8_t)(v+1u); if(!civ_bus_write8(i,{db_addr(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; v=(uint16_t)(v+1u); if(!civ_bus_write16(i,{db_addr(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='INC' and inst.mode=='abs_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; v=(uint8_t)(v+1u); if(!civ_bus_write8(i,{db_addr_x(op)},v))return 0; civ_set_nz8(i,v); }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; v=(uint16_t)(v+1u); if(!civ_bus_write16(i,{db_addr_x(op)},v))return 0; civ_set_nz16(i,v); }}']
    elif m=='INC' and inst.mode=='acc':
        if c.m:L += ['i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|(uint8_t)((uint8_t)i->cpu.a+1u));','civ_set_nz8(i,(uint8_t)i->cpu.a);']
        else:L += ['i->cpu.a=(uint16_t)(i->cpu.a+1u);','civ_set_nz16(i,i->cpu.a);']
    elif m=='SBC' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; if(!civ_sbc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; if(!civ_sbc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='SBC' and inst.mode=='sr':
        if c.m:L += [f'{{ uint8_t v; uint16_t a=(uint16_t)(i->cpu.s+0x{op:02X}u); if(!civ_bus_read8(i,a,&v))return 0; if(!civ_sbc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v,a=(uint16_t)(i->cpu.s+0x{op:02X}u); if(!civ_bus_read16(i,a,&v))return 0; if(!civ_sbc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='SBC' and inst.mode=='imm_m':
        if c.m:L.append(f'if(!civ_sbc8_binary(i,0x{op:02X}u,"{addr}"))return 0;')
        else:L.append(f'if(!civ_sbc16_binary(i,0x{op:04X}u,"{addr}"))return 0;')
    elif m=='ADC' and inst.mode=='dp_long_ind_y':
        if c.m:L += [f'{{ uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read8(i,a,&v))return 0; if(!civ_adc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint8_t p0,p1,p2; uint16_t b=(uint16_t)(i->cpu.d+0x{op:02X}u),v; uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read16(i,a,&v))return 0; if(!civ_adc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='ADC' and inst.mode=='sr':
        if c.m:L += [f'{{ uint8_t v; uint16_t a=(uint16_t)(i->cpu.s+0x{op:02X}u); if(!civ_bus_read8(i,a,&v))return 0; if(!civ_adc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v,a=(uint16_t)(i->cpu.s+0x{op:02X}u); if(!civ_bus_read16(i,a,&v))return 0; if(!civ_adc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='ADC' and inst.mode=='dp_ind':
        if c.m:L += [f'{{ uint16_t ptr; uint8_t v; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; if(!civ_bus_read8(i,((uint32_t)i->cpu.dbr<<16)|ptr,&v))return 0; if(!civ_adc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t ptr,v; if(!civ_bus_read16(i,{dp_addr(op)},&ptr))return 0; if(!civ_bus_read16(i,((uint32_t)i->cpu.dbr<<16)|ptr,&v))return 0; if(!civ_adc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='ADC' and inst.mode=='dp':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{dp_addr(op)},&v))return 0; if(!civ_adc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{dp_addr(op)},&v))return 0; if(!civ_adc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='ADC' and inst.mode=='imm_m':
        if c.m:L.append(f'if(!civ_adc8_binary(i,0x{op:02X}u,"{addr}"))return 0;')
        else:L.append(f'if(!civ_adc16_binary(i,0x{op:04X}u,"{addr}"))return 0;')
    elif m=='ADC' and inst.mode=='long':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,0x{op:06X}u,&v))return 0; if(!civ_adc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,0x{op:06X}u,&v))return 0; if(!civ_adc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='ADC' and inst.mode=='long_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; if(!civ_adc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; if(!civ_adc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='ADC' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; if(!civ_adc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; if(!civ_adc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='ADC' and inst.mode=='abs_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; if(!civ_adc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; if(!civ_adc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='SBC' and inst.mode=='abs':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr(op)},&v))return 0; if(!civ_sbc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr(op)},&v))return 0; if(!civ_sbc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='SBC' and inst.mode=='long':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,0x{op:06X}u,&v))return 0; if(!civ_sbc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,0x{op:06X}u,&v))return 0; if(!civ_sbc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='SBC' and inst.mode=='abs_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,{db_addr_x(op)},&v))return 0; if(!civ_sbc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,{db_addr_x(op)},&v))return 0; if(!civ_sbc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='SBC' and inst.mode=='long_x':
        if c.m:L += [f'{{ uint8_t v; if(!civ_bus_read8(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; if(!civ_sbc8_binary(i,v,"{addr}"))return 0; }}']
        else:L += [f'{{ uint16_t v; if(!civ_bus_read16(i,(0x{op:06X}u+i->cpu.x)&0xFFFFFFu,&v))return 0; if(!civ_sbc16_binary(i,v,"{addr}"))return 0; }}']
    elif m=='TCS': L.append('i->cpu.s=i->cpu.e?(uint16_t)(0x0100u|(i->cpu.a&0x00FFu)):i->cpu.a;')
    elif m=='TXS': L.append('i->cpu.s=i->cpu.e?(uint16_t)(0x0100u|(i->cpu.x&0x00FFu)):i->cpu.x;')
    elif m=='TSC': L += ['i->cpu.a=i->cpu.s;','civ_set_nz16(i,i->cpu.a);']
    elif m=='TSX':
        if c.x:L += ['i->cpu.x=(uint8_t)i->cpu.s;','civ_set_nz8(i,(uint8_t)i->cpu.x);']
        else:L += ['i->cpu.x=i->cpu.s;','civ_set_nz16(i,i->cpu.x);']
    elif m=='TCD': L += ['i->cpu.d=i->cpu.a;','civ_set_nz16(i,i->cpu.d);']
    elif m=='TXA':
        if c.m:L += ['i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|(uint8_t)i->cpu.x);','civ_set_nz8(i,(uint8_t)i->cpu.a);']
        else:L += ['i->cpu.a=i->cpu.x;','civ_set_nz16(i,i->cpu.a);']
    elif m=='TAX':
        if c.x:L += ['i->cpu.x=(uint8_t)i->cpu.a;','civ_set_nz8(i,(uint8_t)i->cpu.x);']
        else:L += ['i->cpu.x=i->cpu.a;','civ_set_nz16(i,i->cpu.x);']
    elif m=='TAY':
        if c.x:L += ['i->cpu.y=(uint8_t)i->cpu.a;','civ_set_nz8(i,(uint8_t)i->cpu.y);']
        else:L += ['i->cpu.y=i->cpu.a;','civ_set_nz16(i,i->cpu.y);']
    elif m=='TYA':
        if c.m:L += ['i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|(uint8_t)i->cpu.y);','civ_set_nz8(i,(uint8_t)i->cpu.a);']
        else:L += ['i->cpu.a=i->cpu.y;','civ_set_nz16(i,i->cpu.a);']
    elif m=='TXY':
        if c.x:L += ['i->cpu.y=(uint8_t)i->cpu.x;','civ_set_nz8(i,(uint8_t)i->cpu.y);']
        else:L += ['i->cpu.y=i->cpu.x;','civ_set_nz16(i,i->cpu.y);']
    elif m=='TYX':
        if c.x:L += ['i->cpu.x=(uint8_t)i->cpu.y;','civ_set_nz8(i,(uint8_t)i->cpu.x);']
        else:L += ['i->cpu.x=i->cpu.y;','civ_set_nz16(i,i->cpu.x);']
    elif m=='INX':
        if c.x:L += ['i->cpu.x=(uint8_t)((uint8_t)i->cpu.x+1u);','civ_set_nz8(i,(uint8_t)i->cpu.x);']
        else:L += ['i->cpu.x=(uint16_t)(i->cpu.x+1u);','civ_set_nz16(i,i->cpu.x);']
    elif m=='DEX':
        if c.x:L += ['i->cpu.x=(uint8_t)((uint8_t)i->cpu.x-1u);','civ_set_nz8(i,(uint8_t)i->cpu.x);']
        else:L += ['i->cpu.x=(uint16_t)(i->cpu.x-1u);','civ_set_nz16(i,i->cpu.x);']
    elif m=='DEY':
        if c.x:L += ['i->cpu.y=(uint8_t)((uint8_t)i->cpu.y-1u);','civ_set_nz8(i,(uint8_t)i->cpu.y);']
        else:L += ['i->cpu.y=(uint16_t)(i->cpu.y-1u);','civ_set_nz16(i,i->cpu.y);']
    elif m=='INY':
        if c.x:L += ['i->cpu.y=(uint8_t)((uint8_t)i->cpu.y+1u);','civ_set_nz8(i,(uint8_t)i->cpu.y);']
        else:L += ['i->cpu.y=(uint16_t)(i->cpu.y+1u);','civ_set_nz16(i,i->cpu.y);']
    elif m=='PHA':
        if c.m:L.append('if(!civ_push8(i,(uint8_t)i->cpu.a))return 0;')
        else:L += ['if(!civ_push8(i,(uint8_t)(i->cpu.a>>8)))return 0;','if(!civ_push8(i,(uint8_t)i->cpu.a))return 0;']
    elif m=='PLA':
        if c.m:L += ['{ uint8_t v; if(!civ_pull8(i,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }']
        else:L += ['{ uint8_t lo,hi; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0; i->cpu.a=(uint16_t)(lo|((uint16_t)hi<<8)); civ_set_nz16(i,i->cpu.a); }']
    elif m=='PHX':
        if c.x:L.append('if(!civ_push8(i,(uint8_t)i->cpu.x))return 0;')
        else:L += ['if(!civ_push8(i,(uint8_t)(i->cpu.x>>8)))return 0;','if(!civ_push8(i,(uint8_t)i->cpu.x))return 0;']
    elif m=='PHY':
        if c.x:L.append('if(!civ_push8(i,(uint8_t)i->cpu.y))return 0;')
        else:L += ['if(!civ_push8(i,(uint8_t)(i->cpu.y>>8)))return 0;','if(!civ_push8(i,(uint8_t)i->cpu.y))return 0;']
    elif m=='PLX':
        if c.x:L += ['{ uint8_t v; if(!civ_pull8(i,&v))return 0; i->cpu.x=v; civ_set_nz8(i,v); }']
        else:L += ['{ uint8_t lo,hi; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0; i->cpu.x=(uint16_t)(lo|((uint16_t)hi<<8)); civ_set_nz16(i,i->cpu.x); }']
    elif m=='PLY':
        if c.x:L += ['{ uint8_t v; if(!civ_pull8(i,&v))return 0; i->cpu.y=v; civ_set_nz8(i,v); }']
        else:L += ['{ uint8_t lo,hi; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0; i->cpu.y=(uint16_t)(lo|((uint16_t)hi<<8)); civ_set_nz16(i,i->cpu.y); }']
    elif m=='PEA': L += [f'if(!civ_push8(i,0x{(op>>8)&0xff:02X}u))return 0;',f'if(!civ_push8(i,0x{op&0xff:02X}u))return 0;']
    elif m=='PHD': L += ['if(!civ_push8(i,(uint8_t)(i->cpu.d>>8)))return 0;','if(!civ_push8(i,(uint8_t)i->cpu.d))return 0;']
    elif m=='PHB': L.append('if(!civ_push8(i,i->cpu.dbr))return 0;')
    elif m=='PHK': L.append('if(!civ_push8(i,i->cpu.pbr))return 0;')
    elif m=='PHP': L.append('if(!civ_push8(i,i->cpu.p))return 0;')
    elif m=='PLD': L += ['{ uint8_t lo,hi; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0; i->cpu.d=(uint16_t)(lo|((uint16_t)hi<<8)); civ_set_nz16(i,i->cpu.d); }']
    elif m=='PLB': L += ['{ uint8_t v; if(!civ_pull8(i,&v))return 0; i->cpu.dbr=v; civ_set_nz8(i,v); }']
    elif m=='PLP': L += ['{ uint8_t v; if(!civ_pull8(i,&v))return 0; i->cpu.p=v;', '  if(i->cpu.e){i->cpu.p|=(CIV_P_M|CIV_P_X);}', '  if(i->cpu.p&CIV_P_X){i->cpu.x&=0x00FFu;i->cpu.y&=0x00FFu;} }']
    elif m in {'JMP','JML','BEQ','BNE','BCC','BCS','BVS','BMI','BPL','BRA','BRL','MVN','MVP','JSL','RTL','JSR','RTS','RTI'}: pass
    else: raise RuntimeError(f'unsupported base W65C816 emitter instruction {inst.text} at {c.pbr:02X}:{c.pc:04X}')
    # Target-qualified rendezvous timing retained from the proved Civilization route.
    # These two immutable wait-loop contexts receive nominal time only when the
    # natural timing scheduler is disabled; this is semantic support, not graph authority.
    if (c.pbr,c.pc)==(0xD4,0x8280): L.append('if(!i->natural_timing_enabled)civ_timing_advance_master(i,8u);')
    if (c.pbr,c.pc)==(0xD4,0x8283): L.append('if(!i->natural_timing_enabled)civ_timing_advance_master(i,6u);')
    return L

__all__ = ['sem']
