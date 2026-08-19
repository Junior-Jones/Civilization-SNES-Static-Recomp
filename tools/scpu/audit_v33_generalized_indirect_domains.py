#!/usr/bin/env python3
"""General Version-33 finite-indirect contract audit over all 69 sites.

The checker recomputes W65C816 indexed-indirect effective pointers from the
instruction operand and the declared raw runtime X, then re-reads every target
from the exact ROM.  It also classifies which sites already have source-derived
producer/value-set completeness and which remain finite fail-closed domains.
"""
from __future__ import annotations
import argparse,hashlib,json
from pathlib import Path

LEGACY_SECOND_METHOD={
 'C0:0331','C0:81E7','C0:A8E9','C0:B420','C0:B88F','C0:C3F4','C0:E2E7',
 'C0:E607','C0:EECF','C0:F058','C2:2475','C2:247A','C2:AD7A','C2:E992',
 'C3:0AC0','D0:BB7C','D4:8D72',
}

def off(b,p):return ((b&0x3f)<<16)|p
def main():
 ap=argparse.ArgumentParser();ap.add_argument('rom',type=Path);ap.add_argument('--analysis',type=Path,required=True);ap.add_argument('--indirect-proof',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
 raw=a.rom.read_bytes();sha=hashlib.sha256(raw).hexdigest();an=json.loads(a.analysis.read_text());cfg=json.loads(a.indirect_proof.read_text());errors=[];rows=[];closure={c['address'] for c in an['contexts']}
 if an.get('rom_sha256')!=sha or an.get('static_analysis_frontier_count')!=0 or not an.get('work_queue_empty'):errors.append('analysis is not a closed exact-ROM fixed point')
 producer_complete=0;prebiased=[]
 for r in cfg.get('proved_sites',[]):
  site=r['address'];b=int(site[:2],16);pc=int(site[3:],16);q=off(b,pc);op=raw[q];operand=raw[q+1]|(raw[q+2]<<8)
  if op not in (0xFC,0x7C):errors.append(f'{site}: expected W65C816 indexed-indirect opcode FC/7C, got {op:02X}')
  if int(r.get('encoded_operand',-1))!=operand:errors.append(f'{site}: encoded operand mismatch')
  contract=r.get('runtime_x_contract','');bias=int(r.get('runtime_x_bias',0));cases=[]
  for c in r.get('cases',[]):
   sx=int(c['selector_offset'])&0xffff;rx=int(c['runtime_x'])&0xffff;eff=(operand+rx)&0xffff
   if eff!=(int(c['effective_pointer'])&0xffff):errors.append(f'{site}: effective pointer mismatch for X={rx:04X}')
   if contract=='raw-x-is-selector-offset' and rx!=sx:errors.append(f'{site}: raw-X contract mismatch')
   if contract=='raw-x-prebiased-table-pointer' and rx!=((bias+sx)&0xffff):errors.append(f'{site}: prebiased-X contract mismatch')
   tq=off(b,eff);target=f'{b:02X}:{raw[tq]|(raw[tq+1]<<8):04X}'
   if target!=c['target']:errors.append(f'{site}: exact ROM target mismatch X={rx:04X}')
   if target not in closure:errors.append(f'{site}: target absent from closed graph: {target}')
   cases.append({'selector_offset':sx,'runtime_x':rx,'effective_pointer':eff,'target':target})
  preclassified=bool(r.get('producer_completeness_contract')) or any(k in r.get('status','') for k in ('complete-caller','complete closed-caller','complete-writer','exact producer/range','procedure-local producer/range','loop-counter','writer set revalidated','state-machine proof','local two-state loop','record provenance','record proof','modulo-phase','producer/classification'))
  if preclassified:evidence_class='prior-explicit-producer-contract'
  elif site in LEGACY_SECOND_METHOD:evidence_class='independent-legacy-second-method'
  elif r.get('proof'):evidence_class='exact-explicit-source-proof'
  else:evidence_class='structurally-exact-only'
  strong=evidence_class!='structurally-exact-only'
  if strong:producer_complete+=1
  if contract=='raw-x-prebiased-table-pointer':prebiased.append(site)
  rows.append({'address':site,'kind':r.get('kind'),'contract':contract,'case_count':len(cases),'producer_value_set_complete':strong,'producer_evidence_class':evidence_class,'cases':cases})
 if len(rows)!=69:errors.append(f'expected 69 finite indirect sites, got {len(rows)}')
 if sorted(prebiased)!=['C2:37A7','C3:236E']:errors.append(f'prebiased site inventory changed: {prebiased}')
 out={'format':'civilization-v33-generalized-indirect-domain-audit-v1','result':'PASS' if not errors else 'FAIL','rom_sha256':sha,'site_count':len(rows),'producer_value_set_complete_site_count':producer_complete,'structurally_exact_fail_closed_site_count':len(rows)-producer_complete,'prebiased_runtime_x_sites':sorted(prebiased),'claim':'Every finite indirect site now has one normalized semantic-selector/raw-X/effective-pointer contract and every target is independently recomputed from exact ROM bytes. Producer/value-set completeness is separately classified; lack of that stronger class never widens runtime authority.','runtime_observation_used_as_proof':False,'sites':rows,'errors':errors}
 a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n');print(json.dumps({'result':out['result'],'sites':len(rows),'producer_complete':producer_complete,'prebiased':prebiased,'errors':len(errors)},sort_keys=True));return 0 if not errors else 1
if __name__=='__main__':raise SystemExit(main())
