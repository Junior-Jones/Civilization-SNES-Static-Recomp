#!/usr/bin/env python3
"""Fail closed if a production source tree contains release-forbidden material."""
from __future__ import annotations
import argparse, hashlib, json, re
from pathlib import Path

FORBIDDEN_DIRS={"__pycache__","CMakeFiles","build","bin","obj"}
FORBIDDEN_NAME_PARTS=(".before-",".bak",".orig","~")
FORBIDDEN_SUFFIXES={".pyc",".pyo",".o",".obj",".a",".so",".dll",".exe",".sfc",".smc",".srm",".sav",".state",".zst",".7z",".rar",".zip"}
TEXT_SUFFIXES={".txt",".md",".json",".c",".h",".cpp",".hpp",".py",".sh",".cmake",""}
ABS_PATTERNS=[re.compile(r"/mnt/data/"),re.compile(r"/home/oai/"),re.compile(r"[A-Za-z]:\\\\Users\\\\",re.I)]
MAGICS=[(b"\x7fELF","ELF"),(b"MZ","PE/MZ")]

def main()->int:
    ap=argparse.ArgumentParser(); ap.add_argument("root",type=Path); ap.add_argument("--rom-sha256",default="")
    a=ap.parse_args(); root=a.root.resolve(); findings=[]; checked=0
    for p in sorted(root.rglob("*")):
        rel=p.relative_to(root).as_posix()
        if p.is_dir():
            if p.name in FORBIDDEN_DIRS or p.name.startswith("build-"): findings.append({"path":rel,"reason":"forbidden directory"})
            continue
        if not p.is_file() or p.is_symlink(): continue
        checked+=1
        low=p.name.lower()
        suffix=p.suffix.lower()
        if any(part in low for part in FORBIDDEN_NAME_PARTS): findings.append({"path":rel,"reason":"backup/transient filename"})
        if suffix in FORBIDDEN_SUFFIXES or low.endswith((".tar.gz",".tar.xz",".tgz")):
            findings.append({"path":rel,"reason":"forbidden file/archive/build suffix"})
        b=p.read_bytes()
        if a.rom_sha256 and hashlib.sha256(b).hexdigest().lower()==a.rom_sha256.lower():
            findings.append({"path":rel,"reason":"exact target ROM content"})
        head=b[:4]
        for magic,label in MAGICS:
            if head.startswith(magic): findings.append({"path":rel,"reason":f"compiled binary magic {label}"})
        if rel != "tools/packaging/validate_source_tree.py" and suffix in TEXT_SUFFIXES and len(b)<=8_000_000:
            try: text=b.decode("utf-8")
            except UnicodeDecodeError: text=""
            for rx in ABS_PATTERNS:
                if rx.search(text): findings.append({"path":rel,"reason":"working-machine absolute path"}); break
    result={"format":"civilization-source-tree-validation-v1","root_name":root.name,"checked_files":checked,"pass":not findings,"findings":findings}
    print(json.dumps(result,indent=2))
    return 0 if not findings else 1
if __name__=="__main__": raise SystemExit(main())
