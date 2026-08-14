# ---------------------------------------------------------------------------
# Compiler warning configuration.
#
# Usage:
#   include(cmake/Warnings.cmake)
#   dnsmgr_enable_warnings(<target>)
# ---------------------------------------------------------------------------

function(dnsmgr_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4
            /permissive-
            /utf-8
            /Zc:preprocessor
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
        )
    endif()
endfunction()
