# modules/07_value_categories — 值类别 / Value Categories
#
# 演示：lvalue/xvalue/prvalue 分类、decltype、引用限定符、
# deducing this、复制消除、RVO/NRVO、隐式移动。

mcpp_add_demo(NAME value_categories SOURCES demos/value_categories.cpp)
mcpp_add_demo(NAME decltype_demo SOURCES demos/decltype_demo.cpp)
mcpp_add_demo(NAME ref_qualifiers SOURCES demos/ref_qualifiers.cpp)
mcpp_add_demo(NAME deducing_this SOURCES demos/deducing_this.cpp STANDARD 23)
mcpp_add_demo(NAME copy_elision SOURCES demos/copy_elision.cpp)
mcpp_add_demo(NAME rvo_nrvo SOURCES demos/rvo_nrvo.cpp)
mcpp_add_demo(NAME implicit_move SOURCES demos/implicit_move.cpp)

mcpp_add_test(NAME test_value_categories SOURCES tests/test_value_categories.cpp)
mcpp_add_test(NAME test_ref_qualifiers SOURCES tests/test_ref_qualifiers.cpp)
mcpp_add_test(NAME test_copy_elision SOURCES tests/test_copy_elision.cpp)
mcpp_add_test(NAME test_rvo SOURCES tests/test_rvo.cpp)
mcpp_add_test(NAME test_implicit_move SOURCES tests/test_implicit_move.cpp)
