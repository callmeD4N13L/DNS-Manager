# Changelog

All notable changes to DNS Manager are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project
adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added

- Custom frameless title bar (`TitleBar.tsx`): drag region with minimize,
  maximize/restore and close buttons, maximized-state sync from the main
  process.
- Close-to-tray dialog: closing the window offers "Minimize to tray" (keeps
  running in the background) or "Quit app".
- Tray profile menu: right-click the tray icon to apply any saved profile,
  add a profile, or flush the DNS cache; single click re-opens the window.

### Fixed

- Light theme: full light palette (`:root.light` overrides for every
  surface/ink/shadow var) plus system-theme detection via
  `prefers-color-scheme`.
- Removed the last `QtQml` include / `QML_ANONYMOUS` / `Q_INVOKABLE`
  remnants from the backend headers; `.editorconfig` / `.gitignore` no longer
  reference QML artifacts.

### Changed

- Leaner, modular packaging: asar bundle + separate `bin/dns-core.exe`
  sidecar, `maximum` compression, pruned file list, LTO + size-optimized
  Release backend, per-arch `dist:x64` / `dist:x86` scripts and x86 CMake
  presets.
- Smoother UI: global smooth scrolling with themed slim scrollbars,
  card lift on hover, staggered entrances, animated boot screen and page
  transitions.

## [2.0.0] — 2026-09-28

Major release: the Qt/QML desktop interface is replaced by an
**Electron + Chromium + React + TypeScript** UI over the same proven C++ backend.

### Added

- New Electron/Chromium shell (`electron-app/`): hardened main/preload bridge
  (`contextIsolation`, `sandbox`, IPC allowlist), system tray, single-instance
  lock, native import/export dialogs.
- React 19 UI with dashboard, profiles, network and settings pages, command
  palette, latency gauge and animated stat cards.
- `dns-core.exe` headless JSON-RPC sidecar (NDJSON over stdio) reusing the
  existing `src/core`, `src/models` and `src/platform` sources with no logic
  duplication.
- Distribution for Windows x64 and x86: portable EXEs, electron-builder NSIS
  installers and **Inno Setup installers**
  (`installer/inno/DNSManager-x64.iss`, `DNSManager-x86.iss`) — all attached
  to the GitHub Release.
- Windows `VERSIONINFO` resource for `dns-core.exe` (`resources/win/dns-core.rc`)
  and a single CMake-driven version source (`DNSMGR_VERSION`).
- Protocol smoke test (`electron-app/scripts/smoke-dnscore.mjs`, 19 assertions)
  running in CI without touching system DNS.

### Changed

- Qt is now a **backend-only utility library** (Core/Network/Concurrent: event
  loop, UDP probes, threads). No Qt Widgets, Qt Quick, QML or `windeployqt`.
- CMake target renamed from the Qt GUI app to the `dns-core` sidecar;
  `electron-builder` bundles it under `resources/bin/dns-core.exe`.
- `CMakePresets.json` gains Visual Studio 2026 presets alongside 2022/Ninja/MinGW.
- Documentation rewritten around the Electron architecture (`docs/BUILD.md`,
  `docs/DEPLOY.md`, `electron-app/README.md`).

### Fixed

- Elevated DNS apply/reset still goes through the standard Windows UAC prompt
  via the `--apply-dns` helper — never silently, never via registry hacks.
- CTest runs without admin rights and never mutates host DNS or the registry.
- Qt runtime DLL resolution for tests on clean CI runners (`PATH` injection in
  `tests/CMakeLists.txt`).
- Dev-time `dns-core` discovery across `ninja-release` / `vs2022-release` /
  `vs2026-release` build trees.

### Removed

- The entire `qml/` tree, `src/ui/` Qt models, `src/main.cpp`, Qt resource
  files (`resources.qrc`, `version.rc.in`) and the legacy `scripts/package.ps1`
  CPack flow.

## [1.0.0] — 2026-08-14

- Initial public release: Qt 6 / QML desktop app with adapter detection, DNS
  read/apply, DHCP reset, cache flush, profiles with import/export, latency
  benchmark, settings, logging and crash minidumps.

[2.0.0]: https://github.com/callmeD4N13L/DNS-Manager/releases/tag/v2.0.0
[1.0.0]: https://github.com/callmeD4N13L/DNS-Manager/releases/tag/v1.0.0
