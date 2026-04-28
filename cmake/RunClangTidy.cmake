# Cross-platform driver for the `tidy` / `tidy-check` build targets.
#
# Invoked via `cmake -P` so we never depend on a host shell, xargs, or any
# GNU-specific extension. Required cache variables (set on the command line
# from the parent project):
#   CLANG_TIDY        - absolute path to the clang-tidy executable
#   SOURCE_DIR        - repository root (must contain a .git directory)
#   BUILD_DIR         - active build directory (must contain compile_commands.json)
#   MODE              - "fix" (clang-tidy --fix) or "check" (read-only)

if(NOT CLANG_TIDY)
    message(FATAL_ERROR "RunClangTidy.cmake: CLANG_TIDY is not set")
endif()
if(NOT SOURCE_DIR)
    message(FATAL_ERROR "RunClangTidy.cmake: SOURCE_DIR is not set")
endif()
if(NOT BUILD_DIR)
    message(FATAL_ERROR "RunClangTidy.cmake: BUILD_DIR is not set")
endif()
if(NOT EXISTS "${BUILD_DIR}/compile_commands.json")
    message(FATAL_ERROR
        "RunClangTidy.cmake: ${BUILD_DIR}/compile_commands.json not found. "
        "Configure & build the active preset first.")
endif()
if(NOT MODE STREQUAL "fix" AND NOT MODE STREQUAL "check")
    message(FATAL_ERROR
        "RunClangTidy.cmake: MODE must be 'fix' or 'check' (got '${MODE}')")
endif()

execute_process(
    COMMAND git -C "${SOURCE_DIR}" ls-files "modules/*.cpp"
    OUTPUT_VARIABLE _files_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)

if(_files_str STREQUAL "")
    message(STATUS "No tracked module sources found; nothing to analyse.")
    return()
endif()

string(REPLACE "\n" ";" _files "${_files_str}")
set(_abs_files "")
foreach(_f IN LISTS _files)
    list(APPEND _abs_files "${SOURCE_DIR}/${_f}")
endforeach()

set(_extra_args)
if(MODE STREQUAL "fix")
    list(APPEND _extra_args --fix)
    message(STATUS "Running ${CLANG_TIDY} --fix over tracked module sources...")
else()
    message(STATUS "Running ${CLANG_TIDY} (read-only) over tracked module sources...")
endif()

execute_process(
    COMMAND "${CLANG_TIDY}"
        -p "${BUILD_DIR}"
        --warnings-as-errors=*
        ${_extra_args}
        ${_abs_files}
    COMMAND_ERROR_IS_FATAL ANY)
