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
    # doesn't match CMAKE_BUILD_TYPE, the IMPORTED targets get created (so
    # GTest_FOUND is TRUE) but their INTERFACE_INCLUDE_DIRECTORIES is wrapped
    # in $<$<CONFIG:OtherType>:...> generator expressions that evaluate to
    # empty under our build_type — configure passes, build later explodes with
    # `gtest/gtest.h: No such file`. Catch this at configure time.
    if(TARGET GTest::gtest_main AND CMAKE_BUILD_TYPE)
        get_target_property(_mcpp_gtest_inc GTest::gtest_main INTERFACE_INCLUDE_DIRECTORIES)
        if(_mcpp_gtest_inc)
            string(FIND "${_mcpp_gtest_inc}" "$<$<CONFIG:" _mcpp_has_genex)
            string(FIND "${_mcpp_gtest_inc}" "$<$<CONFIG:${CMAKE_BUILD_TYPE}>" _mcpp_has_match)
            if(NOT _mcpp_has_genex EQUAL -1 AND _mcpp_has_match EQUAL -1)
                message(FATAL_ERROR
                    "GoogleTest was found but has no include directories for "
                    "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}.\n"
                    "This usually means `conan install` was run with a different "
                    "build_type than the CMake preset. Re-run with: "
                    "-s build_type=${CMAKE_BUILD_TYPE}")
            endif()
        endif()
    endif()

    include(GoogleTest)
endif()
