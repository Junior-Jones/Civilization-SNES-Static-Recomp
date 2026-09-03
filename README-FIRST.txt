Civilization (SNES) Static Recomp 1.3.0

This is a native Windows static recompilation frontend and game-specific core
for Civilization on the Super Nintendo Entertainment System.

The original ROM is not included. Put the exact unheadered USA .sfc ROM in the
Rom folder beside Launcher.exe, then use Browse and Run. Required SHA-256:

  de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32

Version 1.3.0 integrates the current accessible Windows frontend, DirectSound
audio output and resampling, NTSC frame pacing, full-screen presentation and
capture, controller settings, five persistent snapshot slots and battery SRAM.
The saved Wide Screen option expands the live world map to 398x224 with real
terrain and extended cursor input while preserving fog of war. It adds visible
map space on every edge, uses a true near-16:9 presentation without stretching,
and keeps menus and city screens at their original 256x224 geometry.
The Welcome window now restores focus to the launcher or running game when it
closes. The core continues to provide opaque lifecycle, one headed/headless
frame path, explicit host hooks, per-scanline presentation, checked Full Static
audio ownership and integrated source/generated provenance verification.

The frozen static CPU authority remains 103,584 exact PBR:PC:E:M:X contexts in
280 compact AOT shards. It has no runtime opcode decoder, learning or emulator
fallback.

Build instructions and detailed verification reports are under docs.
