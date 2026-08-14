# ---------------------------------------------------------------------------
# DnsManager tests - Qt Test binaries.
#
# Each test compiles the production sources it exercises directly, so the
# suite runs on any machine (including CI) without touching system DNS.
# ---------------------------------------------------------------------------

find_package(Qt6 REQUIRED COMPONENTS Test)

set(DNSMGR_TEST_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(DNSMGR_TEST_LIBS Qt6::Test Qt6::Core Qt6::Network)

function(dnsmgr_add_test name)
    qt_add_executable(${name} ${ARGN})
    target_include_directories(${name} PRIVATE ${DNSMGR_TEST_INCLUDES})
    target_link_libraries(${name} PRIVATE ${DNSMGR_TEST_LIBS})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

dnsmgr_add_test(TestDnsConfiguration
    TestDnsConfiguration.cpp
    ../src/models/DnsConfiguration.cpp
)

dnsmgr_add_test(TestDnsProfile
    TestDnsProfile.cpp
    ../src/models/DnsConfiguration.cpp
    ../src/models/DnsProfile.cpp
)

dnsmgr_add_test(TestProfileStorage
    TestProfileStorage.cpp
    ../src/core/OperationResult.cpp
    ../src/core/ProfileStorage.cpp
    ../src/models/DnsConfiguration.cpp
    ../src/models/DnsProfile.cpp
)

dnsmgr_add_test(TestProfileManager
    TestProfileManager.cpp
    ../src/core/OperationResult.cpp
    ../src/core/ProfileManager.cpp
    ../src/core/ProfileStorage.cpp
    ../src/models/DnsConfiguration.cpp
    ../src/models/DnsProfile.cpp
)

dnsmgr_add_test(TestAppSettings
    TestAppSettings.cpp
    ../src/core/AppSettings.cpp
)

dnsmgr_add_test(TestNetworkAdapter
    TestNetworkAdapter.cpp
    ../src/models/NetworkAdapter.cpp
)

dnsmgr_add_test(TestLogging
    TestLogging.cpp
    ../src/core/Logging.cpp
)
