#!/usr/bin/env python3
"""Rebuild and verify the governing manifest for the frozen exact-PC S-SMP AOT."""
from __future__ import annotations
import argparse, hashlib, json, re
from pathlib import Path

def sha(path: Path) -> str: return hashlib.sha256(path.read_bytes()).hexdigest()

def main() -> None:
    parser=argparse.ArgumentParser(); parser.add_argument("--project",type=Path,required=True); parser.add_argument("--out",type=Path,required=True); parser.add_argument("--check",action="store_true")
    args=parser.parse_args(); root=args.project.resolve(); smp=root/"static-recomp/static-audio/civilization-bapu-aot/smp"
    lookup=smp/"sc_smp_aot_lookup.inc"; dispatch=smp/"sc_smp_aot_dispatch.inc"; bitmap=smp/"sc_smp_aot_code_bitmap.inc"
    lookup_text=lookup.read_text(); dispatch_text=dispatch.read_text(); bitmap_text=bitmap.read_text()
    entries=[{"pc":int(pc,16),"opcode":int(opcode,16)} for pc,opcode in re.findall(r"case\s+0x([0-9a-fA-F]{4})u:\s+expected=0x([0-9a-fA-F]{2})u",lookup_text)]
    if len(entries)!=980 or len({item["pc"] for item in entries})!=980: raise SystemExit("exact-PC lookup must contain 980 unique entries")
    driver=[item for item in entries if item["pc"]<0xffc0]; ipl=[item for item in entries if item["pc"]>=0xffc0]
    if len(driver)!=948 or len(ipl)!=32: raise SystemExit(f"epoch partition mismatch: driver={len(driver)} ipl={len(ipl)}")
    dispatch_opcodes={int(value,16) for value in re.findall(r"^case\s+0x([0-9a-fA-F]{2}):",dispatch_text,re.MULTILINE)}
    expected_opcodes={item["opcode"] for item in entries}
    if expected_opcodes!=dispatch_opcodes: raise SystemExit(f"lookup/dispatcher opcode disagreement: missing={sorted(expected_opcodes-dispatch_opcodes)} extra={sorted(dispatch_opcodes-expected_opcodes)}")
    bitmap_bytes=[int(value,16) for value in re.findall(r"0x([0-9a-fA-F]{2})u",bitmap_text)]
    if len(bitmap_bytes)!=8192: raise SystemExit(f"code bitmap byte count mismatch: {len(bitmap_bytes)}")
    uncovered=[item["pc"] for item in driver if not bitmap_bytes[item["pc"]>>3]&(1<<(item["pc"]&7))]
    if uncovered: raise SystemExit(f"driver instruction starts absent from code bitmap: {uncovered[:8]}")
    manifest={"format":"civilization-smp-exact-pc-authority-v1","origin":"recovered-from-frozen-production-authority","proof_scope":"structural agreement of checked-in exact-PC lookup, used-opcode dispatcher, and driver code bitmap","proof_limitation":"the original ROM-derived closure JSON was not present in the frozen predecessor or searched archives; ROM/oracle recertification remains a separate test","entry_count":len(entries),"uploaded_driver_entry_count":len(driver),"ipl_entry_count":len(ipl),"used_opcodes":[f"{value:02X}" for value in sorted(dispatch_opcodes)],"entries":[{"pc":f"{item['pc']:04X}","opcode":f"{item['opcode']:02X}","epoch":"ipl" if item["pc"]>=0xffc0 else "uploaded-driver"} for item in entries],"hashes":{str(path.relative_to(root)).replace("\\","/"):sha(path) for path in [lookup,dispatch,bitmap]}}
    encoded=json.dumps(manifest,indent=2,sort_keys=True)+"\n"
    if args.check:
        if not args.out.exists() or args.out.read_text()!=encoded: raise SystemExit("governing S-SMP manifest is stale; rebuild it without --check")
    else:
        args.out.parent.mkdir(parents=True,exist_ok=True); args.out.write_text(encoded)
    print(json.dumps({"result":"PASS","entries":len(entries),"driver":len(driver),"ipl":len(ipl),"used_opcodes":len(dispatch_opcodes)},sort_keys=True))
if __name__=="__main__": main()
