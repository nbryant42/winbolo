You're looking at my fork of the classic Winbolo tank game with Windows 11 compat bugfixes (for DirectDraw bugs, etc)
and other improvements. This project is legacy Win32/C, 32-bit only, build system is CMake (assumes Visual Studio
compilers) and builds several outputs:

- `WinBolo.exe`, main game executable.
- `WinBoloDS.exe`, standalone network game server
- `Log Viewer.exe`, views multiplayer game logs for post-hoc strategic analysis and/or mockery purposes
- `Brains/*`, brain (autopilot/AI) plugins.

Unit tests: nonexistent.

Shell environment is PowerShell (or maybe CMD), so Unix-style `&&` command chaining does not work.