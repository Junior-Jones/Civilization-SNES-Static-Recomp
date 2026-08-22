# Civilization (SNES) Static Recomp 1.1.1

Native Windows static recompilation frontend and game-specific core for
Civilization on the Super Nintendo Entertainment System.

The original game ROM is not included. Use an externally supplied, legally
obtained unheadered USA `.sfc` ROM matching `ROM-REQUIREMENTS.txt`.

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
