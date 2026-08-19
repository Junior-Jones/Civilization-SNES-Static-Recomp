#!/usr/bin/env python3
"""Certify that Version 33 production audio is the fail-closed static S-SMP/S-DSP path."""
from __future__ import annotations
import argparse, hashlib, json, re
from pathlib import Path


def sha(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()


def require(condition: bool, message: str):
    if not condition:
        raise SystemExit(message)


def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--project',type=Path,required=True); ap.add_argument('--out',type=Path,required=True)
    a=ap.parse_args(); root=a.project.resolve(); sr=root/'static-recomp'; apu=sr/'static-audio/civilization-bapu-aot'
    cmake=(sr/'CMakeLists.txt').read_text(); core=(apu/'smp/core.cpp').read_text(); lookup=(apu/'smp/sc_smp_aot_lookup.inc').read_text(); dispatch=(apu/'smp/sc_smp_aot_dispatch.inc').read_text(); sdsp=(apu/'dsp/sdsp.cpp').read_text(); bridge=(sr/'src/civilization_audio.cpp').read_text(); prov=(apu/'PROVENANCE.md').read_text()
    production_sources=['static-audio/civilization-bapu-aot/sc_static_apu.cpp','static-audio/civilization-bapu-aot/smp/smp.cpp','static-audio/civilization-bapu-aot/dsp/sdsp.cpp']
    require(all(x in cmake for x in production_sources),'production Full Static APU source list incomplete')
    for macro in ['SC_SMP_AOT=1','SC_STATIC_SDSP_PRIMITIVES=1','SCStaticSPC_DSP_CUSTOM_RUN=1']:
        require(macro in cmake,f'missing production compile definition {macro}')
    require('#ifdef SC_SMP_AOT' in core and '#include "sc_smp_aot_dispatch.inc"' in core and '#else\n  switch(opcode_number)' in core,'S-SMP AOT/generic split not structurally fail closed')
    require('default: return sc_aot_fail(1u,pc,0xffu,actual);' in lookup,'unknown S-SMP PC does not fail closed')
    require('if(actual != expected) return sc_aot_fail(2u,pc,expected,actual);' in lookup,'S-SMP opcode identity guard missing')
    require('default: sc_aot_fail(3u' in dispatch,'unemitted S-SMP opcode does not fail closed')
    require(core.count('sc_aot_fail(4u')>=2,'statically owned S-SMP code-write guard missing')
    phases=sorted({int(x) for x in re.findall(r'case\s+(\d+)\s*:',sdsp) if int(x)<32})
    require(phases==list(range(32)),f'S-DSP phase closure incomplete: {phases}')
    require('sc_static_sdsp_primitive_step(phase)' in sdsp,'compiled S-DSP primitive phase hook missing')
    require('sc_static_apu_sync_to_master' in bridge and 'sc_static_apu_cpu_write_port' in bridge and 'sc_static_apu_cpu_read_port' in bridge,'production bridge is not connected only through static APU API')
    forbidden=['MesenCore','reference.wav','reference.pcm','trace_replay','interpreter fallback']
    hits={x:[str(p.relative_to(root)) for p in [sr/'src/civilization_audio.cpp',sr/'CMakeLists.txt'] if x.lower() in p.read_text(errors='ignore').lower()] for x in forbidden}
    require(not any(hits.values()),f'forbidden production audio dependency marker: {hits}')
    require('There is no interpreter fallback' in prov,'static-audio provenance does not state fail-closed architecture')
    result={
      'format':'civilization-v33-static-audio-authority-audit-v1','result':'PASS',
      'production_sources':production_sources,'required_defines':['SC_SMP_AOT','SC_STATIC_SDSP_PRIMITIVES','SCStaticSPC_DSP_CUSTOM_RUN'],
      'smp_unknown_pc_fail_reason':1,'smp_opcode_mismatch_fail_reason':2,'smp_unemitted_opcode_fail_reason':3,'smp_code_write_fail_reason':4,
      'sdsp_phase_domain':phases,'host_source_rate_hz':32040,
      'hashes':{str(p.relative_to(root)):sha(p) for p in [sr/'CMakeLists.txt',apu/'smp/core.cpp',apu/'smp/sc_smp_aot_lookup.inc',apu/'smp/sc_smp_aot_dispatch.inc',apu/'dsp/sdsp.cpp',sr/'src/civilization_audio.cpp']},
      'runtime_decoder_in_production':False,'runtime_interpreter_fallback':False,'oracle_or_reference_pcm_dependency':False
    }
    a.out.parent.mkdir(parents=True,exist_ok=True); a.out.write_text(json.dumps(result,indent=2,sort_keys=True)+'\n')
    print(json.dumps({'result':'PASS','sdsp_phases':len(phases),'aot_lookup_bytes':(apu/'smp/sc_smp_aot_lookup.inc').stat().st_size,'aot_dispatch_bytes':(apu/'smp/sc_smp_aot_dispatch.inc').stat().st_size},sort_keys=True))
if __name__=='__main__':main()
