#!/usr/bin/env python3
from pathlib import Path
import argparse,json,sys
from civilization_rom import inspect,EXPECTED_SHA,EXPECTED_SIZE
p=argparse.ArgumentParser();p.add_argument('rom',type=Path);p.add_argument('--json',type=Path);p.add_argument('--expect',action='store_true');a=p.parse_args();r=inspect(a.rom)
text=json.dumps(r,indent=2,sort_keys=True)+"\n"; print(text,end='')
if a.json:a.json.parent.mkdir(parents=True,exist_ok=True);a.json.write_text(text)
if a.expect:
 ok=(r['expected_identity'] and r['map_mode']=='0x31' and r['cartridge_type']=='0x02' and r['sram_bytes']==32768 and r['reset_vector']=='00:804A' and r['checksum_pair_ok'] and r['mirrored_checksum']=='0x5082')
 if not ok: sys.exit('ROM expectation failed')
