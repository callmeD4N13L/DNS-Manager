# ---------------------------------------------------------------------------
# DnsManager - modern Windows DNS manager
# Target: Windows 10/11, C++20
#
# Architecture: the ONLY user interface is the Electron + React app in
# electron-app/. The C++ side is a headless JSON-RPC sidecar (dns-core)
# built here from src/core, src/models, src/platform and src/cli. Qt is
# used purely as a C++ utility library (Core/Network/Concurrent: event
# loop, settings, UDP sockets, threads) — there are no Qt Widgets, no
# Qt Quick, no QML and no Qt GUI of any kind.
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.25)

project(DnsManager
    VERSION 2.0.0
    DESCRIPTION "Modern Windows DNS manager (C++ backend + Electron/React UI)"
    HOMEPAGE_URL "https://github.com/callmeD4N13L/DNS-Manager"
    LANGUAGES CXX
)

# --- Compiler / language settings ----------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# --- Qt discovery (C++ utility library for the backend only) ---------------
# Qt can be located in three ways (first match wins):
#   1. The cache variable QT_ROOT (e.g. -DQT_ROOT=C:/Qt/6.8.0/msvc2022_64)
#   2. The environment variable QT_ROOT
#   3. CMAKE_PREFIX_PATH explicitly set by the user / preset
#   4. Auto-detection under C:/Qt (latest installed version)
# Only the Core/Network/Concurrent modules are required — no Gui, Widgets,
# Quick, Qml or QuickControls2.

if(NOT DEFINED QT_ROOT AND DEFINED ENV{QT_ROOT})
    set(QT_ROOT "$ENV{QT_ROOT}")
endif()

if(DEFINED QT_ROOT AND NOT QT_ROOT STREQUAL "")
    list(PREPEND CMAKE_PREFIX_PATH "${QT_ROOT}")
endif()

if(NOT CMAKE_PREFIX_PATH AND WIN32 AND EXISTS "C:/Qt")
    file(GLOB qt_roots "C:/Qt/[0-9]*")
    if(qt_roots)
        list(SORT qt_roots COMPARE NATURAL ORDER DESCENDING)
        list(GET qt_roots 0 qt_latest)
        foreach(qt_subdir msvc2022_64 mingw_64 clang_64)
            set(qt_candidate "${qt_latest}/${qt_subdir}")
            if(EXISTS "${qt_candidate}")
                list(APPEND CMAKE_PREFIX_PATH "${qt_candidate}")
                message(STATUS "Auto-detected Qt at: ${qt_candidate}")
                break()
            endif()
        endforeach()
    endif()
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Network
    Concurrent
)

# Enables AUTOMOC for the Q_OBJECT backend classes (no AUTOUIC/AUTORCC:
# there are no widgets, forms or Qt resources anymore).
set(CMAKE_AUTOMOC ON)

# --- Options --------------------------------------------------------------
option(DNSMGR_BUILD_TESTS "Build DnsManager tests" ON)
# Lightweight modular binary: LTO + size/link-time opts for the Release
# dns-core sidecar. The app is NOT a standalone monolith — the Electron UI
# and this small sidecar ship as separate modules.
option(DNSMGR_ENABLE_LTO "Enable link-time optimization for Release" ON)

if(DNSMGR_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _ipo_ok OUTPUT _ipo_msg)
    if(_ipo_ok)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    endif()
endif()

if(MSVC)
    # Favor small code, fold identical functions, drop unreferenced chunks.
    add_compile_options($<$<CONFIG:Release>:/Os> $<$<CONFIG:Release>:/Gy> $<$<CONFIG:Release>:/Gw>)
    add_link_options($<$<CONFIG:Release>:/OPT:REF> $<$<CONFIG:Release>:/OPT:ICF>)
elseif(MINGW)
    add_compile_options($<$<CONFIG:Release>:-Os> $<$<CONFIG:Release>:-ffunction-sections> $<$<CONFIG:Release>:-fdata-sections>)
    add_link_options($<$<CONFIG:Release>:-Wl,--gc-sections> $<$<CONFIG:Release>:-s>)
endif()

# --- dns-core: headless JSON-RPC sidecar for the Electron/React UI --------
# Reuses the backend sources (core/, models/, platform/) with no business-
# logic duplication. Speaks NDJSON over stdio; see src/cli/dnscore_main.cpp.
# This is a console application; Electron spawns it hidden (windowsHide).
add_executable(dns-core
    src/cli/dnscore_main.cpp
    src/core/OperationResult.cpp
    src/core/Logging.cpp
    src/core/CrashHandler.cpp
    src/core/NetworkManager.cpp
    src/core/ProfileManager.cpp
    src/core/ProfileStorage.cpp
    src/core/DnsLatencyTester.cpp
    src/core/AppSettings.cpp
    src/models/DnsConfiguration.cpp
    src/models/DnsProfile.cpp
    src/models/NetworkAdapter.cpp
    src/platform/WindowsDns.cpp
)

target_include_directories(dns-core PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Single source of truth for the backend version: CMake PROJECT_VERSION is
# injected as DNSMGR_VERSION so the --version flag, the JSON-RPC "meta" block
# and the Windows VERSIONINFO resource can never drift apart.
target_compile_definitions(dns-core PRIVATE
    DNSMGR_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(dns-core PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Concurrent
)

if(WIN32)
    # Target Windows 10 1809+ so GetInterfaceDnsSettings / SetInterfaceDnsSettings
    # (DNS_INTERFACE_SETTINGS) are available in the SDK headers.
    target_compile_definitions(dns-core PRIVATE
        _WIN32_WINNT=0x0A00
        NTDDI_VERSION=0x0A000005
    )
    # Windows IP Helper API (iphlpapi) + Winsock (ws2_32) for adapter and
    # DNS operations; shell32 for launching the elevated helper (runas);
    # dnsapi for the resolver-cache flush; dbghelp for crash minidumps.
    target_link_libraries(dns-core PRIVATE iphlpapi ws2_32 shell32 dnsapi dbghelp)
    # Windows file-version metadata (visible in Explorer > Properties > Details).
    target_sources(dns-core PRIVATE resources/win/dns-core.rc)
endif()

# --- Compiler warnings ----------------------------------------------------
include(cmake/Warnings.cmake)
dnsmgr_enable_warnings(dns-core)

# --- Install (consumed by electron-builder via extraResources) -------------
install(TARGETS dns-core
    RUNTIME DESTINATION .
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md" DESTINATION .)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" DESTINATION .)
endif()

# --- Tests ----------------------------------------------------------------
if(DNSMGR_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "DnsManager ${PROJECT_VERSION} | Qt ${Qt6_VERSION} (Core backend) | C++${CMAKE_CXX_STANDARD}")
