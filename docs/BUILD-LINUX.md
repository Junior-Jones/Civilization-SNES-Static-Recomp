# Civilization 1.1.1 Linux build and static gate

The Civilization ROM is external and must match `ROM-REQUIREMENTS.txt`.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCIVILIZATION_TEST_ROM=/path/to/Civilization-USA.sfc
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

The 1.1.1 release gate contains **52 tests**. It retains the target-bounded
machine, CPU, PPU, DMA/HDMA, controller, ROM-identity, lifecycle, bus-surface,
Full Static audio and finite-state checks and adds the current proof/persistence
acceptance gates:

- `v33-legacy-indirect-second-method` independently recertifies all 17 finite
  sites that remained on the older proof-method inventory in Version 32;
- `v33-generalized-indirect-domains` structurally re-decodes all 69 finite
  indexed-indirect sites from the exact ROM and records producer/value-set
  completeness where source producers are independently closed;
- `v33-special-return-reentry` independently checks the three NMI saved-frame
  re-entry edges, all three threaded-RTS dispatchers and native IRQ/NMI width
  roots;
- `v33-earth-city-lifecycle-clusters` proves ownership of the setup/EARTH,
  tribe/leader, city/menu, production/view/end-turn/research clusters without
  using gameplay PCs as generation authority;
- `v33-sram-persistence` verifies battery SRAM survives console reset and the
  exact 32 KiB image can be copied/loaded independently of snapshots;
- `v33-persistent-snapshot` verifies a numbered snapshot can be saved, the
  headless process exited, a fresh process started, and the same CPU/frame/
  master-clock/framebuffer/Full-Static-audio state restored; corrupt snapshots
  are rejected.

Both GCC and Clang Release builds must pass 52/52 before source freeze. A
release source ZIP is then certified by clean extraction of that exact archive,
internal source-hash verification, fresh GCC configure/build against the
external ROM and another 52/52 CTest pass.

Build directories, binaries, object files, ROM/save/snapshot files, nested
archives, Python caches and working-machine absolute paths are forbidden from
the source ZIP. Natural gameplay testing happens only after the source ZIP is
frozen; a post-freeze failure is evidence for the next version, never
permission to alter the frozen static proof.
