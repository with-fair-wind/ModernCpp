# modules/01_basics — 基础复习与扩展 / Basics Review & Extensions
#
# 演示与测试覆盖现代 C++ 的基础语言特性：
# auto 类型推导、定长整型、安全整数比较、<bit> 辅助函数等。

mcpp_add_demo(NAME auto_deduction      SOURCES demos/auto_deduction.cpp)
mcpp_add_demo(NAME integer_safety      SOURCES demos/integer_safety.cpp)
mcpp_add_demo(NAME bit_ops             SOURCES demos/bit_ops.cpp)
mcpp_add_demo(NAME literals_and_init   SOURCES demos/literals_and_init.cpp)
mcpp_add_demo(NAME enum_scoped         SOURCES demos/enum_scoped.cpp)
mcpp_add_demo(NAME lambdas             SOURCES demos/lambdas.cpp)
mcpp_add_demo(NAME polymorphism        SOURCES demos/polymorphism.cpp)
mcpp_add_demo(NAME spaceship           SOURCES demos/spaceship.cpp)
mcpp_add_demo(NAME attributes          SOURCES demos/attributes.cpp)
mcpp_add_demo(NAME control_flow        SOURCES demos/control_flow.cpp)
mcpp_add_demo(NAME floating_point      SOURCES demos/floating_point.cpp)

mcpp_add_test(NAME test_basics         SOURCES tests/test_basics.cpp)
mcpp_add_test(NAME test_lambda         SOURCES tests/test_lambda.cpp)
mcpp_add_test(NAME test_enum           SOURCES tests/test_enum.cpp)
mcpp_add_test(NAME test_polymorphism   SOURCES tests/test_polymorphism.cpp)
mcpp_add_test(NAME test_spaceship      SOURCES tests/test_spaceship.cpp)
mcpp_add_test(NAME test_floating_point SOURCES tests/test_floating_point.cpp)
