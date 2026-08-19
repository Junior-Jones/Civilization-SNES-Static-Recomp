# Civilization Version 34 milestone ledger

Updated: 2026-08-13

A milestone is checked only after implementation and validation. Natural
observations validate behavior but never become static proof authority.

## M1 - Windows production shape

- [x] Preserve the frozen V33 archive in a separate V34 working tree.
- [x] Port persistence paths while retaining Linux compatibility.
- [x] Add a native Win32 launcher and framebuffer presentation.
- [x] Route Full Static PCM to WinMM without another audio core.
- [x] Retain exact-ROM validation, SRAM and five persistent snapshots.
- [x] Pass warning-as-error builds under MSVC and MinGW GCC.

## M2 - Headless natural-test instrumentation

- [x] Add compact CPU/input/video/audio `checkpoint` output.
- [x] Add deterministic relative `frame-step` and `tap-snes` commands.
- [x] Scale long frame requests with a proportional instruction budget.
- [x] Keep scripts, captures and screenshots outside production source.

## M3 - Inherited C3:B59C observation disposition

- [x] Recheck the note against the exact-ROM Windows headless build.
- [x] Complete a clean 60,000,000-instruction idle run without a frontier.
- [x] Complete a clean interactive setup/first-city route without a frontier.
- [x] Confirm the note is not an unconditional startup or selected-route edge.
- [x] Make no unproved guest/static change for a non-reproduced observation.
- [x] Carry deterministic reproduction/offline proof forward to V35 only if a
  clean route later reproduces it.

## M4 - Natural route closure

- [x] Reach visible non-black Mode 1 output with forced blank clear.
- [x] Drive title, New Game setup, Roman/Caesar naming and world generation.
- [x] Reach the playable map, complete tutorial and move a settler.
- [x] Found Rome, select Militia production and inspect city status.
- [x] Use screenshots to detect the second active settler and issue Sentry.
- [x] Display and confirm End of Turn; advance repeatedly to 3900 BC.
- [x] Keep Full Static audio cycle-synchronized with nonzero PCM.
- [x] Complete a 12,000-frame long step and cumulative 897,953,699-instruction
  saved route without a fail-closed frontier.

## M5 - Version 34 release gate

- [x] Pass clean MSVC Release build and all 36 tests.
- [x] Pass MinGW GCC 16.1.0 Release build and all 36 tests.
- [x] Pass exact/wrong-ROM, SRAM and fresh-process snapshot tests.
- [x] Pass deterministic natural replay from an isolated state directory.
- [ ] Regenerate V34 integrity receipts from final compact source.
- [x] Separate large repeat/history evidence from compact production source.
- [ ] Confirm no ROM, save, snapshot, audio capture or build artifact is packaged.

## M6 - Freeze, archive and handoff

- [ ] Freeze completed source under the project-level `Current` directory.
- [ ] Create matching source, release and evidence ZIP copies in `Current`.
- [ ] Verify each ZIP by hashes and clean extraction.
- [ ] Fresh-build and run all tests from the archived source extraction.
- [ ] Run the final natural test from the verified archived build.
- [x] Write the Version 34 final report and Version 35 milestone list.

## Evidence summary

- Clean idle: 60,000,000 instructions / 4,963 frames, no frontier; visible
  frame hash `4387978BE9904137`; 2,374,080 nonzero PCM frames.
- Clean natural route: title to first city, production/city/unit/end-turn flow;
  screenshots reviewed at each state transition.
- Long saved-route checkpoint: 897,953,699 instructions / 73,139 frames, no
  frontier, synchronized master clocks and nonzero Full Static PCM.
- Static gates: MSVC 36/36 and MinGW GCC 36/36.
