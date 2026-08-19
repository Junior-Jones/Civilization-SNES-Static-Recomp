#!/usr/bin/env python3
"""Independent Version-33 recertification of every legacy finite indirect site.

This is deliberately not the fixed-point analyzer.  It re-reads immutable ROM
pointer words, verifies the selector-forming instruction sequences, inventories
important source writers, and requires every target to be owned by the closed
runtime graph.  It does not widen a proof from adjacent table words.
"""
from __future__ import annotations
import argparse,hashlib,json
from pathlib import Path

EXPECTED={
'C0:0331':list(range(0,26,2)),'C0:81E7':list(range(0,42,2)),
'C0:A8E9':list(range(0,56,2)),'C0:B420':list(range(0,56,2)),
'C0:B88F':[0,2,4,6],'C0:C3F4':list(range(0,12,2)),
'C0:E2E7':[0,2,4],'C0:E607':[0,2,4],
'C0:EECF':list(range(2,44,2)),'C0:F058':list(range(2,50,2)),
'C2:2475':[0,2,4,6,8],'C2:247A':[0,2,4,6,8],
'C2:AD7A':list(range(0,36,2)),'C2:E992':[0,2,4,6],
'C3:0AC0':list(range(0,44,2)),'D0:BB7C':[0,2,4,6,8,10,12],
'D4:8D72':list(range(0,22,2)),
}
SELECTOR_BYTES={
'C0:0331':(0xC0,0x032A,'150629ff000aaafc3603'),
'C0:A8E9':(0xC0,0xA8DA,'a6b1bfbd8c7e297f003a0aaaadb111fceea8'),
'C0:B420':(0xC0,0xB416,'bfbd8c7e297f003a0aaafc4eb4'),
'C0:B88F':(0xC0,0xB884,'20ebcac9fffff0693a0aaafcc2cc'),
'C0:C3F4':(0xC0,0xC3E4,'89c0c9fffff0683a0aaabd86083a0aaafc5fc4'),
'C0:E2E7':(0xC0,0xE2DC,'c0aabf9314c029ff000aaafcece2'),
'C0:E607':(0xC0,0xE5F6,'48222835c0aaa8bf9314c029ff000aaa68fc17e6'),
'C0:EECF':(0xC0,0xEEC4,'a6b9bf5b787e293f000aaafcd7ee'),
'C0:F058':(0xC0,0xF04C,'e220a6b9bf5b787e293f0aaafc5df0'),
'C2:2475':(0xC2,0x246C,'0aaaa5b3c90100d005fc82248003fc8c24'),
'C2:247A':(0xC2,0x246C,'0aaaa5b3c90100d005fc82248003fc8c24'),
'C3:0AC0':(0xC3,0x0AB7,'08c2201a29ff000aaafcc50a'),
'D4:8D72':(0xD4,0x8D6D,'adf31c0aaa7c758d'),
}
OBJECT_WRITERS={'C0:3F40','C0:6221','C0:623F','C0:AC88','C0:BC36','C1:06A6','C1:08C3','C1:2504','C2:295E','C2:2A48','C2:9DAF','C2:9DCE'}
UNIT_STATE_WRITERS={'C0:ECBA','C0:ED31','C0:EDA6','C0:EE2F','C0:EE63','C0:EFC8','C0:F009','C0:F169','C0:F1C7','C0:F2D4','C2:155D','C2:17FC'}
D4_WRITERS={'D4:80BA','D4:8D60','D4:8EB1','D4:8F5B','D4:8FAE','D4:9012','D4:B414','D4:B437'}

def off(bank,pc):return ((bank&0x3f)<<16)|pc
def exact(raw,b,p,h,label):
 e=bytes.fromhex(h);g=raw[off(b,p):off(b,p)+len(e)]
 if g!=e:raise SystemExit(f'{label}: source bytes changed at {b:02X}:{p:04X}')

def main():
 ap=argparse.ArgumentParser();ap.add_argument('rom',type=Path);ap.add_argument('--analysis',type=Path,required=True);ap.add_argument('--indirect-proof',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
 raw=a.rom.read_bytes();sha=hashlib.sha256(raw).hexdigest();an=json.loads(a.analysis.read_text());cfg=json.loads(a.indirect_proof.read_text())
 if an.get('rom_sha256')!=sha or an.get('static_analysis_frontier_count')!=0 or not an.get('work_queue_empty'):raise SystemExit('requires closed exact-ROM analysis')
 closure={c['address'] for c in an['contexts']};by={r['address']:r for r in cfg['proved_sites']};errors=[];rows=[]
 for site,domain in EXPECTED.items():
  r=by.get(site)
  if not r:errors.append(f'{site}: missing proof');continue
  got=[int(c['runtime_x']) for c in r['cases']]
  if got!=domain:errors.append(f'{site}: runtime-X domain changed: {got}')
  bank=int(site[:2],16)
  for c in r['cases']:
   ptr=int(c['effective_pointer'])&0xffff;q=off(bank,ptr);target=f'{bank:02X}:{raw[q]|(raw[q+1]<<8):04X}'
   if target!=c['target']:errors.append(f'{site}: ROM pointer ${ptr:04X} -> {target}, proof says {c["target"]}')
   if c['target'] not in closure:errors.append(f'{site}: target not in closed graph: {c["target"]}')
  if site in SELECTOR_BYTES:
   b,p,h=SELECTOR_BYTES[site];
   try:exact(raw,b,p,h,site)
   except SystemExit as e:errors.append(str(e))
  rows.append({'address':site,'case_count':len(domain),'runtime_x':domain,'selector_receipt':'exact-ROM' if site in SELECTOR_BYTES else 'separate-current-source-proof','targets_closed':True})
 # Independent source-writer inventories for the two shared large object/state families.
 instructions={c['address']:c['instruction'] for c in an['contexts']}
 obj={addr for addr,ins in instructions.items() if ins=='STA $7E8CBD,X'}
 unit={addr for addr,ins in instructions.items() if ins=='STA $7E785B,X'}
 if obj!=OBJECT_WRITERS:errors.append(f'object-type writer inventory changed: {sorted(obj)}')
 if unit!=UNIT_STATE_WRITERS:errors.append(f'unit-state writer inventory changed: {sorted(unit)}')
 d4={addr for addr,ins in instructions.items() if '$1CF3' in ins and (ins.startswith('STZ ') or ins.startswith('INC '))}
 if d4!=D4_WRITERS:errors.append(f'D4 state writer inventory changed: {sorted(d4)}')
 # Guard/form invariants that bound the two 28-case object dispatches and the two masked unit dispatches.
 exact(raw,0xC0,0xA8DC,'bfbd8c7e297f003a0aaa','A8E9 object-type mask/selector')
 exact(raw,0xC0,0xB416,'bfbd8c7e297f003a0aaa','B420 object-type mask/selector')
 exact(raw,0xC0,0xEECA,'293f000aaa','EECF unit-state mask/selector')
 exact(raw,0xC0,0xF050,'bf5b787e293f0aaa','F058 unit-state mask/selector')
 out={'format':'civilization-v33-legacy-indirect-second-method-v1','result':'PASS' if not errors else 'FAIL','rom_sha256':sha,'analysis_sha256':hashlib.sha256(a.analysis.read_bytes()).hexdigest(),'proof_sha256':hashlib.sha256(a.indirect_proof.read_bytes()).hexdigest(),'site_count':len(EXPECTED),'priority_sites':['C3:0AC0','C0:A8E9','C0:B420','C0:F058','C0:EECF','D4:8D72'],'object_type_writer_count':len(obj),'unit_state_writer_count':len(unit),'d4_state_writer_count':len(d4),'rows':rows,'claim':'All 17 former legacy-risk sites are recertified by a checker independent of the fixed-point analyzer: exact selector-forming source bytes, exact ROM table words, closed-target ownership, and complete writer inventories for the priority shared state families. No adjacent table word is admitted.','runtime_observation_used_as_proof':False,'errors':errors}
 a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n');print(json.dumps({'result':out['result'],'sites':len(rows),'errors':len(errors)},sort_keys=True));return 0 if not errors else 1
if __name__=='__main__':raise SystemExit(main())
