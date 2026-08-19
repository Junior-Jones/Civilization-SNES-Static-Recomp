# Civilization Full Static S-SMP / S-DSP provenance

This directory is the current Version 30 **Full Static** audio lane for Civilization (SNES).
It was adapted from the complete SimCity static-recomp reference supplied with the
project.  The hardware implementation is a separately namespaced copy of the
Snes9x BAPU S-SMP and Shay Green `snes_spc` S-DSP sources retained under their
original licenses.

The production build defines `SC_SMP_AOT` and `SC_STATIC_SDSP_PRIMITIVES`.
The generic SPC700 opcode switch is therefore excluded: execution is admitted only
when the exact current PC/opcode pair is present in the generated Civilization
initial-driver AOT authority, and the dispatcher contains only opcode bodies used
by that authority. Unknown PCs, opcode mismatches, unemitted opcodes, and writes to
statically owned S-SMP code fail closed. There is no interpreter fallback.

The Civilization AOT authority is regenerated from
`static-recomp/generated/civilization_v08_spc700_closure.json`, whose 948 driver
instruction starts were proved from the exact Civilization USA ROM, plus the fixed
64-byte SNES IPL ROM instruction starts needed to reach the uploaded epoch from
power-on. The code-byte bitmap is generated only from proved Civilization driver
instruction bytes, not from arbitrary uploaded data.

The S-DSP lane uses the static 32-phase synthesis program and static-only renamed
hardware primitives from the supplied SimCity reference. It consumes the real
Civilization ARAM and S-DSP register state reached by the AOT S-SMP and emits PCM;
it does not emulate or interpret S-CPU program code.
