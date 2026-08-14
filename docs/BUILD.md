# Building DnsManager

This document explains how to configure, build, run and test DnsManager on
Windows 10/11 using Visual Studio, Visual Studio Code, Qt Creator or the
command line.

## Prerequisites

| Tool            | Version / Notes                                     |
|-----------------|-----------------------------------------------------|
| Windows         | 10 (1809+) or 11                                    |
| CMake           | 3.25+                                               |
| Compiler        | MSVC 2022 (v143) or MinGW-w64 (GCC 12+)             |
| Qt              | 6.5+ with `Qt Quick`, `Qt QML`, `Qt Quick Controls 2` |
| Ninja (optional) | Recommended for fast single-config builds          |
| Git             | 2.30+                                               |

### Installing Qt

Use the [Qt Online Installer](https://www.qt.io/download-qt-installer) and install
at least these components for **Qt 6.8** (or newer):

- `Qt 6.8.x > MSVC 2022 64-bit`
- `Qt 6.8.x > Additional Libraries > Qt Quick`
- `Qt 6.8.x > Additional Libraries > Qt Quick Controls 2`
- `Qt 6.8.x > Additional Libraries > Qt QML`

> The development tools (Qt Creator, MinGW, Ninja, CMake, Debugging Tools) are
> optional but convenient. If you install Ninja separately, put it in `PATH`.

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

### Qt Creator

1. Tools > Options > Kits, create or select a **Desktop Qt 6.8.x** kit.
2. Open the top-level `CMakeLists.txt` as a project.
3. Qt Creator reads `CMakePresets.json` — choose a preset in *Projects > Build*.

## Debug vs Release

- **Release** — optimized, no debug symbols, used for distribution.
- **Debug** — assertions, symbols, slower but easier to debug.

For single-config generators (`Ninja`, `MinGW Makefiles`) the build type is
part of the preset (`Debug` / `Release`). For Visual Studio the configuration
is selected at build time (`--config Release` / `--config Debug`).

## Running

```powershell
# Visual Studio
.\build\vs2022-release\Release\DnsManager.exe

# Ninja / MinGW
.\build\ninja-release\DnsManager.exe
```

> Changing the DNS configuration of an adapter requires administrator
> privileges. The application will request elevation via UAC only for those
> operations and explain which operations need it.

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
`vs2022-release` presets on `windows-latest` with Qt 6.8 and runs the full
`ctest` suite on every push/PR to `main`. Tag pushes (`v*`) trigger
`.github/workflows/release.yml`, which packages a portable ZIP and attaches it
to the GitHub Release.

## Common errors

| Symptom                                | Cause / fix                                             |
|----------------------------------------|---------------------------------------------------------|
| `Could not find a package configuration file provided by "Qt6"` | Qt not found. Set `QT_ROOT` or `CMAKE_PREFIX_PATH`. |
| `Qt6 found but the target Qt6::QuickControls2 doesn't exist` | Qt Quick Controls 2 component not installed. Re-run the Qt installer. |
| `CMake was unable to find a build program corresponding to "Ninja"` | Ninja not in `PATH`. Install Ninja or use the `vs2022-*` presets. |
| `CMAKE_CXX_COMPILER not set, after EnableLanguage` | No C++ toolchain. Install Visual Studio Build Tools / MinGW. |
| `error LNK2038: mismatch detected for 'RuntimeLibrary'` | Mixing Debug and Release Qt/compiler settings. Keep one configuration consistently. |
| `unrecognized option ... clang-format` | Code style is enforced by `.clang-format`; run `clang-format -i` on changed files. |

## Packaging

Deployment is covered in Phase 16 — see `docs/DEPLOY.md`. Quick start:

```powershell
.\scripts\package.ps1            # build + windeployqt + portable ZIP
```
