#!/usr/bin/env python3
"""Second-method recertification of the four Version-26 indirect proofs.

This checker does not import or execute the Version-26 proof programs.  It reads
the final closed context inventory, independently verifies source byte patterns,
recomputes each immutable table target from the exact ROM, and compares the
result with the production indirect-proof configuration.
"""
from __future__ import annotations
import argparse,hashlib,json
from pathlib import Path

def off(bank,pc): return ((bank&0x3f)<<16)|pc
def rb(rom,b,p,n): return rom[off(b,p):off(b,p)+n]
def exact(rom,b,p,h,label):
 e=bytes.fromhex(h);g=rb(rom,b,p,len(e))
 if g!=e:raise SystemExit(f'{label}: exact bytes changed at {b:02X}:{p:04X}')
def main():
 ap=argparse.ArgumentParser();ap.add_argument('rom',type=Path);ap.add_argument('--analysis',type=Path,required=True);ap.add_argument('--indirect-proof',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
 rom=a.rom.read_bytes();sha=hashlib.sha256(rom).hexdigest();m=json.loads(a.analysis.read_text());cfg=json.loads(a.indirect_proof.read_text())
 if m.get('rom_sha256')!=sha or m.get('static_analysis_frontier_count')!=0 or not m.get('work_queue_empty'):raise SystemExit('requires closed exact-ROM analysis')
 contexts={(r['address'],r['instruction']) for r in m['contexts']}
 rows={r['address']:r for r in cfg.get('proved_sites',[])}
 expected_domains={'C0:81E7':list(range(0,0x2A,2)),'C2:AD7A':list(range(0,0x24,2)),'C2:E992':[0,2,4,6],'D0:BB7C':[0,2,4,6,8,10,12]}
 tables={'C0:81E7':('C0',0x824D),'C2:AD7A':('C2',0xAD7F),'C2:E992':('C2',0xE9A2),'D0:BB7C':('D0',0xBB91)}
 # Independent source receipts for the selectors and their bounded state machines.
 exact(rom,0xC0,0x81DF,'c230a900005ba604fc4d82','NMI selector')
 exact(rom,0xC0,0x3564,'ad04008d1c07','NMI mode save')
 exact(rom,0xC0,0x358F,'ad1c078d0400','NMI mode restore')
 exact(rom,0xC2,0xAD72,'08c220ade11b0aaafc7fad','AD7A selector')
 exact(rom,0xC2,0xDCA0,'a903008de11b22eb00c0ade11b1a8de11bc90900d0f0','AD7A bounded loop A')
 exact(rom,0xC2,0xDE98,'a903008de11b22eb00c0ade11b1a8de11bc90900d0f0','AD7A bounded loop B')
 exact(rom,0xC2,0xE98E,'08ae3501fca2e9','E992 selector')
 exact(rom,0xC2,0xEAF3,'c2209c3501a910008d0400','E992 reset/enable')
 exact(rom,0xC2,0xEC51,'a202008e350122eb00c0','E992 state 2')
 exact(rom,0xC2,0xEE91,'08ae3501d00422eb00c0a204008e350122eb00c0a206008e350122eb00c0','E992 state 4/6')
 exact(rom,0xD0,0xBB76,'08e220ae0c01fc91bb','BB7C selector')
 exact(rom,0xD0,0xBF79,'a20c008e0c01a208008e0400','BB7C reset/enable')
 exact(rom,0xD0,0xE57E,'a202008e0c0122eb00c0a204008e0c0122eb00c0a206008e0c0122eb00c0a208008e0c0122eb00c0a20a008e0c0122eb00c0a200008e0c0122eb00c0','BB7C state cycle')
 # Rebuild the NMI-mode literal union from the final manifest's direct $0004 stores.
 # Every nonzero producer except the single save/restore path is an immediate load
 # immediately before its store; decode width is already explicit in instruction text.
 nmi_values=set();producer_count=0
 byaddr={r['address']:r for r in m['contexts']}
 for r in m['contexts']:
  if r['instruction'] not in {'STA $0004','STA $04','STX $0004','STX $04','STY $0004','STY $04','STZ $0004','STZ $04'}:continue
  producer_count+=1;addr=r['address']
  if r['instruction'].startswith('STZ '):nmi_values.add(0);continue
  if addr=='C0:3592':continue
  b=int(addr[:2],16);p=int(addr[3:],16)
  # Search the three bytes immediately before the store for the matching immediate
  # load opcode.  8-bit immediates are two bytes, 16-bit immediates three.
  loadop={'STA':0xA9,'STX':0xA2,'STY':0xA0}[r['instruction'][:3]]
  found=None
  for n in (2,3):
   q=(p-n)&0xffff;raw=rb(rom,b,q,n)
   if raw and raw[0]==loadop and q+n==p:
    found=raw[1] if n==2 else raw[1]|(raw[2]<<8);break
  if found is None:raise SystemExit(f'no immediate predecessor for NMI producer {addr}')
  nmi_values.add(found)
 if sorted(nmi_values)!=expected_domains['C0:81E7']:raise SystemExit(f'independent NMI domain mismatch: {sorted(nmi_values)}')
 if producer_count!=61:raise SystemExit(f'NMI producer inventory changed: {producer_count}')
 results=[]
 for site,domain in expected_domains.items():
  row=rows.get(site)
  cases=row.get('cases') if row else None
  if not isinstance(cases,list) or [int(c['selector_offset']) for c in cases]!=domain:raise SystemExit(f'production proof selector domain mismatch at {site}')
  bank=int(tables[site][0],16);base=tables[site][1];targets=[]
  for x in domain:
   q=off(bank,base+x);targets.append(f'{bank:02X}:{rom[q]|(rom[q+1]<<8):04X}')
  if [c['target'] for c in cases]!=targets:raise SystemExit(f'production target table mismatch at {site}')
  if [int(c['runtime_x']) for c in cases]!=domain:raise SystemExit(f'{site}: these four recertified sites must use raw X as selector offset')
  if (site,'JSR ($824D,X)' if site=='C0:81E7' else next((i for a,i in contexts if a==site),'')) not in contexts:
   # Presence check below is more tolerant of table formatting but still requires the site.
   if not any(a0==site and 'JSR (' in i0 for a0,i0 in contexts):raise SystemExit(f'site not present in closed contexts: {site}')
  results.append({'address':site,'legal_x':domain,'targets':targets})
 out={'format':'civilization-v33-independent-indirect-recertification-v1','rom_sha256':sha,'analysis_sha256':hashlib.sha256(a.analysis.read_bytes()).hexdigest(),'production_proof_sha256':hashlib.sha256(a.indirect_proof.read_bytes()).hexdigest(),'nmi_direct_producer_count':producer_count,'nmi_literal_union':sorted(nmi_values),'sites':results,'result':'PASS: second-method exact-ROM recertification reproduces all four finite selector domains and table targets without invoking the Version-26 proof programs.'}
 a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n');print(json.dumps({'result':'PASS','sites':{r['address']:len(r['legal_x']) for r in results},'nmi_producers':producer_count},sort_keys=True))
if __name__=='__main__':main()
