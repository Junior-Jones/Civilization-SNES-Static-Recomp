#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, re, sys
from pathlib import Path

ROM_SHA = 'de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32'
CONTEXTS = 103584
INDIRECT = 69
SHARDS = 280
RETURN_SITES = 2117
RETURN_TARGETS = 7889

def sha(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()

def fail(msg: str) -> None:
    raise RuntimeError(msg)

def main() -> int:
    ap=argparse.ArgumentParser()
    ap.add_argument('--project', type=Path, required=True)
    a=ap.parse_args()
    root=a.project.resolve()
    analysis=root/'docs/research/civilization_v33_fixedpoint.manifest.json'
    proof=root/'config/static-indirect-control-flow-v33.json'
    nameproof=root/'docs/research/civilization_v33_name_state_machine.proof.json'
    transproof=root/'docs/research/civilization_v33_indirect_transition_consistency.json'
    effective=root/'docs/research/civilization_v33_indirect_effective_address.json'
    completeness=root/'docs/research/civilization_v33_indirect_domain_completeness.json'
    newroute=root/'docs/research/civilization_v33_new_context_route.json'
    callreturn=root/'docs/research/civilization_v33_call_return_consistency.json'
    setupcity=root/'docs/research/civilization_v33_setup_city_route_coverage.json'
    legacy2=root/'docs/research/civilization_v33_legacy_indirect_second_method.json'
    generalized=root/'docs/research/civilization_v33_generalized_indirect_domains.json'
    producer35=root/'docs/architecture/civilization_v35_indirect_producer_completeness.json'
    specialret=root/'docs/research/civilization_v33_special_return_reentry.json'
    earthcity=root/'docs/research/civilization_v33_earth_city_lifecycle_clusters.json'
    generated=root/'static-recomp/generated/civilization_generated_core.manifest.json'
    compact=root/'static-recomp/generated/compact-aot/civilization_compact_aot.manifest.json'
    genpy=root/'tools/scpu/generate_v33_closed_core.py'
    sempy=root/'tools/scpu/w65c816_emit_semantics.py'
    basesempy=root/'tools/scpu/w65c816_base_semantics.py'
    runtime=root/'static-recomp/src/civilization_runtime.c'
    snapshot=root/'static-recomp/src/civilization_snapshot.c'
    frontend=root/'frontend/common/civilization_frontend.c'
    headless=root/'frontend/linux/civilization_headless.c'
    cmake=root/'static-recomp/CMakeLists.txt'
    for p in (analysis,proof,nameproof,transproof,effective,completeness,newroute,callreturn,setupcity,legacy2,generalized,producer35,specialret,earthcity,generated,compact,genpy,sempy,basesempy,runtime,snapshot,frontend,headless,cmake):
        if not p.is_file(): fail(f'missing required Version 33 file: {p.relative_to(root)}')
    an=json.loads(analysis.read_text())
    gm=json.loads(generated.read_text())
    compact_manifest=json.loads(compact.read_text())
    ip=json.loads(proof.read_text())
    np=json.loads(nameproof.read_text()); tp=json.loads(transproof.read_text()); ep=json.loads(effective.read_text())
    cp=json.loads(completeness.read_text()); nr=json.loads(newroute.read_text()); cr=json.loads(callreturn.read_text()); sc=json.loads(setupcity.read_text())
    l2=json.loads(legacy2.read_text()); gd=json.loads(generalized.read_text()); p35=json.loads(producer35.read_text()); sr=json.loads(specialret.read_text()); ec=json.loads(earthcity.read_text())
    checks={
        'analysis_format': an.get('format')=='civilization-v33-finite-interprocedural-closure-v1',
        'analysis_contexts': an.get('runtime_context_count')==CONTEXTS,
        'analysis_frontiers_zero': an.get('static_analysis_frontier_count')==0 and an.get('static_analysis_frontiers')==[],
        'analysis_queue_empty': an.get('work_queue_empty') is True,
        'analysis_indirect_count': an.get('indirect_proved_site_count')==INDIRECT,
        'analysis_rom': an.get('rom_sha256')==ROM_SHA,
        'indirect_format': str(ip.get('format','')).startswith('civilization-static-indirect-control-flow-v33-explicit-runtime-x-'),
        'indirect_site_count': len(ip.get('proved_sites',{}))==INDIRECT,
        'name_state_machine_proof_pass': np.get('result','').startswith('PASS') and np.get('legal_x')==list(range(0,0x20,2)) and np.get('natural_test_observations_used') is False,
        'transition_consistency_pass': tp.get('result')=='PASS' and tp.get('declared_legal_x')==list(range(0,0x20,2)) and tp.get('natural_test_observations_used') is False,
        'indirect_effective_address_pass': ep.get('result')=='PASS' and ep.get('site_count')==69 and ep.get('transformed_sites')==['C2:37A7','C3:236E'] and ep.get('runtime_learning_used_as_proof') is False,
        'indirect_domain_completeness_pass': cp.get('result')=='PASS' and cp.get('finite_site_count')==69 and cp.get('transformed_sites')==['C2:37A7','C3:236E'] and len(cp.get('producer_derived',{}).get('C3:236E',{}).get('cases',[]))==4 and cp.get('runtime_observation_used_as_proof') is False,
        'new_context_route_pass': nr.get('result')=='PASS' and nr.get('runtime_context_count')==48 and nr.get('procedure')=='C3:2732/RTS' and nr.get('local_mmio_operands')==[],
        'call_return_consistency_pass': cr.get('result')=='PASS' and cr.get('call_edge_count')==8939 and cr.get('procedure_count')==2425 and cr.get('normal_return_site_count')==2109,
        'setup_city_route_coverage_pass': sc.get('result')=='PASS' and sc.get('strong_current_site_count')==6 and sc.get('legacy_finite_site_count')==1 and sc.get('runtime_observation_used_as_proof') is False,
        'legacy_indirect_second_method_pass': l2.get('result')=='PASS' and len(l2.get('rows',[]))==17 and len(l2.get('priority_sites',[]))==6,
        'generalized_indirect_domains_pass': gd.get('result')=='PASS' and gd.get('site_count')==69 and gd.get('producer_value_set_complete_site_count')==69 and gd.get('runtime_observation_used_as_proof') is False,
        'v35_indirect_producer_completeness_pass': p35.get('result')=='PASS' and p35.get('site_count')==69 and p35.get('producer_value_set_complete_site_count')==69 and p35.get('runtime_observation_used_as_proof') is False,
        'special_return_reentry_pass': sr.get('result')=='PASS' and len(sr.get('nmi_stack_reentry_edges',[]))==3 and len(sr.get('threaded_rts_sites',{}))==3 and sr.get('interrupt_root_count')==12 and sr.get('runtime_observation_used_as_proof') is False,
        'earth_city_lifecycle_clusters_pass': ec.get('result')=='PASS' and len(ec.get('clusters',[]))==4 and ec.get('runtime_context_count')==CONTEXTS and ec.get('runtime_observation_used_as_proof') is False,
        'generated_format': gm.get('format')=='civilization-closed-production-scpu-v33',
        'generated_contexts': gm.get('runtime_context_count')==CONTEXTS,
        'generated_frontiers_zero': gm.get('static_analysis_frontier_count')==0,
        'generated_queue_empty': gm.get('work_queue_empty') is True,
        'generated_indirect_count': gm.get('indirect_proved_site_count')==INDIRECT,
        'generated_shards': gm.get('generated_shard_count')==SHARDS,
        'generated_return_sites': gm.get('return_proof_site_count')==RETURN_SITES,
        'generated_return_targets': gm.get('return_proof_target_count')==RETURN_TARGETS,
        'generated_no_decode': gm.get('runtime_opcode_decode') is False,
        'generated_no_learning': gm.get('runtime_learning') is False,
        'generated_no_fallback': gm.get('runtime_emulator_fallback') is False,
        'generated_rom': gm.get('rom_sha256')==ROM_SHA,
        'generated_analysis_receipt': gm.get('analysis_manifest_sha256')==sha(analysis),
        'generated_indirect_receipt': gm.get('indirect_proof_sha256')==sha(proof),
        'generated_generator_receipt': gm.get('generator_sha256')==sha(genpy),
        'generated_semantics_receipt': gm.get('semantics_generator_sha256')==sha(sempy),
        'generated_base_semantics_receipt': gm.get('base_semantics_generator_sha256')==sha(basesempy),
    }
    shard_dir=root/'static-recomp/generated/compact-aot/civilization_compact_aot_shards'
    shard_files=sorted(shard_dir.glob('*.c'))
    checks['physical_shard_count']=len(shard_files)==SHARDS
    # Every compact generated receipt must match the on-disk shard it names.
    receipts=dict(compact_manifest.get('shards',{}))
    checks['manifest_shard_receipt_count']=len(receipts)==SHARDS
    checks['all_shard_receipts']=all((shard_dir/name).is_file() and sha(shard_dir/name)==digest for name,digest in receipts.items())
    templates=root/'static-recomp/generated/compact-aot/civilization_compact_aot_templates.c'
    checks['compact_format']=compact_manifest.get('format')=='civilization-v35-compact-exact-context-aot-v1'
    checks['compact_contexts']=compact_manifest.get('runtime_context_count')==CONTEXTS
    checks['compact_templates']=compact_manifest.get('semantic_template_count')==299
    checks['compact_parameter_words']=compact_manifest.get('parameter_word_count')==294241
    checks['compact_templates_receipt']=compact_manifest.get('templates_sha256')==sha(templates)
    checks['compact_exact_reconstruction']=compact_manifest.get('all_context_semantics_reconstructed_exactly') is True and compact_manifest.get('original_semantics_sha256')==compact_manifest.get('reconstructed_semantics_sha256')
    checks['compact_no_decode_learning_fallback']=compact_manifest.get('runtime_rom_opcode_decode') is False and compact_manifest.get('runtime_learning') is False and compact_manifest.get('runtime_emulator_fallback') is False
    rt=runtime.read_text()
    st=cmake.read_text()
    snap=snapshot.read_text(); fe=frontend.read_text(); hl=headless.read_text()
    checks['single_dispatch_call']=rt.count('civ_generated_core_step(i)')==1
    checks['runtime_no_old_cpu_layers']=not re.search(r'civ_v(?:0[1-9]|1[0-9]|20)_(?:generated_bootstrap|core_generated)',rt)
    checks['runtime_no_v22_indirect_guard']='civ_v22_unproved_indirect_site' not in rt and 'civilization_v22_indirect_guard.h' not in rt
    checks['cmake_single_v35_compact_authority']='generated/compact-aot' in st and 'civilization_compact_aot_templates.c' in st and 'civilization_generated_core_shards' not in st and 'civilization_v20_bootstrap.c' not in st and 'civilization_v20_core_continuation.c' not in st
    # Old generated CPU layers must not be present in production generated/.
    checks['no_old_generated_cpu_files']=not any(re.search(r'civilization_v(?:0[1-9]|1[0-9]|20)_',p.name) for p in (root/'static-recomp/generated').rglob('*') if p.is_file())
    checks['persistent_snapshot_direct_state']='CVSNAP36' in snap and 'payload_sha256' in snap and 'CIV_CORE_IDENTITY' in snap and 'serialize_runtime' in snap and 'deserialize_runtime' in snap and 'civ_v20_audio_state_save' in snap and 'civ_v20_audio_state_load' in snap
    checks['frontend_has_separate_sram_snapshot_paths']='Civilization.srm' in fe and 'Snapshots' in fe and 'Snapshot%u.civsnap' in fe
    checks['headless_uses_shared_persistence']='civ_frontend_snapshot_save' in hl and 'civ_frontend_snapshot_load' in hl and 'civ_frontend_flush_persistent_sram' in hl and 'CIVILIZATION_STATE_DIR' in hl
    checks['five_one_based_snapshot_slots']='slot<1u||slot>CIV_FRONTEND_SNAPSHOT_SLOTS' in fe and 'CIV_FRONTEND_SNAPSHOT_SLOTS 5u' in (root/'frontend/common/civilization_frontend.h').read_text()
    failed=[k for k,v in checks.items() if not v]
    print(json.dumps({'pass':not failed,'checks':checks,'failed_checks':failed},indent=2,sort_keys=True))
    return 0 if not failed else 1

if __name__=='__main__':
    try: sys.exit(main())
    except Exception as e:
        print(json.dumps({'pass':False,'error':str(e)},indent=2))
        sys.exit(1)
