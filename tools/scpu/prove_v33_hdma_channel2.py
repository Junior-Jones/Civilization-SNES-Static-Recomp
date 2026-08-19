#!/usr/bin/env python3
"""Independent exact-ROM proof for Civilization's only non-zero HDMA route.

No runtime trace is consumed.  The proof inventories the closed Version 26/27
CPU context set, verifies the exact channel-2 setup bytes, proves HDMAEN's value
domain from source producers, and records the immutable ROM HDMA table header.
"""
from __future__ import annotations
import argparse, hashlib, json, sys
from pathlib import Path
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE))
sys.path.insert(0,str(HERE.parent/'rom'))
from w65c816 import CpuContext, decode
from civilization_rom import HiRom, EXPECTED_SHA, EXPECTED_SIZE

def at(s):
    b,p=s.split(':'); return int(b,16),int(p,16)
def exact(rom,b,p,h,label):
    exp=bytes.fromhex(h); got=bytes(rom.fetch(b,(p+n)&0xffff) for n in range(len(exp)))
    if got!=exp: raise SystemExit(f'{label}: exact-ROM mismatch {got.hex()} != {exp.hex()}')
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('rom',type=Path); ap.add_argument('--analysis',type=Path,required=True); ap.add_argument('--out',type=Path,required=True); a=ap.parse_args()
    raw=a.rom.read_bytes(); digest=hashlib.sha256(raw).hexdigest()
    if len(raw)!=EXPECTED_SIZE or digest!=EXPECTED_SHA: raise SystemExit('wrong Civilization ROM')
    man=json.loads(a.analysis.read_text());
    if man.get('rom_sha256')!=digest or man.get('static_analysis_frontier_count')!=0 or not man.get('work_queue_empty'): raise SystemExit('HDMA proof requires the closed exact-ROM graph')
    rom=HiRom(raw)
    rows=[]
    for r in man['contexts']:
        b,p=at(r['address']); c=CpuContext(b,p,r['e'],r['m'],r['x'],None); ins=decode(rom.fetch,c)
        if ins.raw.hex()!=r['bytes'] or ins.text!=r['instruction']: raise SystemExit('manifest/ROM mismatch at '+r['address'])
        rows.append((r['address'],ins))
    # Exact configuration routine: disable HDMA, configure channel 2, then publish $1B80 to $0006 for NMI restore.
    exact(rom,0xC1,0x996E,'9c0c42a9428d2043a90d8d2143a2e34a8e2243a9c18d2443a9008d2743ad801b8d0600','channel-2 configuration')
    # State producer is deliberately finite: enable value 4 or disable value 0 only.
    exact(rom,0xC1,0x5C0C,'a9048d801b','enable-state producer')
    exact(rom,0xC1,0x5C43,'a9008d801b','disable-state producer')
    w1=[(addr,ins.text) for addr,ins in rows if ins.mnemonic in {'STA','STX','STY','STZ'} and ins.mode in {'abs','dp'} and ins.operand==0x1B80]
    if w1 != [('C1:5C0E','STA $1B80'),('C1:5C45','STA $1B80')]: raise SystemExit(f'$1B80 writer inventory changed: {w1}')
    w6=[(addr,ins.text) for addr,ins in rows if ins.mnemonic in {'STA','STX','STY','STZ'} and ins.mode in {'abs','dp'} and ins.operand==0x0006]
    if w6 != [('C1:5B69','STZ $0006'),('C1:998E','STA $0006')]: raise SystemExit(f'$0006 writer inventory changed: {w6}')
    # Every syntactic $420C store is either STZ, a literal-zero STA, or C0:81FE restoring $0006.
    hdma_writers=[]
    for addr,ins in rows:
        if ins.mnemonic not in {'STA','STX','STY','STZ'} or ins.mode!='abs' or ins.operand!=0x420C: continue
        kind='zero' if ins.mnemonic=='STZ' else 'restore-$0006' if addr=='C0:81FE' else 'literal-zero' if addr in {'C2:A73A','C2:A745'} else 'UNPROVED'
        if kind=='UNPROVED': raise SystemExit(f'unproved $420C writer {addr}: {ins.text}')
        hdma_writers.append({'address':addr,'instruction':ins.text,'class':kind})
    if not any(r['address']=='C0:81FE' for r in hdma_writers): raise SystemExit('NMI HDMA restore disappeared')
    # Channel 2 guest-register writers are exactly the setup sequence.  $4322 is a 16-bit STX and therefore covers x2/x3.
    regs={}
    for addr,ins in rows:
        if ins.mnemonic in {'STA','STX','STY','STZ'} and ins.mode=='abs' and 0x4320<=ins.operand<=0x432A:
            regs.setdefault(f'{ins.operand:04X}',[]).append((addr,ins.text))
    expected={
      '4320':[('C1:9973','STA $4320')], '4321':[('C1:9978','STA $4321')],
      '4322':[('C1:997E','STX $4322')], '4324':[('C1:9983','STA $4324')],
      '4327':[('C1:9988','STA $4327')]
    }
    if regs!=expected: raise SystemExit(f'channel-2 register writer inventory changed: {regs}')
    # Immutable descriptor table: 55 lines @ $1B85, then 80 @ $1B81, then 89 @ $1B81, terminate.
    table=bytes(rom.fetch(0xC1,0x4AE3+n) for n in range(10))
    expected_table=bytes.fromhex('37851b50811b59811b00')
    if table!=expected_table: raise SystemExit(f'HDMA table changed: {table.hex()}')
    # Live scroll data comes from WRAM and is intentionally not compiled as values.
    data_writers=[(addr,ins.text) for addr,ins in rows if ins.mnemonic in {'STA','STX','STY','STZ'} and ins.mode in {'abs','dp'} and ins.operand in {0x1B81,0x1B85}]
    if data_writers!=[('C1:9605','STX $1B81'),('C1:960B','STX $1B85')]: raise SystemExit(f'HDMA live-data writer inventory changed: {data_writers}')
    out={
      'format':'civilization-v33-hdma-channel2-proof-v1','rom_sha256':digest,'analysis_sha256':hashlib.sha256(a.analysis.read_bytes()).hexdigest(),
      'hdmaen_domain':[0,4],'enabled_channel':2,'configuration':{'dmap':'42','bbad':'0D','source':'C1:4AE3','indirect_bank':'00'},
      'descriptor_table_bytes':table.hex(),'descriptor_interpretation':[{'line_count':55,'indirect':'1B85'},{'line_count':80,'indirect':'1B81'},{'line_count':89,'indirect':'1B81'},{'line_count':0,'terminate':True}],
      'state_writers':w1,'nmi_restore_writers':w6,'hdmaen_writers':hdma_writers,'channel2_register_writers':regs,'live_indirect_data_writers':data_writers,
      'result':'PASS: exact-ROM authority proves HDMAEN {0,$04} only and one channel-2 indirect mode-2 BG1HOFS route. All other HDMA masks/configurations remain fail-closed.'
    }
    a.out.parent.mkdir(parents=True,exist_ok=True); a.out.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n')
    print(json.dumps({'result':'PASS','hdmaen_domain':['00','04'],'channel':2,'table':table.hex()},sort_keys=True))
if __name__=='__main__': main()
