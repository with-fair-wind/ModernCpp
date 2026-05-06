# CompilerWarnings.cmake
#
# 定义 INTERFACE target `mcpp::warnings`，统一了在 GCC / Clang / MSVC 下的
# 警告集合。任何 demo / test target 链接到它即可获得这套警告。
#
# 受顶层选项 MCPP_WARNINGS_AS_ERRORS（默认 OFF）控制。

add_library(mcpp_warnings INTERFACE)
add_library(mcpp::warnings ALIAS mcpp_warnings)

set(_mcpp_gnu_like_flags
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Woverloaded-virtual
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
)

# cl.exe 与 clang-cl 都吃的 MSVC 风格 flag。
set(_mcpp_msvc_flags
    /W4
    /permissive-
    /utf-8
    /Zc:__cplusplus
)
# 仅 cl.exe 才吃的 MSVC 独占 flag。clang-cl 默认就是符合标准的预处理器，
# /Zc:preprocessor 在它那是 no-op 但会刷 -Wunused-command-line-argument，
# 所以另列一组只给真 cl.exe。
set(_mcpp_cl_only_flags
    /Zc:preprocessor
)
# 说明：/EHsc 是 MSVC 的默认异常模型 —— CMake 会自动把它注入到 C++ target，
# 这里就不重复添加了。

# 用 ${MSVC} 而不是 $<CXX_COMPILER_ID:MSVC> 来分流：CMake 的 MSVC 变量对
# cl.exe 与 clang-cl 都为 true，而 $<CXX_COMPILER_ID:MSVC> 只匹配 cl.exe。
# 直接按 CXX_COMPILER_ID:Clang 把 clang-cl 划进 GNU-like 阵营会让它收到
# -Wall（dash），clang-cl driver 把这条比 /Wall 还激进地展开（含
# -Wc++98-compat、-Wpre-c++14-compat 整组兼容性提示），噪声很大。所以
# clang-cl 走 MSVC 风格 flag 更稳。
if(MSVC)
    target_compile_options(mcpp_warnings INTERFACE ${_mcpp_msvc_flags})
    target_compile_definitions(mcpp_warnings INTERFACE NOMINMAX _CRT_SECURE_NO_WARNINGS)
    # 仅给真 cl.exe（clang-cl 的 CXX_COMPILER_ID 是 Clang）。
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(mcpp_warnings INTERFACE ${_mcpp_cl_only_flags})
    endif()
else()
    target_compile_options(mcpp_warnings INTERFACE ${_mcpp_gnu_like_flags})
endif()

if(MCPP_WARNINGS_AS_ERRORS)
    if(MSVC)
        target_compile_options(mcpp_warnings INTERFACE /WX)
    else()
        target_compile_options(mcpp_warnings INTERFACE -Werror)
    endif()
endif()
