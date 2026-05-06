# modules/06_containers_ranges_p2 — 容器、ranges 与算法 Part 2
#
# 演示与测试覆盖：
#   - Ranges：views（iota / filter / take / drop / transform / reverse / stride）、
#     keys / values / elements、join / split、zip / cartesian_product、
#     chunk / slide / adjacent / pairwise、stdr::to / fold_left。
#   - Function：std::function / std::move_only_function / 成员函数指针 / std::invoke /
#     std::bind_front / std::reference_wrapper / transparent operators。
#   - Algorithms：search / sort / partition / heap / set_*、numeric (accumulate /
#     partial_sum / inner_product / reduce / fold_left)、stdr 版与 projection。
#   - Generator：std::generator (C++23 协程)——仅在编译器/标准库提供 <generator>
#     时构建（GCC ≥ 14 / MSVC 17.10+ / clang+libstdc++14；GCC 13、libc++ 暂无）。

mcpp_add_demo(NAME ranges_views_basic    SOURCES demos/ranges_views_basic.cpp)
mcpp_add_demo(NAME ranges_keys_values    SOURCES demos/ranges_keys_values.cpp)
mcpp_add_demo(NAME ranges_join_split     SOURCES demos/ranges_join_split.cpp)
mcpp_add_demo(NAME ranges_zip_cartesian  SOURCES demos/ranges_zip_cartesian.cpp)
mcpp_add_demo(NAME ranges_chunk_slide    SOURCES demos/ranges_chunk_slide.cpp)
mcpp_add_demo(NAME ranges_to_factory     SOURCES demos/ranges_to_factory.cpp)
mcpp_add_demo(NAME function_wrapper      SOURCES demos/function_wrapper.cpp)
mcpp_add_demo(NAME member_pointers       SOURCES demos/member_pointers.cpp)
mcpp_add_demo(NAME transparent_ops       SOURCES demos/transparent_ops.cpp)
mcpp_add_demo(NAME algo_modifying        SOURCES demos/algo_modifying.cpp)
mcpp_add_demo(NAME algo_sort_search      SOURCES demos/algo_sort_search.cpp)
mcpp_add_demo(NAME algo_numeric          SOURCES demos/algo_numeric.cpp)

mcpp_add_test(NAME test_ranges_views        SOURCES tests/test_ranges_views.cpp)
mcpp_add_test(NAME test_ranges_combinators  SOURCES tests/test_ranges_combinators.cpp)
mcpp_add_test(NAME test_ranges_to           SOURCES tests/test_ranges_to.cpp)
mcpp_add_test(NAME test_function            SOURCES tests/test_function.cpp)
mcpp_add_test(NAME test_member_ptr          SOURCES tests/test_member_ptr.cpp)
mcpp_add_test(NAME test_transparent         SOURCES tests/test_transparent.cpp)
mcpp_add_test(NAME test_algo_search         SOURCES tests/test_algo_search.cpp)
mcpp_add_test(NAME test_algo_sort           SOURCES tests/test_algo_sort.cpp)
mcpp_add_test(NAME test_algo_modifying      SOURCES tests/test_algo_modifying.cpp)
mcpp_add_test(NAME test_algo_set_minmax     SOURCES tests/test_algo_set_minmax.cpp)
mcpp_add_test(NAME test_algo_numeric        SOURCES tests/test_algo_numeric.cpp)

# ------------------------------------------------------------------
# std::generator：基于 C++23 协程，只有 libstdc++ 14+ / MSVC STL 17.10+
# 提供 <generator>。GCC 13（CI 使用）与 libc++（macOS）目前都没有。
# 用 try_compile 在 configure 期检测；缺失则跳过这两个目标。
# ------------------------------------------------------------------
include(CheckCXXSourceCompiles)
set(_mcpp_saved_required_flags "${CMAKE_REQUIRED_FLAGS}")
if(MSVC)
    # 顶层已强制 /std:c++latest，但 try_compile 不一定继承；这里显式补上。
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} /std:c++latest")
else()
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++23")
endif()
check_cxx_source_compiles([[
    #include <version>
    #if !defined(__cpp_lib_generator) || __cpp_lib_generator < 202207L
    #  error "std::generator not available"
    #endif
    #include <generator>
    int main() {}
]] MCPP_HAVE_STD_GENERATOR)
set(CMAKE_REQUIRED_FLAGS "${_mcpp_saved_required_flags}")

if(MCPP_HAVE_STD_GENERATOR)
    mcpp_add_demo(NAME generator_demo SOURCES demos/generator_demo.cpp)
    mcpp_add_test(NAME test_generator SOURCES tests/test_generator.cpp)
else()
    message(STATUS
        "06_containers_ranges_p2: <generator> 不可用，跳过 generator demo/test "
        "（需 GCC ≥ 14 / MSVC 17.10+ / libstdc++ 14+；libc++ 暂无）")
endif()
