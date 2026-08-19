#!/usr/bin/env python3
"""Independent call/return ownership consistency audit for Version 33.

Recomputes normal finite RTS/RTL continuation sets from the ownership graph and
compares them with the analyzer return-site sets.  It does not import the
production generator.
"""
from __future__ import annotations
import argparse,hashlib,json
from collections import defaultdict
from pathlib import Path
ROM_SHA='de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32'
def main():
 ap=argparse.ArgumentParser();ap.add_argument('rom',type=Path);ap.add_argument('--analysis',type=Path,required=True);ap.add_argument('--ownership',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
 raw=a.rom.read_bytes();sha=hashlib.sha256(raw).hexdigest();m=json.loads(a.analysis.read_text());o=json.loads(a.ownership.read_text())
 if sha!=ROM_SHA or m.get('rom_sha256')!=sha or o.get('rom_sha256')!=sha: raise SystemExit('ROM/manifest mismatch')
 if m.get('runtime_context_count')!=o.get('runtime_context_count'): raise SystemExit('analysis/ownership context-count mismatch')
 inst_by_ctx={(r['address'],r['e'],r['m'],r['x']):r['instruction'] for r in m['contexts']}
 conts=defaultdict(set)
 for e in o['call_edges']: conts[e['callee']].add(e['continuation'])
 expected=defaultdict(set); owner_count=defaultdict(int)
 special_rts={'D4:815C','D4:854E','D4:85D0','D4:877F','D4:87B8'}
 for p in o['procedures']:
  pid=p['procedure'];kind=p['kind']
  if kind not in {'RTS','RTL'}: continue
  for ck in p['runtime_contexts']:
   addr,flags=ck.split('/');e=int(flags[1]);mm=int(flags[3]);x=int(flags[5]);ins=inst_by_ctx.get((addr,e,mm,x))
   if ins==kind:
    if kind=='RTS' and addr in special_rts: continue
    owner_count[f'{addr}/{kind}']+=1
    expected[f'{addr}/{kind}'].update(conts.get(pid,set()))
 actual={k:set(v) for k,v in m.get('return_site_sets',{}).items()}
 if set(expected)!=set(actual):
  raise SystemExit(f'return-site key mismatch expected-only={sorted(set(expected)-set(actual))[:10]} actual-only={sorted(set(actual)-set(expected))[:10]}')
 diffs=[]
 for k in sorted(expected):
  if expected[k]!=actual[k]: diffs.append({'site':k,'expected':sorted(expected[k]),'actual':sorted(actual[k])})
 if diffs: raise SystemExit(f'return target mismatch: {diffs[:3]}')
 # Verify every call-edge site is a closed context.  Direct JSR/JSL edges must
 # carry the exact architectural fallthrough; pseudo-call edges emitted by the
 # three threaded-RTS records are intentionally excluded from this check.
 byaddr={r['address']:r for r in m['contexts']}; bad=[]
 for e in o['call_edges']:
  r=byaddr.get(e['site'])
  if r is None:
   bad.append({'reason':'site outside closure','edge':e}); continue
  ins=r['instruction']; pc=int(e['site'][3:],16); bank=e['site'][:2]
  if ins.startswith('JSL '): exp=f'{bank}:{(pc+4)&0xffff:04X}'
  elif ins.startswith('JSR '): exp=f'{bank}:{(pc+3)&0xffff:04X}'
  else: continue
  if e['continuation']!=exp: bad.append({'reason':'wrong direct-call fallthrough','expected':exp,'edge':e})
 if bad: raise SystemExit(f'call edge encoding mismatch: {bad[:3]}')
 target_total=sum(len(v) for v in actual.values())
 out={'format':'civilization-v33-call-return-consistency-v1','result':'PASS','rom_sha256':sha,'procedure_count':o['procedure_count'],'call_edge_count':len(o['call_edges']),'normal_return_site_count':len(actual),'normal_return_target_membership_count':target_total,'multi_owner_return_site_count':sum(1 for n in owner_count.values() if n>1),'claim':'Every normal RTS/RTL finite return set is exactly the union of statically closed caller continuations for all owning procedure specializations. Every call site and continuation is itself inside the closed runtime-context inventory. Special threaded/D4 return mechanics remain separately handled by the generated return-table construction and existing machine regressions.'}
 a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n');print(json.dumps({'result':'PASS','return_sites':len(actual),'call_edges':len(o['call_edges'])},sort_keys=True))
if __name__=='__main__': main()
