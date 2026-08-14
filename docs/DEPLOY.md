# Deploying & Packaging DnsManager

This document describes how to turn a build into something other people can
run: a self-contained **portable ZIP** or an **NSIS installer**. Both ship the
Qt runtime (DLLs, plugins, QML) alongside the executable, so the result runs
on any Windows 10/11 machine without installing Qt.

> DnsManager changes system DNS settings. The app itself needs no
> administrator rights for the UI; it requests elevation (UAC) only for the
> actual DNS change / DHCP reset operations.

## Prerequisites

| Tool            | Why                                                        |
|-----------------|-------------------------------------------------------------|
| CMake 3.25+     | Configure/build                                             |
| MSVC 2022 or MinGW | Compiler                                                |
| Qt 6.5+         | Framework (used by windeployqt)                             |
| windeployqt     | Deploys the Qt runtime; ships with every Qt kit             |
| NSIS (optional) | Only needed for the NSIS installer (`cpack`)                |

`windeployqt` is found automatically on `PATH`, via `$env:QT_ROOT`, or by
scanning the newest kit under `C:\Qt`.

## Option A — one-command packaging script (recommended)

From the repository root:

```powershell
# Visual Studio 2022, Release, portable ZIP
.\scripts\package.ps1

# Ninja build
.\scripts\package.ps1 -Preset ninja-release

# ZIP + NSIS installer (requires NSIS + a -DDNSMGR_BUILD_NSIS=ON configure)
.\scripts\package.ps1 -Packager all

# Repackage an already-built configuration without rebuilding
.\scripts\package.ps1 -SkipBuild
```

What it does:

1. Builds the chosen preset (`vs2022-release` by default).
2. Stages `DnsManager.exe`, `README.md` and `LICENSE`.
3. Runs `windeployqt --release --qmldir <repo>\qml` so Qt DLLs, plugins and
   the QML modules land next to the executable.
4. Produces `dist\DnsManager-<version>-windows-x64.zip`.

## Option B — CPack

The CMake build already defines an install rule that runs the Qt deployment
script (`qt_generate_deploy_app_script`), so `cpack` produces a fully
self-contained artifact:

```powershell
cmake --preset vs2022-release
cmake --build --preset vs2022-release
cpack --config build\vs2022-release\CPackConfig.cmake -C Release
# -> build\vs2022-release\DnsManager-1.0.0-windows-x64.zip
```

For the NSIS installer, configure once with `-DDNSMGR_BUILD_NSIS=ON`
(sets `CPACK_GENERATOR` to `ZIP;NSIS`), then run `cpack` again.

## Option C — manual deployment

If you prefer to do it by hand:

```powershell
# stage the exe first
mkdir build\stage
copy build\vs2022-release\Release\DnsManager.exe build\stage\

# deploy the Qt runtime next to it
windeployqt --release --qmldir qml build\stage\DnsManager.exe

# pack
Compress-Archive -Path build\stage\* -DestinationPath DnsManager.zip
```

## What's inside the package

```
DnsManager\
  DnsManager.exe
  Qt6Core.dll / Qt6Gui.dll / Qt6Network.dll / ... (Qt runtime)
  platforms\qwindows.dll
  imageformats\...        (as needed)
  qml\QtQuick\...         (QML modules used by the UI)
  README.md
  LICENSE
```

The folder is portable: copy it anywhere and run `DnsManager.exe`.

## Code signing (optional but recommended)

Windows SmartScreen shows "unknown publisher" warnings for unsigned apps.
Sign the executable (or the installer) with a certificate, then verify:

```powershell
# sign the portable zip contents
signtool sign /fd SHA256 /a /tr http://timestamp.digicert.com `
    /td SHA256 /f path\to\cert.pfx /p <password> build\stage\DnsManager.exe

# verify
signtool verify /pa build\stage\DnsManager.exe
```

After signing, re-zip the staged folder. Signing the NSIS installer requires
signing the generated `.exe` after `cpack` finishes.

## MSIX

MSIX packaging (Microsoft Store / sideloading) is not wired up yet. It
requires an identity certificate, an AppX manifest, and the Windows SDK
`makeappx` + `signtool` toolchain. If you need MSIX, open an issue; a
`resources/msix/` manifest template is the natural starting point.

## Verifying a release

1. Unzip / install on a clean machine *without Qt installed*.
2. Launch `DnsManager.exe` — the window appears, tray icon works.
3. Check the app-about card shows the expected version.
4. Look for `%APPDATA%\DnsManager\logs\dnsmanager.log` after a session; it
   should contain startup INFO lines and no FATAL entries.

## Troubleshooting

| Symptom                              | Fix                                                        |
|--------------------------------------|------------------------------------------------------------|
| `windeployqt.exe not found`          | Install Qt, set `$env:QT_ROOT`, or add the kit `bin` dir to `PATH`. |
| Missing DLL at startup               | Re-run windeployqt; make sure the DLLs sit next to the exe. |
| QML module not found at runtime      | `--qmldir` must point at the repo `qml` folder.            |
| `cpack` reports "NSIS not found"     | Install NSIS or reconfigure with `-DDNSMGR_BUILD_NSIS=OFF`. |
| SmartScreen "Unknown publisher"      | Sign the binaries (see above).                             |
| Installed app writes to wrong logs   | Expected: logs live under `%APPDATA%\DnsManager\logs`.     |
