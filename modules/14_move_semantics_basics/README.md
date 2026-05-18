# 14_move_semantics_basics — 移动语义基础 / Move Semantics Basics

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/move_intro.cpp` | 移动语义动机与 `std::move` 基础 |
| `demos/move_ctor_assign.cpp` | 移动构造函数与移动赋值运算符 |
| `demos/noexcept_move.cpp` | `noexcept` 移动与 `vector` 重分配优化 |
| `demos/rule_of_five_zero.cpp` | 五法则 / 零法则 |
| `demos/moved_from_state.cpp` | moved-from 状态：合法但未指定 |
| `demos/move_algorithms.cpp` | `std::move` 算法（`<algorithm>` 中的范围移动） |
| `demos/move_iterators.cpp` | `std::move_iterator` 与 `make_move_iterator` |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_move_ctor.cpp` | 移动构造资源转移验证 |
| `tests/test_move_assign.cpp` | 移动赋值与自赋值安全 |
| `tests/test_noexcept_move.cpp` | `noexcept` 对 `vector::push_back` 的影响 |
| `tests/test_rule_of_five.cpp` | 五法则 / 零法则行为验证 |
| `tests/test_move_algorithms.cpp` | `std::move` / `std::move_backward` 算法 |
| `tests/test_move_iterators.cpp` | `move_iterator` 与容器配合 |

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    14_move_semantics_basics__test_move_ctor

# 跑本模块全部测试
ctest --preset gcc-debug -L 14_move_semantics_basics
```
