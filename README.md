# Civilization (SNES) Static Recomp 1.1.1

Native Windows static recompilation frontend and game-specific core for
Civilization on the Super Nintendo Entertainment System.

The original game ROM is not included. Use an externally supplied, legally
obtained unheadered USA `.sfc` ROM matching `ROM-REQUIREMENTS.txt`.

## What "fully static recompilation" means

Civilization is a fully static, ahead-of-time recompilation. The game's
executable W65C816 instructions were analysed and translated into native C code
before the application was built. The production launcher executes this
generated code directly.

At runtime, the application does not use a general-purpose SNES CPU
interpreter, runtime opcode decoder, dynamic recompiler, JIT compiler, learning
system or emulator fallback. Its frozen CPU authority contains 103,584 exact
processor contexts compiled across 280 compact ahead-of-time shards. If
execution reaches an unknown context, the core stops with a diagnostic error
instead of silently switching to interpreted execution.

The complete runtime includes:

- Generated W65C816 game-code execution.
- Native cartridge mapping, WRAM and machine-bus handling.
- Native PPU register, rendering and per-scanline presentation logic.
- Native DMA and HDMA handling.
- NTSC scheduling, refresh, NMI, IRQ and controller timing.
- Full Static S-SMP and S-DSP audio producing native PCM.
- Deterministic input, framebuffer, snapshot and persistence handling.
- Exact-ROM SHA-256 validation and fail-closed runtime checks.
- A native Windows launcher with accessible standard controls, keyboard
  operation and statically linked host libraries.

"Fully static" does not mean that gameplay, graphics or sound are prerecorded.
The game continues to respond live to player input and evolving machine state.
It means that game instructions are compiled ahead of time and no interpretive
or fallback execution engine is used in the production application.

Development tests may use Python scripts or reference-emulator comparisons to
verify results. Those tools are not included in, or used by, the production
runtime.

## Building on Windows

Requirements:

- Windows 10 or later
- CMake 3.20 or later
- A C11/C++17 Windows compiler
- Internet access during the first configure so pinned SDL 3.4.10 can download

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build --config Release --target civilization-launcher
```

The launcher links SDL statically, so no SDL DLL is required beside
`Launcher.exe`. Tests, Python and the exact ROM are not production runtime
dependencies.

Version 1.1.1 retains 103,584 exact generated CPU contexts, checked Full Static
audio authority and fail-closed dispatch. See
`docs/PART-05-THROUGH-12-COMPLETION-REPORT.txt` for the completed architecture
work and validation evidence.
