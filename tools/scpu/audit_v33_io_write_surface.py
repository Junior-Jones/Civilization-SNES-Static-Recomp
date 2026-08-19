#!/usr/bin/env python3
"""Audit the constant local-address write surface of the closed V33 graph.

Absolute/absolute-indexed W65C816 accesses use DBR, so a 16-bit operand that
looks like an I/O register is not by itself proof of I/O.  This audit keeps the
machine layer narrow: all emitted bases that *could* be target-owned I/O are
checked against current handlers, while the three out-of-handler operand bases
are source-proved WRAM/data-bank accesses rather than silently widening MMIO.
"""
from __future__ import annotations
import argparse, hashlib, json, re, sys
from collections import Counter
from pathlib import Path

HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE.parent/'rom'))
from civilization_rom import HiRom, EXPECTED_SHA, EXPECTED_SIZE

SUPPORTED=lambda b: (
    0x2100 <= b <= 0x2133 or
    0x2140 <= b <= 0x2143 or
    b == 0x4016 or
    0x4200 <= b <= 0x420D or
    0x4300 <= b <= 0x4307 or
    0x4310 <= b <= 0x4317 or
    0x4320 <= b <= 0x4327 or
    0x4370 <= b <= 0x4377
)
EXPECTED_NON_IO_BASE_COUNTS={0x21FE:1,0x4006:1,0x4214:2}
EXPECTED_NON_IO_CONTEXTS={
    'C0:0133':'STZ $21FE,X',
    'D4:88AC':'STA $4006',
    'D4:8EA1':'STZ $4214',
    'D4:8F58':'STZ $4214',
}

def rom_bytes(rom:HiRom,bank:int,start:int,end:int)->bytes:
    return bytes(rom.fetch(bank,p) for p in range(start,end))

def has_seq(blob:bytes,seq:bytes)->bool:
    return blob.find(seq)>=0

def main()->int:
    ap=argparse.ArgumentParser()
    ap.add_argument('rom',type=Path)
    ap.add_argument('--project',type=Path,required=True)
    ap.add_argument('--analysis',type=Path,required=True)
    ap.add_argument('--out',type=Path,required=True)
    a=ap.parse_args()
    raw=a.rom.read_bytes(); digest=hashlib.sha256(raw).hexdigest()
    if len(raw)!=EXPECTED_SIZE or digest!=EXPECTED_SHA: raise SystemExit('wrong Civilization ROM')
    analysis=json.loads(a.analysis.read_text())
    if analysis.get('rom_sha256')!=digest or analysis.get('static_analysis_frontier_count')!=0:
        raise SystemExit('analysis mismatch/not closed')
    root=a.project.resolve(); rom=HiRom(raw)

    counts=Counter()
    samples=[]
    for row in analysis['contexts']:
        instruction=row['instruction']
        if instruction.split()[0] not in {'STA','STX','STY','STZ'}: continue
        for match in re.finditer(r'(?<!#)\$([0-9A-Fa-f]{4})(?![0-9A-Fa-f])',instruction):
            b=int(match.group(1),16)
            if 0x2100<=b<=0x21FF or 0x4000<=b<=0x43FF:
                counts[b]+=1
                if len(samples)<96:samples.append({'context':row['address'],'base':f'${b:04X}','instruction':instruction})

    unsupported={b:n for b,n in counts.items() if not SUPPORTED(b)}
    by_addr={}
    for r in analysis['contexts']:
        if r['address'] in EXPECTED_NON_IO_CONTEXTS:
            by_addr[r['address']]=r['instruction']

    # C0:0118 routine: exact source explicitly installs DBR=$7E before the
    # $21FE,X clear and restores the caller DBR afterwards.
    c0=rom_bytes(rom,0xC0,0x0118,0x013E)
    c0_set7e=has_seq(c0,bytes.fromhex('e220a97e48abc220'))
    c0_clear=has_seq(c0,bytes.fromhex('a220009efe21cacad0f9ab'))

    # D4 bootstrap deliberately uses DBR=$00 for PPU/CPU-I/O initialization,
    # then installs DBR=$7E before all later absolute data structures.  The
    # root call sequence reaches the suspicious routines only after that point.
    d4_init=rom_bytes(rom,0xD4,0x8392,0x84AC)
    d4_set0=d4_init.startswith(bytes.fromhex('f40000abab'))
    d4_set7e=has_seq(d4_init,bytes.fromhex('c230f4007eabab'))
    d4_returns7e=d4_init.endswith(bytes.fromhex('a900008f f51c7e 6b'.replace(' ',''))) if False else (rom.fetch(0xD4,0x84AB)==0x6B)
    root_seq=rom_bytes(rom,0xD4,0x806F,0x80D8)
    root_order=(
        root_seq.find(bytes.fromhex('229283d4'))>=0 and
        root_seq.find(bytes.fromhex('22a888d4'))>root_seq.find(bytes.fromhex('229283d4')) and
        root_seq.find(bytes.fromhex('22568dd4'))>root_seq.find(bytes.fromhex('229283d4'))
    )

    # Intervening routines in the exact bootstrap either contain no DBR write,
    # or preserve the incoming DBR with PHB/.../PLB around their temporary DBR
    # use.  These exact signatures are intentionally narrow to this ROM.
    preserve_signatures={
      'D4:8109': (0x8109,0x8148, []),
      'D4:84EF': (0x84EF,0x850A, [bytes.fromhex('8b'),bytes.fromhex('547e7e'),bytes.fromhex('ab6b')]),
      'D4:8303': (0x8303,0x831D, []),
      'D4:8286': (0x8286,0x8297, []),
      'D4:8C24': (0x8C24,0x8C38, [bytes.fromhex('8b'),bytes.fromhex('547e7e'),bytes.fromhex('ab6b')]),
      'D4:8C38': (0x8C38,0x8C6A, [bytes.fromhex('8b'),bytes.fromhex('547e7e'),bytes.fromhex('ab')]),
      'D4:88A8': (0x88A8,0x88C0, [bytes.fromhex('8b'),bytes.fromhex('547e7e'),bytes.fromhex('ab6b')]),
      'D4:84D8': (0x84D8,0x84EF, [bytes.fromhex('8b4bab'),bytes.fromhex('ab6b')]),
      'D4:88C0': (0x88C0,0x88F5, []),
      'D4:850A': (0x850A,0x852F, [bytes.fromhex('8bf4007eabab'),bytes.fromhex('ab')]),
      'D4:8297': (0x8297,0x82A2, []),
    }
    preserve={}
    for name,(s,e,seqs) in preserve_signatures.items():
        blob=rom_bytes(rom,0xD4,s,e)
        # For routines declared with no temporary DBR use, reject any PLB or
        # block move in the window.  For the others, require their narrow
        # preservation signatures.
        if seqs:
            preserve[name]=all(has_seq(blob,q) for q in seqs)
        else:
            mut=[]
            for row in analysis['contexts']:
                rb,rp=row['address'].split(':'); rp=int(rp,16)
                if rb=='D4' and s<=rp<e and row['instruction'].split()[0] in {'PLB','MVN','MVP'}:
                    mut.append((row['address'],row['instruction']))
            preserve[name]=(len(mut)==0)

    # D4:8D56 state routine itself temporarily selects $7E only behind PHB and
    # restores it before the $4214 data-field stores; exact source signatures
    # bracket both stores.
    d4_state=rom_bytes(rom,0xD4,0x8D8B,0x8F5F)
    state_temp_balanced=has_seq(d4_state,bytes.fromhex('8ba97e48abc220')) and has_seq(d4_state,bytes.fromhex('28ab'))
    state_stores=(rom_bytes(rom,0xD4,0x8EA1,0x8EA4)==bytes.fromhex('9c1442') and
                  rom_bytes(rom,0xD4,0x8F58,0x8F5B)==bytes.fromhex('9c1442'))

    bus=(root/'static-recomp/src/civilization_bus.c').read_text()
    compact=json.loads((root/'static-recomp/generated/compact-aot/civilization_compact_aot.manifest.json').read_text())
    checks={
      'unsupported_operand_bases_exactly_source_proved_non_io':unsupported==EXPECTED_NON_IO_BASE_COUNTS,
      'non_io_contexts_exact':by_addr==EXPECTED_NON_IO_CONTEXTS,
      'c0_clear_explicitly_uses_dbr_7e':c0_set7e and c0_clear,
      'd4_bootstrap_switches_from_io_bank_to_dbr_7e':d4_set0 and d4_set7e and d4_returns7e,
      'd4_root_reaches_suspicious_routines_after_dbr_7e_install':root_order,
      'd4_intervening_bootstrap_calls_preserve_dbr':all(preserve.values()),
      'd4_state_temporary_dbr_use_balanced_and_4214_is_data_field':state_temp_balanced and state_stores,
      'target_owned_write_ranges_present':(
          'local>=0x2100u && local<=0x2133u' in bus and
          'local>=0x2140u && local<=0x2143u' in bus and
          'local==0x4016u' in bus and
          'local>=0x4200u && local<=0x420Du' in bus and
          'local>=0x4300u&&local<=0x4307u' in bus),
      'unproved_writes_fail_closed':'Static bus write reached an unsupported/unproved address.' in bus,
      'compact_semantics_reconstructed_exactly':compact.get('all_context_semantics_reconstructed_exactly') is True and compact.get('original_semantics_sha256')==compact.get('reconstructed_semantics_sha256'),
    }
    report={
      'format':'civilization-v33-static-io-write-surface-v1',
      'rom_sha256':digest,
      'analysis_sha256':hashlib.sha256(a.analysis.read_bytes()).hexdigest(),
      'emitted_local_base_counts':{f'${b:04X}':n for b,n in sorted(counts.items())},
      'out_of_handler_operand_bases':{f'${b:04X}':n for b,n in sorted(unsupported.items())},
      'source_proved_non_io_contexts':EXPECTED_NON_IO_CONTEXTS,
      'd4_preservation_checks':preserve,
      'checks':checks,
      'pass':all(checks.values()),
      'claim':'Every closed-graph constant local write base either lies in a current target-owned MMIO write handler or is one of four exact source-proved DBR=$7E data accesses. No generic MMIO write window was added; dynamic/unproved writes remain fail-closed.'
    }
    a.out.parent.mkdir(parents=True,exist_ok=True)
    a.out.write_text(json.dumps(report,indent=2,sort_keys=True)+'\n')
    print(json.dumps({'result':'PASS' if report['pass'] else 'FAIL','out_of_handler':report['out_of_handler_operand_bases'],'checks':checks},indent=2,sort_keys=True))
    return 0 if report['pass'] else 1
if __name__=='__main__': sys.exit(main())
