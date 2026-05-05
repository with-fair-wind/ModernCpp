# 05_containers_ranges_p1 — 容器、ranges 与算法 Part 1 / Containers, Ranges & Algorithms Part 1

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

本模块基于讲义系统覆盖容器与迭代器；ranges 与算法见
`modules/06_containers_ranges_p2`。

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/iterators_traits.cpp` | 迭代器类别（concept）、`iterator_traits`、`advance` / `next` / `prev` / `distance` |
| `demos/iterator_adapters.cpp` | `reverse_iterator`、`back/front/inserter`、`istream_iterator` / `ostream_iterator` |
| `demos/vector_capacity.cpp` | `size` / `capacity`、`reserve`、`shrink_to_fit`、`erase` 返回值 |
| `demos/vector_bool_proxy.cpp` | `vector<bool>` 代理引用（auto 推导陷阱） |
| `demos/span_view.cpp` | `std::span`：非拥有视图、子 span、静态 vs 动态 extent |
| `demos/bitset_demo.cpp` | `std::bitset`：位运算、`to_string` / `to_ullong` 互转 |
| `demos/deque_list_demo.cpp` | `deque` 双端推入；`list` 的 `splice` / `merge` / `sort` / `unique` |
| `demos/priority_queue_demo.cpp` | `stack` / `queue` / `priority_queue`（最大堆、`std::greater` 最小堆、范围构造） |
| `demos/map_idioms.cpp` | `map`：`operator[]` vs `find`、`insert_or_assign`、`try_emplace`、节点提取 |
| `demos/unordered_custom_hash.cpp` | `unordered_*`：自定义哈希、负载因子、桶接口 |

### Tests

| 文件 | 主题 |
| ---- | ---- |
| `tests/test_iterators.cpp` | 迭代器 concept / traits / 反向迭代器 / 流迭代器 |
| `tests/test_array.cpp` | `std::array` 行为、`to_array`、结构化绑定 |
| `tests/test_vector.cpp` | 容量 / 失效边界 / `erase` 返回值 / `vector<bool>` 代理 |
| `tests/test_span.cpp` | `span` 构造、`first` / `last` / `subspan`、`as_bytes` |
| `tests/test_bitset.cpp` | `bitset` 位运算与互转 |
| `tests/test_sequence.cpp` | `deque` / `list` / `forward_list` 的关键 API |
| `tests/test_adapters.cpp` | `stack` / `queue` / `priority_queue`（默认 + 自定义比较器） |
| `tests/test_map.cpp` | `map` / `set` / `multimap` / 节点提取 / `equal_range` |
| `tests/test_unordered.cpp` | `unordered_*`：负载因子、桶、自定义哈希、`multimap` 等价性 |
| `tests/test_tuple.cpp` | `tuple` / `pair` / 结构化绑定 / `std::tie` / `std::ignore` / `tuple_cat` |

## 运行 / Run

```bash
# 构建本模块的 demo 与测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    05_containers_ranges_p1__test_iterators

# 跑本模块全部测试
ctest --preset gcc-debug -L 05_containers_ranges_p1
```
