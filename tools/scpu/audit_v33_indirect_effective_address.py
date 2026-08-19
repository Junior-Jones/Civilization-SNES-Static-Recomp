#!/usr/bin/env python3
"""Independent exact-ROM validator for every Version 33 abs-indexed-indirect proof.

This checker does not import the production analyzer or generator.  It decodes the
site instruction, recomputes raw-X transforms and effective pointer addresses,
reads the target word directly from the verified ROM, and rejects any ambiguity.
"""
from __future__ import annotations
import argparse, hashlib, json, sys
from pathlib import Path
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE)); sys.path.insert(0,str(HERE.parent/'rom'))
from civilization_rom import HiRom,EXPECTED_SHA,EXPECTED_SIZE
from w65c816 import CpuContext,decode

def pa(s): b,p=s.split(':'); return int(b,16),int(p,16)
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('rom',type=Path); ap.add_argument('--indirect-proof',type=Path,required=True); ap.add_argument('--out',type=Path,required=True); a=ap.parse_args()
    raw=a.rom.read_bytes(); digest=hashlib.sha256(raw).hexdigest()
    if len(raw)!=EXPECTED_SIZE or digest!=EXPECTED_SHA: raise SystemExit('wrong Civilization ROM')
    doc=json.loads(a.indirect_proof.read_text()); rom=HiRom(raw)
    if not str(doc.get('format','')).startswith('civilization-static-indirect-control-flow-v33-explicit-runtime-x-'): raise SystemExit('not a Version 33 explicit-runtime-X proof')
    rows=doc.get('proved_sites',[])
    if len(rows)!=69: raise SystemExit(f'expected 69 finite sites, got {len(rows)}')
    results=[]; transformed=[]
    for row in rows:
        b,p=pa(row['address']); inst=decode(rom.fetch,CpuContext(b,p,0,0,0,None))
        if inst.mode!='abs_ind_x' or inst.mnemonic not in {'JSR','JMP'}: raise SystemExit(f'{row["address"]}: not abs-indexed-indirect')
        expected_kind=f'{inst.mnemonic} (abs,X)'
        if row.get('kind')!=expected_kind: raise SystemExit(f'{row["address"]}: kind mismatch')
        operand=inst.operand&0xffff
        if int(row.get('encoded_operand',-1))!=operand: raise SystemExit(f'{row["address"]}: encoded operand mismatch')
        tb,tp=pa(row['table_base'])
        if tb!=b: raise SystemExit(f'{row["address"]}: table must be in program bank')
        bias=(tp-operand)&0xffff
        if int(row.get('runtime_x_bias',-1))!=bias: raise SystemExit(f'{row["address"]}: runtime-X bias mismatch')
        mode='raw-x-is-selector-offset' if bias==0 else 'raw-x-prebiased-table-pointer'
        if row.get('runtime_x_contract')!=mode: raise SystemExit(f'{row["address"]}: runtime-X contract mismatch')
        cases=row.get('cases')
        if not isinstance(cases,list) or not cases: raise SystemExit(f'{row["address"]}: missing cases')
        seen_x=set(); seen_off=set(); case_out=[]
        for case in cases:
            off=int(case['selector_offset'])&0xffff; rx=int(case['runtime_x'])&0xffff; ep=int(case['effective_pointer'])&0xffff
            if off in seen_off or rx in seen_x: raise SystemExit(f'{row["address"]}: duplicate selector/runtime X')
            seen_off.add(off); seen_x.add(rx)
            expected_rx=(bias+off)&0xffff; expected_ep=(operand+rx)&0xffff; expected_entry=(tp+off)&0xffff
            if rx!=expected_rx: raise SystemExit(f'{row["address"]}: runtime X {rx:04X} != bias+selector {expected_rx:04X}')
            if ep!=expected_ep or ep!=expected_entry: raise SystemExit(f'{row["address"]}: effective pointer mismatch')
            target=rom.fetch(b,ep)|(rom.fetch(b,(ep+1)&0xffff)<<8); tb2,tp2=pa(case['target'])
            if tb2!=b or tp2!=target: raise SystemExit(f'{row["address"]}: ROM target mismatch at {ep:04X}')
            case_out.append({'selector_offset':f'{off:04X}','runtime_x':f'{rx:04X}','effective_pointer':f'{ep:04X}','target':f'{b:02X}:{target:04X}'})
        if bias: transformed.append(row['address'])
        results.append({'address':row['address'],'opcode':f'{inst.opcode:02X}','encoded_operand':f'{operand:04X}','table_base':row['table_base'],'runtime_x_contract':mode,'runtime_x_bias':f'{bias:04X}','case_count':len(cases),'cases':case_out})
    if transformed!=['C2:37A7','C3:236E']: raise SystemExit(f'unexpected transformed-site set {transformed}')
    out={'format':'civilization-v33-indirect-effective-address-audit-v1','result':'PASS','rom_sha256':digest,'site_count':len(results),'transformed_site_count':len(transformed),'transformed_sites':transformed,'runtime_learning_used_as_proof':False,'sites':results}
    a.out.parent.mkdir(parents=True,exist_ok=True); a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n')
    print(json.dumps({'result':'PASS','sites':len(results),'transformed_sites':transformed},sort_keys=True))
if __name__=='__main__': main()
