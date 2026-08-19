#!/usr/bin/env python3
"""Independent producer-domain completeness audit for Civilization Version 33.

This does not import the production analyzer/generator.  It uses the final closed
manifest only to enumerate *reachable direct caller sites*, then re-derives the
value domains from immutable ROM instructions and compares those derived domains
with the production proof.  The two transformed-X dispatches are fully derived;
the remaining 67 sites are required to remain raw-X contracts and are covered by
the independent exact-effective-address audit plus their source-specific proof
receipts.
"""
from __future__ import annotations
import argparse,hashlib,json
from pathlib import Path
ROM_SHA='de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32'
ROM_SIZE=1572864

def off(bank,pc): return ((bank&0x3f)<<16)|(pc&0xffff)
def rb(rom,b,p,n): return rom[off(b,p):off(b,p)+n]
def word(rom,b,p): q=off(b,p); return rom[q]|(rom[q+1]<<8)
def parse_addr(s): b,p=s.split(':'); return int(b,16),int(p,16)
def require_bytes(rom,b,p,h,label):
    e=bytes.fromhex(h); g=rb(rom,b,p,len(e))
    if g!=e: raise SystemExit(f'{label}: ROM bytes changed at {b:02X}:{p:04X}: {g.hex()}')

def caller_sites(manifest, callee_entry):
    out=set()
    for e in manifest.get('call_edges',[]):
        if e.get('callee_proc','').startswith(callee_entry+'/'):
            out.add(e['site'])
    return sorted(out)

def derive_c237a7(rom,m):
    sites=caller_sites(m,'C2:378F')
    vals=[]; rows=[]
    for s in sites:
        b,p=parse_addr(s)
        # Every reachable caller is source-required to be LDA #imm16; JSL C2:378F.
        if rb(rom,b,p,4)!=bytes.fromhex('228f37c2'): raise SystemExit(f'C2:378F caller opcode changed at {s}')
        pre=rb(rom,b,(p-3)&0xffff,3)
        if len(pre)!=3 or pre[0]!=0xA9: raise SystemExit(f'{s}: C2:378F caller lacks immediate LDA producer')
        v=pre[1]|(pre[2]<<8); vals.append(v); rows.append({'site':s,'input_a':v})
    vals=sorted(set(vals))
    if vals!=[1,2,3,4,5]: raise SystemExit(f'C2:37A7 caller A-domain changed: {vals}')
    cases=[]
    for v in vals:
        sel=((v&0xff)-1)*2; rx=0x37AC+sel; target=word(rom,0xC2,rx)
        cases.append({'selector_offset':sel,'runtime_x':rx,'effective_pointer':rx,'target':f'C2:{target:04X}'})
    return rows,cases

def derive_c3236e(rom,m):
    sites=caller_sites(m,'C1:3942')
    vals=[]; rows=[]
    for s in sites:
        b,p=parse_addr(s)
        if rb(rom,b,p,4)!=bytes.fromhex('224239c1'): raise SystemExit(f'C1:3942 caller opcode changed at {s}')
        pre3=rb(rom,b,(p-3)&0xffff,3)
        pre5=rb(rom,b,(p-5)&0xffff,5)
        if len(pre3)==3 and pre3[0]==0xA2:
            v=pre3[1]|(pre3[2]<<8)
        elif len(pre5)==5 and pre5[0]==0xA2 and pre5[3]==0xA5:
            v=pre5[1]|(pre5[2]<<8)
        else:
            raise SystemExit(f'{s}: C1:3942 caller lacks bounded immediate LDX producer pattern')
        vals.append(v); rows.append({'site':s,'input_x':v})
    vals=sorted(set(vals))
    if vals!=[0,1,2,3,4,5]: raise SystemExit(f'C3:236E caller X-domain changed: {vals}')
    require_bytes(rom,0xC1,0x394A,'8e231a','C1:394A STX $1A23')
    require_bytes(rom,0xC1,0x3575,'ad231af00dc90500f0083aac1e1a223423c3','C1:3575 filter/dispatch')
    require_bytes(rom,0xC3,0x2339,'29ff008db8010a1869732348','C3:2334 selector transform')
    require_bytes(rom,0xC3,0x236D,'fafc0000','C3:236D pointer restore/dispatch')
    survivors=[v for v in vals if v not in (0,5)]
    cases=[]
    for v in survivors:
        sel=(v-1)*2; rx=0x2373+sel; target=word(rom,0xC3,rx)
        cases.append({'selector_offset':sel,'runtime_x':rx,'effective_pointer':rx,'target':f'C3:{target:04X}'})
    return rows,cases

def norm(cases):
    return [{k:(int(c[k]) if k!='target' else c[k]) for k in ('selector_offset','runtime_x','effective_pointer','target')} for c in cases]

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('rom',type=Path); ap.add_argument('--analysis',type=Path,required=True); ap.add_argument('--indirect-proof',type=Path,required=True); ap.add_argument('--out',type=Path,required=True); a=ap.parse_args()
    rom=a.rom.read_bytes(); sha=hashlib.sha256(rom).hexdigest()
    if len(rom)!=ROM_SIZE or sha!=ROM_SHA: raise SystemExit('wrong Civilization ROM')
    m=json.loads(a.analysis.read_text()); p=json.loads(a.indirect_proof.read_text())
    if m.get('rom_sha256')!=sha or m.get('static_analysis_frontier_count')!=0 or m.get('work_queue_empty') is not True: raise SystemExit('requires final closed exact-ROM manifest')
    rows=p.get('proved_sites',[])
    if len(rows)!=69: raise SystemExit(f'expected 69 finite sites, got {len(rows)}')
    by={r['address']:r for r in rows}
    transformed=sorted(r['address'] for r in rows if r.get('runtime_x_contract')=='raw-x-prebiased-table-pointer')
    if transformed!=['C2:37A7','C3:236E']: raise SystemExit(f'unexpected transformed sites {transformed}')
    c2call,c2=derive_c237a7(rom,m); c3call,c3=derive_c3236e(rom,m)
    for site,derived in [('C2:37A7',c2),('C3:236E',c3)]:
        declared=norm(by[site].get('cases',[])); derived=norm(derived)
        if declared!=derived: raise SystemExit(f'{site}: declared proof is not producer-domain complete\ndeclared={declared}\nderived={derived}')
    # Global schema checks: every finite site has nonempty explicit cases, exact target labels,
    # and every non-transformed site is explicitly raw-X-as-selector.  This makes omissions at
    # transformed producer-fed sites impossible while preserving fail-closed source-specific
    # domains at the other 67 sites.
    raw_sites=[]
    for r in rows:
        if not r.get('cases'): raise SystemExit(f'{r["address"]}: no explicit cases')
        if r['address'] not in transformed:
            if r.get('runtime_x_contract')!='raw-x-is-selector-offset': raise SystemExit(f'{r["address"]}: unexpected runtime-X contract')
            for c in r['cases']:
                if int(c['runtime_x'])!=int(c['selector_offset']): raise SystemExit(f'{r["address"]}: raw-X case mismatch')
            raw_sites.append(r['address'])
    out={'format':'civilization-v33-indirect-domain-completeness-v1','result':'PASS','rom_sha256':sha,'analysis_sha256':hashlib.sha256(a.analysis.read_bytes()).hexdigest(),'proof_sha256':hashlib.sha256(a.indirect_proof.read_bytes()).hexdigest(),'finite_site_count':69,'raw_x_site_count':len(raw_sites),'transformed_sites':transformed,'producer_derived':{'C2:37A7':{'reachable_callers':c2call,'cases':c2},'C3:236E':{'reachable_callers':c3call,'cases':c3}},'runtime_observation_used_as_proof':False,'method':'closed caller sites from final static manifest + immutable immediate producer instructions + exact ROM transforms/table words; transformed domains compared for set equality with production proof'}
    a.out.parent.mkdir(parents=True,exist_ok=True); a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n')
    print(json.dumps({'result':'PASS','sites':69,'transformed':transformed,'C3_236E_cases':len(c3)},sort_keys=True))
if __name__=='__main__': main()
