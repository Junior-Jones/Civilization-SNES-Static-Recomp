# Civilization 1.1.0 Windows build

Civilization 1.1.0 builds with Visual Studio 2022 and the Windows 10/11 SDK.
Warnings are errors. CMake downloads the pinned SDL 3.4.10 source used by the
native presentation, audio and gamepad layers; the release links SDL statically.

The ROM stays outside the source tree. Its SHA-256 is
`de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32`.

From a Visual Studio developer shell:

```powershell
cmake -S . -B build-msvc -A x64
cmake --build build-msvc --config Release --target civilization-launcher --parallel
```

The native app output is `build-msvc/Release/Launcher.exe`. Copy it together
with `gamecontrollerdb.txt` and `SDL-LICENSE.txt`. At runtime the app looks for
an exact `.sfc` ROM in a `Rom` folder beside `Launcher.exe`.

The headless frontend remains available for future scripted checks:

```powershell
cmake --build build-msvc --config Release --target civilization-headless --parallel
```

Configure with `CIVILIZATION_TEST_ROM` pointing to the exact external ROM to
enable the complete Release test set. The current gate contains 52 tests. The
deterministic Mesen/static cold-boot-to-gameplay certification is documented in
`CIVILIZATION-STATIC-CORE-AUDIT-2026-08-19.txt`.
