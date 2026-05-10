# vcpkg overlay triplet: MSYS2 CLANG64 (Clang + libc++ + UCRT).
# Keep vcpkg-built packages in the same C++ stdlib ABI family as the
# project's mingw-clang-* presets. Run from a CLANG64 shell, or make sure
# clang64/bin is on PATH.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME MinGW)
set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)

# Let vcpkg subprocesses inherit PATH so the chainload toolchain can find
# the CLANG64 compiler drivers.
set(VCPKG_ENV_PASSTHROUGH PATH)

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE
    "${CMAKE_CURRENT_LIST_DIR}/clang64-toolchain.cmake")
