#!/usr/bin/env python3
import pathlib, sys

root=pathlib.Path(sys.argv[1])
header=(root/'static-recomp/include/civilization_static_recomp.h').read_text(encoding='utf-8')
machine=(root/'static-recomp/src/civilization_machine.c').read_text(encoding='utf-8')
scheduler=(root/'static-recomp/src/civilization_scheduler.c').read_text(encoding='utf-8')
renderer=(root/'static-recomp/src/civilization_renderer.c').read_text(encoding='utf-8')
audio=(root/'static-recomp/src/civilization_audio.cpp').read_text(encoding='utf-8')
snapshot=(root/'static-recomp/src/civilization_snapshot.c').read_text(encoding='utf-8')
cmake=(root/'CMakeLists.txt').read_text(encoding='utf-8')
bus=(root/'static-recomp/src/civilization_bus.c').read_text(encoding='utf-8')
input_c=(root/'static-recomp/src/civilization_input.c').read_text(encoding='utf-8')

checks={
 'authoritative_frame_api': 'int civ_run_frame(' in machine and 'civ_run_frame(' in (root/'frontend/common/civilization_frontend.c').read_text(encoding='utf-8'),
 'all_host_hooks': all(x in header for x in ('CivHostFrameSink','CivHostFailureSink','CivHostDiagnosticSink','CivHostPcmSink')),
 'scanline_state_every_frame': 'scanline_ppu[i->vcounter]=i->ppu' in scheduler and 'scanline_state_valid[y]' in renderer,
 'instance_pcm_capture': 'v20_pcm_capture[8192u * 2u]' in header and 'g_capture' not in audio and 'g_host_sink' not in audio,
 'checked_audio_lease': 'g_owner->v20_full_static_audio_acquired=0u' in audio,
 'canonical_sha256_snapshot': all(x in snapshot for x in ('CVSNAP36','rom_sha256','payload_sha256','CIV_CORE_IDENTITY','replace_file_atomically','serialize_runtime','deserialize_runtime','civilization_snapshot_fields.inc')) and 'fwrite(runtime_copy' not in snapshot,
 'certification_bloat_removed': not any(x in header for x in ('v19_first_visible_vram[','v19_first_visible_cgram[','v19_first_visible_oam[','v19_first_visible_scanline_ppu_regs[')),
 'input_subsystem_split': 'civ_controller_serial_read' in input_c and 'static uint8_t civ_mouse_scaled_axis' not in bus,
 'test_build_hygiene': 'include(CTest)' in cmake and 'if(BUILD_TESTING)' in cmake and cmake.index('find_package(Python3')>cmake.index('if(BUILD_TESTING)'),
 'no_global_handwritten_warning_suppression': '/wd4310 /wd4702' not in cmake,
}
failed=[name for name,ok in checks.items() if not ok]
for name,ok in checks.items(): print(f"{'PASS' if ok else 'FAIL'} {name}")
if failed: raise SystemExit('architecture completion failures: '+', '.join(failed))
