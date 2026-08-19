#!/usr/bin/env python3
"""Create a sorted ZIP with fixed timestamps for reproducible project releases."""
from __future__ import annotations
import argparse
from pathlib import Path
import zipfile
FIXED=(2026,1,1,0,0,0)

def main()->int:
    ap=argparse.ArgumentParser()
    ap.add_argument('source',type=Path)
    ap.add_argument('output',type=Path)
    args=ap.parse_args()
    root=args.source.resolve()
    files=sorted(p for p in root.rglob('*') if p.is_file() and not p.is_symlink())
    args.output.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(args.output,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
        for p in files:
            rel=Path(root.name)/p.relative_to(root)
            info=zipfile.ZipInfo(rel.as_posix(),FIXED)
            info.compress_type=zipfile.ZIP_DEFLATED
            mode = 0o755 if (p.stat().st_mode & 0o111) else 0o644
            info.external_attr=((0o100000 | mode) & 0xFFFF)<<16
            z.writestr(info,p.read_bytes(),compress_type=zipfile.ZIP_DEFLATED,compresslevel=9)
    return 0
if __name__=='__main__': raise SystemExit(main())
