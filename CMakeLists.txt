cmake_minimum_required(VERSION 3.25)

project(ModernCpp
    VERSION 0.1.0
    DESCRIPTION "Modern C++ learning modules: per-doc demos and tests"
    LANGUAGES C CXX)

# ----------------------------------------------------------------------------
# Guardrails
# ----------------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. "
        "Create a build directory, e.g.  cmake --preset gcc-debug")
endif()

# ----------------------------------------------------------------------------
# Baseline C++ standard (per-target override available via mcpp_add_demo STANDARD)
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC: CMAKE_CXX_STANDARD=23 maps to /std:c++23, but parts of the C++23 library
# (e.g. <print>, std::expected, second-batch ranges) only land under /std:c++latest
# in current MSVC versions. Force the latest flag globally so demos using newer
# library features still compile under the msvc preset.
if(MSVC)
    add_compile_options(/std:c++latest)
endif()

# Always emit compile_commands.json for clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Mirror compile_commands.json into the source root so .clangd's default search
# (which walks up from the source file) finds the active preset's DB regardless
# of which build/<preset>/ directory the user picked. Skipped on multi-config
# generators (VS) where compile_commands.json is not produced by CMake.
get_property(_mcpp_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _mcpp_is_multi_config)
    add_custom_target(mcpp_link_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/compile_commands.json"
        BYPRODUCTS "${CMAKE_SOURCE_DIR}/compile_commands.json"
        COMMENT "Mirroring compile_commands.json into source root for clangd"
        VERBATIM)
endif()

# Keep binaries grouped under build/<preset>/bin.
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# ----------------------------------------------------------------------------
# Options
# ----------------------------------------------------------------------------
option(MCPP_BUILD_DEMOS        "Build per-module demo executables" ON)
option(MCPP_BUILD_TESTS        "Build GoogleTest unit tests"       ON)
option(MCPP_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(MCPP_ENABLE_SANITIZERS  "Enable ASan (+UBSan on GCC/Clang) for Debug/RelWithDebInfo" OFF)

# ----------------------------------------------------------------------------
# CMake module path
# ----------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompilerWarnings)
include(CompilerSanitizers)
include(ModuleHelpers)

if(MCPP_BUILD_TESTS)
    enable_testing()
    include(Dependencies)
endif()

# ----------------------------------------------------------------------------
# `format` target: run clang-format -i over every tracked C/C++ source.
# Only registered if clang-format is on PATH (so non-developer machines still
# configure cleanly). Skipped in non-git checkouts (e.g. tarball releases).
# ----------------------------------------------------------------------------
find_program(MCPP_CLANG_FORMAT_EXECUTABLE
    NAMES clang-format-18 clang-format
    DOC "clang-format binary used by the `format` target")

if(MCPP_CLANG_FORMAT_EXECUTABLE AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} -E echo
            "Running ${MCPP_CLANG_FORMAT_EXECUTABLE} -i over tracked C/C++ sources..."
        COMMAND git -C "${CMAKE_SOURCE_DIR}" ls-files
            "*.cpp" "*.cc" "*.cxx" "*.hpp" "*.h" "*.hxx"
            | xargs -r ${MCPP_CLANG_FORMAT_EXECUTABLE} -i
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Formatting tracked C/C++ sources in-place via clang-format"
        VERBATIM USES_TERMINAL)

    add_custom_target(format-check
        COMMAND ${CMAKE_COMMAND} -E echo
            "Running ${MCPP_CLANG_FORMAT_EXECUTABLE} --dry-run --Werror over tracked sources..."
        COMMAND git -C "${CMAKE_SOURCE_DIR}" ls-files
            "*.cpp" "*.cc" "*.cxx" "*.hpp" "*.h" "*.hxx"
            | xargs -r ${MCPP_CLANG_FORMAT_EXECUTABLE} --dry-run --Werror
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Checking formatting of tracked C/C++ sources (dry-run)"
        VERBATIM USES_TERMINAL)
endif()

# ----------------------------------------------------------------------------
# Add every modules/NN_* subdirectory that has its own CMakeLists.txt.
# New modules drop in with zero top-level edits.
# ----------------------------------------------------------------------------
file(GLOB _mcpp_module_dirs
    LIST_DIRECTORIES true
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/*")
list(SORT _mcpp_module_dirs)

foreach(_dir IN LISTS _mcpp_module_dirs)
    set(_abs "${CMAKE_CURRENT_SOURCE_DIR}/modules/${_dir}")
    if(IS_DIRECTORY "${_abs}" AND EXISTS "${_abs}/CMakeLists.txt")
        add_subdirectory("modules/${_dir}")
    endif()
endforeach()

# ----------------------------------------------------------------------------
# Summary (printed at configure time)
# ----------------------------------------------------------------------------
if(_mcpp_is_multi_config)
    set(_mcpp_build_type_display "multi-config: ${CMAKE_CONFIGURATION_TYPES}")
else()
    set(_mcpp_build_type_display "${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "")
message(STATUS "ModernCpp configuration")
message(STATUS "  Generator          : ${CMAKE_GENERATOR}")
message(STATUS "  Build type         : ${_mcpp_build_type_display}")
message(STATUS "  C++ standard       : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler           : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build demos        : ${MCPP_BUILD_DEMOS}")
message(STATUS "  Build tests        : ${MCPP_BUILD_TESTS}")
message(STATUS "  Warnings as errors : ${MCPP_WARNINGS_AS_ERRORS}")
message(STATUS "  Sanitizers         : ${MCPP_ENABLE_SANITIZERS}")
message(STATUS "")
