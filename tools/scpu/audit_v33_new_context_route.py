#!/usr/bin/env python3
"""Source-only audit of the C3:2732 route newly admitted in Version 33."""
from __future__ import annotations
import argparse,hashlib,json,re
from pathlib import Path
ROM_SHA='de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32'
EXPECTED_CALLS={
 'C3:273F':('C0:031C','C3:2743'),
 'C3:279B':('C6:0000','C3:279F'),
 'C3:279F':('C3:27CA','C3:27A3'),
 'C3:27A3':('C3:280D','C3:27A7'),
}
def main():
 ap=argparse.ArgumentParser();ap.add_argument('rom',type=Path);ap.add_argument('--analysis',type=Path,required=True);ap.add_argument('--ownership',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
 raw=a.rom.read_bytes();sha=hashlib.sha256(raw).hexdigest();m=json.loads(a.analysis.read_text());o=json.loads(a.ownership.read_text())
 if sha!=ROM_SHA or m.get('rom_sha256')!=sha or o.get('rom_sha256')!=sha: raise SystemExit('ROM/manifest mismatch')
 if m.get('static_analysis_frontier_count')!=0 or not m.get('work_queue_empty'): raise SystemExit('analysis not closed')
 procs=[p for p in o['procedures'] if p['entry']=='C3:2732' and p['kind']=='RTS']
 if len(procs)!=1: raise SystemExit(f'expected one C3:2732/RTS procedure, got {len(procs)}')
 p=procs[0]; ctxkeys=p['runtime_contexts']
 if len(ctxkeys)!=48: raise SystemExit(f'C3:2732 procedure context count changed: {len(ctxkeys)}')
 ctx={(r['address'],r['e'],r['m'],r['x']):r for r in m['contexts']}
 rows=[]; mmio=[]; controls=[]
 for k in ctxkeys:
  addr,flags=k.split('/'); e=int(flags[1]);mm=int(flags[3]);x=int(flags[5]);r=ctx.get((addr,e,mm,x))
  if not r: raise SystemExit(f'ownership context missing from analysis: {k}')
  pc=int(addr[3:],16)
  if not (0x2732<=pc<=0x27A8): raise SystemExit(f'new procedure escaped expected exact-ROM range: {addr}')
  ins=r['instruction']; rows.append({'address':addr,'instruction':ins,'bytes':r['bytes']})
  for h in re.findall(r'\$([0-9A-Fa-f]{4})',ins):
   v=int(h,16)
   if 0x2100<=v<=0x21ff or 0x4000<=v<=0x43ff: mmio.append((addr,ins))
  if ins.startswith(('JSR ','JSL ','JMP ','RTL','RTS','RTI','BRK','COP','WAI','STP')): controls.append((addr,ins))
 if mmio: raise SystemExit(f'C3:2732 route unexpectedly contains local MMIO-looking operands: {mmio}')
 # Only four direct calls and one RTS are allowed in this newly admitted procedure.
 got={e['site']:(e['callee'].split('/')[0],e['continuation']) for e in o['call_edges'] if e['caller']==p['procedure']}
 if got!=EXPECTED_CALLS: raise SystemExit(f'C3:2732 call-edge set changed: {got}')
 if controls!=[('C3:273F','JSL $C0031C'),('C3:279B','JSL $C60000'),('C3:279F','JSL $C327CA'),('C3:27A3','JSL $C3280D'),('C3:27A8','RTS')]: raise SystemExit(f'unexpected control ops: {controls}')
 ret=m['return_site_sets'].get('C3:27A8/RTS')
 if ret!=['C3:2371']: raise SystemExit(f'C3:27A8 return proof changed: {ret}')
 out={'format':'civilization-v33-new-context-route-audit-v1','result':'PASS','rom_sha256':sha,'procedure':'C3:2732/RTS','runtime_context_count':48,'address_range':'C3:2732-C3:27A8','new_direct_calls':EXPECTED_CALLS,'return_site':'C3:27A8/RTS','return_targets':ret,'local_mmio_operands':[],'control_instructions':controls,'claim':'The 48 newly admitted contexts form one bounded exact-ROM setup/data-copy procedure. They add no local MMIO operand, no indirect control transfer, and call only already-owned closed procedures.'}
 a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n');print(json.dumps({'result':'PASS','contexts':48,'calls':4},sort_keys=True))
if __name__=='__main__': main()
