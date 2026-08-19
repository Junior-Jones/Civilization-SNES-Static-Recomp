#!/usr/bin/env python3
from pathlib import Path
import argparse, subprocess, tempfile, sys, json
ap=argparse.ArgumentParser(); ap.add_argument('headless',type=Path); ap.add_argument('rom',type=Path); a=ap.parse_args()
with tempfile.TemporaryDirectory() as td:
    p=Path(td)/'wrong.sfc'; b=bytearray(a.rom.read_bytes()); b[0]^=1; p.write_bytes(b)
    r=subprocess.run([str(a.headless),str(p)],text=True,capture_output=True)
    ok=r.returncode==4 and 'ROM rejected' in r.stderr
    print(json.dumps({'pass':ok,'returncode':r.returncode,'stderr':r.stderr.strip()}))
    sys.exit(0 if ok else 1)
