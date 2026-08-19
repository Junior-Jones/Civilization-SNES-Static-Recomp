# Civilization Version 10 observation patch for MesenCE 2.2.1

Oracle-only. Never compile or link these files into the production static recomp.

The patch is based on pristine MesenCE 2.2.1 commit `20ba206cef5ba207c21203176d02cb9f43dda9fb`. It records bank-qualified S-CPU register reads/writes, typed DMA `DR/DW` rows, instruction state and optional WRAM writes. The bounded trace ends when the S-CPU reaches `C0:8088`, immediately before Civilization calls its real upload routine at `C0:04E4`.

Version 10 static behavior and all target tests are completed independently before Mesen source/runtime comparison. Mesen output remains comparison-only and may not generate contexts, resolve targets, or supply endpoint state. The separately compiled `SnesDisUtils` and `SpcDisUtils` debugger checks are decoder cross-checks, not runtime execution certification.
