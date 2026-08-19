#!/usr/bin/env python3
"""Current Civilization W65C816 C-semantics extension wrapper.

The neutral base module owns shared instruction semantics only. This wrapper adds the
four addressing forms first required by the later closed Civilization graph. All
emission is offline; generated C performs no runtime opcode decoding.
"""
from w65c816_base_semantics import sem as _base_sem


def _dp_x(op: int) -> str:
    return f'(uint16_t)(i->cpu.d+0x{op:02X}u+i->cpu.x)'


def _dp(op: int) -> str:
    return f'(uint16_t)(i->cpu.d+0x{op:02X}u)'


def _db_y(op: int) -> str:
    return f'(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x{op:04X}u+i->cpu.y))'


def _db_x(op: int) -> str:
    return f'(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x{op:04X}u+i->cpu.x))'


def sem(inst, c):
    try:
        return _base_sem(inst, c)
    except RuntimeError as exc:
        m, mode, op = inst.mnemonic, inst.mode, inst.operand
        addr = f'{c.pbr:02X}:{c.pc:04X}'
        if m == 'ADC' and mode == 'abs_y':
            if c.m:
                return [f'{{ uint8_t v; if(!civ_bus_read8(i,{_db_y(op)},&v))return 0; if(!civ_adc8_binary(i,v,"{addr}"))return 0; }}']
            return [f'{{ uint16_t v; if(!civ_bus_read16(i,{_db_y(op)},&v))return 0; if(!civ_adc16_binary(i,v,"{addr}"))return 0; }}']
        if m == 'ORA' and mode == 'dp_x':
            if c.m:
                return [f'{{ uint8_t v; if(!civ_bus_read8(i,{_dp_x(op)},&v))return 0; v=(uint8_t)((uint8_t)i->cpu.a|v); i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }}']
            return [f'{{ uint16_t v; if(!civ_bus_read16(i,{_dp_x(op)},&v))return 0; v=(uint16_t)(i->cpu.a|v); i->cpu.a=v; civ_set_nz16(i,v); }}']
        if m == 'LSR' and mode == 'abs_x':
            if c.m:
                return [f'{{ uint8_t v; if(!civ_bus_read8(i,{_db_x(op)},&v))return 0; if(v&0x01u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v>>1); if(!civ_bus_write8(i,{_db_x(op)},v))return 0; civ_set_nz8(i,v); }}']
            return [f'{{ uint16_t v; if(!civ_bus_read16(i,{_db_x(op)},&v))return 0; if(v&0x0001u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v>>1); if(!civ_bus_write16(i,{_db_x(op)},v))return 0; civ_set_nz16(i,v); }}']
        if m == 'ROL' and mode == 'dp':
            if c.m:
                return [f'{{ uint8_t v,cin=(uint8_t)((i->cpu.p&CIV_P_C)!=0u); if(!civ_bus_read8(i,{_dp(op)},&v))return 0; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)((v<<1)|cin); if(!civ_bus_write8(i,{_dp(op)},v))return 0; civ_set_nz8(i,v); }}']
            return [f'{{ uint16_t v,cin=(uint16_t)((i->cpu.p&CIV_P_C)!=0u); if(!civ_bus_read16(i,{_dp(op)},&v))return 0; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)((v<<1)|cin); if(!civ_bus_write16(i,{_dp(op)},v))return 0; civ_set_nz16(i,v); }}']
        raise exc

__all__ = ['sem']
