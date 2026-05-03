# `tidy` / `tidy-check` target 的跨平台驱动脚本。
#
# 通过 `cmake -P` 调用，这样就不用依赖宿主 shell、xargs，也不用任何 GNU 特有
# 的扩展。需要由父项目从命令行传入的 cache 变量：
#   CLANG_TIDY        - clang-tidy 可执行文件的绝对路径
#   SOURCE_DIR        - 仓库根（必须包含 .git 目录）
#   BUILD_DIR         - 当前激活的 build 目录（必须包含 compile_commands.json）
#   MODE              - "fix"（clang-tidy --fix）或 "check"（只读）
#
# 待分析的源文件列表从 compile_commands.json 推导（仅保留位于
# SOURCE_DIR/modules/ 下的条目）。基于 CDB 而非 `git ls-files`，意味着
# configure 阶段被跳过的模块（例如当前工具链 stdlib 没有 <expected> 时
# 的 11_error_handling）会自动从分析中排除 —— clang-tidy 不会看到 CDB 里
# 没有的 cpp，也就不会回退到自己的默认 flag。

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

# 读取 compile_commands.json，并取出每条记录里的 "file" 字段。
file(READ "${BUILD_DIR}/compile_commands.json" _ccdb_str)
string(JSON _ccdb_len ERROR_VARIABLE _err LENGTH "${_ccdb_str}")
if(_err)
    message(FATAL_ERROR "RunClangTidy.cmake: failed to parse compile_commands.json: ${_err}")
endif()

# 只保留 `file` 位于 SOURCE_DIR/modules/ 下、且扩展名是 C++ 源文件的条目。
# 头文件本来就不会出现在 CDB 中。
set(_modules_root "${SOURCE_DIR}/modules")
set(_abs_files "")
if(_ccdb_len GREATER 0)
    math(EXPR _last "${_ccdb_len} - 1")
    foreach(_i RANGE 0 ${_last})
        string(JSON _f GET "${_ccdb_str}" ${_i} file)
        # 统一路径分隔符以便做前缀匹配（处理 Windows 反斜杠）。
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
