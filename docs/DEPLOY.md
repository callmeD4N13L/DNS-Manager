# Deploying & Packaging Dancy

The shipped product is the **Electron desktop app** (`electron-app/`) with the
C++ `dns-core.exe` sidecar bundled inside it. There is no Qt UI to deploy and
no `windeployqt` step — `electron-builder` produces a self-contained
**portable EXE** (and optionally an **NSIS installer**) that runs on any
Windows 10/11 machine without Qt, Node or admin rights (UAC is requested only
for actual DNS changes, by the backend).

> DnsManager changes system DNS settings. The app itself needs no
> administrator rights for the UI; it requests elevation (UAC) only for the
> actual DNS change / DHCP reset operations.

## Prerequisites

| Tool               | Why                                                     |
|--------------------|----------------------------------------------------------|
| CMake 3.25+        | Builds `dns-core.exe`                                    |
| MSVC 2022 or MinGW | Compiler for the backend                                 |
| Qt 6.5+ (base kit) | C++ utility library for the backend (Core/Network only)  |
| Node.js 20+ / npm  | Builds and packages `electron-app/`                      |

## One-command portable build (recommended)

From the repository root:

```powershell
# 1. Backend
cmake --preset vs2022-release
cmake --build --preset vs2022-release --target dns-core

# 2. UI + packaging
cd electron-app
npm install
npm run dist:portable
```

Output: `electron-app/release/DNSManager-<version>-windows-x64-portable.exe`
(~82 MB, sidecar included under `resources/bin/dns-core.exe`).

For an NSIS installer instead (or as well):

```powershell
cd electron-app
npm run dist
```

Behavior is configured in `electron-app/electron-builder.yml`:
`extraResources` bundles the freshly built `../build/<preset>/dns-core.exe`
and `resources/win/app.ico` (tray + installer icon). Both `x64` and `ia32`
targets are built; on 64-bit Windows the 32-bit shell spawns the same
`dns-core.exe` sidecar via standard process creation (WOW64), so both
installers work on every supported machine (Windows 10/11 are 64-bit only
from Windows 11 onward).

## Inno Setup installers (x64 + x86)

Classic wizard-style installers live in `installer/inno/`:

| Script | Output |
| ------ | ------ |
| `DNSManager-x64.iss` | `installer/output/DNSManager-<version>-windows-x64-setup.exe` |
| `DNSManager-x86.iss` | `installer/output/DNSManager-<version>-windows-x86-setup.exe` |

They wrap the `electron-builder` unpacked trees
(`electron-app/release/win-unpacked`, `win-ia32-unpacked`) and ship a
`SUPPORT.txt` donation card next to the app. Build them locally with
[Inno Setup 6](https://jrsoftware.org/isinfo.php) installed:

```powershell
iscc installer/inno/DNSManager-x64.iss /DAppVersion=2.0.0
iscc installer/inno/DNSManager-x86.iss /DAppVersion=2.0.0
```

CI compiles both scripts automatically on every `v*` tag.

## What gets shipped

| Artifact inside the package | Source                              |
|-----------------------------|-------------------------------------|
| Electron + Chromium runtime | `electron` npm package              |
| Built React UI              | `electron-app/renderer/dist` (vite) |
| Main/preload (compiled)     | `electron-app/electron/dist` (tsc)  |
| `bin/dns-core.exe`          | CMake `dns-core` target             |
| `app.ico`                   | `resources/win/app.ico`             |

## Smoke-testing a package

```powershell
cd electron-app
npm run smoke:bridge   # 19 assertions against the real sidecar protocol
npx electron . --smoke-test   # launches the app, exits 0 once live data paints
```

## CI / releases

Tag pushes (`v*`) trigger `.github/workflows/release.yml`, which builds the
backend, runs the C++ tests, builds the portable EXE via electron-builder and
attaches it to the GitHub Release.
