#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, sys
from pathlib import Path

def sha(p:Path)->str:return hashlib.sha256(p.read_bytes()).hexdigest()

def main()->int:
    ap=argparse.ArgumentParser()
    ap.add_argument('--mesen-ppu',type=Path,required=True)
    ap.add_argument('--project',type=Path,required=True)
    ap.add_argument('--out',type=Path,required=True)
    a=ap.parse_args(); src=a.mesen_ppu.read_text(errors='replace'); root=a.project.resolve()
    bus=(root/'static-recomp/src/civilization_bus.c').read_text()
    header=(root/'static-recomp/include/civilization_static_recomp.h').read_text()
    checks={
      'oracle_has_2134':'case 0x2134:' in src,
      'oracle_has_2135':'case 0x2135:' in src,
      'oracle_has_2136':'case 0x2136:' in src,
      'oracle_signed_a':'(int16_t)_state.Mode7.Matrix[0]' in src,
      'oracle_signed_b_high':'((int16_t)_state.Mode7.Matrix[1] >> 8)' in src,
      'oracle_ppu1_open_bus':'_state.Ppu1OpenBus' in src,
      'oracle_shared_value_latch':'_state.Mode7.ValueLatch = value;' in src,
      'oracle_matrix_write':'_state.Mode7.Matrix[addr - 0x211B] = (value << 8) | _state.Mode7.ValueLatch;' in src,
      'production_signed_a':'(int16_t)i->ppu.mode7_matrix[0]' in bus,
      'production_signed_b_high':'(int8_t)(i->ppu.mode7_matrix[1] >> 8)' in bus,
      'production_24bit_mask':'&0x00FFFFFFu' in bus.replace(' ',''),
      'production_ppu1_open_bus':'i->ppu.ppu1_open_bus=' in bus.replace(' ',''),
      'production_shared_latch':'mode7_value_latch' in bus and 'mode7_value_latch' in header,
      'production_narrow_read_window':'local>=0x2134u && local<=0x2136u' in bus,
    }
    report={
      'format':'civilization-v33-ppu-multiply-oracle-contract-v1',
      'oracle':'MesenCE 2.2.1 Core/SNES/SnesPpu.cpp',
      'oracle_sha256':sha(a.mesen_ppu),
      'oracle_role':'post-implementation hardware behavior cross-check only; never generation or runtime authority',
      'contract':{
        '$211B-$2120':'single shared low-byte/value latch; each write forms (new<<8)|previous_latch then updates latch',
        '$2134-$2136':'low/mid/high bytes of signed int16 M7A times signed high byte of M7B, exposed as 24-bit two-complement result',
        'ppu1_open_bus':'each $2134-$2136 read updates the PPU1 open-bus latch to the returned byte',
      },
      'checks':checks,'pass':all(checks.values())
    }
    a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(report,indent=2,sort_keys=True)+'\n')
    print(json.dumps(report,indent=2,sort_keys=True))
    return 0 if report['pass'] else 1
if __name__=='__main__':
    try:sys.exit(main())
    except Exception as e:print(json.dumps({'pass':False,'error':str(e)},indent=2));sys.exit(1)
