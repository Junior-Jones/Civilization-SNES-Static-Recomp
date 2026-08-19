#!/usr/bin/env python3
"""Generate the ordered top-level guest-state field list used by CVSNAP36."""
import re, sys
from pathlib import Path

root=Path(sys.argv[1])
text=(root/'static-recomp/include/civilization_static_recomp.h').read_text(encoding='utf-8')
body=text.split('struct CivRecomp {',1)[1].split('\n};',1)[0]
body=re.sub(r'/\*.*?\*/','',body,flags=re.S)
excluded={'rom','rom_size','host_hooks','headless_frame_stop_enabled','headless_frame_stop_reached','headless_frame_stop_target'}
fields=[]
for statement in body.split(';'):
    statement=statement.strip()
    if not statement or statement.startswith('#'): continue
    match=re.search(r'([A-Za-z_]\w*)\s*(?:\[[^;]+\])?\s*$',statement)
    if match and match.group(1) not in excluded: fields.append(match.group(1))
out=root/'static-recomp/internal/include/civilization_snapshot_fields.inc'
out.write_text('/* Generated ordered CVSNAP36 guest-state schema. */\n'+''.join(f'CIV_STATE_FIELD({f})\n' for f in fields),encoding='utf-8',newline='\n')
print(f'wrote {len(fields)} snapshot fields')
