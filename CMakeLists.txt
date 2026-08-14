# ---------------------------------------------------------------------------
# DnsManager - modern Windows DNS manager
# Target: Windows 10/11, Qt 6, C++20, QML
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.25)

project(DnsManager
    VERSION 1.0.0
    DESCRIPTION "Modern Windows DNS manager"
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

# --- Qt discovery --------------------------------------------------------
# Qt can be located in three ways (first match wins):
#   1. The cache variable QT_ROOT (e.g. -DQT_ROOT=C:/Qt/6.8.0/msvc2022_64)
#   2. The environment variable QT_ROOT
#   3. CMAKE_PREFIX_PATH explicitly set by the user / preset
#   4. Auto-detection under C:/Qt (latest installed version)

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
    Gui
    Network
    Quick
    Qml
    QuickControls2
    Concurrent
    Widgets
)

# Enables AUTOMOC/AUTOUIC/AUTORCC and standard Qt project defaults.
qt_standard_project_setup()

# --- Options --------------------------------------------------------------
option(DNSMGR_BUILD_TESTS "Build DnsManager tests" ON)
option(DNSMGR_BUILD_NSIS "Build an NSIS installer with CPack (requires NSIS)" OFF)

# --- Windows version resource --------------------------------------------
# Generate version.rc from a template so FILEVERSION always matches the
# project version instead of being hard-coded.
if(WIN32)
    string(REPLACE "." ";" _dnsmgr_version_parts "${PROJECT_VERSION}")
    list(LENGTH _dnsmgr_version_parts _dnsmgr_version_count)
    foreach(_idx RANGE 0 3)
        if(_idx LESS _dnsmgr_version_count)
            list(GET _dnsmgr_version_parts ${_idx} _dnsmgr_part)
        else()
            set(_dnsmgr_part 0)
        endif()
        if(NOT _dnsmgr_part MATCHES "^[0-9]+$")
            set(_dnsmgr_part 0)
        endif()
        if(_idx EQUAL 0)
            set(DNSMGR_VERSION_MAJOR "${_dnsmgr_part}")
        elseif(_idx EQUAL 1)
            set(DNSMGR_VERSION_MINOR "${_dnsmgr_part}")
        elseif(_idx EQUAL 2)
            set(DNSMGR_VERSION_PATCH "${_dnsmgr_part}")
        else()
            set(DNSMGR_VERSION_BUILD "${_dnsmgr_part}")
        endif()
    endforeach()
    set(DNSMGR_APP_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/win/app.ico")

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/win/version.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/resources/win/version.rc"
        @ONLY
    )
endif()

# --- Application target ---------------------------------------------------
qt_add_executable(DnsManager WIN32
    src/main.cpp
)

# The QML module bundles all .qml files into the executable and generates
# the qmldir needed by QQmlApplicationEngine::loadFromModule(). C++ types
# declared with QML_ELEMENT are registered for QML via SOURCES.
qt_add_qml_module(DnsManager
    URI DnsManager
    VERSION 1.0
    SOURCES
        src/ui/Backend.cpp
        src/ui/ProfileListModel.cpp
        src/ui/AdapterListModel.cpp
    QML_FILES
        qml/Main.qml
        qml/theme/Theme.qml
        qml/components/Icon.qml
        qml/components/Card.qml
        qml/components/PrimaryButton.qml
        qml/components/SecondaryButton.qml
        qml/components/IconButton.qml
        qml/components/StatusCard.qml
        qml/components/DnsCard.qml
        qml/components/PageHeader.qml
        qml/components/SidebarItem.qml
        qml/components/Sidebar.qml
        qml/components/FormField.qml
        qml/components/ProfileDialog.qml
        qml/components/ConfirmDialog.qml
        qml/components/ToggleSwitch.qml
        qml/pages/Dashboard.qml
        qml/pages/Profiles.qml
        qml/pages/Network.qml
        qml/pages/Settings.qml
)

# Plain value models and non-QML services are ordinary target sources.
target_sources(DnsManager PRIVATE
    src/core/OperationResult.cpp
    src/core/Logging.cpp
    src/core/CrashHandler.cpp
    src/core/NetworkManager.cpp
    src/core/ProfileManager.cpp
    src/core/ProfileStorage.cpp
    src/core/DnsLatencyTester.cpp
    src/core/AppSettings.cpp
    src/ui/TrayController.cpp
    src/models/DnsConfiguration.cpp
    src/models/DnsProfile.cpp
    src/models/NetworkAdapter.cpp
    src/platform/WindowsDns.cpp
)

target_include_directories(DnsManager PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(DnsManager PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickControls2
    Qt6::Concurrent
    Qt6::Widgets
)

if(WIN32)
    # Target Windows 10 1809+ so GetInterfaceDnsSettings / SetInterfaceDnsSettings
    # (DNS_INTERFACE_SETTINGS) are available in the SDK headers.
    target_compile_definitions(DnsManager PRIVATE
        _WIN32_WINNT=0x0A00
        NTDDI_VERSION=0x0A000005
    )
    # Windows IP Helper API (iphlpapi) + Winsock (ws2_32) for adapter and
    # DNS operations; shell32 for launching the elevated helper (runas);
    # dnsapi for the resolver-cache flush; dbghelp for crash minidumps.
    target_link_libraries(DnsManager PRIVATE iphlpapi ws2_32 shell32 dnsapi dbghelp)
    target_sources(DnsManager PRIVATE
        resources/resources.qrc
        "${CMAKE_CURRENT_BINARY_DIR}/resources/win/version.rc"
    )
endif()

# --- Compiler warnings ----------------------------------------------------
include(cmake/Warnings.cmake)
dnsmgr_enable_warnings(DnsManager)

# --- Install / packaging --------------------------------------------------
install(TARGETS DnsManager
    RUNTIME DESTINATION .
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md" DESTINATION .)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" DESTINATION .)
endif()

# Deploy the Qt runtime (DLLs + plugins + QML) next to the installed
# executable via windeployqt at install time (Qt 6.3+). This is what makes a
# "portable" install that runs on machines without Qt.
if(WIN32)
    qt_generate_deploy_app_script(
        TARGET DnsManager
        OUTPUT_SCRIPT dnsmgr_deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${dnsmgr_deploy_script})
endif()

# --- CPack (portable ZIP / optional NSIS installer) ------------------------
set(CPACK_PACKAGE_NAME "DnsManager")
set(CPACK_PACKAGE_VENDOR "DnsManager")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern Windows DNS manager")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/callmeD4N13L/DNS-Manager")
set(CPACK_PACKAGE_CONTACT "https://github.com/callmeD4N13L/DNS-Manager")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "DnsManager")
set(CPACK_PACKAGING_INSTALL_PREFIX "DnsManager")
set(CPACK_PACKAGE_FILE_NAME "DnsManager-${PROJECT_VERSION}-windows-x64")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/win/app.ico")
set(CPACK_GENERATOR "ZIP")

if(DNSMGR_BUILD_NSIS)
    list(APPEND CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "DNS Manager")
    set(CPACK_NSIS_PACKAGE_NAME "DNS Manager ${PROJECT_VERSION}")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/win/app.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "DnsManager.exe")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/callmeD4N13L/DNS-Manager")
    set(CPACK_NSIS_CONTACT "https://github.com/callmeD4N13L/DNS-Manager")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
        set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    endif()
endif()

include(CPack)

# --- Tests ----------------------------------------------------------------
if(DNSMGR_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()

# --- CMake version compatibility guard ------------------------------------
# qt_add_executable / qt_standard_project_setup require a recent-enough
# CMake + Qt combination; this is enforced by find_package above.
message(STATUS "DnsManager ${PROJECT_VERSION} | Qt ${QT_VERSION} | C++${CMAKE_CXX_STANDARD}")
