# modules/14_move_semantics_basics — 移动语义基础 / Move Semantics Basics
#
# 演示：移动构造/赋值、noexcept 与 vector 重分配、五法则/零法则、
# moved-from 状态、移动算法与迭代器。

mcpp_add_demo(NAME move_intro           SOURCES demos/move_intro.cpp)
mcpp_add_demo(NAME move_ctor_assign     SOURCES demos/move_ctor_assign.cpp)
mcpp_add_demo(NAME noexcept_move        SOURCES demos/noexcept_move.cpp)
mcpp_add_demo(NAME rule_of_five_zero    SOURCES demos/rule_of_five_zero.cpp)
mcpp_add_demo(NAME moved_from_state     SOURCES demos/moved_from_state.cpp)
mcpp_add_demo(NAME move_algorithms      SOURCES demos/move_algorithms.cpp)
mcpp_add_demo(NAME move_iterators       SOURCES demos/move_iterators.cpp)

mcpp_add_test(NAME test_move_ctor       SOURCES tests/test_move_ctor.cpp)
mcpp_add_test(NAME test_move_assign     SOURCES tests/test_move_assign.cpp)
mcpp_add_test(NAME test_noexcept_move   SOURCES tests/test_noexcept_move.cpp)
mcpp_add_test(NAME test_rule_of_five    SOURCES tests/test_rule_of_five.cpp)
mcpp_add_test(NAME test_move_algorithms SOURCES tests/test_move_algorithms.cpp)
mcpp_add_test(NAME test_move_iterators  SOURCES tests/test_move_iterators.cpp)
