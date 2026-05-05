# modules/05_containers_ranges_p1 — 容器、ranges 与算法 Part 1
#
# 演示与测试覆盖：
#   - 迭代器（类别 / traits / 适配器 / 流迭代器）
#   - 顺序容器（array / vector / vector<bool> / bitset / span / deque / list / forward_list）
#   - 容器适配器（stack / queue / priority_queue）
#   - 关联容器（map / set / multimap / multiset 与 unordered_* 家族）
#   - 结构化绑定与 tuple

mcpp_add_demo(NAME iterators_traits      SOURCES demos/iterators_traits.cpp)
mcpp_add_demo(NAME iterator_adapters     SOURCES demos/iterator_adapters.cpp)
mcpp_add_demo(NAME vector_capacity       SOURCES demos/vector_capacity.cpp)
mcpp_add_demo(NAME vector_bool_proxy     SOURCES demos/vector_bool_proxy.cpp)
mcpp_add_demo(NAME span_view             SOURCES demos/span_view.cpp)
mcpp_add_demo(NAME bitset_demo           SOURCES demos/bitset_demo.cpp)
mcpp_add_demo(NAME deque_list_demo       SOURCES demos/deque_list_demo.cpp)
mcpp_add_demo(NAME priority_queue_demo   SOURCES demos/priority_queue_demo.cpp)
mcpp_add_demo(NAME map_idioms            SOURCES demos/map_idioms.cpp)
mcpp_add_demo(NAME unordered_custom_hash SOURCES demos/unordered_custom_hash.cpp)

mcpp_add_test(NAME test_iterators        SOURCES tests/test_iterators.cpp)
mcpp_add_test(NAME test_array            SOURCES tests/test_array.cpp)
mcpp_add_test(NAME test_vector           SOURCES tests/test_vector.cpp)
mcpp_add_test(NAME test_span             SOURCES tests/test_span.cpp)
mcpp_add_test(NAME test_bitset           SOURCES tests/test_bitset.cpp)
mcpp_add_test(NAME test_sequence         SOURCES tests/test_sequence.cpp)
mcpp_add_test(NAME test_adapters         SOURCES tests/test_adapters.cpp)
mcpp_add_test(NAME test_map              SOURCES tests/test_map.cpp)
mcpp_add_test(NAME test_unordered        SOURCES tests/test_unordered.cpp)
mcpp_add_test(NAME test_tuple            SOURCES tests/test_tuple.cpp)
