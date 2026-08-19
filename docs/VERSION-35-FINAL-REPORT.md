# Civilization Version 35 Test final static report

Version 35 Test completes the architecture-first static work while leaving both
frozen `Current` Version 35 folders unchanged. The exact-ROM graph contains
103,584 `PBR:PC:E:M:X` contexts and 102,236 unique instruction addresses. Its
analysis queue is empty, its fixed-point frontier count is zero, and all 69
indirect sites are producer/value-set complete.

## Compact production authority

The production S-CPU authority retains every exact context key but factors the
repeated emitted C structure into 299 immutable semantic templates and 294,241
frozen parameter words across 280 address shards. The generator reconstructs
all 103,584 original emitted bodies and asserts that both semantic streams have
the same SHA-256:

`144dcfc5b91247bd22a227b8a274d5b1e8a4cac5abe6367762f5a6c219e9b700`

This is static AOT data, not bytecode interpretation. Runtime execution performs
an exact context lookup and invokes the selected compiled template. It never
decodes a ROM opcode, learns a transition, widens an indirect target set, or
falls back to an emulator. The former large body-per-context shards are absent
from the test source and cannot be selected by CMake.

## Size

- compact generated authority source: 7,287,393 bytes (6.9498 MiB)
- compiled `libcivilization-static-recomp.a`: 2,572,804 bytes (2.4536 MiB)
- complete `civilization-headless.exe`: 2,959,686 bytes (2.8226 MiB)
- complete Windows `Launcher.exe`: 8,464,757 bytes (8.0726 MiB)

The compiled core is comfortably inside the requested 3-8 MB neighborhood and
is no longer in the tens-of-megabytes range.

For source-layout context, the installed complete generated folders measured
13.1941 MiB for Top Gear Version 27 and 29.5023 MiB for the SimCity GitHub
source. Those projects partition generated authority differently, so their
folder totals are architectural comparisons rather than identical link-unit
measurements. The apples-to-apples Civilization compiled library figure is
2.4536 MiB.

## Headless validation

Strict MinGW GCC Release compilation completed with warnings treated as errors.
CTest passed 39/39 in 1.01 seconds. The suite covers authority receipts and all
280 shard hashes, indirect/return proofs, MMIO, DMA/HDMA, PPU, renderer, SRAM,
snapshots, controller serial behavior, Full Static audio and the screenshot log.

The final compact executable and the preserved body-per-context baseline both
exited zero on the maintained CLI equivalence route. Their 14,028-byte output
files were byte-identical with SHA-256
`e9687dc52fa39016c1b45baae964d7af976b8aa3cbc8eb06e2c4195b31c71c08`.
The route checks reset, instruction stepping, frame boundaries, input injection,
CPU/PPU/memory/video/audio state through frame 10. It is a deterministic static
comparison, not natural gameplay testing.

## Screenshot diagnostics

Every native screenshot now creates a same-named text file in
`Screenshots/Logs`. It records the exact CPU context/key and registers,
instruction/frame/master-clock counters, interrupt and controller state,
PPU/video hashes, WRAM/SRAM/guest-state hashes, DMA counters, Full Static audio
counters and any fail-closed frontier. A dedicated headless test boots one frame,
writes the log and verifies its required fields.

Natural testing remains paused by user instruction. No result in this report is
presented as a substitute for later user-guided gameplay validation.
