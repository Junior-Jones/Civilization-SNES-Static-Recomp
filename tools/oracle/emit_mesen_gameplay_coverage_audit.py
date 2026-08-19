#!/usr/bin/env python3
"""Emit a deterministic Mesen gameplay route with S-CPU/S-SMP ownership checks.

Runtime observations are comparison evidence only. They must never be imported
into either static authority or used to infer that an unobserved path is dead.
"""

import argparse
import json
import re
from pathlib import Path


BUTTONS = {"a", "b", "x", "y", "l", "r", "up", "down", "left", "right", "start", "select"}


def load_route(path: Path) -> dict:
    route = json.loads(path.read_text(encoding="utf-8"))
    if route.get("schema") != 1:
        raise SystemExit("route schema must be 1")
    frame_limit = int(route["frame_limit"])
    if frame_limit < 1:
        raise SystemExit("frame_limit must be positive")
    for tap in route.get("taps", []):
        if tap["button"] not in BUTTONS:
            raise SystemExit(f"unsupported button: {tap['button']}")
        if not 0 <= int(tap["start"]) <= int(tap["end"]) < frame_limit:
            raise SystemExit(f"invalid tap interval: {tap}")
    for row in route.get("checkpoints", []):
        if not 1 <= int(row["frame"]) <= frame_limit:
            raise SystemExit(f"invalid checkpoint: {row}")
    return route


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--smp-lookup", type=Path, required=True)
    parser.add_argument("--route", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    snes = sorted({int(row["address"].replace(":", ""), 16) for row in manifest["contexts"]})
    lookup = args.smp_lookup.read_text(encoding="utf-8")
    spc = sorted({int(value, 16) for value in re.findall(r"case 0x([0-9a-fA-F]{4})u:", lookup)})
    if not snes or not spc:
        raise SystemExit("empty static authority")
    route = load_route(args.route)

    lines = [
        "-- Generated gameplay comparator. Runtime observations never widen authority.",
        "local snesAuthority = {",
        *(f"  [0x{address:06X}] = true," for address in snes),
        "}",
        "local spcAuthority = {",
        *(f"  [0x{address:04X}] = true," for address in spc),
        "}",
        "local frame = 0",
        "local snesInstructions, spcInstructions = 0, 0",
        "local snesSeen, spcSeen, snesMissing, spcMissing = {}, {}, {}, {}",
        "local snesUnique, spcUnique, snesOwned, spcOwned = 0, 0, 0, 0",
        "local taps = {",
    ]
    lines.extend(
        f'  {{{int(tap["start"])},{int(tap["end"])},"{tap["button"]}"}},'
        for tap in route.get("taps", [])
    )
    lines.extend(["}", "local checkpoints = {"])
    lines.extend(
        f'  [{int(row["frame"])}] = "{row["label"]}",'
        for row in route.get("checkpoints", [])
    )
    lines.extend(
        [
            "}",
            "local function inputPolled()",
            "  local value = {a=false,b=false,x=false,y=false,l=false,r=false,up=false,down=false,left=false,right=false,start=false,select=false}",
            "  for _, tap in ipairs(taps) do if frame >= tap[1] and frame <= tap[2] then value[tap[3]] = true end end",
            "  emu.setInput(value, 0)",
            "end",
            "local function onSnesExec(address)",
            "  snesInstructions = snesInstructions + 1",
            "  if not snesSeen[address] then",
            "    snesSeen[address] = true; snesUnique = snesUnique + 1",
            "    if snesAuthority[address] then snesOwned = snesOwned + 1 else snesMissing[address] = true end",
            "  end",
            "end",
            "local function onSpcExec(address)",
            "  spcInstructions = spcInstructions + 1",
            "  if not spcSeen[address] then",
            "    spcSeen[address] = true; spcUnique = spcUnique + 1",
            "    if spcAuthority[address] then spcOwned = spcOwned + 1 else spcMissing[address] = true end",
            "  end",
            "end",
            "local function screenCertificate()",
            "  local buffer, hash, nonblack, colors = emu.getScreenBuffer(), 2166136261, 0, {}",
            "  for i = 1, #buffer do",
            "    local pixel = buffer[i]",
            "    local r, g, b = (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF",
            "    local p = math.floor((r*31+127)/255) | (math.floor((g*31+127)/255)<<5) | (math.floor((b*31+127)/255)<<10)",
            "    hash = ((hash ~ (p & 0xFF)) * 16777619) & 0xFFFFFFFF",
            "    hash = ((hash ~ ((p >> 8) & 0xFF)) * 16777619) & 0xFFFFFFFF",
            "    if p ~= 0 then nonblack = nonblack + 1 end; colors[p] = true",
            "  end",
            "  local unique = 0; for _ in pairs(colors) do unique = unique + 1 end",
            "  return hash, #buffer, nonblack, unique",
            "end",
            "local function renderMissing(values, digits)",
            "  local rows = {}; for address in pairs(values) do table.insert(rows, address) end; table.sort(rows)",
            "  local out = {}; for _, address in ipairs(rows) do table.insert(out, string.format('%0' .. digits .. 'X', address)) end",
            "  return #rows, table.concat(out, ',')",
            "end",
            "local function endFrame()",
            "  frame = frame + 1",
            "  local label = checkpoints[frame]",
            "  if label then",
            "    local hash, pixels, nonblack, unique = screenCertificate()",
            "    print(string.format('CIV_GAMEPLAY_CHECKPOINT label=%s frame=%d pixels=%d bgr555FNV32=%08X nonblack=%d unique=%d', label, frame, pixels, hash, nonblack, unique))",
            "  end",
            f"  if frame == {int(route['frame_limit'])} then",
            "    local snesMissingCount, snesMissingText = renderMissing(snesMissing, 6)",
            "    local spcMissingCount, spcMissingText = renderMissing(spcMissing, 4)",
            "    local result = (snesMissingCount == 0 and spcMissingCount == 0) and 'PASS' or 'FAIL'",
            "    print(string.format('CIV_GAMEPLAY_COVERAGE result=%s frames=%d snes_instructions=%d snes_unique=%d snes_owned=%d snes_missing=%d spc_instructions=%d spc_unique=%d spc_owned=%d spc_missing=%d', result, frame, snesInstructions, snesUnique, snesOwned, snesMissingCount, spcInstructions, spcUnique, spcOwned, spcMissingCount))",
            "    print('CIV_GAMEPLAY_SNES_MISSING addresses=' .. snesMissingText)",
            "    print('CIV_GAMEPLAY_SPC_MISSING addresses=' .. spcMissingText)",
            "    emu.stop(result == 'PASS' and 0 or 1)",
            "  end",
            "end",
            "emu.addEventCallback(inputPolled, emu.eventType.inputPolled)",
            "emu.addMemoryCallback(onSnesExec, emu.callbackType.exec, 0, 0xFFFFFF, emu.cpuType.snes, emu.memType.snesMemory)",
            "emu.addMemoryCallback(onSpcExec, emu.callbackType.exec, 0, 0xFFFF, emu.cpuType.spc, emu.memType.spcMemory)",
            "emu.addEventCallback(endFrame, emu.eventType.endFrame)",
            f"print('CIV_GAMEPLAY_COVERAGE_START schema=1 snes_authority={len(snes)} spc_authority={len(spc)}')",
        ]
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
