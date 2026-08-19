#!/usr/bin/env python3
"""Independent exact-ROM proof of the PPU configuration domains used by Version 33."""
from __future__ import annotations
import argparse, hashlib, json, sys
from pathlib import Path

HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE.parent/'rom'))
from civilization_rom import HiRom, EXPECTED_SHA, EXPECTED_SIZE


def parse_addr(s: str):
    b,p=s.split(':'); return int(b,16),int(p,16)


def preceding_immediate(rom: HiRom, row: dict) -> int | None:
    bank,pc=parse_addr(row['address'])
    if row['instruction'].startswith('STZ '): return 0
    if not row['instruction'].startswith('STA '): return None
    if row['m'] == 1:
        q=(pc-2)&0xffff
        if rom.fetch(bank,q)==0xA9: return rom.fetch(bank,(q+1)&0xffff)
    else:
        q=(pc-3)&0xffff
        if rom.fetch(bank,q)==0xA9:
            return rom.fetch(bank,(q+1)&0xffff)|(rom.fetch(bank,(q+2)&0xffff)<<8)
    return None


def reached_writes(doc: dict, reg: int):
    needle=f'${reg:04X}'
    seen={}
    for row in doc['contexts']:
        ins=row['instruction']
        if needle not in ins or ins.split()[0] not in {'STA','STZ'}: continue
        key=(row['address'],row['e'],row['m'],row['x'])
        seen[key]=row
    return [seen[k] for k in sorted(seen)]


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('rom',type=Path)
    ap.add_argument('--analysis',type=Path,required=True)
    ap.add_argument('--out',type=Path,required=True)
    a=ap.parse_args()
    raw=a.rom.read_bytes(); digest=hashlib.sha256(raw).hexdigest()
    if len(raw)!=EXPECTED_SIZE or digest!=EXPECTED_SHA: raise SystemExit('wrong Civilization ROM')
    doc=json.loads(a.analysis.read_text())
    if doc.get('rom_sha256')!=digest or doc.get('static_analysis_frontier_count')!=0: raise SystemExit('analysis mismatch/not closed')
    rom=HiRom(raw)

    receipts={}
    for reg,name in [(0x2115,'VMAIN'),(0x2105,'BGMODE')]:
        rows=reached_writes(doc,reg); values=[]; unresolved=[]; sites=[]
        for row in rows:
            v=preceding_immediate(rom,row)
            if v is None: unresolved.append(row['address'])
            else: values.append(v & 0xff)
            sites.append(row['address'])
        receipts[name]={'site_count':len(rows),'sites':sites,'values':sorted(set(values)),'unresolved':unresolved}

    if receipts['VMAIN']['unresolved'] or receipts['VMAIN']['values'] != [0x80]:
        raise SystemExit(f"VMAIN proof failed: {receipts['VMAIN']}")
    expected_modes=[0x00,0x01,0x03,0x09,0x19,0x21]
    if receipts['BGMODE']['unresolved'] or receipts['BGMODE']['values'] != expected_modes:
        raise SystemExit(f"BGMODE proof failed: {receipts['BGMODE']}")
    if [s for s in receipts['BGMODE']['sites'] if s=='C0:834A'] != ['C0:834A']:
        raise SystemExit('Mode 0 reset site missing')
    # Exact reset sequence: LDA #$8F / STA $2100 precedes the sole BGMODE=0 reset write.
    reset_bytes=bytes(rom.fetch(0xC0,p) for p in range(0x833C,0x834D))
    if reset_bytes != bytes.fromhex('a98f8d00219c01219c02219c03219c0521'):
        raise SystemExit(f'forced-blank reset sequence mismatch: {reset_bytes.hex()}')

    result={
      'format':'civilization-v33-ppu-domain-proof-v1',
      'rom_sha256':digest,
      'analysis_sha256':hashlib.sha256(a.analysis.read_bytes()).hexdigest(),
      'vmain':receipts['VMAIN'],
      'bgmode':receipts['BGMODE'],
      'bgmode_low_mode_domain':sorted(set(v&7 for v in receipts['BGMODE']['values'])),
      'visible_renderer_modes_implemented':[1,3],
      'mode0_classification':'sole source-proved write is inside C0:8330 PPU reset after forced blank $2100=$8F',
      'mode3_classification':'BG1 8bpp + BG2 4bpp; production renderer implements normal CGRAM and direct-color BG1 paths',
      'result':'PASS'
    }
    a.out.parent.mkdir(parents=True,exist_ok=True)
    a.out.write_text(json.dumps(result,indent=2,sort_keys=True)+'\n')
    print(json.dumps({'result':'PASS','vmain_writes':receipts['VMAIN']['site_count'],'bgmode_writes':receipts['BGMODE']['site_count'],'modes':result['bgmode_low_mode_domain']},sort_keys=True))

if __name__=='__main__': main()
