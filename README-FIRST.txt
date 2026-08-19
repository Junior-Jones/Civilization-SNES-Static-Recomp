Civilization (SNES) Static Recomp 1.1.0

This is a native Windows static recompilation frontend and game-specific core
for Civilization on the Super Nintendo Entertainment System.

The original ROM is not included. Put the exact unheadered USA .sfc ROM in the
Rom folder beside Launcher.exe, then use Browse and Run. Required SHA-256:

  de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32

Version 1.1.0 completes the architecture improvement roadmap: opaque lifecycle,
one headed/headless frame path, explicit host hooks, per-scanline presentation,
checked Full Static audio ownership, canonical SHA-256 snapshots, reduced core
state, separated machine subsystems, optional test dependencies and integrated
source/generated provenance verification.

The frozen static CPU authority remains 103,584 exact PBR:PC:E:M:X contexts in
280 compact AOT shards. It has no runtime opcode decoder, learning or emulator
fallback.

Build instructions and detailed verification reports are under docs.
