#!/usr/bin/env python3
"""Source-only check that major fail-closed machine guards cover the closed V33 graph."""
from __future__ import annotations
import argparse,hashlib,json,sys
from pathlib import Path
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE.parent/'rom'))
from civilization_rom import HiRom,EXPECTED_SHA,EXPECTED_SIZE

def parse(s): b,p=s.split(':'); return int(b,16),int(p,16)
def pred_imm(rom,row):
    b,p=parse(row['address'])
    if row['instruction'].startswith('STZ '):return 0
    if not row['instruction'].startswith('STA '):return None
    n=2 if row['m'] else 3; q=(p-n)&0xffff
    if rom.fetch(b,q)!=0xA9:return None
    return rom.fetch(b,q+1) if row['m'] else rom.fetch(b,q+1)|(rom.fetch(b,q+2)<<8)
def writes(doc,reg):
    needle=f'${reg:04X}';out={}
    for r in doc['contexts']:
        if needle in r['instruction'] and r['instruction'].split()[0] in {'STA','STZ'}:out[(r['address'],r['e'],r['m'],r['x'])]=r
    return list(out.values())
def main():
    ap=argparse.ArgumentParser();ap.add_argument('rom',type=Path);ap.add_argument('--analysis',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
    raw=a.rom.read_bytes();digest=hashlib.sha256(raw).hexdigest()
    if len(raw)!=EXPECTED_SIZE or digest!=EXPECTED_SHA:raise SystemExit('wrong Civilization ROM')
    doc=json.loads(a.analysis.read_text());
    if doc.get('rom_sha256')!=digest or doc.get('static_analysis_frontier_count')!=0:raise SystemExit('analysis mismatch/not closed')
    rom=HiRom(raw)
    mnemonics={r['instruction'].split()[0] for r in doc['contexts']}
    forbidden=sorted({'SED','WAI','STP','BRK','COP'} & mnemonics)
    if forbidden:raise SystemExit(f'unsupported control/arithmetic state instruction reached: {forbidden}')
    mdma=[];unknown=[]
    for r in writes(doc,0x420B):
        v=pred_imm(rom,r)
        if v is None:unknown.append(r['address'])
        else:mdma.append(v&0xff)
    # C2:A72C loads A=$00 and C2:A72E/A731/A734 are only stores before
    # C2:A737 STA $420B, so the one non-immediate predecessor is also zero.
    if unknown!=['C2:A737']:
        raise SystemExit(f'unexpected MDMA unresolved producers {unknown}')
    seq=bytes(rom.fetch(0xC2,p) for p in range(0xA72C,0xA73D))
    expected=bytes.fromhex('a9008d08428d09428d0a428d0b428d0c42')
    if seq!=expected:raise SystemExit(f'C2:A737 zero-preservation sequence mismatch {seq.hex()}')
    mdma.append(0)
    mdma_domain=sorted(set(mdma))
    if mdma_domain!=[0,1,2,128]:raise SystemExit(f'MDMA mask domain outside implementation {mdma_domain}')
    dmap={}
    for reg,ch in [(0x4300,0),(0x4310,1),(0x4370,7)]:
        vals=[];unk=[]
        for r in writes(doc,reg):
            v=pred_imm(rom,r)
            if v is None:unk.append(r['address'])
            else:vals.append(v&0xff)
        if unk:raise SystemExit(f'DMAP{ch} unresolved {unk}')
        dmap[str(ch)]=sorted(set(vals))
    if dmap!={'0':[0,1,2,9],'1':[0,1,2,9],'7':[1]}:raise SystemExit(f'DMAP domain mismatch {dmap}')
    if any(v&0x80 for vals in dmap.values() for v in vals):raise SystemExit('B-bus to A-bus DMA became source-reachable')
    modes={ch:sorted(set(v&7 for v in vals)) for ch,vals in dmap.items()}
    if modes!={'0':[0,1,2],'1':[0,1,2],'7':[1]}:raise SystemExit(f'DMA mode domain mismatch {modes}')
    result={'format':'civilization-v33-machine-guard-audit-v1','result':'PASS','rom_sha256':digest,'analysis_sha256':hashlib.sha256(a.analysis.read_bytes()).hexdigest(),'decimal_mode_source_reachable':False,'unsupported_stop_or_software_interrupt_mnemonics':forbidden,'mdmaen_domain':mdma_domain,'manual_dma_dmap_domain':dmap,'manual_dma_mode_domain':modes,'manual_dma_direction':'A-bus to B-bus only','vram_mapping_domain':'proved separately by civilization_v33_ppu_domains.json','hdma_domain':'proved separately by civilization_v33_hdma_channel2.proof.json'}
    a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(result,indent=2,sort_keys=True)+'\n')
    print(json.dumps({'result':'PASS','mdmaen':mdma_domain,'dmap':dmap,'decimal_mode':False},sort_keys=True))
if __name__=='__main__':main()
