# Building DnsManager

This document explains how to configure, build, run and test DnsManager on
Windows 10/11 using Visual Studio, Visual Studio Code or the command line.

The C++ build produces `dns-core.exe`, a headless backend sidecar. The user
interface is the Electron + React app in `electron-app/` — there is no Qt UI
(QML/Widgets/Quick were removed); Qt is used only as a C++ utility library
(Core/Network/Concurrent) inside the backend.

## Prerequisites

| Tool            | Version / Notes                                     |
|-----------------|-----------------------------------------------------|
| Windows         | 10 (1809+) or 11                                    |
| CMake           | 3.25+                                               |
| Compiler        | MSVC 2022 (v143) or MinGW-w64 (GCC 12+)             |
| Qt              | 6.5+ base kit (`Qt Core`, `Network`, `Concurrent`, `Test`) — backend library only |
| Ninja (optional) | Recommended for fast single-config builds          |
| Git             | 2.30+                                               |
| Node.js         | 20+ with npm (for `electron-app/`)                  |

### Installing Qt

Use the [Qt Online Installer](https://www.qt.io/download-qt-installer) and install
at least **Qt 6.8.x > MSVC 2022 64-bit** (the base kit already contains every
module the backend needs — no Quick, QML or Widgets components required).

> Ninja (via the separate installer or `choco`/`winget`) must be in `PATH`
> for the `ninja-*` presets. If you install Ninja separately, put it in `PATH`.

## How CMake finds Qt

Qt is located in this order of priority:

1. `-DQT_ROOT=<path>` on the command line / preset cache variable
2. the environment variable `QT_ROOT`
3. an explicit `-DCMAKE_PREFIX_PATH=<path>`
4. automatic detection of the newest version under `C:/Qt`

Typical Qt install paths:

| Kit                  | Path                                      |
|----------------------|-------------------------------------------|
| MSVC 2022 64-bit     | `C:\Qt\6.8.0\msvc2022_64`                 |
| MinGW 64-bit         | `C:\Qt\6.8.0\mingw_64`                    |
| Ninja / Clang 64-bit | `C:\Qt\6.8.0\clang_64`                    |

## Build presets

Configure presets are defined in `CMakePresets.json`:

| Preset               | Generator / build type               |
|----------------------|--------------------------------------|
| `vs2022-debug`       | Visual Studio 17 2022, Debug         |
| `vs2022-release`     | Visual Studio 17 2022, Release       |
| `ninja-debug`        | Ninja, Debug                         |
| `ninja-release`      | Ninja, Release                       |
| `mingw-debug`        | MinGW Makefiles, Debug               |
| `mingw-release`      | MinGW Makefiles, Release             |

Override the Qt location locally (not committed) by editing
`CMakeUserPresets.json`.

### Command line (recommended)

```powershell
# from the repository root
cmake --preset vs2022-release
cmake --build --preset vs2022-release
ctest --preset vs2022-release
```

### Visual Studio

1. Open the folder (File > Open > Folder).
2. Visual Studio detects the CMake presets automatically.
3. Pick `vs2022-release` from the configuration dropdown and build (Ctrl+Shift+B).

### Visual Studio Code

Requires the **CMake Tools** extension.

1. Open the folder.
2. Run `CMake: Select Configure Preset` from the command palette.
3. Run `CMake: Build`.

For Ninja support with CMake Tools, add the Ninja directory to `PATH` and set
`cmake.generator` in settings if needed.

## Debug vs Release

- **Release** — optimized, no debug symbols, used for distribution.
- **Debug** — assertions, symbols, slower but easier to debug.

For single-config generators (`Ninja`, `MinGW Makefiles`) the build type is
part of the preset (`Debug` / `Release`). For Visual Studio the configuration
is selected at build time (`--config Release` / `--config Debug`).

## Running

`dns-core.exe` is headless (NDJSON over stdio) — you run it through the
Electron UI, which spawns and supervises it automatically:

```powershell
# 1. Build the backend once
cmake --preset vs2022-release
cmake --build --preset vs2022-release --target dns-core

# 2. Run the desktop app (dev, with hot reload)
cd electron-app
npm install
npm run dev

# or launch the packaged portable build:
.\electron-app\release\DNSManager-2.0.0-windows-x64-portable.exe
```

> Changing the DNS configuration of an adapter requires administrator
> privileges. The backend requests elevation via UAC only for those
> operations and explains which operations need it.

## Running tests

```powershell
cmake --preset vs2022-release
cmake --build --preset vs2022-release
ctest --preset vs2022-release --output-on-failure
```

Tests that require modifying the machine's DNS configuration are **never run
automatically** — only non-privileged unit tests run in CI.

### Continuous integration

GitHub Actions (`.github/workflows/ci.yml`) builds the `vs2022-debug` and
`vs2022-release` presets on `windows-2022` with Qt 6.8 and runs the full
`ctest` suite on every push/PR to `main`. Tag pushes (`v*`, matching
`electron-app/package.json` and `CMakeLists.txt`) trigger
`.github/workflows/release.yml`, which builds the backend, runs the C++ tests,
packages portable + NSIS + Inno Setup installers for x64 and x86, and attaches
them to the GitHub Release with notes rendered from `CHANGELOG.md`.

## Common errors

| Symptom                                | Cause / fix                                             |
|----------------------------------------|---------------------------------------------------------|
| `Could not find a package configuration file provided by "Qt6"` | Qt not found. Set `QT_ROOT` or `CMAKE_PREFIX_PATH`. |
| `CMake was unable to find a build program corresponding to "Ninja"` | Ninja not in `PATH`. Install Ninja or use the `vs2022-*` presets. |
| `CMAKE_CXX_COMPILER not set, after EnableLanguage` | No C++ toolchain. Run from a Visual Studio Developer shell, or install Build Tools / MinGW. |
| `error LNK2038: mismatch detected for 'RuntimeLibrary'` | Mixing Debug and Release Qt/compiler settings. Keep one configuration consistently. |
| `unrecognized option ... clang-format` | Code style is enforced by `.clang-format`; run `clang-format -i` on changed files. |

## Packaging

Deployment is covered in `docs/DEPLOY.md`. Quick start:

```powershell
cd electron-app
npm run dist:portable   # portable EXE with the dns-core sidecar bundled
```
