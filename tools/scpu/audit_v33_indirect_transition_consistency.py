#!/usr/bin/env python3
"""Independent finite-selector transition checker for Civilization Version 33.

This checker intentionally does not import the Version 33 state-machine proof.
It classifies the exact C1:0EEA table targets from independent ROM write-site
signatures, computes every possible selector successor, and rejects a proof
configuration whose declared legal domain is not transition-closed.
"""
from __future__ import annotations
import argparse, hashlib, json, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'rom'))
from civilization_rom import HiRom, EXPECTED_SHA, EXPECTED_SIZE

SITE='C1:0EEA'; BANK=0xC1; TABLE=0x0F1A

# Independent target classifier. Each class is guarded by an immutable ROM
# signature at the actual selector-write site.
CLASSIFIERS={
    0x18DC: ('stay_or_set4', 0x1901, bytes.fromhex('a9048d0b19')),
    0x1914: ('stay_or_terminal', 0x1937, bytes.fromhex('a2ffff8e0b19')),
    0x0F3A: ('plus2', 0x0F70, bytes.fromhex('ee0b19ee0b19')),
    0x108D: ('stay_or_plus2', 0x11BB, bytes.fromhex('ee0b19ee0b19')),
    0x0F7E: ('stay_or_plus2', 0x0F9A, bytes.fromhex('ee0b19ee0b19')),
    0x12F6: ('stay_or_plus2', 0x13A7, bytes.fromhex('ee0b19ee0b19')),
    0x11DB: ('stay_or_plus2', 0x1280, bytes.fromhex('ee0b19ee0b19')),
    0x0F78: ('set1a', 0x0F78, bytes.fromhex('a91a8d0b19')),
    0x0FA2: ('set18_or_2', 0x0FBB, bytes.fromhex('a9188d0b198005a9028d0b19')),
}

def rb(rom,pc,n): return bytes(rom.fetch(BANK,(pc+i)&0xffff) for i in range(n))

def successors(selector:int, effect:str):
    if effect=='stay_or_set4': return {selector,0x04}
    if effect=='stay_or_terminal': return {selector,'terminal'}
    if effect=='plus2': return {selector+2}
    if effect=='stay_or_plus2': return {selector,selector+2}
    if effect=='set1a': return {0x1A}
    if effect=='set18_or_2': return {0x18,0x02}
    raise ValueError(effect)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('rom',type=Path)
    ap.add_argument('--indirect-proof',type=Path,required=True)
    ap.add_argument('--out',type=Path)
    ap.add_argument('--expect-fail',action='store_true')
    a=ap.parse_args()
    raw=a.rom.read_bytes(); digest=hashlib.sha256(raw).hexdigest()
    if len(raw)!=EXPECTED_SIZE or digest!=EXPECTED_SHA: raise SystemExit('wrong Civilization ROM')
    rom=HiRom(raw); doc=json.loads(a.indirect_proof.read_text())
    row=next((r for r in doc.get('proved_sites',[]) if r.get('address')==SITE),None)
    if row is None: raise SystemExit(f'{SITE}: proof row missing')
    cases=row.get('cases')
    if not isinstance(cases,list) or not cases: raise SystemExit(f'{SITE}: explicit Version 33 cases missing')
    selectors=[int(c['selector_offset']) for c in cases]
    legal=set(selectors)

    rows=[]; escapes=[]
    for case in cases:
        selector=int(case['selector_offset']); target_text=case['target']
        tb,tp=target_text.split(':'); target=int(tp,16)
        if int(tb,16)!=BANK: raise SystemExit(f'{SITE}: unexpected target bank {target_text}')
        rom_target=rom.fetch(BANK,TABLE+selector)|(rom.fetch(BANK,TABLE+selector+1)<<8)
        if rom_target!=target: raise SystemExit(f'{SITE}: configured target mismatch for X={int(selector):04X}')
        if target not in CLASSIFIERS: raise SystemExit(f'{SITE}: no independent transition classifier for {target_text}')
        effect,pc,sig=CLASSIFIERS[target]
        if rb(rom,pc,len(sig))!=sig: raise SystemExit(f'{SITE}: classifier signature mismatch at C1:{pc:04X}')
        succ=successors(selector,effect)
        # Caller C1:0F00-C1:0F12 independently permits exact force-to-2.
        succ.add(0x02)
        outside=sorted(x for x in succ if x!='terminal' and x not in legal)
        if outside:
            escapes.append({'selector':selector,'target':target_text,'effect':effect,'outside_declared_domain':outside})
        rows.append({'selector':selector,'target':target_text,'effect':effect,
                     'successors':['FFFF' if x=='terminal' else x for x in sorted(succ,key=lambda z:(z=='terminal',str(z)))],
                     'outside_declared_domain':outside})

    result='PASS' if not escapes else 'FAIL'
    out={
        'format':'civilization-v33-indirect-transition-consistency-v2-explicit-selector-offset',
        'method':'independent exact-ROM target classifier + transition-domain closure over explicit selector_offset cases; does not import prove_v33_name_state_machine.py',
        'rom_sha256':digest,'site':SITE,'declared_legal_x':sorted(legal),
        'rows':rows,'escapes':escapes,'result':result,
        'natural_test_observations_used':False,
    }
    if a.out:
        a.out.parent.mkdir(parents=True,exist_ok=True); a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n')
    print(json.dumps({'site':SITE,'declared_count':len(legal),'escape_count':len(escapes),'result':result},sort_keys=True))
    should_fail=bool(escapes)
    if a.expect_fail:
        if not should_fail: raise SystemExit('expected a transition-domain failure but checker passed')
        return
    if should_fail:
        first=escapes[0]
        raise SystemExit(f"{SITE}: selector {first['selector']:04X} can transition outside declared domain to {first['outside_declared_domain']}")

if __name__=='__main__': main()
