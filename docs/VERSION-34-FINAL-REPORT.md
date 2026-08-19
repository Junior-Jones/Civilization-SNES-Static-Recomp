# Civilization Version 34 final report

Date: 2026-08-13

## Outcome

Version 34 completes the Windows production layer around the frozen Version 33
Civilization static authority. The release has a native Win32 framebuffer and
keyboard launcher, WinMM playback of Full Static PCM, Windows-safe SRAM and five
persistent snapshots, and improved deterministic headless testing.

No guest/static widening was made for the earlier C3:B59C runtime note. A clean
60,000,000-instruction idle run and a clean interactive route did not reproduce
that frontier. The interactive route reached the title, New Game setup,
Roman/Caesar naming, world generation, first playable map, tutorial, movement,
Rome founding, production choice, city status, unit orders and repeated turn
advancement. The cumulative saved route later reached 897,953,699 instructions;
a 12,000-frame relative step completed without a frontier.

Video remained visible in Mode 1 with forced blank clear. Full Static audio was
cycle-synchronized at every checkpoint and continued producing nonzero PCM.
Screenshot inspection caught an active second settler and corrected the test
route before end-turn validation, demonstrating that the route was driven by
actual rendered state rather than assumed menus.

## Release gates

- MSVC 19.40, Windows SDK 10.0.22621.0, `/W4 /WX`: 36/36 tests passed.
- MinGW GCC 16.1.0, `-Wall -Wextra -Wpedantic -Werror`: 36/36 tests passed.
- GCC exposed and Version 34 fixed one launcher indentation warning and the
  MinGW Unicode GUI entry-point link option.
- Wrong-ROM rejection, cartridge identity, SRAM and fresh-process persistent
  snapshots are covered by the complete test gate.
- Long `frame-step` requests now use a proportional instruction budget; the
  discovered 12,000-frame artificial timeout was removed and regression-run.

## Compact structure

Production source retains the primary fixed-point/ownership manifests needed by
the 36-test static gate. Large byte-identical repeat manifests are stored in the
separate evidence archive. The external ROM, saves, snapshots, screenshots,
audio captures and compiler build directories are excluded from source/release
packages.

Final archive sizes and SHA-256 values are recorded in the project-level
`Current/ARCHIVE-MANIFEST.sha256` and `Current/SIZE-COMPARISON.txt` files.
