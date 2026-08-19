#!/usr/bin/env python3
"""Independent Version-33 audit of Civilization's non-normal return mechanisms."""
from __future__ import annotations
import argparse,hashlib,json
from pathlib import Path

REENTRY=[
 ('C1:397E','C1:39BD','C1:3942/RTL',0xC1,0x397B,'a2bd398e0206a9c18d0406'),
 ('C1:A664','C1:A6AD','C1:A632/RTL',0xC1,0xA661,'a2ada68e0206a9c18d0406'),
 ('C1:B001','C1:B024','C1:AFF5/RTS',0xC1,0xAFFE,'a224b08e0206a9c18d0406'),
]
THREAD={'C0:84E8':{0,13,14,15,17},'C0:858A':{0,1,4,5,11,12,13,22,26},'C0:8665':set(range(32))}
def off(b,p):return ((b&0x3f)<<16)|p
def main():
 ap=argparse.ArgumentParser();ap.add_argument('rom',type=Path);ap.add_argument('--analysis',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
 raw=a.rom.read_bytes();sha=hashlib.sha256(raw).hexdigest();an=json.loads(a.analysis.read_text());errors=[];closure={c['address'] for c in an['contexts']}
 got={(e['writer'],e['resume'],e['owner_proc']) for e in an.get('nmi_stack_reentry_edges',[])}
 expected={(w,r,o) for w,r,o,*_ in REENTRY}
 if got!=expected:errors.append(f'NMI re-entry edge set changed: {sorted(got)}')
 rows=[]
 for writer,resume,owner,b,p,h in REENTRY:
  e=bytes.fromhex(h);g=raw[off(b,p):off(b,p)+len(e)]
  if g!=e:errors.append(f'{writer}: exact return-frame writer bytes changed')
  if writer not in closure or resume not in closure:errors.append(f'{writer}: writer/resume absent from closed graph')
  rows.append({'writer':writer,'resume':resume,'owner_proc':owner,'exact_writer_bytes':h})
 # Exact NMI handler loads $0604/$0602 and pushes replacement PBR:PC into the interrupted RTI frame.
 nmi=bytes.fromhex('a6028a18690900aae220ad04069500cacac220ad02069500')
 if raw[off(0xC0,0x8214):off(0xC0,0x8214)+len(nmi)]!=nmi:errors.append('C0:8214 NMI saved-frame replacement sequence changed')
 got_thread={k:set(v) for k,v in an.get('threaded_record_ids',{}).items()}
 if got_thread!=THREAD:errors.append(f'threaded RTS record-ID proof set changed: {got_thread}')
 for site in THREAD:
  if site not in closure:errors.append(f'threaded return site missing from closure: {site}')
 roots=an.get('interrupt_roots',[])
 nmi_roots={(r['address'],r['e'],r['m'],r['x']) for r in roots if r.get('label')=='nmi'}
 irq_roots={(r['address'],r['e'],r['m'],r['x']) for r in roots if r.get('label')=='irq'}
 expected_nmi={('00:81B9',0,m,x) for m in (0,1) for x in (0,1)}
 expected_irq={('00:81B0',0,m,x) for m in (0,1) for x in (0,1)}
 expected_brk={('00:81B5',0,m,x) for m in (0,1) for x in (0,1)}
 brk_roots={(r['address'],r['e'],r['m'],r['x']) for r in roots if r.get('label')=='native_brk'}
 if nmi_roots!=expected_nmi:errors.append('native NMI width-root set changed')
 if irq_roots!=expected_irq:errors.append('native IRQ width-root set changed')
 if brk_roots!=expected_brk:errors.append('native BRK width-root set changed')
 out={'format':'civilization-v33-special-return-reentry-v1','result':'PASS' if not errors else 'FAIL','rom_sha256':sha,'analysis_sha256':hashlib.sha256(a.analysis.read_bytes()).hexdigest(),'normal_return_sites_separately_audited':len(an.get('return_site_sets',{})),'nmi_stack_reentry_edges':rows,'threaded_rts_sites':{k:sorted(v) for k,v in THREAD.items()},'interrupt_root_count':len(roots),'claim':'Independent exact-ROM recertification covers the specialized NMI saved-return-frame replacements, the three threaded-RTS record dispatchers, and all native BRK/IRQ/NMI width roots excluded from the normal call/return checker.','runtime_observation_used_as_proof':False,'errors':errors}
 a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n');print(json.dumps({'result':out['result'],'reentries':len(rows),'threaded':len(THREAD),'errors':len(errors)},sort_keys=True));return 0 if not errors else 1
if __name__=='__main__':raise SystemExit(main())
