# Civilization 1.1.1 provenance

The exact target ROM is 1,572,864 bytes with SHA-256
`de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32`.
The ROM is external test input and excluded from production archives.

Earlier completed static recomp projects were architecture/method references
only: one current generated CPU authority, compact game-specific hardware
support, Full Static audio, shared frontend hooks and fail-closed unknowns.
Civilization contains only its own generated game logic and runtime state.

The current exact-ROM fixed point contains 103,584 runtime contexts, 122,224
proof states, 2,425 procedures and 8,939 call edges with zero frontiers. It is
never widened from gameplay observation. All 69 finite indirect sites are
independently decoded from ROM bytes and have complete producer/value-set
proofs. Special NMI saved-frame re-entry, threaded RTS and native IRQ/NMI roots
are independently checked.

Host persistence does not add another guest execution path. Cartridge SRAM is
a separate 32 KiB battery image that survives console
reset. Five numbered snapshot slots serialize the exact paused Civilization
machine plus Full Static audio, omit the external ROM pointer, validate
version/ROM/integrity on load and can restore in a fresh frontend/headless
process. Headless commands call the same shared frontend persistence APIs.

Pinned MesenCE 2.2.1 source remains a post-implementation SNES hardware oracle
only. Western Design Center W65C816S documentation remains the primary CPU
architecture reference. Full Static audio provenance/licenses remain under
`static-recomp/static-audio/civilization-bapu-aot/`.

Historical milestone receipts remain documentation only and are superseded by
the current 1.1.1 closure and audit reports.
