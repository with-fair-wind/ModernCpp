# Cross-platform driver for the `tidy` / `tidy-check` build targets.
#
# Invoked via `cmake -P` so we never depend on a host shell, xargs, or any
# GNU-specific extension. Required cache variables (set on the command line
# from the parent project):
#   CLANG_TIDY        - absolute path to the clang-tidy executable
#   SOURCE_DIR        - repository root (must contain a .git directory)
#   BUILD_DIR         - active build directory (must contain compile_commands.json)
#   MODE              - "fix" (clang-tidy --fix) or "check" (read-only)
#
# The file list is derived from compile_commands.json (filtered to entries
# under SOURCE_DIR/modules/). Driving off the CDB instead of `git ls-files`
# means modules that get skipped at configure time (e.g. 11_error_handling
# when <expected> is unavailable in the active toolchain's stdlib) are
# automatically excluded from analysis — clang-tidy never sees a cpp it
# can't look up in the CDB, so it never falls back to its default flags.

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

# Read compile_commands.json and pull out every entry's "file" field.
file(READ "${BUILD_DIR}/compile_commands.json" _ccdb_str)
string(JSON _ccdb_len ERROR_VARIABLE _err LENGTH "${_ccdb_str}")
if(_err)
    message(FATAL_ERROR "RunClangTidy.cmake: failed to parse compile_commands.json: ${_err}")
endif()

# Keep only entries whose `file` is under SOURCE_DIR/modules/ and whose
# extension is a C++ source. Headers don't appear in the CDB anyway.
set(_modules_root "${SOURCE_DIR}/modules")
set(_abs_files "")
if(_ccdb_len GREATER 0)
    math(EXPR _last "${_ccdb_len} - 1")
    foreach(_i RANGE 0 ${_last})
        string(JSON _f GET "${_ccdb_str}" ${_i} file)
        # Normalise path separators for the prefix check (Windows backslashes).
        string(REPLACE "\\" "/" _f_norm "${_f}")
        string(REPLACE "\\" "/" _root_norm "${_modules_root}")
        string(LENGTH "${_root_norm}/" _root_len)
        string(SUBSTRING "${_f_norm}" 0 ${_root_len} _f_prefix)
        if(_f_prefix STREQUAL "${_root_norm}/")
            if(_f MATCHES "\\.(cpp|cc|cxx)$")
                list(APPEND _abs_files "${_f}")
            endif()
        endif()
    endforeach()
endif()

list(REMOVE_DUPLICATES _abs_files)

if(NOT _abs_files)
    message(STATUS "No module sources in compile_commands.json; nothing to analyse.")
    return()
endif()

set(_extra_args)
if(MODE STREQUAL "fix")
    list(APPEND _extra_args --fix)
    message(STATUS "Running ${CLANG_TIDY} --fix over ${_abs_files}")
else()
    message(STATUS
        "Running ${CLANG_TIDY} (read-only) over module sources from compile_commands.json...")
endif()

execute_process(
    COMMAND "${CLANG_TIDY}"
        -p "${BUILD_DIR}"
        --warnings-as-errors=*
        ${_extra_args}
        ${_abs_files}
    COMMAND_ERROR_IS_FATAL ANY)
