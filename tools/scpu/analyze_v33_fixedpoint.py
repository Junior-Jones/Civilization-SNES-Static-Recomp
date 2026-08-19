#!/usr/bin/env python3
"""Finite offline W65C816 control-flow closure for Civilization Version 33.

This is build-time proof tooling only.  It deliberately separates runtime
identity (PBR:PC:E:M:X) from interprocedural proof metadata.  Direct calls are
summarized by finite caller/exit relations instead of carrying complete return
stacks through every path.  PHP/PLP state is local to one procedure; caller
status-stack history is never duplicated into callees.
"""
from __future__ import annotations

import argparse, hashlib, json, sys
from collections import defaultdict, deque
from dataclasses import dataclass, replace
from pathlib import Path
from typing import Optional

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'rom'))
from civilization_rom import HiRom, EXPECTED_SHA, EXPECTED_SIZE
from w65c816 import CpuContext, decode

P_C=0x01; P_X=0x10; P_M=0x20

# Civilization installs three native-NMI stack return overrides by writing a
# literal resume PC to $0602 and bank to $0604 before enabling $0600.  The NMI
# service at C0:8214-C0:822A rewrites the interrupted RTI frame with those
# values.  These source-proved edges are therefore part of offline closure, not
# runtime learning.  The exact producer bytes are verified before analysis.
NMI_STACK_REENTRY_PROOFS={
    (0xC1,0x397E):(0x39BD,b'\xA2\xBD\x39\x8E\x02\x06\xA9\xC1\x8D\x04\x06'),
    (0xC1,0xA664):(0xA6AD,b'\xA2\xAD\xA6\x8E\x02\x06\xA9\xC1\x8D\x04\x06'),
    (0xC1,0xB001):(0xB024,b'\xA2\x24\xB0\x8E\x02\x06\xA9\xC1\x8D\x04\x06'),
}

@dataclass(frozen=True)
class SavedP:
    m:int; x:int; carry:Optional[int]

@dataclass(frozen=True)
class Proc:
    bank:int; pc:int; e:int; m:int; x:int; carry:Optional[int]; kind:str

@dataclass(frozen=True)
class State:
    proc:Proc
    cpu:CpuContext
    pstack:tuple[SavedP,...]=()

@dataclass(frozen=True)
class Continuation:
    caller:Proc
    bank:int
    pc:int
    pstack:tuple[SavedP,...]


def cpu_key(c:CpuContext):
    return (c.pbr,c.pc,c.e,c.m,c.x,c.carry)


def runtime_key(c:CpuContext):
    return (c.pbr,c.pc,c.e,c.m,c.x)


def state_key(s:State):
    return (s.proc, cpu_key(s.cpu), s.pstack)


def status_after(state:State, inst):
    c=state.cpu; e,m,x,carry=c.e,c.m,c.x,c.carry; ps=state.pstack
    if inst.mnemonic=='CLC': carry=0
    elif inst.mnemonic=='SEC': carry=1
    elif inst.mnemonic=='REP':
        if inst.operand & P_C: carry=0
        if not e and inst.operand & P_M: m=0
        if not e and inst.operand & P_X: x=0
    elif inst.mnemonic=='SEP':
        if inst.operand & P_C: carry=1
        if inst.operand & P_M: m=1
        if inst.operand & P_X: x=1
    elif inst.mnemonic=='XCE':
        if carry is None:
            return None, ps, 'XCE reached with statically unknown carry'
        e,carry=carry,e
        if e: m=x=1
    elif inst.mnemonic in {'CMP','CPX','CPY','ADC','SBC','LSR','ROL','ASL'}:
        carry=None
    elif inst.mnemonic=='PHP':
        ps=ps+(SavedP(m,x,carry),)
    elif inst.mnemonic=='PLP':
        if not ps:
            return None,ps,'PLP underflows procedure-local PHP stack; inherited-P proof required'
        saved=ps[-1]; ps=ps[:-1]
        m,x,carry=saved.m,saved.x,saved.carry
        if e: m=x=1
    return replace(c,e=e,m=m,x=x,carry=carry),ps,None


def load_indirect_proofs(path:Path):
    data=json.loads(path.read_text())
    out={}
    for row in data.get('proved_sites',[]):
        b,p=row['address'].split(':'); site=(int(b,16),int(p,16))
        cases=row.get('cases')
        if not isinstance(cases,list) or not cases: raise SystemExit(f"{row['address']}: v33 proof row lacks explicit cases")
        targets=[]
        for case in cases:
            t=case['target']; tb,tp=t.split(':'); targets.append((int(tb,16),int(tp,16)))
        out[site]={'kind':row['kind'],'targets':tuple(targets),'row':row}
    return data,out


def scan_threaded_record_targets(rom:HiRom):
    # Exact producer form required by the legacy proof: LDA #record_id; JSL C0:entry.
    entries={0x843D:0x84E8,0x8517:0x858A,0x85F2:0x8665}
    ids=defaultdict(set)
    # HiROM executable cartridge banks in this image are C0-FF aliases.  Scan exact bytes
    # only for the producer instruction form; this is not execution-trace discovery.
    for bank in range(0xC0,0x100):
        for pc in range(0x0000,0xFFFA):
            if rom.fetch(bank,pc)!=0xA9: continue
            lo=rom.fetch(bank,(pc+1)&0xffff); hi=rom.fetch(bank,(pc+2)&0xffff)
            if rom.fetch(bank,(pc+3)&0xffff)!=0x22: continue
            target=rom.fetch(bank,(pc+4)&0xffff)|(rom.fetch(bank,(pc+5)&0xffff)<<8)
            tbank=rom.fetch(bank,(pc+6)&0xffff)
            if tbank==0xC0 and target in entries:
                ids[entries[target]].add(lo|(hi<<8))
    # C0:843D has two internally substituted record IDs already proved structurally.
    ids[0x84E8].update((0x000D,0x000E))
    result={}
    for site,record_ids in ids.items():
        targets=set()
        for rid in record_ids:
            rec=(0xFA18+rid*12)&0xffff
            target=rom.fetch(0xC5,(rec+6)&0xffff)|(rom.fetch(0xC5,(rec+7)&0xffff)<<8)
            if target>=0x8000: targets.add((0xC0,target))
        result[(0xC0,site)]=tuple(sorted(targets))
    return result, {f'C0:{k:04X}':sorted(v) for k,v in ids.items()}


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('rom',type=Path)
    ap.add_argument('--indirect-proof',type=Path,required=True)
    ap.add_argument('--manifest',type=Path,required=True)
    ap.add_argument('--ownership-manifest',type=Path)
    a=ap.parse_args()
    raw=a.rom.read_bytes(); digest=hashlib.sha256(raw).hexdigest()
    if len(raw)!=EXPECTED_SIZE or digest!=EXPECTED_SHA: raise SystemExit('wrong Civilization ROM')
    rom=HiRom(raw); proof_doc,indirect=load_indirect_proofs(a.indirect_proof)
    threaded,threaded_ids=scan_threaded_record_targets(rom)
    for (bank,writer),(resume,expected) in NMI_STACK_REENTRY_PROOFS.items():
        start=(writer-3)&0xffff
        actual=bytes(rom.fetch(bank,(start+n)&0xffff) for n in range(len(expected)))
        if actual!=expected:
            raise SystemExit(f'NMI stack-reentry producer proof mismatch at {bank:02X}:{writer:04X}')

    q=deque(); queued=set(); seen=set()
    proc_states=defaultdict(set)
    proc_exits=defaultdict(set)     # Proc -> {(e,m,x,carry)}
    callers=defaultdict(set)        # Proc -> Continuation records
    runtime_contexts={}
    return_instructions=defaultdict(set)  # Proc -> {(bank,pc,kind)}; continuations joined after fixed point
    frontiers=[]; call_edges=set(); direct_edges=set(); interrupt_roots=[]; nmi_stack_reentry_edges=[]
    proc_runtime_contexts=defaultdict(set); proc_direct_edges=defaultdict(set)

    def enqueue(st:State):
        k=state_key(st)
        if k not in seen and k not in queued:
            q.append(st);queued.add(k)

    def make_proc(c:CpuContext,kind:str):
        return Proc(c.pbr,c.pc,c.e,c.m,c.x,c.carry,kind)

    def seed(c:CpuContext,kind:str,label:str):
        p=make_proc(c,kind); enqueue(State(p,c,()))
        if kind=='RTI': interrupt_roots.append({'label':label,'address':f'{c.pbr:02X}:{c.pc:04X}','e':c.e,'m':c.m,'x':c.x})
        return p

    reset=rom.vector16(0xFFFC)
    seed(CpuContext(0x00,reset,1,1,1,None),'ROOT','reset')
    # Native NMI/IRQ/BRK handlers can receive any native width state.  Seed all
    # width specializations.  BRK is an intentional JML-to-self trap in this ROM;
    # retaining it closes the hardware-vector inventory even though no reachable
    # production instruction emits BRK.
    for off,label in [(0xFFE6,'native_brk'),(0xFFEA,'nmi'),(0xFFEE,'irq')]:
        vec=rom.vector16(off)
        for m in (0,1):
            for x in (0,1):
                seed(CpuContext(0x00,vec,0,m,x,None),'RTI',label)

    def register_call(caller_state:State, target_cpu:CpuContext, ret_kind:str, fall_bank:int, fall_pc:int):
        callee=make_proc(target_cpu,ret_kind)
        cont=Continuation(caller_state.proc,fall_bank,fall_pc,caller_state.pstack)
        if cont not in callers[callee]:
            callers[callee].add(cont)
            for ex in proc_exits.get(callee,()):
                e,m,x,carry=ex
                enqueue(State(cont.caller,CpuContext(cont.bank,cont.pc,e,m,x,carry),cont.pstack))
        enqueue(State(callee,target_cpu,()))
        call_edges.add((caller_state.proc,caller_state.cpu.pbr,caller_state.cpu.pc,callee,fall_bank,fall_pc))

    def add_exit(st:State):
        if st.pstack:
            frontiers.append((st,'procedure returns with unbalanced local PHP stack'))
            return
        return_instructions[st.proc].add((st.cpu.pbr,st.cpu.pc,st.proc.kind))
        ex=(st.cpu.e,st.cpu.m,st.cpu.x,st.cpu.carry)
        if ex in proc_exits[st.proc]: return
        proc_exits[st.proc].add(ex)
        for cont in callers.get(st.proc,()):
            enqueue(State(cont.caller,CpuContext(cont.bank,cont.pc,*ex),cont.pstack))

    while q:
        st=q.popleft(); queued.discard(state_key(st))
        sk=state_key(st)
        if sk in seen: continue
        seen.add(sk); proc_states[st.proc].add((cpu_key(st.cpu),st.pstack))
        c=st.cpu
        proc_runtime_contexts[st.proc].add(runtime_key(c))
        try: inst=decode(rom.fetch,c)
        except Exception as e:
            frontiers.append((st,f'decode failed: {e}')); continue
        rk=runtime_key(c)
        prior=runtime_contexts.get(rk)
        if prior is not None and prior.raw!=inst.raw:
            frontiers.append((st,'same runtime width context decoded conflicting ROM bytes')); continue
        runtime_contexts[rk]=inst
        after,ps,err=status_after(st,inst)
        if err:
            frontiers.append((st,err)); continue
        fall=(c.pc+inst.length)&0xffff

        def intra(cpu):
            direct_edges.add((c.pbr,c.pc,cpu.pbr,cpu.pc))
            proc_direct_edges[st.proc].add((c.pbr,c.pc,cpu.pbr,cpu.pc))
            enqueue(State(st.proc,cpu,ps))

        site=(c.pbr,c.pc)
        if site in NMI_STACK_REENTRY_PROOFS:
            resume,_=NMI_STACK_REENTRY_PROOFS[site]
            edge={'owner_proc':f'{st.proc.bank:02X}:{st.proc.pc:04X}/{st.proc.kind}',
                  'writer':f'{c.pbr:02X}:{c.pc:04X}','resume':f'{c.pbr:02X}:{resume:04X}'}
            if edge not in nmi_stack_reentry_edges: nmi_stack_reentry_edges.append(edge)
            # The NMI can replace the saved RTI PBR:PC while preserving whichever
            # native P width state was interrupted.  Close all native M/X states.
            for mm in (0,1):
                for xx in (0,1):
                    enqueue(State(st.proc,CpuContext(c.pbr,resume,0,mm,xx,None),ps))
        mnem=inst.mnemonic
        if mnem=='JML': intra(replace(after,pbr=(inst.operand>>16)&0xff,pc=inst.operand&0xffff))
        elif mnem=='JMP' and inst.mode=='abs': intra(replace(after,pc=inst.operand))
        elif mnem=='JMP' and inst.mode in {'abs_ind','abs_ind_x','abs_long_ind'}:
            proof=indirect.get(site)
            if not proof:
                frontiers.append((st,f'unproved indirect JMP {inst.mode} operand=${inst.operand:04X}')); continue
            for b,p in proof['targets']: intra(replace(after,pbr=b,pc=p))
        elif mnem in {'BEQ','BNE','BCC','BCS','BVC','BVS','BMI','BPL'}:
            intra(replace(after,pc=fall)); intra(replace(after,pc=inst.operand))
        elif mnem in {'BRA','BRL'}: intra(replace(after,pc=inst.operand))
        elif mnem=='JSR' and inst.mode=='abs':
            register_call(State(st.proc,after,ps),replace(after,pc=inst.operand),'RTS',c.pbr,fall)
        elif mnem=='JSR' and inst.mode=='abs_ind_x':
            proof=indirect.get(site)
            if not proof:
                frontiers.append((st,f'unproved indexed-indirect JSR operand=${inst.operand:04X}')); continue
            for b,p in proof['targets']:
                register_call(State(st.proc,after,ps),replace(after,pbr=b,pc=p),'RTS',c.pbr,fall)
        elif mnem=='JSL':
            register_call(State(st.proc,after,ps),replace(after,pbr=(inst.operand>>16)&0xff,pc=inst.operand&0xffff),'RTL',c.pbr,fall)
        elif mnem=='RTS':
            if site in threaded:
                continuation={(0xC0,0x84E8):0x84E9,(0xC0,0x858A):0x858B,(0xC0,0x8665):0x8666}[site]
                for b,p in threaded[site]:
                    register_call(State(st.proc,after,ps),replace(after,pbr=b,pc=p),'RTS',c.pbr,continuation)
            elif site==(0xD4,0x815C):
                for p in (0x8165,0x81C2):
                    register_call(State(st.proc,after,ps),replace(after,pc=p),'RTS',0xD4,0x815D)
            elif site==(0xD4,0x854E):
                for p in (0x8746,0x8780,0x8598): intra(replace(after,pc=p))
            elif site in {(0xD4,0x877F),(0xD4,0x87B8),(0xD4,0x85D0)}:
                intra(replace(after,pc=0x8541))
            elif st.proc.kind=='RTS': add_exit(State(st.proc,after,ps))
            else: frontiers.append((st,f'RTS reached in procedure kind {st.proc.kind}'))
        elif mnem=='RTL':
            if site==(0xD4,0x82EA):
                for p in (0x8D56,0x8D68,0xB31B): intra(replace(after,pbr=0xD4,pc=p))
                if st.proc.kind=='RTL': add_exit(State(st.proc,after,ps))
            elif st.proc.kind=='RTL': add_exit(State(st.proc,after,ps))
            else: frontiers.append((st,f'RTL reached in procedure kind {st.proc.kind}'))
        elif mnem=='RTI':
            if st.proc.kind!='RTI': frontiers.append((st,f'RTI reached in procedure kind {st.proc.kind}'))
            elif ps: frontiers.append((st,'RTI with unbalanced procedure-local PHP stack'))
        elif mnem in {'BRK','COP'}:
            vecoff=0xFFE6 if mnem=='BRK' else 0xFFE4
            vec=rom.vector16(vecoff)
            for mm in (0,1):
                for xx in (0,1): seed(CpuContext(0x00,vec,0,mm,xx,None),'RTI',mnem.lower())
            intra(replace(after,pc=fall))
        elif mnem=='STP':
            pass
        else:
            intra(replace(after,pc=fall))

    # Deduplicate frontiers by runtime context + reason so summary is stable.
    uniq={}
    for st,reason in frontiers:
        k=(runtime_key(st.cpu),reason)
        uniq[k]=(st,reason)
    frontier_rows=[]
    for st,reason in sorted(uniq.values(),key=lambda x:(runtime_key(x[0].cpu),x[1])):
        c=st.cpu
        frontier_rows.append({'address':f'{c.pbr:02X}:{c.pc:04X}','context':f'E{c.e}M{c.m}X{c.x}',
                              'carry':c.carry,'procedure':f'{st.proc.bank:02X}:{st.proc.pc:04X}/{st.proc.kind}',
                              'reason':reason})
    banks=sorted({b for b,pc,e,m,x in runtime_contexts})
    rows=[]
    for (b,p,e,m,x),inst in sorted(runtime_contexts.items()):
        rows.append({'address':f'{b:02X}:{p:04X}','e':e,'m':m,'x':x,'bytes':inst.raw.hex(),'instruction':inst.text})
    # Build finite return-target sets only after the call graph reaches its fixed point.
    # This avoids discovery-order-dependent metadata when a new caller is found after a
    # callee has already returned during analysis.
    return_sites=defaultdict(set)
    for proc, insns in return_instructions.items():
        continuations=callers.get(proc,())
        for b,p,kind in insns:
            for cont in continuations:
                return_sites[(b,p,kind)].add((cont.bank,cont.pc))
    retrows={f'{b:02X}:{p:04X}/{kind}':[f'{rb:02X}:{rp:04X}' for rb,rp in sorted(vals)]
             for (b,p,kind),vals in sorted(return_sites.items())}
    manifest={
        'format':'civilization-v33-finite-interprocedural-closure-v1',
        'rom_sha256':digest,
        'generator_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        'runtime_context_identity':'PBR:PC:E:M:X',
        'proof_state_model':'procedure-entry + CPU E/M/X/carry + procedure-local PHP stack; finite call/return summaries',
        'artificial_graph_cap':None,
        'work_queue_empty':True,
        'proof_state_count':len(seen),
        'procedure_count':len(proc_states),
        'runtime_context_count':len(runtime_contexts),
        'runtime_banks':[f'{b:02X}' for b in banks],
        'call_edge_count':len(call_edges),
        'call_edges':[{'caller_proc':f'{ce[0].bank:02X}:{ce[0].pc:04X}/{ce[0].kind}','site':f'{ce[1]:02X}:{ce[2]:04X}','callee_proc':f'{ce[3].bank:02X}:{ce[3].pc:04X}/{ce[3].kind}','continuation':f'{ce[4]:02X}:{ce[5]:04X}'} for ce in sorted(call_edges,key=lambda z:(z[1],z[2],z[0].bank,z[0].pc,z[0].e,z[0].m,z[0].x,-1 if z[0].carry is None else z[0].carry,z[0].kind,z[3].bank,z[3].pc,z[3].e,z[3].m,z[3].x,-1 if z[3].carry is None else z[3].carry,z[3].kind,z[4],z[5]))],
        'direct_edge_count':len(direct_edges),
        'return_site_sets':retrows,
        'threaded_record_ids':threaded_ids,
        'interrupt_roots':interrupt_roots,
        'nmi_stack_reentry_edges':sorted(nmi_stack_reentry_edges,key=lambda r:(r['writer'],r['owner_proc'],r['resume'])),
        'indirect_proof_format':proof_doc.get('format'),
        'indirect_proof_sha256':hashlib.sha256(a.indirect_proof.read_bytes()).hexdigest(),
        'indirect_proved_site_count':len(indirect),
        'emitter_status':'ANALYSIS_ONLY_NOT_YET_PRODUCTION_GENERATED_AUTHORITY',
        'static_analysis_frontier_count':len(frontier_rows),
        'static_analysis_frontiers':frontier_rows,
        'contexts':rows,
    }
    a.manifest.parent.mkdir(parents=True,exist_ok=True)
    a.manifest.write_text(json.dumps(manifest,indent=2,sort_keys=True)+'\n')
    if a.ownership_manifest:
        def proc_id(p):
            cv='?' if p.carry is None else str(p.carry)
            return f'{p.bank:02X}:{p.pc:04X}/E{p.e}M{p.m}X{p.x}C{cv}/{p.kind}'
        owners=[]
        for proc in sorted(proc_states,key=lambda p:(p.bank,p.pc,p.e,p.m,p.x,-1 if p.carry is None else p.carry,p.kind)):
            contexts=[f'{b:02X}:{pc:04X}/E{e}M{m}X{x}' for b,pc,e,m,x in sorted(proc_runtime_contexts[proc])]
            edges=[{'from':f'{b:02X}:{pc:04X}','to':f'{tb:02X}:{tp:04X}'} for b,pc,tb,tp in sorted(proc_direct_edges[proc])]
            owners.append({'procedure':proc_id(proc),'entry':f'{proc.bank:02X}:{proc.pc:04X}','kind':proc.kind,
                           'entry_e':proc.e,'entry_m':proc.m,'entry_x':proc.x,'entry_carry':proc.carry,
                           'proof_state_count':len(proc_states[proc]),'runtime_context_count':len(contexts),
                           'runtime_contexts':contexts,'direct_edges':edges})
        full_calls=[]
        for ce in sorted(call_edges,key=lambda z:(z[0].bank,z[0].pc,z[0].e,z[0].m,z[0].x,-1 if z[0].carry is None else z[0].carry,z[0].kind,z[1],z[2],z[3].bank,z[3].pc,z[3].e,z[3].m,z[3].x,-1 if z[3].carry is None else z[3].carry,z[3].kind,z[4],z[5])):
            full_calls.append({'caller':proc_id(ce[0]),'site':f'{ce[1]:02X}:{ce[2]:04X}','callee':proc_id(ce[3]),'continuation':f'{ce[4]:02X}:{ce[5]:04X}'})
        own={'format':'civilization-v33-procedure-ownership-v1','rom_sha256':digest,
             'analysis_manifest_sha256':hashlib.sha256(a.manifest.read_bytes()).hexdigest(),
             'procedure_count':len(owners),'runtime_context_count':len(runtime_contexts),
             'procedures':owners,'call_edges':full_calls,
             'nmi_stack_reentry_edges':sorted(nmi_stack_reentry_edges,key=lambda r:(r['writer'],r['owner_proc'],r['resume']))}
        a.ownership_manifest.parent.mkdir(parents=True,exist_ok=True)
        a.ownership_manifest.write_text(json.dumps(own,indent=2,sort_keys=True)+'\n')
    print(json.dumps({k:manifest[k] for k in ['proof_state_count','procedure_count','runtime_context_count','runtime_banks','call_edge_count','static_analysis_frontier_count','work_queue_empty']},sort_keys=True))

if __name__=='__main__': main()
