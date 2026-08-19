#!/usr/bin/env python3
import argparse, hashlib, json
from pathlib import Path

def digest(path): return hashlib.sha256(path.read_bytes()).hexdigest()

ap=argparse.ArgumentParser()
ap.add_argument('--project',type=Path,required=True)
ap.add_argument('--write',action='store_true')
a=ap.parse_args(); root=a.project.resolve()
manifest_path=root/'SOURCE-PROVENANCE.json'
paths=[p for p in root.rglob('*') if p.is_file() and p!=manifest_path
       and '.git' not in p.parts and '__pycache__' not in p.parts]
paths=sorted(paths,key=lambda p:p.relative_to(root).as_posix())
doc={'format':'civilization-v36-source-provenance-v1','files':{p.relative_to(root).as_posix():digest(p) for p in paths}}
if a.write:
    manifest_path.write_text(json.dumps(doc,indent=2,sort_keys=True)+'\n',encoding='utf-8',newline='\n')
else:
    expected=json.loads(manifest_path.read_text(encoding='utf-8'))
    if expected!=doc: raise SystemExit('source/generated provenance manifest differs; regenerate intentionally with --write')

compact=json.loads((root/'static-recomp/generated/compact-aot/civilization_compact_aot.manifest.json').read_text(encoding='utf-8'))
shards=root/'static-recomp/generated/compact-aot/civilization_compact_aot_shards'
for name,want in compact['shards'].items():
    got=digest(shards/name)
    if got!=want: raise SystemExit(f'generated shard hash mismatch: {name}')
if len(compact['shards'])!=280: raise SystemExit('generated shard manifest count is not 280')
print(f"PASS provenance_files={len(doc['files'])} generated_shards={len(compact['shards'])}")
