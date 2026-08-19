#!/usr/bin/env python3
"""Source-only ownership audit for the already validated EARTH/first-city route."""
from __future__ import annotations
import argparse,hashlib,json
from collections import defaultdict,deque
from pathlib import Path

# These are static menu/state dispatcher sites already source-identified by the
# Version-32 route audit; no runtime PC trace is used to create this list.
CLUSTERS={
 'setup-earth':['C3:0194','C3:0266','C3:0435'],
 'tribe-leader':['C3:08B8','C3:0C9F'],
 'first-city-and-city-menu':['C3:0AC0','C3:236E'],
 'city-production-view-endturn-research':['C0:A8E9','C0:B420','C0:EECF','C0:F058'],
}
def proc_addr(proc):return proc.split('/')[0]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('rom',type=Path);ap.add_argument('--analysis',type=Path,required=True);ap.add_argument('--ownership',type=Path,required=True);ap.add_argument('--indirect-proof',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
 raw=a.rom.read_bytes();sha=hashlib.sha256(raw).hexdigest();an=json.loads(a.analysis.read_text());own=json.loads(a.ownership.read_text());proof=json.loads(a.indirect_proof.read_text());errors=[]
 proof_by={r['address']:r for r in proof['proved_sites']};contexts={c['address'] for c in an['contexts']}
 edges=defaultdict(set)
 for e in an['call_edges']:edges[e['caller_proc']].add(e['callee_proc'])
 # Map every dispatch site to the owning procedure using ownership manifest.
 proc_rows=own.get('procedures',[]);owner={}
 for p in proc_rows:
  for addr in p.get('runtime_contexts',[]):owner[addr.split('/')[0]]=p.get('procedure') or p.get('id')
 # Older ownership format may not list runtime_contexts. Fall back to the analyzer's
 # procedure starts by locating callers whose site matches.
 for e in an['call_edges']:owner.setdefault(e['site'],e['caller_proc'])
 rows=[]
 for name,sites in CLUSTERS.items():
  targets=set();owners=set()
  for site in sites:
   r=proof_by.get(site)
   if not r:errors.append(f'{name}: missing finite proof {site}');continue
   if site not in contexts:errors.append(f'{name}: site absent from closed graph {site}')
   targets.update(c['target'] for c in r['cases']);
   if owner.get(site):owners.add(owner[site])
   for c in r['cases']:
    if c['target'] not in contexts:errors.append(f'{name}: target not closed {c["target"]}')
  # Calculate a bounded static call-ownership neighborhood from the procedures
  # containing these source-identified dispatchers. This is ownership evidence,
  # not an assertion that every reachable utility belongs exclusively to one UI scene.
  reached=set(owners);q=deque(owners)
  while q and len(reached)<5000:
   p=q.popleft()
   for t in edges.get(p,()):
    if t not in reached:reached.add(t);q.append(t)
  rows.append({'cluster':name,'finite_dispatch_sites':sites,'site_count':len(sites),'declared_target_count':len(targets),'owner_procedures':sorted(owners),'static_call_neighborhood_procedures':len(reached)})
 if an.get('static_analysis_frontier_count')!=0 or not an.get('work_queue_empty'):errors.append('fixed point is not closed')
 out={'format':'civilization-v33-earth-city-lifecycle-clusters-v1','result':'PASS' if not errors else 'FAIL','rom_sha256':sha,'runtime_context_count':an.get('runtime_context_count'),'procedure_count':an.get('procedure_count'),'clusters':rows,'claim':'The passed EARTH/setup/leader/first-city/Production/View/End-Turn/research route is represented as source-identified finite dispatcher ownership clusters inside the closed graph. Runtime gameplay PCs are not used as proof inputs.','runtime_observation_used_as_proof':False,'errors':errors}
 a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n');print(json.dumps({'result':out['result'],'clusters':len(rows),'errors':len(errors)},sort_keys=True));return 0 if not errors else 1
if __name__=='__main__':raise SystemExit(main())
