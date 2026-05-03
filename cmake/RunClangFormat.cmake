# `format` / `format-check` target 的跨平台驱动脚本。
#
# 通过 `cmake -P` 调用，这样就不用依赖宿主 shell、xargs，也不用任何 GNU 特有
# 的扩展。需要由父项目从命令行传入的 cache 变量：
#   CLANG_FORMAT  - clang-format 可执行文件的绝对路径
#   SOURCE_DIR    - 仓库根（必须包含 .git 目录）
#   MODE          - "fix"（clang-format -i）或 "check"（--dry-run --Werror）

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

# 把路径转成绝对路径，这样无论调用时 cwd 是什么，clang-format 都能正常工作。
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
