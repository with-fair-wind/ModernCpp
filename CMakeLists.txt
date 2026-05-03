cmake_minimum_required(VERSION 3.25)

project(ModernCpp
    VERSION 0.1.0
    DESCRIPTION "Modern C++ learning modules: per-doc demos and tests"
    LANGUAGES C CXX)

# ----------------------------------------------------------------------------
# 防护栏
# ----------------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. "
        "Create a build directory, e.g.  cmake --preset gcc-debug")
endif()

# ----------------------------------------------------------------------------
# 基线 C++ 标准（mcpp_add_demo 的 STANDARD 参数可针对单 target 覆盖）
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC：CMAKE_CXX_STANDARD=23 会映射到 /std:c++23，但 C++23 标准库的部分内容
# （例如 <print>、std::expected、第二批 ranges）在当前的 MSVC 版本上只有用
# /std:c++latest 才能启用。这里全局强制 latest，让用到这些较新库特性的 demo
# 在 msvc preset 下也能编过。
if(MSVC)
    add_compile_options(/std:c++latest)
endif()

# 始终输出 compile_commands.json，供 clangd 使用。
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 把 compile_commands.json 镜像一份到源码根目录 —— .clangd 默认会从源码文件
# 一路向上查找，这样无论用户当前用的是哪个 build/<preset>/，都能找到当前激活
# preset 的 DB。多配置生成器（VS）下 CMake 不产 compile_commands.json，跳过此机制。
get_property(_mcpp_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _mcpp_is_multi_config)
    # 镜像动作通过 add_custom_command(OUTPUT ...) 驱动，让规则真正依赖
    # build-tree 里的 compile_commands.json。之前用 ALL custom target 时，
    # 没有显式 DEPENDS 会导致每次构建都重跑（虽然 copy_if_different 让它
    # 无害，但仍是浪费），且没有把"输入是 build-tree DB"这一事实告诉 Ninja。
    add_custom_command(
        OUTPUT  "${CMAKE_SOURCE_DIR}/compile_commands.json"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/compile_commands.json"
        DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
        COMMENT "Mirroring compile_commands.json into source root for clangd"
        VERBATIM)

    add_custom_target(mcpp_link_compile_commands ALL
        DEPENDS "${CMAKE_SOURCE_DIR}/compile_commands.json")
endif()

# 把可执行文件统一放在 build/<preset>/bin 下。
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# ----------------------------------------------------------------------------
# 顶层选项
# ----------------------------------------------------------------------------
option(MCPP_BUILD_DEMOS        "Build per-module demo executables" ON)
option(MCPP_BUILD_TESTS        "Build GoogleTest unit tests"       ON)
option(MCPP_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(MCPP_ENABLE_SANITIZERS  "Enable ASan (+UBSan on GCC/Clang) for Debug/RelWithDebInfo" OFF)

# ----------------------------------------------------------------------------
# CMake 模块搜索路径
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
# `format` target：对所有被 git 跟踪的 C/C++ 源文件原地跑 clang-format -i。
# 仅当 PATH 中能找到 clang-format 时才注册（这样非开发机也能干净 configure）。
# 非 git 检出（例如 tarball 发布）下也跳过。
# ----------------------------------------------------------------------------
find_program(MCPP_CLANG_FORMAT_EXECUTABLE
    NAMES clang-format-18 clang-format
    DOC "clang-format binary used by the `format` target")

if(MCPP_CLANG_FORMAT_EXECUTABLE)
    message(STATUS "Found clang-format: ${MCPP_CLANG_FORMAT_EXECUTABLE}")
else()
    message(STATUS "clang-format not found — `format` / `format-check` targets will not be registered")
endif()

if(MCPP_CLANG_FORMAT_EXECUTABLE AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    # 通过 cmake -P 脚本驱动 clang-format，这样 Linux/macOS/Windows 上行为
    # 完全一致 —— 不依赖宿主 shell、xargs，也不依赖 `xargs -r` 这种 GNU 扩展。
    set(_mcpp_run_clang_format "${CMAKE_SOURCE_DIR}/cmake/RunClangFormat.cmake")

    add_custom_target(format
        COMMAND ${CMAKE_COMMAND}
            "-DCLANG_FORMAT=${MCPP_CLANG_FORMAT_EXECUTABLE}"
            "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
            "-DMODE=fix"
            -P "${_mcpp_run_clang_format}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Formatting tracked C/C++ sources in-place via clang-format"
        VERBATIM USES_TERMINAL)

    add_custom_target(format-check
        COMMAND ${CMAKE_COMMAND}
            "-DCLANG_FORMAT=${MCPP_CLANG_FORMAT_EXECUTABLE}"
            "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
            "-DMODE=check"
            -P "${_mcpp_run_clang_format}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Checking formatting of tracked C/C++ sources (dry-run)"
        VERBATIM USES_TERMINAL)
endif()

# ----------------------------------------------------------------------------
# `tidy` / `tidy-check` target：对所有被跟踪的模块源文件跑 clang-tidy。
# 与 format / format-check 配对。clang-tidy 需要当前 build 的
# compile_commands.json，所以这两个 target 用单配置生成器作为门槛
# （多配置生成器不产 compile_commands.json）。
# ----------------------------------------------------------------------------
find_program(MCPP_CLANG_TIDY_EXECUTABLE
    NAMES clang-tidy-18 clang-tidy
    DOC "clang-tidy binary used by the `tidy` target")

if(MCPP_CLANG_TIDY_EXECUTABLE)
    message(STATUS "Found clang-tidy: ${MCPP_CLANG_TIDY_EXECUTABLE}")
else()
    message(STATUS "clang-tidy not found — `tidy` / `tidy-check` targets will not be registered")
endif()

if(MCPP_CLANG_TIDY_EXECUTABLE
        AND EXISTS "${CMAKE_SOURCE_DIR}/.git"
        AND NOT _mcpp_is_multi_config)
    set(_mcpp_run_clang_tidy "${CMAKE_SOURCE_DIR}/cmake/RunClangTidy.cmake")

    add_custom_target(tidy
        COMMAND ${CMAKE_COMMAND}
            "-DCLANG_TIDY=${MCPP_CLANG_TIDY_EXECUTABLE}"
            "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
            "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
            "-DMODE=fix"
            -P "${_mcpp_run_clang_tidy}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Running clang-tidy --fix over tracked module sources"
        VERBATIM USES_TERMINAL)

    add_custom_target(tidy-check
        COMMAND ${CMAKE_COMMAND}
            "-DCLANG_TIDY=${MCPP_CLANG_TIDY_EXECUTABLE}"
            "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
            "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
            "-DMODE=check"
            -P "${_mcpp_run_clang_tidy}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Running clang-tidy (read-only, --warnings-as-errors=*)"
        VERBATIM USES_TERMINAL)
endif()

# ----------------------------------------------------------------------------
# 自动收录每个带 CMakeLists.txt 的 modules/NN_* 子目录。
# 新增模块无需改动顶层 CMakeLists.txt。
#
# CONFIGURE_DEPENDS 让 CMake 在每次构建时重新评估 glob，所以新建一个
# modules/NN_*/ 目录后无需手动 reconfigure 就能被收录。代价是每次构建
# 多 ~15 次目录 stat —— 与"我加了个模块结果 CMake 静默忽略了"这种坑相比
# 完全可以接受。
# ----------------------------------------------------------------------------
file(GLOB _mcpp_module_dirs
    LIST_DIRECTORIES true
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/modules"
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/*")
list(SORT _mcpp_module_dirs)

foreach(_dir IN LISTS _mcpp_module_dirs)
    set(_abs "${CMAKE_CURRENT_SOURCE_DIR}/modules/${_dir}")
    if(IS_DIRECTORY "${_abs}" AND EXISTS "${_abs}/CMakeLists.txt")
        add_subdirectory("modules/${_dir}")
    endif()
endforeach()

# ----------------------------------------------------------------------------
# 配置摘要（configure 时打印）
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
