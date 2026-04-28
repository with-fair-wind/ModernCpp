# modules/02_lifetime_type_safety — 生命周期与类型安全 / Lifetime & Type Safety
#
# Demos and tests covering object lifetime, storage duration,
# references, type safety, union safety, placement new.

mcpp_add_demo(NAME storage_duration SOURCES demos/storage_duration.cpp)

mcpp_add_test(NAME test_lifetime    SOURCES tests/test_lifetime.cpp)
