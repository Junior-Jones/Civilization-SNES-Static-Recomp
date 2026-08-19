#!/usr/bin/env python3
"""Small build-time W65C816 decoder used by the Civilization static analyser.

This module is developer tooling.  Production binaries never import it and
never decode opcodes at runtime.  Instruction lengths are selected from the
incoming E/M/X context.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

# Addressing-mode names are deliberately explicit so length and formatting
# tests do not depend on mnemonic-specific guesses.
_ROWS = [
    # 0x00
    "BRK:imm8 ORA:dp_ind_x COP:imm8 ORA:sr TSB:dp ORA:dp ASL:dp ORA:dp_long_ind PHP:imp ORA:imm_m ASL:acc PHD:imp TSB:abs ORA:abs ASL:abs ORA:long",
    # 0x10
    "BPL:rel8 ORA:dp_ind_y ORA:dp_ind ORA:sr_ind_y TRB:dp ORA:dp_x ASL:dp_x ORA:dp_long_ind_y CLC:imp ORA:abs_y INC:acc TCS:imp TRB:abs ORA:abs_x ASL:abs_x ORA:long_x",
    # 0x20
    "JSR:abs AND:dp_ind_x JSL:long AND:sr BIT:dp AND:dp ROL:dp AND:dp_long_ind PLP:imp AND:imm_m ROL:acc PLD:imp BIT:abs AND:abs ROL:abs AND:long",
    # 0x30
    "BMI:rel8 AND:dp_ind_y AND:dp_ind AND:sr_ind_y BIT:dp_x AND:dp_x ROL:dp_x AND:dp_long_ind_y SEC:imp AND:abs_y DEC:acc TSC:imp BIT:abs_x AND:abs_x ROL:abs_x AND:long_x",
    # 0x40
    "RTI:imp EOR:dp_ind_x WDM:imm8 EOR:sr MVP:block EOR:dp LSR:dp EOR:dp_long_ind PHA:imp EOR:imm_m LSR:acc PHK:imp JMP:abs EOR:abs LSR:abs EOR:long",
    # 0x50
    "BVC:rel8 EOR:dp_ind_y EOR:dp_ind EOR:sr_ind_y MVN:block EOR:dp_x LSR:dp_x EOR:dp_long_ind_y CLI:imp EOR:abs_y PHY:imp TCD:imp JML:long EOR:abs_x LSR:abs_x EOR:long_x",
    # 0x60
    "RTS:imp ADC:dp_ind_x PER:rel16 ADC:sr STZ:dp ADC:dp ROR:dp ADC:dp_long_ind PLA:imp ADC:imm_m ROR:acc RTL:imp JMP:abs_ind ADC:abs ROR:abs ADC:long",
    # 0x70
    "BVS:rel8 ADC:dp_ind_y ADC:dp_ind ADC:sr_ind_y STZ:dp_x ADC:dp_x ROR:dp_x ADC:dp_long_ind_y SEI:imp ADC:abs_y PLY:imp TDC:imp JMP:abs_ind_x ADC:abs_x ROR:abs_x ADC:long_x",
    # 0x80
    "BRA:rel8 STA:dp_ind_x BRL:rel16 STA:sr STY:dp STA:dp STX:dp STA:dp_long_ind DEY:imp BIT:imm_m TXA:imp PHB:imp STY:abs STA:abs STX:abs STA:long",
    # 0x90
    "BCC:rel8 STA:dp_ind_y STA:dp_ind STA:sr_ind_y STY:dp_x STA:dp_x STX:dp_y STA:dp_long_ind_y TYA:imp STA:abs_y TXS:imp TXY:imp STZ:abs STA:abs_x STZ:abs_x STA:long_x",
    # 0xA0
    "LDY:imm_x LDA:dp_ind_x LDX:imm_x LDA:sr LDY:dp LDA:dp LDX:dp LDA:dp_long_ind TAY:imp LDA:imm_m TAX:imp PLB:imp LDY:abs LDA:abs LDX:abs LDA:long",
    # 0xB0
    "BCS:rel8 LDA:dp_ind_y LDA:dp_ind LDA:sr_ind_y LDY:dp_x LDA:dp_x LDX:dp_y LDA:dp_long_ind_y CLV:imp LDA:abs_y TSX:imp TYX:imp LDY:abs_x LDA:abs_x LDX:abs_y LDA:long_x",
    # 0xC0
    "CPY:imm_x CMP:dp_ind_x REP:imm8 CMP:sr CPY:dp CMP:dp DEC:dp CMP:dp_long_ind INY:imp CMP:imm_m DEX:imp WAI:imp CPY:abs CMP:abs DEC:abs CMP:long",
    # 0xD0
    "BNE:rel8 CMP:dp_ind_y CMP:dp_ind CMP:sr_ind_y PEI:dp CMP:dp_x DEC:dp_x CMP:dp_long_ind_y CLD:imp CMP:abs_y PHX:imp STP:imp JMP:abs_long_ind CMP:abs_x DEC:abs_x CMP:long_x",
    # 0xE0
    "CPX:imm_x SBC:dp_ind_x SEP:imm8 SBC:sr CPX:dp SBC:dp INC:dp SBC:dp_long_ind INX:imp SBC:imm_m NOP:imp XBA:imp CPX:abs SBC:abs INC:abs SBC:long",
    # 0xF0
    "BEQ:rel8 SBC:dp_ind_y SBC:dp_ind SBC:sr_ind_y PEA:abs SBC:dp_x INC:dp_x SBC:dp_long_ind_y SED:imp SBC:abs_y PLX:imp XCE:imp JSR:abs_ind_x SBC:abs_x INC:abs_x SBC:long_x",
]

_ENTRIES = [item for row in _ROWS for item in row.split()]
if len(_ENTRIES) != 256:
    raise RuntimeError(f"opcode table contains {len(_ENTRIES)} entries")

MODE_FIXED_LENGTH = {
    "imp": 1,
    "acc": 1,
    "imm8": 2,
    "dp": 2,
    "dp_x": 2,
    "dp_y": 2,
    "dp_ind_x": 2,
    "dp_ind_y": 2,
    "dp_ind": 2,
    "dp_long_ind": 2,
    "dp_long_ind_y": 2,
    "sr": 2,
    "sr_ind_y": 2,
    "abs": 3,
    "abs_x": 3,
    "abs_y": 3,
    "abs_ind": 3,
    "abs_ind_x": 3,
    "abs_long_ind": 3,
    "long": 4,
    "long_x": 4,
    "rel8": 2,
    "rel16": 3,
    "block": 3,
}


@dataclass(frozen=True)
class CpuContext:
    pbr: int
    pc: int
    e: int
    m: int
    x: int
    carry: Optional[int] = None

    def key(self) -> str:
        c = "u" if self.carry is None else str(self.carry)
        return f"{self.pbr:02X}:{self.pc:04X}/E{self.e}M{self.m}X{self.x}C{c}"


@dataclass(frozen=True)
class DecodedInstruction:
    opcode: int
    mnemonic: str
    mode: str
    length: int
    raw: bytes
    operand: Optional[int]
    text: str


def instruction_length(mode: str, m_flag: int, x_flag: int) -> int:
    if mode == "imm_m":
        return 2 if m_flag else 3
    if mode == "imm_x":
        return 2 if x_flag else 3
    try:
        return MODE_FIXED_LENGTH[mode]
    except KeyError as exc:
        raise ValueError(f"unknown addressing mode {mode}") from exc


def opcode_info(opcode: int) -> tuple[str, str]:
    mnemonic, mode = _ENTRIES[opcode & 0xFF].split(":", 1)
    return mnemonic, mode


def _u16(b: bytes) -> int:
    return b[0] | (b[1] << 8)


def _u24(b: bytes) -> int:
    return b[0] | (b[1] << 8) | (b[2] << 16)


def _signed8(value: int) -> int:
    return value - 0x100 if value & 0x80 else value


def _signed16(value: int) -> int:
    return value - 0x10000 if value & 0x8000 else value


def format_operand(mode: str, operand_bytes: bytes, context: CpuContext, length: int) -> tuple[Optional[int], str]:
    if mode in {"imp", "acc"}:
        return None, "A" if mode == "acc" else ""
    if mode in {"imm8", "imm_m", "imm_x"}:
        value = operand_bytes[0] if len(operand_bytes) == 1 else _u16(operand_bytes)
        digits = 2 if len(operand_bytes) == 1 else 4
        return value, f"#${value:0{digits}X}"
    if mode in {"dp", "dp_x", "dp_y", "dp_ind_x", "dp_ind_y", "dp_ind", "dp_long_ind", "dp_long_ind_y", "sr", "sr_ind_y"}:
        value = operand_bytes[0]
        forms = {
            "dp": f"${value:02X}", "dp_x": f"${value:02X},X", "dp_y": f"${value:02X},Y",
            "dp_ind_x": f"(${value:02X},X)", "dp_ind_y": f"(${value:02X}),Y", "dp_ind": f"(${value:02X})",
            "dp_long_ind": f"[${value:02X}]", "dp_long_ind_y": f"[${value:02X}],Y",
            "sr": f"${value:02X},S", "sr_ind_y": f"(${value:02X},S),Y",
        }
        return value, forms[mode]
    if mode in {"abs", "abs_x", "abs_y", "abs_ind", "abs_ind_x", "abs_long_ind"}:
        value = _u16(operand_bytes)
        forms = {
            "abs": f"${value:04X}", "abs_x": f"${value:04X},X", "abs_y": f"${value:04X},Y",
            "abs_ind": f"(${value:04X})", "abs_ind_x": f"(${value:04X},X)", "abs_long_ind": f"[${value:04X}]",
        }
        return value, forms[mode]
    if mode in {"long", "long_x"}:
        value = _u24(operand_bytes)
        return value, f"${value:06X}" + (",X" if mode == "long_x" else "")
    if mode == "rel8":
        target = (context.pc + length + _signed8(operand_bytes[0])) & 0xFFFF
        return target, f"${context.pbr:02X}:{target:04X}"
    if mode == "rel16":
        value = _u16(operand_bytes)
        target = (context.pc + length + _signed16(value)) & 0xFFFF
        return target, f"${context.pbr:02X}:{target:04X}"
    if mode == "block":
        destination, source = operand_bytes[0], operand_bytes[1]
        return destination | (source << 8), f"${destination:02X},${source:02X}"
    raise ValueError(f"unhandled mode {mode}")


def decode(fetch, context: CpuContext) -> DecodedInstruction:
    opcode = fetch(context.pbr, context.pc)
    mnemonic, mode = opcode_info(opcode)
    length = instruction_length(mode, context.m, context.x)
    raw = bytes(fetch(context.pbr, (context.pc + index) & 0xFFFF) for index in range(length))
    operand, rendered = format_operand(mode, raw[1:], context, length)
    text = mnemonic if not rendered else f"{mnemonic} {rendered}"
    return DecodedInstruction(opcode, mnemonic, mode, length, raw, operand, text)


def all_opcode_rows() -> list[dict[str, object]]:
    rows = []
    for opcode in range(256):
        mnemonic, mode = opcode_info(opcode)
        rows.append({"opcode": opcode, "mnemonic": mnemonic, "mode": mode})
    return rows
