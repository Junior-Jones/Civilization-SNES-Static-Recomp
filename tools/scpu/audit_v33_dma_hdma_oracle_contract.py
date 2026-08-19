#!/usr/bin/env python3
"""Compare Civilization's target-specific DMA/HDMA arbitration with pinned MesenCE source.

This is an offline development audit only.  It never participates in production
execution and accepts the oracle source as an explicit external input.
"""
from __future__ import annotations
import argparse, hashlib, json, re
from pathlib import Path

PIN_COMMIT = "20ba206cef5ba207c21203176d02cb9f43dda9fb"
PIN_FILE_SHA256 = "32d459840fd06084fe46b87955bf6d831ac85023772b313aaa243af4c6be78d4"

def sha256(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()

def require(text: str, needle: str, label: str, checks: dict[str,bool]) -> None:
    checks[label] = needle in text
    if not checks[label]:
        raise SystemExit(f"missing contract: {label}")

def extract(text: str, signature: str, next_signature: str | None = None) -> str:
    start=text.find(signature)
    if start<0: raise SystemExit(f"missing function {signature}")
    if next_signature:
        end=text.find(next_signature,start+len(signature))
        if end<0: end=len(text)
    else: end=len(text)
    return text[start:end]

def main() -> None:
    ap=argparse.ArgumentParser()
    ap.add_argument('--project',type=Path,required=True)
    ap.add_argument('--mesen-dma-source',type=Path,required=True)
    ap.add_argument('--out',type=Path,required=True)
    a=ap.parse_args()
    oracle=a.mesen_dma_source.read_text()
    prod=(a.project/'static-recomp/src/civilization_bus.c').read_text()
    if sha256(a.mesen_dma_source)!=PIN_FILE_SHA256:
        raise SystemExit('MesenCE DMA source does not match pinned 2.2.1 source hash')

    checks={}
    run=extract(oracle,'void SnesDmaController::RunDma','bool SnesDmaController::InitHdmaChannels')
    pending=extract(oracle,'bool SnesDmaController::ProcessPendingTransfers','void SnesDmaController::Write')
    init=extract(oracle,'bool SnesDmaController::InitHdmaChannels','void SnesDmaController::SyncStartDma')
    scan=extract(oracle,'bool SnesDmaController::ProcessHdmaChannels','bool SnesDmaController::IsLastActiveHdmaChannel')

    # Independent oracle contract: pending work after channel startup and each byte;
    # HDMA nested in active MDMA suppresses independent SyncStart/SyncEnd.
    checks['mesen_rundma_pending_checkpoint_count'] = run.count('ProcessPendingTransfers();') == 2
    checks['mesen_rundma_after_byte_checkpoint'] = 'i++;\n\t\tProcessPendingTransfers();' in run
    checks['mesen_global_startup_pending_checkpoint'] = '_dmaClockCounter += 8;\n\t\tProcessPendingTransfers();' in pending
    checks['mesen_pending_priority_hdma_before_init_before_mdma'] = (
        pending.find('if(_hdmaPending)') < pending.find('else if(_hdmaInitPending)') < pending.find('else if(_dmaPending)')
    )
    checks['mesen_hdma_init_nested_sync_suppression'] = 'bool needSync = !HasActiveDmaChannel();' in init and 'if(needSync) {\n\t\tSyncEndDma();' in init
    checks['mesen_hdma_scanline_nested_sync_suppression'] = 'bool needSync = !HasActiveDmaChannel();' in scan and 'if(needSync) {\n\t\t//If we ran a HDMA transfer, sync\n\t\tSyncEndDma();' in scan
    for k,v in checks.items():
        if k.startswith('mesen_') and not v: raise SystemExit(f'oracle contract failed: {k}')

    # Civilization implementation must have exactly the analogous target-specific
    # checkpoints and must retain its narrow proof guard rather than generic channel loops.
    require(prod,'if(!civ_dma_process_nested_hdma(i,cpu_speed,dma_clock_counter))return 0;','production_per_byte_pending_checkpoint',checks)
    checks['production_per_byte_checkpoint_after_8_clocks'] = bool(re.search(r'\*dma_clock_counter\+=8u;\s*civ_timing_advance_master\(i,8u\);\s*if\(!civ_dma_process_nested_hdma',prod))
    checks['production_global_checkpoint_after_8_clocks'] = bool(re.search(r'/\* Global manual-DMA startup overhead.*?dma_clock_counter\+=8u;\s*civ_timing_advance_master\(i,8u\);\s*if\(!civ_dma_process_nested_hdma',prod,re.S))
    checks['production_channel_checkpoint_after_8_clocks'] = bool(re.search(r'/\* One channel is source-proved active.*?dma_clock_counter\+=8u;\s*civ_timing_advance_master\(i,8u\);\s*if\(!civ_dma_process_nested_hdma',prod,re.S))
    checks['production_nested_hdma_suppresses_sync_start'] = 'if(!nested_manual_dma && !civ_hdma2_sync_start(i,&clocks))return 0;' in prod
    checks['production_nested_hdma_folds_clock_counter'] = '*manual_dma_clock_counter+=clocks;' in prod
    checks['production_hdma_mask_fail_closed_04'] = 'i->cpu_io[0x0Cu]!=0x04u' in prod
    checks['production_exact_channel2_config'] = all(x in prod for x in ['i->dma2.dmap==0x42u','i->dma2.bbad==0x0Du','i->dma2.source_address==0x4AE3u','i->dma2.source_bank==0xC1u','i->dma2.indirect_bank==0x00u'])
    checks['production_manual_dma_channels_bounded'] = 'channel_index!=0u && channel_index!=1u && channel_index!=7u' in prod
    for k,v in checks.items():
        if k.startswith('production_') and not v: raise SystemExit(f'production contract failed: {k}')

    # Exact isolated HDMA-disabled long-DMA accounting used by the V33 C selftest.
    # 6 MDMAEN bus write + 6 delayed CPU cycle + 4 start alignment + 8 global +
    # 8 channel startup + 65536*8 bytes + 2 end sync + 6 resumed CPU = 524328.
    isolated={
        'mdmaen_bus_write':6,'start_delay_cpu_cycle':6,'start_alignment':4,
        'global_startup':8,'channel_startup':8,'transfer_bytes':65536,
        'per_byte_clocks':8,'end_sync':2,'resumed_cpu_cycle':6,
    }
    isolated['expected_master_delta'] = 6+6+4+8+8+65536*8+2+6
    if isolated['expected_master_delta'] != 524328: raise AssertionError(isolated)

    doc={
      'schema':'civilization-v33-dma-hdma-oracle-contract-v1',
      'production_scope':'Civilization exact-ROM target-specific DMA/HDMA only',
      'mesence':{'repository':'nesdev-org/MesenCE','commit':PIN_COMMIT,'file':'Core/SNES/SnesDmaController.cpp','sha256':PIN_FILE_SHA256},
      'checks':checks,'isolated_hdma_disabled_65536_byte_case':isolated,
      'passed':all(checks.values()),
      'production_not_oracle_dependent':True,
    }
    a.out.parent.mkdir(parents=True,exist_ok=True)
    a.out.write_text(json.dumps(doc,indent=2,sort_keys=True)+'\n')
    print(json.dumps({'passed':doc['passed'],'checks':len(checks),'expected_master_delta':isolated['expected_master_delta']},sort_keys=True))

if __name__=='__main__': main()
