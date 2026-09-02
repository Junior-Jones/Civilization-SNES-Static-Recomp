# Civilization Full Static S-SMP / S-DSP provenance

Civilization's production audio lane contains two statically owned parts:

- The S-SMP executes only exact PC/opcode pairs in the generated Civilization authority. Unknown PCs, opcode mismatches, unemitted opcodes, writes to compiled code, and genuinely unknown ARAM reads fail closed. Its deterministic power-on model initializes all 64 KiB of ARAM to zero and marks that initialized state known, matching the inherited SimCity/Top Gear/Snes9x static lane. Its SPC700 instruction semantics were derived from the Snes9x BAPU implementation retained under `SNES9X-LICENSE.txt`; no generic opcode interpreter is compiled into production.
- The S-DSP is the project's fixed 32-phase SNES hardware program in `civilization-project-dsp`. It was adapted from the project-owned Jungle Strike static DSP, operates directly on Civilization ARAM/register state, tracks ARAM and output knownness, and produces PCM into a core-owned bounded FIFO. The previous `snes_spc` runtime S-DSP files have been removed.

The host advances both processors from the native SNES master-clock timeline and pulls stereo frames plus per-frame knownness through `civ_audio_read`. FIFO availability and overflow counts are explicit public diagnostics; the frontend is not permitted to invent or silently discard core timing.

The checked-in exact S-SMP authority contains 948 Civilization uploaded-driver instruction starts and 32 fixed SNES IPL instruction starts. `tools/audio/rebuild_civilization_smp_authority.py` regenerates its governing manifest directly from the frozen lookup, dispatch, and code-byte bitmap and verifies those three authorities agree.
