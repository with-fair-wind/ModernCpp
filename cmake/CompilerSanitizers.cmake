# CompilerSanitizers.cmake
#
# Optional runtime sanitizers. Controlled by MCPP_ENABLE_SANITIZERS (default OFF).
# Exposes INTERFACE target `mcpp::sanitizers` — ModuleHelpers links every demo
# and test against it, so turning the option on is enough to sanitize everything.
#
# Only takes effect in Debug and RelWithDebInfo; Release/MinSizeRel stay clean
# (sanitizers defeat the point of those configs).
#
#   GCC / Clang (Unix-driver) : -fsanitize=address,undefined -fno-omit-frame-pointer
#                               (compile + link flags; linker autoselects the runtime)
#   MSVC / clang-cl           : /fsanitize=address
#                               UBSan is not supported on the MSVC ABI.
#                               ASan is incompatible with /RTC1, which the default
#                               Debug flags inject — we strip /RTC1 from the Debug
#                               flags of this build tree.
#
# Why clang-cl is grouped with MSVC, not Clang:
# clang-cl has CMAKE_CXX_COMPILER_ID == "Clang", but it is the MSVC-compatible
# driver — it accepts /MD/MDd, links against the MSVC CRT, and supports the
# /fsanitize=address spelling that links the MSVC ASan runtime. Routing it
# through the GNU/Clang branch produces "-MDd not allowed with -fsanitize=address"
# and unresolved __asan_*/__ubsan_* symbols at link time.

include_guard(GLOBAL)

add_library(mcpp_sanitizers INTERFACE)
add_library(mcpp::sanitizers ALIAS mcpp_sanitizers)

if(NOT MCPP_ENABLE_SANITIZERS)
    return()
endif()

set(_san_configs $<CONFIG:Debug,RelWithDebInfo>)

# MSVC ABI = cl.exe OR clang-cl. The MSVC variable is TRUE for both; that's
# what we key on so the GNU-driver Clang (linux/mac) still hits the Unix branch.
if(NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set(_san_flags
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all)
    target_compile_options(mcpp_sanitizers INTERFACE $<${_san_configs}:${_san_flags}>)
    target_link_options   (mcpp_sanitizers INTERFACE $<${_san_configs}:${_san_flags}>)
    message(STATUS "Sanitizers: AddressSanitizer + UBSan (Debug, RelWithDebInfo)")

elseif(MSVC)
    # /fsanitize=address conflicts with /RTC1; strip it from Debug flags.
    #
    # CAVEAT — global side effect, not scoped to mcpp::sanitizers:
    # this rewrites the cache variables CMAKE_*_FLAGS_{DEBUG,RELWITHDEBINFO},
    # which CMake injects into every C/C++ target in the build tree (not just
    # the ones that link mcpp::sanitizers). MSVC has no per-target way to
    # un-inject /RTC1, so this is the practical option, but be aware that
    # turning on MCPP_ENABLE_SANITIZERS under MSVC means the whole build
    # loses /RTC1 — including any third-party subdirectory you might add
    # later. If you need /RTC1 preserved on some targets, keep
    # MCPP_ENABLE_SANITIZERS OFF and instrument them with ASan separately.
    foreach(_flag_var
            CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(DEFINED ${_flag_var})
            string(REGEX REPLACE "[/-]RTC[1csu]+" "" ${_flag_var} "${${_flag_var}}")
            set(${_flag_var} "${${_flag_var}}" CACHE STRING "" FORCE)
        endif()
    endforeach()

    target_compile_options(mcpp_sanitizers INTERFACE $<${_san_configs}:/fsanitize=address>)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Sanitizers: AddressSanitizer (Debug, RelWithDebInfo) via clang-cl /fsanitize=address; UBSan unavailable on MSVC ABI")
    else()
        message(STATUS "Sanitizers: AddressSanitizer (Debug, RelWithDebInfo); UBSan unavailable on MSVC")
    endif()

else()
    message(WARNING "MCPP_ENABLE_SANITIZERS: unknown compiler '${CMAKE_CXX_COMPILER_ID}', no sanitizer flags applied.")
endif()
