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
    # MSVC CRT alignment: gtest defaults to static CRT; force shared CRT to
    # match the default CMAKE_MSVC_RUNTIME_LIBRARY so linking does not fail.
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
