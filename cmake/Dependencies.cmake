# Dependencies.cmake
#
# Resolves third-party dependencies via `find_package`. Users are expected
# to provision dependencies with either vcpkg (vcpkg.json manifest) or
# Conan (conanfile.txt) and point CMake at the resulting toolchain file.
# See README.md for setup instructions.
#
# Currently managed deps:
#   * GoogleTest  (only if MCPP_BUILD_TESTS is ON)

if(MCPP_BUILD_TESTS)
    # MSVC CRT alignment: gtest defaults to static CRT (/MT); force shared CRT
    # (/MD) to match this repo's vcpkg triplet (x64-windows = dynamic CRT) and
    # CMake's default CMAKE_MSVC_RUNTIME_LIBRARY (also dynamic).
    #
    # If you ever switch to a static-CRT triplet (x64-windows-static, etc.),
    # change this to OFF AND set CMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded"
    # accordingly — leaving them mismatched causes LNK2038 "RuntimeLibrary
    # mismatch" linker errors.
    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    find_package(GTest CONFIG QUIET)
    if(NOT GTest_FOUND)
        find_package(GTest MODULE QUIET)
    endif()

    if(NOT GTest_FOUND)
        message(FATAL_ERROR
            "GoogleTest not found.\n"
            "Install it via one of:\n"
            "  * vcpkg:  add to vcpkg.json and pass "
            "-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake\n"
            "  * Conan:  conan install . --output-folder=build/<preset> --build=missing "
            "then pass -DCMAKE_TOOLCHAIN_FILE=build/<preset>/conan_toolchain.cmake\n"
            "See README.md for details. "
            "To skip tests entirely pass -DMCPP_BUILD_TESTS=OFF.")
    endif()

    # Defensive check for the Conan + CMakeDeps build_type mismatch trap:
    # CMakeDeps only emits per-config target files (GTest-Target-<lower>.cmake)
    # for the build_type passed to `conan install -s build_type=...`. If that
    # doesn't match the active CMake config, the IMPORTED targets get created
    # (so GTest_FOUND is TRUE) but their INTERFACE_INCLUDE_DIRECTORIES is
    # wrapped in $<$<CONFIG:OtherType>:...> generator expressions that
    # evaluate to empty for our config — configure passes, build later
    # explodes with `gtest/gtest.h: No such file`. Catch this at configure time.
    #
    # We need to validate against EVERY config CMake will actually drive:
    #   * single-config (Ninja, Makefiles, ...): just CMAKE_BUILD_TYPE
    #   * multi-config (VS, Ninja Multi-Config): every entry of
    #     CMAKE_CONFIGURATION_TYPES — Conan's per-build-type toolchain only
    #     populates one of them, so the others would silently break at build
    #     time. We catch that at configure time too.
    if(TARGET GTest::gtest_main)
        set(_mcpp_configs_to_check)
        if(CMAKE_CONFIGURATION_TYPES)
            list(APPEND _mcpp_configs_to_check ${CMAKE_CONFIGURATION_TYPES})
        elseif(CMAKE_BUILD_TYPE)
            list(APPEND _mcpp_configs_to_check "${CMAKE_BUILD_TYPE}")
        endif()

        if(_mcpp_configs_to_check)
            get_target_property(_mcpp_gtest_inc GTest::gtest_main INTERFACE_INCLUDE_DIRECTORIES)
            if(_mcpp_gtest_inc)
                string(FIND "${_mcpp_gtest_inc}" "$<$<CONFIG:" _mcpp_has_genex)
                if(NOT _mcpp_has_genex EQUAL -1)
                    foreach(_mcpp_cfg IN LISTS _mcpp_configs_to_check)
                        string(FIND "${_mcpp_gtest_inc}" "$<$<CONFIG:${_mcpp_cfg}>" _mcpp_has_match)
                        if(_mcpp_has_match EQUAL -1)
                            message(FATAL_ERROR
                                "GoogleTest was found but has no include "
                                "directories for config '${_mcpp_cfg}'.\n"
                                "This usually means `conan install` was run "
                                "with a different build_type than the CMake "
                                "preset wants to drive. Re-run with: "
                                "-s build_type=${_mcpp_cfg}\n"
                                "(For multi-config generators every config in "
                                "CMAKE_CONFIGURATION_TYPES must be matched by "
                                "a separate `conan install`; in practice that "
                                "means using a single-config preset instead — "
                                "see docs/conan-guide.md.)")
                        endif()
                    endforeach()
                endif()
            endif()
        else()
            message(WARNING
                "Dependencies.cmake: neither CMAKE_BUILD_TYPE nor "
                "CMAKE_CONFIGURATION_TYPES is set — skipping the Conan "
                "build_type/include-dir consistency check. If a Conan "
                "build_type mismatch is present, the build will fail later "
                "with `gtest/gtest.h: No such file`.")
        endif()
    endif()

    include(GoogleTest)
endif()
