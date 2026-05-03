# CompilerSanitizers.cmake
#
# 可选的运行期 sanitizer。由 MCPP_ENABLE_SANITIZERS（默认 OFF）控制。
# 暴露 INTERFACE target `mcpp::sanitizers` —— ModuleHelpers 会把每个 demo
# 与 test 都链接到它，所以打开开关一次就能让全部 target 被检测。
#
# 仅在 Debug 与 RelWithDebInfo 下生效；Release / MinSizeRel 保持干净
# （sanitizer 会让这两个配置丧失意义）。
#
#   GCC / Clang（Unix 驱动） : -fsanitize=address,undefined -fno-omit-frame-pointer
#                              （编译 + 链接 flag；linker 会自动选择运行时）
#   MSVC / clang-cl          : /fsanitize=address
#                              MSVC ABI 不支持 UBSan。
#                              ASan 与 /RTC1 不兼容，而默认 Debug flag 会注入
#                              /RTC1 —— 我们会在本构建树里把 /RTC1 从 Debug
#                              flag 中剥掉。
#
# 为什么把 clang-cl 与 MSVC 归为一组而非 Clang：
# clang-cl 的 CMAKE_CXX_COMPILER_ID 是 "Clang"，但它是 MSVC 兼容驱动 ——
# 接受 /MD/MDd，链 MSVC CRT，且支持 /fsanitize=address 这种链 MSVC ASan
# 运行时的拼写。把它送进 GNU/Clang 分支会得到
# "-MDd not allowed with -fsanitize=address" 以及链接期未解析的
# __asan_*/__ubsan_* 符号。

include_guard(GLOBAL)

add_library(mcpp_sanitizers INTERFACE)
add_library(mcpp::sanitizers ALIAS mcpp_sanitizers)

if(NOT MCPP_ENABLE_SANITIZERS)
    return()
endif()

set(_san_configs $<CONFIG:Debug,RelWithDebInfo>)

# MSVC ABI = cl.exe 或 clang-cl。MSVC 这个变量对两者都为 TRUE，正好用来区分
# 走 GNU 驱动的 Clang（Linux/Mac），让后者继续走 Unix 分支。
if(NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set(_san_flags
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all)
    target_compile_options(mcpp_sanitizers INTERFACE $<${_san_configs}:${_san_flags}>)
    target_link_options   (mcpp_sanitizers INTERFACE $<${_san_configs}:${_san_flags}>)
    message(STATUS "Sanitizers: AddressSanitizer + UBSan (Debug, RelWithDebInfo)")

elseif(MSVC)
    # /fsanitize=address 与 /RTC1 冲突；从 Debug flag 中剥掉 /RTC1。
    #
    # 注意 —— 全局副作用，作用范围不限于 mcpp::sanitizers：
    # 这里改写的是 cache 变量 CMAKE_*_FLAGS_{DEBUG,RELWITHDEBINFO}，CMake 会把
    # 它们注入到本构建树里的每一个 C/C++ target（不仅是链了 mcpp::sanitizers
    # 的那些）。MSVC 没有 per-target 的方式去掉 /RTC1，所以这是较实用的折衷，
    # 但要明白：在 MSVC 下打开 MCPP_ENABLE_SANITIZERS 意味着整个构建都失去
    # /RTC1 —— 包括之后可能 add 进来的第三方子目录。如果某些 target 必须
    # 保留 /RTC1，那就把 MCPP_ENABLE_SANITIZERS 关闭，单独给它们启用 ASan。
    foreach(_flag_var
            CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(DEFINED ${_flag_var})
            string(REGEX REPLACE "[/-]RTC[1csu]+" "" ${_flag_var} "${${_flag_var}}")
            set(${_flag_var} "${${_flag_var}}" CACHE STRING "" FORCE)
        endif()
    endforeach()

    target_compile_options(mcpp_sanitizers INTERFACE $<${_san_configs}:/fsanitize=address>)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Sanitizers: AddressSanitizer (Debug, RelWithDebInfo) via clang-cl /fsanitize=address; UBSan unavailable on MSVC ABI")
    else()
        message(STATUS "Sanitizers: AddressSanitizer (Debug, RelWithDebInfo); UBSan unavailable on MSVC")
    endif()

else()
    message(WARNING "MCPP_ENABLE_SANITIZERS: unknown compiler '${CMAKE_CXX_COMPILER_ID}', no sanitizer flags applied.")
endif()
