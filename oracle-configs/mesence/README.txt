MesenCE is the primary independent runtime/debugger oracle for this project.
It is never production fallback or generation authority.

Version 06 uses the user-supplied pristine MesenCE 2.2.1 source in two analysis-only ways:
1. Its actual SNES debugger disassembler (`SnesDisUtils`) independently checks exact-ROM instruction bytes/normalized mnemonics after the static gate. All 831 generated Version 06 contexts pass, and the C0:04E4 audio bootstrap was also independently decoded.
2. A separate patched working copy provides bounded observation. The trace stops only at D4:8D72 when X=$0004, so earlier valid X=0/2 stage dispatches are not mistaken for completion. `DR`/`DW` identify DMA events and full bank qualification prevents cartridge addresses from being mislabeled as I/O.

Use civilization-v06-profile.json for the current boundary. Full runtime comparison remains pending required Mesen build/runtime dependencies in this environment.

Version 09: civilization-v09-profile.json / civilization-v09.lua; bounded at D4:8D72 with X=$000A.
