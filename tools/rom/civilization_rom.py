#!/usr/bin/env python3
from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
import hashlib, zlib
EXPECTED_SHA="de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32"
EXPECTED_SIZE=1572864

def mirror_address(address:int,size:int)->int:
    if size<=0: raise ValueError("empty ROM")
    base=0
    mask=1 << (max(address,size-1).bit_length()-1)
    while address>=size:
        while mask and not(address&mask): mask >>=1
        if not mask: return base + (address % size)
        address-=mask
        if size>mask:
            size-=mask; base+=mask
        mask>>=1
    return base+address

class HiRom:
    def __init__(self,data:bytes): self.data=data
    def cpu_to_linear(self,bank:int,address:int):
        bank&=0xff; address&=0xffff
        if bank in (0x7e,0x7f): return None
        if (bank<=0x3f or 0x80<=bank<=0xbf):
            if address<0x8000: return None
            return ((bank&0x3f)<<16)|address
        if 0x40<=bank<=0x7d or 0xc0<=bank<=0xff:
            return ((bank&0x3f)<<16)|address
        return None
    def cpu_to_offset(self,bank:int,address:int):
        linear=self.cpu_to_linear(bank,address)
        return None if linear is None else mirror_address(linear,len(self.data))
    def fetch(self,bank:int,address:int)->int:
        off=self.cpu_to_offset(bank,address)
        if off is None: raise ValueError(f"{bank:02X}:{address:04X} is not cartridge ROM")
        return self.data[off]
    def vector16(self,address:int)->int:
        return self.fetch(0,address)|(self.fetch(0,(address+1)&0xffff)<<8)

def mirrored_checksum(data:bytes)->int:
    # SNES non-power-of-two checksum: recursively mirror the trailing region.
    size=len(data)
    power=1<<(size.bit_length()-1)
    if power==size: return sum(data)&0xffff
    remainder=size-power
    factor=power//remainder
    return (sum(data[:power]) + factor*sum(data[power:])) & 0xffff

def inspect(path:Path)->dict:
    raw=path.read_bytes()
    h=raw[0xffc0:0x10000] if len(raw)>=0x10000 else b''
    sha=hashlib.sha256(raw).hexdigest(); crc=f"{zlib.crc32(raw)&0xffffffff:08x}"
    if len(h)<64: raise ValueError("ROM too small for HiROM header")
    u16=lambda off: h[off]|(h[off+1]<<8)
    title=h[:21].decode('ascii','replace').rstrip(' \0')
    sram_bytes=0 if h[0x18]==0 else 1 << (h[0x18]+10)
    declared_rom_bytes=1 << (h[0x17]+10)
    return {
      "file_bytes":len(raw),"sha256":sha,"crc32":crc,"header_offset":"0x00FFC0","title":title,
      "map_mode":f"0x{h[0x15]:02X}","mapping":"HiROM","speed":"FastROM" if h[0x15]&0x10 else "SlowROM",
      "cartridge_type":f"0x{h[0x16]:02X}","rom_size_code":h[0x17],"declared_rom_bytes":declared_rom_bytes,
      "sram_size_code":h[0x18],"sram_bytes":sram_bytes,"region_code":h[0x19],"developer_code":h[0x1a],"version":h[0x1b],
      "checksum_complement":f"0x{u16(0x1c):04X}","checksum":f"0x{u16(0x1e):04X}",
      "mirrored_checksum":f"0x{mirrored_checksum(raw):04X}","checksum_pair_ok":((u16(0x1c)^u16(0x1e))==0xffff),
      "native_nmi":f"00:{u16(0x2a):04X}","native_irq":f"00:{u16(0x2e):04X}",
      "emulation_nmi":f"00:{u16(0x3a):04X}","reset_vector":f"00:{u16(0x3c):04X}","emulation_irq_brk":f"00:{u16(0x3e):04X}",
      "extended_maker":raw[0xffb0:0xffb2].decode('ascii','replace'),"game_code":raw[0xffb2:0xffb6].decode('ascii','replace').rstrip(),
      "expected_identity": sha==EXPECTED_SHA and len(raw)==EXPECTED_SIZE
    }
