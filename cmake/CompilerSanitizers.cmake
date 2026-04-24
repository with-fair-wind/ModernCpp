# CompilerSanitizers.cmake
#
# Optional runtime sanitizers. Controlled by MCPP_ENABLE_SANITIZERS (default OFF).
# Exposes INTERFACE target `mcpp::sanitizers` — ModuleHelpers links every demo
# and test against it, so turning the option on is enough to sanitize everything.
#
# Only takes effect in Debug and RelWithDebInfo; Release/MinSizeRel stay clean
# (sanitizers defeat the point of those configs).
#
#   GCC / Clang : -fsanitize=address,undefined -fno-omit-frame-pointer
#                 (compile + link flags; linker autoselects the runtime)
#   MSVC        : /fsanitize=address
#                 UBSan is not supported. ASan on MSVC is incompatible with the
#                 default /RTC1 runtime checks injected into Debug, so we strip
#                 /RTC1 from the Debug flags of this build tree.

include_guard(GLOBAL)

add_library(mcpp_sanitizers INTERFACE)
add_library(mcpp::sanitizers ALIAS mcpp_sanitizers)

if(NOT MCPP_ENABLE_SANITIZERS)
    return()
endif()

set(_san_configs $<CONFIG:Debug,RelWithDebInfo>)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set(_san_flags
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all)
    target_compile_options(mcpp_sanitizers INTERFACE $<${_san_configs}:${_san_flags}>)
    target_link_options   (mcpp_sanitizers INTERFACE $<${_san_configs}:${_san_flags}>)
    message(STATUS "Sanitizers: AddressSanitizer + UBSan (Debug, RelWithDebInfo)")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # /fsanitize=address conflicts with /RTC1; strip it from Debug flags.
    foreach(_flag_var
            CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(DEFINED ${_flag_var})
            string(REGEX REPLACE "[/-]RTC[1csu]+" "" ${_flag_var} "${${_flag_var}}")
            set(${_flag_var} "${${_flag_var}}" CACHE STRING "" FORCE)
        endif()
    endforeach()

    target_compile_options(mcpp_sanitizers INTERFACE $<${_san_configs}:/fsanitize=address>)
    message(STATUS "Sanitizers: AddressSanitizer (Debug, RelWithDebInfo); UBSan unavailable on MSVC")

else()
    message(WARNING "MCPP_ENABLE_SANITIZERS: unknown compiler '${CMAKE_CXX_COMPILER_ID}', no sanitizer flags applied.")
endif()
