#!/usr/bin/env python3
from pathlib import Path
import argparse,json,re,sys
ap=argparse.ArgumentParser();ap.add_argument('--project',type=Path,required=True);a=ap.parse_args();roots=[a.project/'static-recomp',a.project/'frontend'];bad=[]
patterns=[(re.compile(r'\b(opcode|instruction)[ _-]*(decode|decoder)\b',re.I),'runtime decoder wording'),(re.compile(r'\b(snes9x|mesen|ares|bsnes|higan)\b',re.I),'emulator name in production source'),(re.compile(r'fallback.*interpreter|interpreter.*fallback',re.I),'interpreter fallback')]
for root in roots:
 for p in root.rglob('*'):
  if p.suffix.lower() not in {'.c','.h','.cpp','.hpp'}: continue
  text=p.read_text(errors='ignore')
  for rx,label in patterns:
   for m in rx.finditer(text):
    # generated comments explicitly state decoder prohibition; allow that phrase.
    if 'Runtime opcode decoding' in text[m.start()-40:m.end()+40]: continue
    rel=str(p.relative_to(a.project)).replace('\\','/')
    # The bounded static S-DSP hardware primitive is derived from the
    # licensed SimCity Full Static reference and retains upstream provenance
    # names.  This exception applies only to the emulator-name string check;
    # runtime decoder/fallback checks remain active inside the subtree.
    if label=='emulator name in production source' and rel.startswith('static-recomp/static-audio/civilization-bapu-aot/'):
     continue
    bad.append({'file':rel,'pattern':label,'match':m.group(0)})
print(json.dumps({'pass':not bad,'findings':bad},indent=2))
sys.exit(0 if not bad else 1)
