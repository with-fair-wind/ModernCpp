# modules/03_multi_file — 多文件编程 / Multi-file Programming
#
# 演示与测试覆盖：
#   - 预处理器（宏、条件编译、__VA_OPT__、feature test macro）
#   - 翻译单元与 ODR（声明/定义分离、头文件保护、前置声明）
#   - 命名空间（嵌套、inline namespace、别名）
#   - 内联（inline 函数与 C++17 inline 变量）
#   - 链接属性（外部/内部链接、static、匿名命名空间、extern）
#   - 单例（Meyers' Singleton、静态初始化顺序）
#   - 语言链接（extern "C"、C/C++ 互操作）
#   - 模板显式实例化

# 模块内头文件搜索路径（per-target PRIVATE，不泄漏到其他模块）
set(_mod_include ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ---- 预处理器 demo ----
mcpp_add_demo(NAME preprocessor_demo
    SOURCES demos/preprocessor_demo.cpp
    INCLUDE_DIRS ${_mod_include})

# ---- ODR / 声明与定义 demo ----
mcpp_add_demo(NAME odr_basics
    SOURCES demos/odr_basics.cpp demos/odr_helper.cpp
    INCLUDE_DIRS ${_mod_include})

mcpp_add_demo(NAME forward_decl
    SOURCES demos/forward_decl.cpp demos/forward_a.cpp demos/forward_b.cpp
    INCLUDE_DIRS ${_mod_include})

# ---- 命名空间 demo ----
mcpp_add_demo(NAME namespaces
    SOURCES demos/namespaces.cpp
    INCLUDE_DIRS ${_mod_include})

# ---- 内联 demo ----
mcpp_add_demo(NAME inline_var
    SOURCES demos/inline_var.cpp demos/inline_var_user.cpp
    INCLUDE_DIRS ${_mod_include})

# ---- 链接属性 demo ----
mcpp_add_demo(NAME linkage_demo
    SOURCES demos/linkage_demo.cpp demos/linkage_internal.cpp
    INCLUDE_DIRS ${_mod_include})

# ---- 单例 demo ----
mcpp_add_demo(NAME singleton_demo
    SOURCES demos/singleton_demo.cpp
    INCLUDE_DIRS ${_mod_include})

# ---- extern "C" demo ----
mcpp_add_demo(NAME extern_c_demo
    SOURCES demos/extern_c_demo.cpp demos/c_api_impl.cpp
    INCLUDE_DIRS ${_mod_include})

# ---- 模板显式实例化 demo ----
mcpp_add_demo(NAME explicit_instantiation
    SOURCES demos/explicit_instantiation.cpp demos/tmpl_impl.cpp
    INCLUDE_DIRS ${_mod_include})

# ---- 测试 ----
mcpp_add_test(NAME test_preprocessor
    SOURCES tests/test_preprocessor.cpp
    INCLUDE_DIRS ${_mod_include})

mcpp_add_test(NAME test_odr
    SOURCES tests/test_odr.cpp tests/test_odr_helper.cpp
    INCLUDE_DIRS ${_mod_include})

mcpp_add_test(NAME test_namespaces
    SOURCES tests/test_namespaces.cpp
    INCLUDE_DIRS ${_mod_include})

mcpp_add_test(NAME test_inline_var
    SOURCES tests/test_inline_var.cpp tests/test_inline_var_tu2.cpp
    INCLUDE_DIRS ${_mod_include})

mcpp_add_test(NAME test_linkage
    SOURCES tests/test_linkage.cpp tests/test_linkage_tu2.cpp
    INCLUDE_DIRS ${_mod_include})

mcpp_add_test(NAME test_singleton
    SOURCES tests/test_singleton.cpp tests/test_singleton_tu2.cpp
    INCLUDE_DIRS ${_mod_include})

mcpp_add_test(NAME test_extern_c
    SOURCES tests/test_extern_c.cpp tests/test_extern_c_impl.cpp
    INCLUDE_DIRS ${_mod_include})

mcpp_add_test(NAME test_forward_decl
    SOURCES tests/test_forward_decl.cpp
            tests/test_forward_decl_impl_a.cpp
            tests/test_forward_decl_impl_b.cpp
    INCLUDE_DIRS ${_mod_include})

mcpp_add_test(NAME test_explicit_instantiation
    SOURCES tests/test_explicit_instantiation.cpp
            tests/test_tmpl_impl.cpp
    INCLUDE_DIRS ${_mod_include})
