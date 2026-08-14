# DnsManager

[![CI](https://github.com/DnsManager/DnsManager/actions/workflows/ci.yml/badge.svg)](https://github.com/DnsManager/DnsManager/actions/workflows/ci.yml)

A modern, fast, lightweight **Windows DNS Manager** built with C++20, Qt 6,
QML and CMake for Windows 10/11.

## Features

- View current DNS configuration per network adapter
- Detect and enumerate network adapters (Ethernet, Wi-Fi, virtual, VPN)
- Change IPv4 / IPv6 DNS servers
- Save, edit, duplicate, favorite and search DNS profiles
- One-click profile switching
- Reset DNS settings back to DHCP
- Flush the Windows DNS cache
- Verified application + cache flush + read-back confirmation
- Optional DNS latency benchmark across providers
- Dark / light / system theme
- System tray integration
- Profile import / export (JSON)
- Administrator elevation handled explicitly, never silently

## Technology Stack

| Layer    | Technology                                  |
| -------- | ------------------------------------------- |
| Language | C++20                                       |
| UI       | Qt 6, QML, Qt Quick, Qt Quick Controls 2    |
| Build    | CMake (>= 3.25)                             |
| Backend  | Windows networking APIs (`IPHelper`, Winsock, `SetInterfaceDnsSettings`) |
| Storage  | Local JSON (`dns_profiles.json`)            |

## Project Layout

```
DnsManager/
├── CMakeLists.txt            # Top-level build
├── cmake/                    # CMake helper modules
├── config/                   # Default profiles / app data
├── docs/                     # Design & deployment notes
├── resources/                # Icons, fonts, qrc
├── scripts/                  # Build / deploy / package scripts
├── .github/workflows/        # CI (build + tests) and release packaging
├── src/
│   ├── main.cpp
│   ├── core/                 # DnsManager, NetworkManager
│   ├── models/               # DnsProfile, NetworkAdapter, DnsConfiguration
│   ├── platform/             # WindowsDns (API bindings)
│   ├── storage/              # ProfileManager (JSON)
│   └── ui/                   # QML-facing controllers
├── qml/
│   ├── Main.qml
│   ├── pages/                # Dashboard, Profiles, Network, Settings
│   ├── components/           # Sidebar, Cards, Buttons...
│   └── theme/                # Theme.qml
└── tests/                    # Unit tests (no admin privileges required)
```

## Requirements

- Windows 10 or 11
- Qt 6.5+ (Qt Quick + Qt QML modules, MSVC or MinGW kit)
- CMake 3.25+
- C++20 compiler (MSVC 2022 or MinGW 12+)

## Building

See [docs/BUILD.md](docs/BUILD.md) for detailed instructions for Visual
Studio, VS Code, Qt Creator and the command line.

```powershell
cmake --preset vs2022-release
cmake --build --preset vs2022-release
```

Qt is auto-detected under `C:/Qt`; override with `-DQT_ROOT=<path>` or edit
`CMakeUserPresets.json`.

## Testing & Continuous Integration

Unit tests are pure logic (no admin rights, never modify DNS). Run them locally:

```powershell
cmake --preset vs2022-release
cmake --build --preset vs2022-release
ctest --preset vs2022-release --output-on-failure
```

GitHub Actions runs on every push/PR to `main` (`.github/workflows/ci.yml`):
install Qt 6.8 (MSVC 2022), build the `vs2022-debug`/`vs2022-release` presets
and execute the full test suite on `windows-latest`.

Releases are automated from tags (`v*`) via `.github/workflows/release.yml`,
which builds, runs tests, packages a portable ZIP (windeployqt + QML) and
uploads it to the GitHub Release.

## Roadmap

The project is built incrementally:

- [x] 1. Project initialization
- [x] 2. CMake configuration
- [x] 3. Basic Qt/QML window
- [x] 4. Modern UI layout
- [x] 5. C++ <-> QML communication
- [x] 6. Network adapter detection
- [x] 7. Read current DNS
- [x] 8. DNS profile system
- [x] 9. Change DNS
- [x] 10. Reset DHCP
- [x] 11. DNS cache flush
- [x] 12. DNS testing / latency
- [x] 13. Settings and system tray
- [x] 14. Error handling and polish
- [x] 15. Testing
- [x] 16. Release packaging
- [x] 17. GitHub Actions CI/CD

## License

MIT — see [LICENSE](LICENSE).
