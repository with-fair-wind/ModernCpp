# modules/02_lifetime_type_safety — 生命周期与类型安全 / Lifetime & Type Safety
#
# 演示与测试覆盖：
#   - 生命周期 / 存储期、临时对象延寿、悬空陷阱、placement new、std::launder、
#     存储复用、std::byte / unsigned char 缓冲区、严格别名（type-punning）。
#   - 继承扩展：切片问题（slicing）+ clone() 模式、多重继承 + using 声明、
#     致命菱形 + 虚继承、混入模式（mixin）。
#   - 类型安全：static_cast / const_cast / reinterpret_cast / std::bit_cast、
#     dynamic_cast 与 RTTI（typeid / type_index）。
#   - 类型安全的 union 与 void*：std::variant + std::visit、std::any。
#
# 备注：start_lifetime_as / start_lifetime_as_array 是 C++23 的库特性
# （__cpp_lib_start_lifetime_as ≥ 202207L）。源文件用 feature test 兜底，
# 缺失时跳过该段代码 —— 不需要 try_compile。

include(CheckCXXSourceCompiles)

# ---- 生命周期 / 存储期相关 demo ----
mcpp_add_demo(NAME storage_duration    SOURCES demos/storage_duration.cpp)
mcpp_add_demo(NAME lifetime_extension  SOURCES demos/lifetime_extension.cpp)
mcpp_add_demo(NAME dangling_pitfalls   SOURCES demos/dangling_pitfalls.cpp)
mcpp_add_demo(NAME placement_new       SOURCES demos/placement_new.cpp)
mcpp_add_demo(NAME storage_reuse       SOURCES demos/storage_reuse.cpp)
mcpp_add_demo(NAME byte_storage        SOURCES demos/byte_storage.cpp)

# ---- 继承扩展相关 demo ----
mcpp_add_demo(NAME slicing               SOURCES demos/slicing.cpp)
mcpp_add_demo(NAME multiple_inheritance  SOURCES demos/multiple_inheritance.cpp)
mcpp_add_demo(NAME virtual_inheritance   SOURCES demos/virtual_inheritance.cpp)
mcpp_add_demo(NAME mixin_pattern         SOURCES demos/mixin_pattern.cpp)

# ---- 类型安全相关 demo ----
mcpp_add_demo(NAME casts_overview     SOURCES demos/casts_overview.cpp)
mcpp_add_demo(NAME dynamic_cast_rtti  SOURCES demos/dynamic_cast_rtti.cpp)
mcpp_add_demo(NAME variant_visit      SOURCES demos/variant_visit.cpp)
mcpp_add_demo(NAME any_demo           SOURCES demos/any_demo.cpp)

# ---- 测试 ----
mcpp_add_test(NAME test_lifetime            SOURCES tests/test_lifetime.cpp)
mcpp_add_test(NAME test_lifetime_extension  SOURCES tests/test_lifetime_extension.cpp)
mcpp_add_test(NAME test_placement_new       SOURCES tests/test_placement_new.cpp)
mcpp_add_test(NAME test_storage_reuse       SOURCES tests/test_storage_reuse.cpp)
mcpp_add_test(NAME test_aliasing            SOURCES tests/test_aliasing.cpp)
mcpp_add_test(NAME test_slicing             SOURCES tests/test_slicing.cpp)
mcpp_add_test(NAME test_inheritance         SOURCES tests/test_inheritance.cpp)
mcpp_add_test(NAME test_casts               SOURCES tests/test_casts.cpp)
mcpp_add_test(NAME test_dynamic_cast        SOURCES tests/test_dynamic_cast.cpp)
mcpp_add_test(NAME test_variant             SOURCES tests/test_variant.cpp)
mcpp_add_test(NAME test_any                 SOURCES tests/test_any.cpp)
