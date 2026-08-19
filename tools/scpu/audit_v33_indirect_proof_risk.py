#!/usr/bin/env python3
"""Non-authoritative prioritization audit for remaining finite indirect proofs."""
from __future__ import annotations
import argparse,json
from pathlib import Path
WEAK=('Version 25 reconstructed','finite current proof')
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--indirect-proof',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args();d=json.loads(a.indirect_proof.read_text())
 rows=[]
 for r in d['proved_sites']:
  status=r.get('status',''); proof=r.get('proof','')
  risk='strengthened'
  recertified='Version 33 independent second-method recertified' in status
  if (any(x in status for x in WEAK) or not proof) and not recertified: risk='legacy-proof-needs-second-method-recertification'
  elif recertified: risk='second-method-recertified'
  rows.append({'address':r['address'],'case_count':len(r['cases']),'status':status,'risk':risk})
 weak=[r for r in rows if r['risk']=='legacy-proof-needs-second-method-recertification']
 recert=[r for r in rows if r['risk']=='second-method-recertified']
 out={'format':'civilization-v33-indirect-proof-risk-inventory-v1','result':'PASS_INVENTORY_ONLY','site_count':len(rows),'legacy_second_method_priority_count':len(weak),'legacy_second_method_priorities':weak,'second_method_recertified_count':len(recert),'second_method_recertified_sites':[r['address'] for r in recert],'method_note':'This report never widens proof. Version 33 closes the former legacy second-method backlog; remaining strengthening work is producer/value-set completeness, tracked separately by the generalized indirect-domain audit.'}
 a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n');print(json.dumps({'sites':len(rows),'legacy_priority':len(weak)},sort_keys=True))
if __name__=='__main__':main()
