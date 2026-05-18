# 07_value_categories — 值分类与移动语义 / Value Categories & Move Semantics

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/value_categories.cpp` | lvalue / xvalue / prvalue 分类与 `decltype` 判别 |
| `demos/decltype_demo.cpp` | `decltype` 推导规则：表达式 vs 实体 |
| `demos/ref_qualifiers.cpp` | 成员函数的 `&` / `&&` 引用限定符 |
| `demos/deducing_this.cpp` | C++23 deducing this（显式对象形参） |
| `demos/copy_elision.cpp` | 强制与非强制复制消除 |
| `demos/rvo_nrvo.cpp` | RVO / NRVO 返回值优化 |
| `demos/implicit_move.cpp` | 隐式移动（return 局部变量自动移动） |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_value_categories.cpp` | 值类别判别、`decltype` 推导 |
| `tests/test_ref_qualifiers.cpp` | 引用限定符重载选择 |
| `tests/test_copy_elision.cpp` | 强制 / 非强制复制消除验证 |
| `tests/test_rvo.cpp` | RVO / NRVO 消除构造次数 |
| `tests/test_implicit_move.cpp` | 隐式移动与 `std::move` 对比 |

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    07_value_categories__test_value_categories

# 跑本模块全部测试
ctest --preset gcc-debug -L 07_value_categories
```
