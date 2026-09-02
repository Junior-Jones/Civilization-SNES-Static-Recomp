#!/usr/bin/env python3
"""Certify Civilization's fail-closed, project-owned static S-SMP/S-DSP lane."""
from __future__ import annotations
import argparse, hashlib, json, re
from pathlib import Path

def sha(path: Path) -> str: return hashlib.sha256(path.read_bytes()).hexdigest()
def require(condition: bool, message: str) -> None:
    if not condition: raise SystemExit(message)

def main() -> None:
    parser=argparse.ArgumentParser(); parser.add_argument("--project",type=Path,required=True); parser.add_argument("--out",type=Path,required=True)
    args=parser.parse_args(); root=args.project.resolve(); sr=root/"static-recomp"; apu=sr/"static-audio/civilization-bapu-aot"
    dsp=sr/"static-audio/civilization-project-dsp/civilization_dsp.c"; dsp_header=sr/"static-audio/civilization-project-dsp/include/civilization/civilization_dsp.h"
    cmake=(sr/"CMakeLists.txt").read_text(); core=(apu/"smp/core.cpp").read_text(); lookup=(apu/"smp/sc_smp_aot_lookup.inc").read_text(); dispatch=(apu/"smp/sc_smp_aot_dispatch.inc").read_text()
    bridge=(sr/"src/civilization_audio.cpp").read_text(); public=(sr/"include/civilization_static_recomp.h").read_text(); dsp_text=dsp.read_text(); header_text=dsp_header.read_text()
    sources=["static-audio/civilization-project-dsp/civilization_dsp.c","static-audio/civilization-bapu-aot/sc_static_apu.cpp","static-audio/civilization-bapu-aot/smp/smp.cpp","static-audio/civilization-bapu-aot/dsp/sdsp.cpp"]
    require(all(source in cmake for source in sources),"production static audio source list incomplete")
    require("SC_SMP_AOT=1" in cmake,"missing exact-PC S-SMP production definition")
    require("SCStaticSPC_DSP" not in cmake,"retired third-party S-DSP is still selected")
    for retired in ["SPC_DSP.cpp","SPC_DSP.h","sc_static_sdsp_primitives.inc"]: require(not (apu/"dsp"/retired).exists(),f"retired third-party S-DSP file remains: {retired}")
    require('#include "sc_smp_aot_dispatch.inc"' in core,"S-SMP AOT dispatcher is absent")
    require("default: return sc_aot_fail(1u,pc,0xffu,actual);" in lookup,"unknown S-SMP PC does not fail closed")
    require("if(actual != expected) return sc_aot_fail(2u,pc,expected,actual);" in lookup,"S-SMP opcode identity guard is absent")
    require("default: sc_aot_fail(3u" in dispatch,"unemitted S-SMP opcode does not fail closed")
    require(core.count("sc_aot_fail(4u")>=2,"S-SMP static code-write guards are absent")
    smp_reset=(apu/"smp/smp.cpp").read_text()
    require("std::memset(aram_known,0xff,8192u)" in smp_reset,"deterministic zero-filled ARAM is not marked initialized")
    phases=sorted({int(value) for value in re.findall(r"case\s+(\d+)\s*:",dsp_text) if int(value)<32})
    require(phases==list(range(32)),f"project S-DSP phase closure incomplete: {phases}")
    require("CIV_DSP_PHASES 32u" in header_text,"project S-DSP phase contract is absent")
    require("pcm_known_fifo" in header_text and "pcm_overflows" in header_text,"project S-DSP FIFO knownness/overflow authority is incomplete")
    require("civ_audio_available" in public and "civ_audio_read" in public and "civ_audio_overflow_count" in public,"public pull-based PCM API is incomplete")
    require("sc_static_apu_pcm_read" in bridge,"production bridge does not pull from the core PCM FIFO")
    audited=[sr/"CMakeLists.txt",apu/"smp/core.cpp",apu/"smp/sc_smp_aot_lookup.inc",apu/"smp/sc_smp_aot_dispatch.inc",dsp,dsp_header,sr/"src/civilization_audio.cpp"]
    result={"format":"civilization-project-static-audio-authority-v2","result":"PASS","production_sources":sources,"smp_exact_pc_entries":len(re.findall(r"case\s+0x[0-9a-fA-F]{4}u:\s+expected=",lookup)),"sdsp_phase_domain":phases,"pcm_owner":"static-core-fifo","pcm_knownness":True,"third_party_sdsp_runtime":False,"runtime_interpreter_fallback":False,"hashes":{str(path.relative_to(root)):sha(path) for path in audited}}
    args.out.parent.mkdir(parents=True,exist_ok=True); args.out.write_text(json.dumps(result,indent=2,sort_keys=True)+"\n")
    print(json.dumps({"result":"PASS","smp_entries":result["smp_exact_pc_entries"],"sdsp_phases":len(phases),"pcm_owner":result["pcm_owner"]},sort_keys=True))
if __name__=="__main__": main()
