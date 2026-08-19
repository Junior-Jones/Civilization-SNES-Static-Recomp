#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'rom'))
from civilization_rom import HiRom, EXPECTED_SHA, EXPECTED_SIZE

STRONG={
 'C3:0194':9,
 'C3:0266':6,
 'C3:0435':5,
 'C3:08B8':6,
 'C3:0C9F':21,
 'C3:236E':4,
}
LEGACY={'C3:0AC0':22}

def main()->int:
 ap=argparse.ArgumentParser()
 ap.add_argument('rom',type=Path); ap.add_argument('--analysis',type=Path,required=True)
 ap.add_argument('--indirect-proof',type=Path,required=True); ap.add_argument('--out',type=Path,required=True)
 a=ap.parse_args()
 raw=a.rom.read_bytes(); digest=hashlib.sha256(raw).hexdigest()
 if len(raw)!=EXPECTED_SIZE or digest!=EXPECTED_SHA: raise SystemExit('wrong Civilization ROM')
 rom=HiRom(raw); an=json.loads(a.analysis.read_text()); proof=json.loads(a.indirect_proof.read_text())
 by={r['address']:r for r in proof.get('proved_sites',[])}
 closure={c['address'] for c in an.get('contexts',[])}
 errors=[]; rows=[]
 for addr,expected in {**STRONG,**LEGACY}.items():
  r=by.get(addr)
  if not r: errors.append(f'{addr}: missing proof row'); continue
  cases=r.get('cases',[])
  if len(cases)!=expected: errors.append(f'{addr}: expected {expected} cases, got {len(cases)}')
  bank=int(addr[:2],16)
  case_rows=[]
  for c in cases:
   ptr=int(c['effective_pointer'])&0xffff
   lo=rom.fetch(bank,ptr); hi=rom.fetch(bank,(ptr+1)&0xffff)
   target=f'{bank:02X}:{(lo|(hi<<8)):04X}'
   if target!=c['target']: errors.append(f"{addr}: ROM pointer {ptr:04X} -> {target}, proof says {c['target']}")
   if c['target'] not in closure: errors.append(f"{addr}: target absent from closed runtime contexts: {c['target']}")
   case_rows.append({'runtime_x':c['runtime_x'],'effective_pointer':ptr,'target':c['target']})
  status=r.get('status','')
  if addr in STRONG:
   if not any(k in status for k in ('exact ROM hitbox','complete-writer/caller','complete closed-caller')):
    errors.append(f'{addr}: strong route site lacks expected source-proof status: {status}')
  rows.append({'address':addr,'case_count':len(cases),'status':status,'proof_strength':'strong-current' if addr in STRONG else 'legacy-finite-needs-second-method','cases':case_rows})
 if an.get('static_analysis_frontier_count')!=0 or an.get('static_analysis_frontiers')!=[]:
  errors.append('analysis is not zero-frontier')
 out={
  'format':'civilization-v33-setup-city-route-coverage-v1',
  'result':'PASS' if not errors else 'FAIL',
  'rom_sha256':digest,
  'runtime_context_count':an.get('runtime_context_count'),
  'strong_current_site_count':len(STRONG),
  'legacy_finite_site_count':len(LEGACY),
  'claim':'Known setup/city menu-return and city-name dispatch surfaces are present in the exact-ROM closure; every declared target is re-read from immutable ROM and is a closed runtime context. This audit does not upgrade the legacy C3:0AC0 domain proof.',
  'runtime_observation_used_as_proof':False,
  'sites':rows,
  'future_strengthening':['C3:0AC0'],
  'errors':errors,
 }
 a.out.parent.mkdir(parents=True,exist_ok=True); a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n')
 print(json.dumps({'result':out['result'],'strong':len(STRONG),'legacy':len(LEGACY),'errors':len(errors)},sort_keys=True))
 return 0 if not errors else 1
if __name__=='__main__': raise SystemExit(main())
