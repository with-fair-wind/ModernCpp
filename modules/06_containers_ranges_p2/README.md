# 06_containers_ranges_p2 — 容器、ranges 与算法 Part 2 / Containers, Ranges & Algorithms Part 2

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

本模块基于讲义系统覆盖 ranges、function（可调用对象）与算法；容器与迭代器见
`modules/05_containers_ranges_p1`。

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/ranges_views_basic.cpp` | `iota` / `filter` / `take` / `drop` / `transform` / `reverse` / `stride` |
| `demos/ranges_keys_values.cpp` | `keys` / `values` / `elements<i>` / `as_const` / `common_view` |
| `demos/ranges_join_split.cpp` | `join` / `join_with` / `split` / `lazy_split` |
| `demos/ranges_zip_cartesian.cpp` | `zip` / `zip_transform` / `cartesian_product` / `pairwise` |
| `demos/ranges_chunk_slide.cpp` | `chunk` / `chunk_by` / `slide` / `adjacent<W>` |
| `demos/ranges_to_factory.cpp` | `stdr::to` / `iota` / `single` / `repeat` / `subrange` / `fold_left` |
| `demos/function_wrapper.cpp` | `std::function` / `move_only_function` / `reference_wrapper` |
| `demos/member_pointers.cpp` | 成员函数/数据指针 / `std::invoke` / `bind_front` |
| `demos/transparent_ops.cpp` | `std::less<>` / 透明哈希 / 异构查找 |
| `demos/algo_modifying.cpp` | `remove` / `unique` / `replace` / `rotate` / `shift_left` |
| `demos/algo_sort_search.cpp` | `sort` / `stable_sort` / `partial_sort` / `nth_element` / `binary_search` |
| `demos/algo_numeric.cpp` | `iota` / `accumulate` / `partial_sum` / `inner_product` / `reduce` / `fold_left` |
| `demos/generator_demo.cpp`† | `std::generator` (C++23 协程)：Fibonacci / 无限序列 / 组合 ranges |

### Tests

| 文件 | 主题 |
| ---- | ---- |
| `tests/test_ranges_views.cpp` | 基础视图：iota / filter / take / drop / stride / keys / values / as_const |
| `tests/test_ranges_combinators.cpp` | zip / cartesian_product / chunk / chunk_by / slide / adjacent |
| `tests/test_ranges_to.cpp` | `stdr::to` / factory / `subrange` / `fold_left` / `contains` / `starts_with` |
| `tests/test_function.cpp` | `std::function` / `move_only_function` / `ref` / `cref` |
| `tests/test_member_ptr.cpp` | 成员指针 / `invoke` / `bind_front` / `bind_back` |
| `tests/test_transparent.cpp` | `std::less<>` 异构查找 / 自定义 transparent hash |
| `tests/test_algo_search.cpp` | find / search / lower_bound / equal_range / boyer_moore |
| `tests/test_algo_sort.cpp` | sort / stable_sort / partial_sort / nth_element / partition / merge / heap |
| `tests/test_algo_modifying.cpp` | remove / unique / replace / fill / generate / rotate / copy |
| `tests/test_algo_set_minmax.cpp` | set_intersection/union/difference / minmax / clamp / next_permutation |
| `tests/test_algo_numeric.cpp` | accumulate / reduce / partial_sum / inner_product / fold_left / gcd / lcm |
| `tests/test_generator.cpp`† | `std::generator` 是 input_range / view、与 take/filter/transform 组合、一次性消费 |

> † 仅在编译器与标准库提供 `<generator>` 时构建（GCC ≥ 14 / MSVC 17.10+ /
> libstdc++ 14+）。CI 上的 GCC 13 与 macOS 的 libc++ 暂时跳过。

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    06_containers_ranges_p2__test_ranges_views

# 跑本模块全部测试
ctest --preset gcc-debug -L 06_containers_ranges_p2
```
