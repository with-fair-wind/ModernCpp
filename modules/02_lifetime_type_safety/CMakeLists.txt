# modules/02_lifetime_type_safety — 生命周期与类型安全 / Lifetime & Type Safety
#
# 演示与测试覆盖：对象生命周期、存储期、引用、类型安全、union 的安全使用、
# placement new。

mcpp_add_demo(NAME storage_duration SOURCES demos/storage_duration.cpp)

mcpp_add_test(NAME test_lifetime    SOURCES tests/test_lifetime.cpp)
