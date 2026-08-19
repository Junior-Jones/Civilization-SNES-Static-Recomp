#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,re,sys
from collections import Counter
from pathlib import Path

EXPECTED={0x2134,0x2135,0x2136,0x2140,0x4016,0x4210,0x4211,0x4212,0x4214,0x4215,0x4216,0x4218,0x421A}

def main()->int:
    ap=argparse.ArgumentParser();ap.add_argument('--project',type=Path,required=True);ap.add_argument('--out',type=Path,required=True);a=ap.parse_args()
    root=a.project.resolve(); counts=Counter(); expressions=[]
    analysis=json.loads((root/'docs/research/civilization_v33_fixedpoint.manifest.json').read_text())
    compact=json.loads((root/'static-recomp/generated/compact-aot/civilization_compact_aot.manifest.json').read_text())
    non_reads={'STA','STX','STY','STZ','JMP','JML','JSR','JSL','BRA','BRL',
               'BCC','BCS','BEQ','BNE','BMI','BPL','BVC','BVS','PEA','PEI','PER'}
    for row in analysis['contexts']:
        instruction=row['instruction']; mnemonic=instruction.split()[0]
        if mnemonic in non_reads: continue
        for match in re.finditer(r'(?<!#)\$([0-9A-Fa-f]{4})(?![0-9A-Fa-f])',instruction):
            base=int(match.group(1),16)
            if 0x2100<=base<=0x21FF or 0x4000<=base<=0x43FF:
                counts[base]+=1
                if len(expressions)<64: expressions.append({'context':row['address'],'base':f'${base:04X}','instruction':instruction})
    bus=(root/'static-recomp/src/civilization_bus.c').read_text()
    checks={
      'emitted_io_read_bases_exact':set(counts)==EXPECTED,
      'ppu_2134_2136_supported':'local>=0x2134u && local<=0x2136u' in bus,
      'apuio_2140_2143_supported':'local>=0x2140u && local<=0x2143u' in bus,
      'controller_4016_4017_supported':'local==0x4016u || local==0x4017u' in bus,
      'cpu_status_4210_4212_supported':'local==0x4210u' in bus and 'local==0x4211u' in bus and 'local==0x4212u' in bus,
      'cpu_math_4214_4217_supported':'local>=0x4214u && local<=0x4217u' in bus,
      'autojoy_4218_421f_supported':'local>=0x4218u && local<=0x421Fu' in bus,
      'unproved_reads_fail_closed':'Static bus read reached an unsupported/unproved address.' in bus,
      'compact_semantics_reconstructed_exactly':compact.get('all_context_semantics_reconstructed_exactly') is True and compact.get('original_semantics_sha256')==compact.get('reconstructed_semantics_sha256'),
    }
    report={'format':'civilization-v33-static-io-read-surface-v1','base_counts':{f'${k:04X}':v for k,v in sorted(counts.items())},
            'checks':checks,'pass':all(checks.values()),
            'claim':'All statically emitted I/O-read base addresses in the closed generated graph fall inside already-owned target-specific handlers. Indexed/dynamic addresses remain runtime fail-closed outside those owned ranges.'}
    a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(report,indent=2,sort_keys=True)+'\n');print(json.dumps(report,indent=2,sort_keys=True));return 0 if report['pass'] else 1
if __name__=='__main__':
    try:sys.exit(main())
    except Exception as e:print(json.dumps({'pass':False,'error':str(e)},indent=2));sys.exit(1)
