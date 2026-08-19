#!/usr/bin/env python3
"""Prove Civilization's reset/setup settling cut and procedure lifecycle partition."""
from __future__ import annotations
import argparse,hashlib,json
from pathlib import Path

def main():
 ap=argparse.ArgumentParser();ap.add_argument('rom',type=Path);ap.add_argument('--ownership',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
 raw=a.rom.read_bytes();digest=hashlib.sha256(raw).hexdigest();o=json.loads(a.ownership.read_text())
 if o.get('rom_sha256')!=digest:raise SystemExit('ownership/ROM mismatch')
 roots=[p for p in o['procedures'] if p['kind']=='ROOT']
 if len(roots)!=1 or roots[0]['entry']!='00:804A':raise SystemExit('unexpected reset root')
 root=roots[0];rid=root['procedure'];cut='C0:8155'
 # Exact source transition: final one-time call C0:8151 JSL C0:1185 returns to C0:8155,
 # after which the root enters its persistent state loop.
 exp=bytes.fromhex('223083c0228511c0c22022c31ec02226adc16404c90200d0062232a6c180ef')
 got=raw[0x814D:0x814D+len(exp)]
 if got!=exp:raise SystemExit('C0:814D persistent-loop transition bytes changed')
 rg={}
 for e in root['direct_edges']:rg.setdefault(e['from'],set()).add(e['to'])
 for e in o['call_edges']:
  if e['caller']==rid:rg.setdefault(e['site'],set()).add(e['continuation'])
 def reach(start,stop=None):
  seen=set();q=[start]
  while q:
   n=q.pop()
   if n in seen:continue
   seen.add(n)
   if n==stop:continue
   q.extend(rg.get(n,()))
  return seen
 pre=reach('00:804A',cut);post=reach(cut)
 if pre&post!={cut}:raise SystemExit(f'boot/post root regions overlap beyond cut: {sorted(pre&post)}')
 back=[(x,y) for x in post for y in rg.get(x,()) if y in pre and y!=cut]
 if back:raise SystemExit(f'persistent loop returns to one-time root code: {back}')
 cg={}
 for e in o['call_edges']:cg.setdefault(e['caller'],set()).add(e['callee'])
 pre_roots={e['callee'] for e in o['call_edges'] if e['caller']==rid and e['site'] in (pre-post)}
 post_roots={e['callee'] for e in o['call_edges'] if e['caller']==rid and e['site'] in post}
 irq_roots={p['procedure'] for p in o['procedures'] if p['kind']=='RTI' and p['entry'] in {'00:81B5','00:81B9','00:81B0'}}
 def closure(rs):
  seen=set();q=list(rs)
  while q:
   n=q.pop()
   if n in seen or n==rid:continue
   seen.add(n);q.extend(cg.get(n,()))
  return seen
 boot=closure(pre_roots);runtime=closure(post_roots);interrupt=closure(irq_roots)|irq_roots
 allp={p['procedure'] for p in o['procedures']};covered={rid}|boot|runtime|interrupt
 if covered!=allp:raise SystemExit(f'unclassified procedures: {len(allp-covered)}')
 classes={
  'boot_only':sorted(boot-runtime-interrupt),
  'runtime_only':sorted(runtime-boot-interrupt),
  'interrupt_only':sorted(interrupt-boot-runtime),
  'boot_runtime_shared':sorted((boot&runtime)-interrupt),
  'boot_interrupt_shared':sorted((boot&interrupt)-runtime),
  'runtime_interrupt_shared':sorted((runtime&interrupt)-boot),
  'all_three_shared':sorted(boot&runtime&interrupt),
 }
 top_pre=sorted({(e['site'],e['callee']) for e in o['call_edges'] if e['caller']==rid and e['site'] in pre-post})
 top_post=sorted({(e['site'],e['callee']) for e in o['call_edges'] if e['caller']==rid and e['site'] in post})
 out={'format':'civilization-v33-lifecycle-partition-v1','rom_sha256':digest,'ownership_sha256':hashlib.sha256(a.ownership.read_bytes()).hexdigest(),
      'settling_checkpoint':cut,'checkpoint_meaning':'first instruction after the final one-time C0:1185 initialization call; entry to the persistent root game loop',
      'root_runtime_context_count':root['runtime_context_count'],'root_boot_region_nodes':len(pre),'root_persistent_region_nodes':len(post),
      'root_region_intersection':sorted(pre&post),'post_to_boot_back_edges':back,
      'procedure_count':len(allp),'boot_reachable_procedures':len(boot),'persistent_runtime_reachable_procedures':len(runtime),'interrupt_reachable_procedures':len(interrupt),
      'classification_counts':{k:len(v) for k,v in classes.items()},'classification':classes,
      'boot_top_level_calls':[{'site':s,'callee':c} for s,c in top_pre],
      'persistent_top_level_calls':[{'site':s,'callee':c} for s,c in top_post],
      'result':'PASS: C0:8155 is a source-proved one-way settling cut inside the single static console. Reset/setup code does not become a second runtime authority, and all proved procedures are classified as boot, persistent runtime, interrupt, or explicit shared utility.'}
 a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n')
 print(json.dumps({'result':'PASS','cut':cut,'root_boot_nodes':len(pre),'root_runtime_nodes':len(post),'procedures':len(allp),'classes':out['classification_counts']},sort_keys=True))
if __name__=='__main__':main()
