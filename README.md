# Windows DNS Manager

> Switch, benchmark and troubleshoot your Windows DNS configuration from a
> modern desktop UI — no registry hacking, no command line.

[![CI](https://github.com/callmeD4N13L/windows-dns-manager/actions/workflows/ci.yml/badge.svg)](https://github.com/callmeD4N13L/windows-dns-manager/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/callmeD4N13L/windows-dns-manager)](https://github.com/callmeD4N13L/windows-dns-manager/releases)
[![License](https://img.shields.io/github/license/callmeD4N13L/windows-dns-manager)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11-0078d6)](#requirements)
[![C++](https://img.shields.io/badge/C%2B%2B-20-00599c)](CMakeLists.txt)
[![Qt](https://img.shields.io/badge/Qt-6.5%2B-41cd52)](CMakeLists.txt)

A fast, lightweight **DNS manager for Windows 10/11** built with **C++20**,
**Qt 6** and **QML**. Inspect every adapter's DNS, switch between saved
profiles with one click, benchmark resolvers, flush the cache and reset back
to DHCP — with explicit, never-silent UAC elevation for the privileged bits.

---

## Features

**Adapters & insights**

- Enumerates Ethernet, Wi-Fi, virtual and VPN adapters with live status
- Reads current IPv4 / IPv6 DNS (static vs. DHCP) and resolver latency
- Detect connected / disconnected adapters at a glance

**DNS profiles**

- Save, edit, favorite and one-click apply named profiles
- Import / export profiles as portable JSON
- Seeded with popular resolvers (Cloudflare, Google, Quad9, AdGuard)

**Actions**

- Apply any profile to the selected adapter (with confirmation option)
- Reset DNS back to automatic (DHCP)
- Flush the Windows DNS resolver cache
- Benchmark the adapter's DNS servers and show the fastest latency

**Application**

- Dark / light / system theme, system tray integration
- Launch with Windows, start minimized
- Rotating application log and crash minidumps
- Administrator elevation requested explicitly (UAC), never silently

## Quick start

Download the latest portable ZIP from
[Releases](https://github.com/callmeD4N13L/windows-dns-manager/releases),
extract it anywhere and run `DnsManager.exe`.

Or build from source (Qt 6.5+ required — see
[docs/BUILD.md](docs/BUILD.md)):

```powershell
cmake --preset vs2022-release
cmake --build --preset vs2022-release
```

## Screenshots

Screenshots are welcome — add them under `docs/screenshots/` and link them
here via a pull request.

## Documentation

| Topic                 | Where                                        |
| --------------------- | -------------------------------------------- |
| Building & running    | [docs/BUILD.md](docs/BUILD.md)               |
| Packaging & deploying | [docs/DEPLOY.md](docs/DEPLOY.md)             |
| CI / GitHub Actions   | [.github/workflows](.github/workflows)       |

## Security & reliability

- DNS changes and DHCP resets are performed by an elevated helper launched
  through the standard Windows UAC prompt — never silently and never by
  `regedit`-style hacks.
- All system calls go through the documented Windows APIs
  (`Get/SetInterfaceDnsSettings`, `IP Helper`, Winsock, `dnsapi`).
- The test suite runs without admin rights and **never modifies the host's
  DNS** or registry, so it is safe in CI.
- Logs rotate automatically; crashes produce minidumps for diagnosis.

## Technology stack

| Layer     | Technology                                        |
| --------- | ------------------------------------------------- |
| Language  | C++20                                             |
| UI        | Qt 6, QML, Qt Quick, Qt Quick Controls 2          |
| Backend   | Windows networking APIs (`IP Helper`, Winsock, `SetInterfaceDnsSettings`) |
| Storage   | Local JSON (`dns_profiles.json`)                  |
| Build     | CMake 3.25+, presets for MSVC / Ninja / MinGW     |
| Packaging | CPack (ZIP / NSIS) + `windeployqt` deployment     |
| CI/CD     | GitHub Actions (build, test, release)             |

## Project layout

```
├── src/               C++ core, platform and QML-facing controllers
├── qml/               QML UI (pages, components, theme)
├── tests/             Qt Test suite (no admin privileges required)
├── resources/         Icons and Windows resources
├── cmake/             CMake helper modules
├── scripts/           Build / deploy / packaging scripts
└── docs/              Build and deployment guides
```

## Roadmap

Delivered incrementally in 17 phases — from project initialization through
UI, DNS operations, profiles, tray, logging, testing, packaging and CI.

- [x] Project initialization, CMake, Qt/QML window and UI
- [x] Adapter detection, read/change DNS, DHCP reset, cache flush
- [x] DNS profiles, favorites, import / export
- [x] Latency benchmark, settings, system tray
- [x] Error handling, logging, crash minidumps
- [x] Unit tests, release packaging, GitHub Actions CI/CD

## Contributing

Contributions are welcome! Open an issue for bugs and feature requests, or a
pull request for improvements. Follow the existing code style
(`.clang-format`) and keep the test suite green (`ctest`).

## License

[MIT](LICENSE) — DnsManager Contributors.
