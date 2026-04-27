# Cross-platform driver for the `format` / `format-check` build targets.
#
# Invoked via `cmake -P` so we never depend on a host shell, xargs, or any
# GNU-specific extension. Required cache variables (set on the command line
# from the parent project):
#   CLANG_FORMAT  - absolute path to the clang-format executable
#   SOURCE_DIR    - repository root (must contain a .git directory)
#   MODE          - "fix" (clang-format -i) or "check" (--dry-run --Werror)

if(NOT CLANG_FORMAT)
    message(FATAL_ERROR "RunClangFormat.cmake: CLANG_FORMAT is not set")
endif()
if(NOT SOURCE_DIR)
    message(FATAL_ERROR "RunClangFormat.cmake: SOURCE_DIR is not set")
endif()
if(NOT MODE STREQUAL "fix" AND NOT MODE STREQUAL "check")
    message(FATAL_ERROR
        "RunClangFormat.cmake: MODE must be 'fix' or 'check' (got '${MODE}')")
endif()

execute_process(
    COMMAND git -C "${SOURCE_DIR}" ls-files
        "*.cpp" "*.cc" "*.cxx" "*.hpp" "*.h" "*.hxx"
    OUTPUT_VARIABLE _files_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)

if(_files_str STREQUAL "")
    message(STATUS "No tracked C/C++ sources found; nothing to format.")
    return()
endif()

string(REPLACE "\n" ";" _files "${_files_str}")

# Make paths absolute so clang-format works regardless of cwd at invocation.
set(_abs_files "")
foreach(_f IN LISTS _files)
    list(APPEND _abs_files "${SOURCE_DIR}/${_f}")
endforeach()

if(MODE STREQUAL "fix")
    message(STATUS "Running ${CLANG_FORMAT} -i over tracked C/C++ sources...")
    execute_process(
        COMMAND "${CLANG_FORMAT}" -i ${_abs_files}
        COMMAND_ERROR_IS_FATAL ANY)
else()
    message(STATUS
        "Running ${CLANG_FORMAT} --dry-run --Werror over tracked sources...")
    execute_process(
        COMMAND "${CLANG_FORMAT}" --dry-run --Werror ${_abs_files}
        COMMAND_ERROR_IS_FATAL ANY)
endif()
