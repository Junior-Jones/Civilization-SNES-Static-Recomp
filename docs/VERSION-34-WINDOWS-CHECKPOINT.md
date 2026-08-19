# Civilization Version 34 Windows working checkpoint

Date: 2026-08-13

## Outcome

Version 34 is now a native Windows working tree based on the frozen Version 33
static authority. The guest CPU, machine, PPU, DMA, controller and Full Static
audio architecture remain exact-ROM specialized. No interpreter, runtime
learning path, general emulator fallback or ROM content was added.

The native launcher is built as `Civilization-Launcher.exe` and provides:

- exact-ROM selection and validation;
- Win32 framebuffer presentation;
- WinMM playback fed by a host-only PCM sink on the existing Full Static audio;
- play/pause and reset;
- keyboard SNES controls;
- persistent 32 KiB cartridge SRAM;
- five persistent snapshot slots.

## Windows build authority

- Visual Studio Enterprise 2022
- MSVC 19.40.33811 x64
- Windows SDK 10.0.22621.0
- CMake 3.28.3 (Visual Studio bundle)
- Ninja 1.11 is also available

The MSVC Release build passes `/W4 /WX`. CTest passes 36/36 inherited tests,
including exact-ROM rejection, closed-authority, static audio, bus/PPU/DMA,
controller, SRAM and persistent snapshot checks.

The Windows launcher was also exercised interactively with the exact ROM. It
accepts the ROM and enters the static core. The screen remains black because
the inherited natural-execution frontier is not repaired in this checkpoint.

## Remaining offline proof frontier

The inherited natural test fails closed at `C3:B59C`, instruction
`LDA $0106,Y`, with runtime `Y=$2C00`, `DBR=0`, producing effective address
`$00:2D06`.

Offline tracing shows the surrounding routine derives the index from the
shared work-buffer word at `$020E` and a loop index. That word is also exposed
to byte-indexed writes, so treating the observed value as a simple counter or
forcing it into range would be unsound. Before changing guest behavior,
Version 34 must prove:

1. every producer and value domain for `$020E/$020F` at the call site;
2. the complete effective-address domain of `LDA $0106,Y`;
3. the correct SNES read/open-bus semantics for every address in that domain;
4. natural replay beyond the frontier without adding a fallback path.

## Size and planned compaction

Current uncompressed source-tree sizes (files only) are:

| Core | Bytes | MiB | Decimal MB |
|---|---:|---:|---:|
| Civilization V34 Windows WIP | 95,941,133 | 91.50 | 95.94 |
| SimCity complete source | 32,343,597 | 30.85 | 32.34 |
| Top Gear V27 Windows source | 15,306,248 | 14.60 | 15.31 |

Civilization is currently about 2.97 times SimCity and 6.27 times Top Gear.
Most of the difference is proof evidence, not host media: `docs/research` is
about 57.77 MiB and the generated static CPU authority is about 33.14 MiB.

The two byte-identical repeat proof manifests total 30,082,088 bytes
(28.69 MiB). Moving those reproducibility copies to release evidence after the
Version 34 proof is frozen would reduce the working source tree to about
62.80 MiB without touching production authority. Do not remove them yet: an
inherited closed-authority verifier and integrity record currently require
them. The final Top Gear/SimCity-style package should separate compact
production source from reproducibility/history evidence and regenerate the
integrity manifest after the new frontier proof is complete.

## Cleanup performed

Two duplicate SimCity handbook example WAV files (524,332 bytes each) were
removed from extracted reference material. No production audio source was
removed. The deleted reference WAVs are recoverable by re-extracting their
original archives.
