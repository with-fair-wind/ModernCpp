# modules/01_basics — 基础复习与扩展 / Basics Review & Extensions
#
# Demos and tests covering fundamental Modern C++ language features:
# auto deduction, fixed-width integers, safe comparisons, <bit> helpers.

mcpp_add_demo(NAME auto_deduction  SOURCES demos/auto_deduction.cpp)
mcpp_add_demo(NAME integer_safety  SOURCES demos/integer_safety.cpp)
mcpp_add_demo(NAME bit_ops         SOURCES demos/bit_ops.cpp)

mcpp_add_test(NAME test_basics     SOURCES tests/test_basics.cpp)
