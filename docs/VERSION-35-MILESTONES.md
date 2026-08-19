# Civilization Version 35 Test milestone completion

- [x] Preserve the frozen `Current` Version 35 source and app unchanged.
- [x] Classify the exact 1,572,864-byte ROM and independently scan orphan code.
- [x] Close the static graph at 103,584 exact `PBR:PC:E:M:X` contexts,
  102,236 unique addresses, 122,224 proof states, 2,425 procedures and 8,939
  call edges, with an empty queue and zero frontiers.
- [x] Reconcile 582 strict external-disassembler candidates with zero unresolved.
- [x] Prove all 69 indirect sites producer/value-set complete without gameplay
  observations, runtime learning or widened target sets.
- [x] Preserve 2,117 return-proof sites and 7,889 finite return targets.
- [x] Reject the basic-block prototype after measurement showed larger output.
- [x] Factor the exact emitted semantics into 299 frozen AOT templates and
  294,241 parameter words while retaining all 103,584 context keys.
- [x] Prove all-context semantic reconstruction exact by matching SHA-256
  `144dcfc5b91247bd22a227b8a274d5b1e8a4cac5abe6367762f5a6c219e9b700`.
- [x] Make the compact AOT graph the only production CPU authority.
- [x] Remove the superseded 32 MB body-per-context copy from the test source.
- [x] Retain no runtime opcode decoder, learning, emulator fallback or selectable
  historical core.
- [x] Add screenshot companion logs under `Screenshots/Logs` with CPU, PPU,
  memory, DMA, audio, hashes and fail-closed state.
- [x] Pass strict GCC Release compilation and 39/39 CLI/headless tests.
- [x] Match the former body-per-context core byte-for-byte on a deterministic
  reset/instruction/frame/input/video/audio checkpoint route through frame 10.
- [x] Keep natural gameplay testing paused as explicitly requested by the user.
