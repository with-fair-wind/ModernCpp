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

    include(GoogleTest)
endif()
