# `tidy` / `tidy-check` target 的跨平台驱动脚本。
#
# 通过 `cmake -P` 调用，这样就不用依赖宿主 shell、xargs，也不用任何 GNU 特有
# 的扩展。需要由父项目从命令行传入的 cache 变量：
#   CLANG_TIDY        - clang-tidy 可执行文件的绝对路径
#   SOURCE_DIR        - 仓库根（必须包含 .git 目录）
#   BUILD_DIR         - 当前激活的 build 目录（必须包含 compile_commands.json）
#   MODE              - "fix"（clang-tidy --fix）或 "check"（只读）
#
# 加速策略：
#   优先调用 LLVM 自带的 run-clang-tidy（根据 ${CLANG_TIDY} 同目录推断），
#   它内部会按 -j 并发派发多个 clang-tidy 子进程；找不到则 fallback 串行。
#   apt 装的 clang-tidy-20 通常带 /usr/bin/run-clang-tidy-20。
#
# 待分析的源文件列表交由 run-clang-tidy 从 compile_commands.json 推导（用
# 一个 path-prefix 正则限制到 ${SOURCE_DIR}/modules/ 下）。configure 阶段
# 被跳过的模块（例如当前工具链 stdlib 没有 <expected> 时的 11_error_handling）
# 不会进 CDB，自然也不会被分析 —— clang-tidy 不会回退到自己的默认 flag。

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

# ----------------------------------------------------------------------
# 试图找到与 ${CLANG_TIDY} 同版本的 run-clang-tidy 包装。
# 例如 /usr/bin/clang-tidy-20 → /usr/bin/run-clang-tidy-20。
# ----------------------------------------------------------------------
get_filename_component(_clang_tidy_dir  "${CLANG_TIDY}" DIRECTORY)
get_filename_component(_clang_tidy_name "${CLANG_TIDY}" NAME)
string(REGEX REPLACE "^clang-tidy" "run-clang-tidy" _run_name "${_clang_tidy_name}")
set(_run_clang_tidy "${_clang_tidy_dir}/${_run_name}")

# 路径前缀正则：CDB 里 file 字段以 ${SOURCE_DIR}/modules/ 开头则纳入分析。
# 三方依赖（vcpkg / conan cache）的 file 字段在别的目录，不会被命中。
# Windows 路径里的反斜杠在 Python re 里要转成正斜杠才稳。
string(REPLACE "\\" "/" _modules_root_re "${SOURCE_DIR}/modules/")

set(_extra_args)
if(MODE STREQUAL "fix")
    list(APPEND _extra_args -fix)
endif()

if(EXISTS "${_run_clang_tidy}")
    # ------------------------------------------------------------------
    # 并行路径：用 LLVM 官方 run-clang-tidy。它内部按 -j 派发多个 clang-tidy
    # 子进程，CI 4 核机器实测 ~3-4× 加速（10 分钟 → 2-3 分钟）。
    # ------------------------------------------------------------------
    include(ProcessorCount)
    ProcessorCount(_ncpu)
    if(_ncpu EQUAL 0)
        set(_ncpu 4)
    endif()

    if(MODE STREQUAL "fix")
        message(STATUS
            "Running ${_run_clang_tidy} -fix (parallel j=${_ncpu}) over module sources...")
    else()
        message(STATUS
            "Running ${_run_clang_tidy} (read-only, parallel j=${_ncpu}) over module sources...")
    endif()

    execute_process(
        COMMAND "${_run_clang_tidy}"
            -p "${BUILD_DIR}"
            -clang-tidy-binary "${CLANG_TIDY}"
            -j ${_ncpu}
            -quiet
            -warnings-as-errors=*
            ${_extra_args}
            "${_modules_root_re}"
        COMMAND_ERROR_IS_FATAL ANY)
    return()
endif()

# ----------------------------------------------------------------------
# Fallback：找不到 run-clang-tidy，从 CDB 自己 filter 文件，串行调一次
# clang-tidy。这条路 PR 阶段会偶尔触发——例如本地装的 clang-tidy 包不带
# run-clang-tidy 包装。
# ----------------------------------------------------------------------
message(STATUS
    "RunClangTidy.cmake: ${_run_clang_tidy} 不存在，回退到串行 clang-tidy")

file(READ "${BUILD_DIR}/compile_commands.json" _ccdb_str)
string(JSON _ccdb_len ERROR_VARIABLE _err LENGTH "${_ccdb_str}")
if(_err)
    message(FATAL_ERROR "RunClangTidy.cmake: failed to parse compile_commands.json: ${_err}")
endif()

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

if(MODE STREQUAL "fix")
    message(STATUS "Running ${CLANG_TIDY} --fix (serial) over ${_abs_files}")
else()
    message(STATUS
        "Running ${CLANG_TIDY} (read-only, serial) over module sources from compile_commands.json...")
endif()

execute_process(
    COMMAND "${CLANG_TIDY}"
        -p "${BUILD_DIR}"
        --warnings-as-errors=*
        ${_extra_args}
        ${_abs_files}
    COMMAND_ERROR_IS_FATAL ANY)
