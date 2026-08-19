#!/usr/bin/env python3
"""Emit the complete Civilization W65C816 production authority from a closed manifest.

This tool never explores gameplay and never decodes at runtime.  It consumes the
already-proved finite fixed-point manifest plus finite indirect-control-flow proofs,
verifies every context against the exact ROM, and emits bounded C shards with a
compact dispatch index and shared finite return-target proof tables.
"""
from __future__ import annotations
import argparse, hashlib, json, shutil, sys
from pathlib import Path
from collections import defaultdict

HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE))
sys.path.insert(0,str(HERE.parent/'rom'))
from w65c816 import CpuContext,decode
from w65c816_emit_semantics import sem
from civilization_rom import HiRom,EXPECTED_SHA,EXPECTED_SIZE
from analyze_v33_fixedpoint import scan_threaded_record_targets

CONTROL={'BEQ','BNE','BCC','BCS','BVC','BVS','BMI','BPL','BRA','BRL','JMP','JML','JSR','JSL','RTS','RTL','RTI','MVN','MVP'}


def parse_addr(s:str):
    b,p=s.split(':');return int(b,16),int(p,16)

def ctx_key(c:CpuContext):
    return ((c.pbr<<16)|c.pc|(c.e<<24)|(c.m<<25)|(c.x<<26))

def sha(path:Path): return hashlib.sha256(path.read_bytes()).hexdigest()

def load_proofs(path:Path):
    doc=json.loads(path.read_text()); out={}
    for row in doc.get('proved_sites',[]):
        site=parse_addr(row['address'])
        cases=row.get('cases')
        if not isinstance(cases,list) or not cases: raise RuntimeError(f"{row['address']} v33 proof lacks explicit cases")
        mapping={}
        for case in cases:
            runtime_x=int(case['runtime_x']); target=parse_addr(case['target'])
            if runtime_x in mapping: raise RuntimeError(f"{row['address']} duplicate runtime_x {runtime_x:04X}")
            mapping[runtime_x]=target
        out[site]=(row,mapping)
    return doc,out

def make_return_proofs(manifest,rom):
    sets={}
    for name,targets in manifest['return_site_sets'].items():
        addr,kind=name.split('/')
        sets[(parse_addr(addr)[0],parse_addr(addr)[1],kind)]=set(parse_addr(t) for t in targets)
    threaded,_=scan_threaded_record_targets(rom)
    for (b,p),targets in threaded.items(): sets[(b,p,'RTS')]=set(targets)
    sets[(0xD4,0x815C,'RTS')]={(0xD4,0x8165),(0xD4,0x81C2)}
    sets[(0xD4,0x854E,'RTS')]={(0xD4,0x8746),(0xD4,0x8780),(0xD4,0x8598)}
    for p in (0x877F,0x87B8,0x85D0): sets[(0xD4,p,'RTS')]={(0xD4,0x8541)}
    sets.setdefault((0xD4,0x82EA,'RTL'),set()).update({(0xD4,0x8D56),(0xD4,0x8D68),(0xD4,0xB31B)})
    ordered=sorted(sets)
    ids={k:n for n,k in enumerate(ordered)}
    pool=[];meta=[]
    for k in ordered:
        vals=sorted(sets[k]); off=len(pool)
        pool.extend((b<<16)|p for b,p in vals)
        meta.append((off,len(vals),k))
    return sets,ids,pool,meta

def emit_return_tables(path:Path,pool,meta):
    L=['/* AUTO-GENERATED finite return-target proof tables for Civilization. */',
       '#include "civilization_internal.h"','#include "civilization_generated_core.h"','',
       'typedef struct { uint32_t offset; uint16_t count; } CivReturnProofMeta;','']
    L.append(f'static const uint32_t civ_return_targets[{len(pool)}]={{')
    for n in range(0,len(pool),8): L.append('  '+','.join(f'0x{x:06X}u' for x in pool[n:n+8])+',')
    L += ['};','',f'static const CivReturnProofMeta civ_return_meta[{len(meta)}]={{']
    for off,count,k in meta:
        L.append(f'  {{{off}u,{count}u}}, /* {k[0]:02X}:{k[1]:04X}/{k[2]} */')
    L += ['};','',
          'int civ_generated_return_allowed(uint16_t proof_id,uint32_t target){',
          f'  if(proof_id>={len(meta)}u)return 0;',
          '  { uint32_t lo=civ_return_meta[proof_id].offset, hi=lo+civ_return_meta[proof_id].count;',
          '    while(lo<hi){uint32_t mid=lo+(hi-lo)/2u,v=civ_return_targets[mid]; if(v<target)lo=mid+1u;else hi=mid;}',
          '    return lo<civ_return_meta[proof_id].offset+civ_return_meta[proof_id].count && civ_return_targets[lo]==target; }',
          '}','']
    path.write_text('\n'.join(L),newline='\n')

def indirect_lines(site,inst,proofs,addr):
    if site not in proofs: raise RuntimeError(f'unproved indirect site reached during emission: {addr}')
    row,mapping=proofs[site]
    kind=row['kind']
    expected='JSR (abs,X)' if inst.mnemonic=='JSR' else 'JMP (abs,X)'
    if kind!=expected: raise RuntimeError(f'{addr} proof kind {kind} != {expected}')
    L=[]
    for n,(x,(b,p)) in enumerate(mapping.items()):
        kw='if' if n==0 else 'else if'
        if b==site[0]: L.append(f'{kw}(i->cpu.x==0x{x:04X}u)i->cpu.pc=0x{p:04X}u;')
        else: L.append(f'{kw}(i->cpu.x==0x{x:04X}u){{i->cpu.pbr=0x{b:02X}u;i->cpu.pc=0x{p:04X}u;}}')
    L.append(f'else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.","{addr}");')
    return L

def emit_case(c,inst,proofs,ret_ids):
    site=(c.pbr,c.pc);addr=f'{c.pbr:02X}:{c.pc:04X}';fall=(c.pc+inst.length)&0xffff
    key=ctx_key(c); L=[f'  case 0x{key:08X}u: /* {inst.raw.hex(" ").upper()} {inst.text}; {addr} E{c.e}M{c.m}X{c.x} */',
       f'    if(!civ_require_width(i,{c.e}u,{c.m}u,{c.x}u,"{addr}"))return 0;']
    timing_operand=int.from_bytes(inst.raw[1:],'little') if len(inst.raw)>1 else 0
    L.append(f'    civ_cpu_begin_static_instruction(i,0x{inst.opcode:02X}u,{inst.length}u,0x{timing_operand:06X}u);')
    for line in sem(inst,c): L.append('    '+line)
    # Keep existing diagnostic counters without making them execution authority.
    if site==(0xC0,0x8114): L.append('    i->v16_wram_clear_bytes+=1u;')
    if site==(0xC0,0x812A): L.append('    i->v16_wram_clear_bytes+=2u;')
    if inst.mnemonic in {'MVN','MVP'}:
        dest,src=inst.raw[1],inst.raw[2];adj=1 if inst.mnemonic=='MVN' else -1
        L.append(f'    return civ_block_move_step(i,0x{dest:02X}u,0x{src:02X}u,{adj},"{addr}");')
        return L
    m=inst.mnemonic
    if m=='BEQ': L.append(f'    i->cpu.pc=(i->cpu.p&CIV_P_Z)?0x{inst.operand:04X}u:0x{fall:04X}u;')
    elif m=='BNE': L.append(f'    i->cpu.pc=(i->cpu.p&CIV_P_Z)?0x{fall:04X}u:0x{inst.operand:04X}u;')
    elif m=='BCC': L.append(f'    i->cpu.pc=(i->cpu.p&CIV_P_C)?0x{fall:04X}u:0x{inst.operand:04X}u;')
    elif m=='BCS': L.append(f'    i->cpu.pc=(i->cpu.p&CIV_P_C)?0x{inst.operand:04X}u:0x{fall:04X}u;')
    elif m=='BVC': L.append(f'    i->cpu.pc=(i->cpu.p&CIV_P_V)?0x{fall:04X}u:0x{inst.operand:04X}u;')
    elif m=='BVS': L.append(f'    i->cpu.pc=(i->cpu.p&CIV_P_V)?0x{inst.operand:04X}u:0x{fall:04X}u;')
    elif m=='BMI': L.append(f'    i->cpu.pc=(i->cpu.p&CIV_P_N)?0x{inst.operand:04X}u:0x{fall:04X}u;')
    elif m=='BPL': L.append(f'    i->cpu.pc=(i->cpu.p&CIV_P_N)?0x{fall:04X}u:0x{inst.operand:04X}u;')
    elif m in {'BRA','BRL'}: L.append(f'    i->cpu.pc=0x{inst.operand:04X}u;')
    elif m=='JSR':
        r=(c.pc+inst.length-1)&0xffff
        L += [f'    if(!civ_push8(i,0x{(r>>8)&0xff:02X}u))return 0;',f'    if(!civ_push8(i,0x{r&0xff:02X}u))return 0;']
        if inst.mode=='abs': L.append(f'    i->cpu.pc=0x{inst.operand:04X}u;')
        elif inst.mode=='abs_ind_x': L += ['    '+x for x in indirect_lines(site,inst,proofs,addr)]
        else: raise RuntimeError(f'unhandled JSR mode {inst.mode} {addr}')
    elif m=='JSL':
        r=(c.pc+inst.length-1)&0xffff
        L += [f'    if(!civ_push8(i,0x{c.pbr:02X}u))return 0;',f'    if(!civ_push8(i,0x{(r>>8)&0xff:02X}u))return 0;',f'    if(!civ_push8(i,0x{r&0xff:02X}u))return 0;',f'    i->cpu.pbr=0x{(inst.operand>>16)&0xff:02X}u;i->cpu.pc=0x{inst.operand&0xffff:04X}u;']
    elif m=='RTS':
        proof=(c.pbr,c.pc,'RTS')
        if proof not in ret_ids: raise RuntimeError(f'no finite RTS proof for {addr}')
        pid=ret_ids[proof]
        L += ['    { uint8_t lo,hi; uint16_t target; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0;',
              '      target=(uint16_t)((uint16_t)(lo|((uint16_t)hi<<8))+1u);',
              f'      if(!civ_generated_return_allowed({pid}u,((uint32_t)i->cpu.pbr<<16)|target))return civ_fail_frontier(i,"RTS returned outside the finite Civilization proof set.","{addr}");',
              '      i->cpu.pc=target; }']
        if site==(0xD4,0x854E):
            L += ['    if(i->cpu.pc==0x8746u)i->descriptor_handler_8746_count++;',
                  '    else if(i->cpu.pc==0x8780u)i->descriptor_handler_8780_count++;',
                  '    else if(i->cpu.pc==0x8598u)i->descriptor_handler_8598_count++;']
    elif m=='RTL':
        proof=(c.pbr,c.pc,'RTL')
        if proof not in ret_ids: raise RuntimeError(f'no finite RTL proof for {addr}')
        pid=ret_ids[proof]
        L += ['    { uint8_t lo,hi,bank; uint16_t target; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi)||!civ_pull8(i,&bank))return 0;',
              '      target=(uint16_t)((uint16_t)(lo|((uint16_t)hi<<8))+1u);',
              f'      if(!civ_generated_return_allowed({pid}u,((uint32_t)bank<<16)|target))return civ_fail_frontier(i,"RTL returned outside the finite Civilization proof set.","{addr}");',
              '      i->cpu.pbr=bank;i->cpu.pc=target; }']
    elif m=='RTI': L.append('    if(!civ_rti_native(i))return 0;')
    elif m=='JML': L.append(f'    i->cpu.pbr=0x{(inst.operand>>16)&0xff:02X}u;i->cpu.pc=0x{inst.operand&0xffff:04X}u;')
    elif m=='JMP':
        if inst.mode=='abs': L.append(f'    i->cpu.pc=0x{inst.operand:04X}u;')
        elif inst.mode=='abs_ind_x':
            L += ['    '+x for x in indirect_lines(site,inst,proofs,addr)]
            if site==(0xD4,0x8D72):
                L += ['    i->stage_dispatch_count++;','    i->last_stage_index=(uint16_t)(i->cpu.x>>1);']
        else: raise RuntimeError(f'unhandled JMP mode {inst.mode} {addr}')
    elif m in {'BRK','COP','WAI','STP'}: raise RuntimeError(f'unexpected closed graph control opcode {m} at {addr}')
    else: L.append(f'    i->cpu.pc=0x{fall:04X}u;')
    L += ['    i->instruction_count++;return 1;']
    return L

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('rom',type=Path);ap.add_argument('--analysis',type=Path,required=True);ap.add_argument('--indirect-proof',type=Path,required=True);ap.add_argument('--out-dir',type=Path,required=True)
    a=ap.parse_args(); raw=a.rom.read_bytes(); digest=hashlib.sha256(raw).hexdigest()
    if len(raw)!=EXPECTED_SIZE or digest!=EXPECTED_SHA: raise SystemExit('wrong Civilization ROM')
    analysis=json.loads(a.analysis.read_text())
    if analysis.get('rom_sha256')!=digest: raise SystemExit('analysis ROM mismatch')
    if analysis.get('static_analysis_frontier_count')!=0 or not analysis.get('work_queue_empty') or analysis.get('artificial_graph_cap') is not None: raise SystemExit('analysis is not a closed uncapped fixed point')
    proof_doc,proofs=load_proofs(a.indirect_proof)
    if hashlib.sha256(a.indirect_proof.read_bytes()).hexdigest()!=analysis.get('indirect_proof_sha256'): raise SystemExit('indirect proof SHA mismatch')
    if len(proofs)!=analysis.get('indirect_proved_site_count'): raise SystemExit('indirect proof count mismatch')
    rom=HiRom(raw); contexts=[]
    for row in analysis['contexts']:
        b,p=parse_addr(row['address']);c=CpuContext(b,p,row['e'],row['m'],row['x'],None);inst=decode(rom.fetch,c)
        if inst.raw.hex()!=row['bytes'] or inst.text!=row['instruction']: raise RuntimeError(f'exact-ROM context mismatch {row["address"]}')
        sem(inst,c) # semantic coverage check before writing anything
        contexts.append((c,inst))
    if len(contexts)!=analysis['runtime_context_count']: raise RuntimeError('context count mismatch')
    ret_sets,ret_ids,pool,meta=make_return_proofs(analysis,rom)
    # Prove every reached return instruction has a finite runtime target set.
    for c,inst in contexts:
        if inst.mnemonic in {'RTS','RTL'} and (c.pbr,c.pc,inst.mnemonic) not in ret_ids:
            raise RuntimeError(f'missing finite return proof at {c.pbr:02X}:{c.pc:04X}/{inst.mnemonic}')
    out=a.out_dir; shards=out/'civilization_generated_core_shards';out.mkdir(parents=True,exist_ok=True)
    if shards.exists(): shutil.rmtree(shards)
    shards.mkdir()
    emit_return_tables(out/'civilization_generated_returns.c',pool,meta)
    # Compact only contexts at the same exact ROM address whose already-emitted
    # instruction bodies are byte-for-byte identical.  The switch retains every
    # PBR:PC:E:M:X key, while one shared body replaces duplicate width variants.
    # This is source-level static factoring: there is still no runtime decode,
    # opcode dispatch, learning, or emulator fallback.
    group_contexts=defaultdict(list)
    for c,inst in contexts:
        group_contexts[((c.pbr<<16)|c.pc)>>10].append((c,inst))
    groups={}
    compacted_body_groups=0
    compacted_context_bodies=0
    for group,entries in group_contexts.items():
        emitted=[]
        for c,inst in entries:
            lines=emit_case(c,inst,proofs,ret_ids)
            emitted.append((c,lines))
        identical=defaultdict(list)
        for c,lines in emitted:
            identical[((c.pbr,c.pc),tuple(lines[2:]))].append((c,lines))
        body=[]
        for same in identical.values():
            if len(same)==1:
                body.extend(same[0][1])
                continue
            compacted_body_groups+=1
            compacted_context_bodies+=len(same)-1
            for _c,lines in same:
                body.append(lines[0])
            c=same[0][0]
            addr=f'{c.pbr:02X}:{c.pc:04X}'
            body.append(f'    if(!civ_require_width(i,(uint8_t)((key>>24)&1u),(uint8_t)((key>>25)&1u),(uint8_t)((key>>26)&1u),"{addr}"))return 0;')
            body.extend(same[0][1][2:])
        groups[group]=body
    shard_hashes={}
    for group,body in sorted(groups.items()):
        name=f'civilization_core_group_{group:05X}'
        text='\n'.join(['/* AUTO-GENERATED exact-ROM Civilization 1 KiB static CPU shard. */','#include "civilization_internal.h"','#include "civilization_generated_core.h"','',f'int {name}(CivRecomp *i,uint32_t key){{','  if(!i)return 0;','  switch(key){']+body+['  default:return -1;','  }','}',''])
        p=shards/(name+'.c');p.write_text(text,newline='\n');shard_hashes[p.name]=hashlib.sha256(text.encode()).hexdigest()
    group_ids=sorted(groups)
    index=['/* AUTO-GENERATED closed Civilization S-CPU dispatch index. */','#include "civilization_internal.h"','#include "civilization_generated_core.h"','']
    index += [f'int civilization_core_group_{g:05X}(CivRecomp*,uint32_t);' for g in group_ids]
    index += ['',
      'static uint32_t civ_generated_context_key_values(uint8_t pbr,uint16_t pc,uint8_t e,uint8_t m,uint8_t x){return ((uint32_t)pbr<<16)|(uint32_t)pc|((uint32_t)(e&1u)<<24)|((uint32_t)(m&1u)<<25)|((uint32_t)(x&1u)<<26);}',
      'uint32_t civ_generated_core_context_key(const CivRecomp *i){uint8_t m,x;if(!i)return 0u;m=(uint8_t)((i->cpu.p&CIV_P_M)!=0u);x=(uint8_t)((i->cpu.p&CIV_P_X)!=0u);return civ_generated_context_key_values(i->cpu.pbr,i->cpu.pc,i->cpu.e,m,x);}',
      f'unsigned civ_generated_core_context_count(void){{return {len(contexts)}u;}}',
      'int civ_generated_core_step(CivRecomp *i){uint32_t key,group;int result;if(!i)return 0;key=civ_generated_core_context_key(i);group=(key&0x00FFFFFFu)>>10u;switch(group){']
    index += [f'  case 0x{g:X}u:result=civilization_core_group_{g:05X}(i,key);break;' for g in group_ids]
    index += ['  default:result=-1;break;','  }','  if(result>=0)return result;','  return civ_fail_frontier(i,"Civilization reached a W65C816 context outside the closed generated authority.",NULL);','}','']
    index_text='\n'.join(index);(out/'civilization_generated_core.c').write_text(index_text,newline='\n')
    header='''#ifndef CIVILIZATION_GENERATED_CORE_H\n#define CIVILIZATION_GENERATED_CORE_H\n#include <stdint.h>\n#include "civilization_static_recomp.h"\nint civ_generated_core_step(CivRecomp *i);\nunsigned civ_generated_core_context_count(void);\nuint32_t civ_generated_core_context_key(const CivRecomp *i);\nint civ_generated_return_allowed(uint16_t proof_id,uint32_t target);\n#endif\n'''
    (out/'civilization_generated_core.h').write_text(header,newline='\n')
    cmake='set(CIVILIZATION_GENERATED_SHARD_COUNT %d)\n'%len(group_ids)
    (out/'civilization_generated_core.cmake').write_text(cmake,newline='\n')
    genmanifest={
      'format':'civilization-closed-production-scpu-v33','rom_sha256':digest,'analysis_manifest_sha256':sha(a.analysis),'indirect_proof_sha256':sha(a.indirect_proof),'generator_sha256':sha(Path(__file__)),'semantics_generator_sha256':sha(HERE/'w65c816_emit_semantics.py'),'base_semantics_generator_sha256':sha(HERE/'w65c816_base_semantics.py'),
      'runtime_context_identity':'PBR:PC:E:M:X','runtime_context_count':len(contexts),'static_analysis_frontier_count':0,'work_queue_empty':True,'artificial_graph_cap':None,'runtime_opcode_decode':False,'runtime_learning':False,'runtime_emulator_fallback':False,
      'indirect_proved_site_count':len(proofs),'return_proof_site_count':len(meta),'return_proof_target_count':len(pool),'dispatch_layout':'1KiB-address-shards + shared finite return tables + identical same-address body folding','generated_shard_count':len(group_ids),'compacted_identical_body_groups':compacted_body_groups,'compacted_redundant_context_bodies':compacted_context_bodies,'generated_shards':shard_hashes,'generated_index_sha256':hashlib.sha256(index_text.encode()).hexdigest(),'emitter_status':'CLOSED_GENERATED_AUTHORITY_READY_FOR_BUILD_VALIDATION'}
    (out/'civilization_generated_core.manifest.json').write_text(json.dumps(genmanifest,indent=2,sort_keys=True)+'\n',newline='\n')
    print(json.dumps({k:genmanifest[k] for k in ['runtime_context_count','indirect_proved_site_count','return_proof_site_count','return_proof_target_count','generated_shard_count','compacted_identical_body_groups','compacted_redundant_context_bodies','static_analysis_frontier_count']},sort_keys=True))
if __name__=='__main__':main()
