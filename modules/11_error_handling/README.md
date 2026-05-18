# 11_error_handling — 错误处理 / Error Handling

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/optional_basics.cpp` | `std::optional` 可空值语义 |
| `demos/exception_basics.cpp` | 异常抛出、捕获、标准异常层次 |
| `demos/exception_safety.cpp` | 异常安全级别（基本 / 强 / 不抛出） |
| `demos/noexcept_demo.cpp` | `noexcept` 规范与条件 noexcept |
| `demos/assertion_demo.cpp` | `assert` / `static_assert` |
| `demos/source_location_demo.cpp` | `std::source_location`（C++20）替代 `__FILE__` / `__LINE__` |
| `demos/error_code_demo.cpp` | `std::error_code` / `std::error_category` 系统错误码 |
| `demos/expected_basics.cpp`† | `std::expected`（C++23）值或错误 |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_optional.cpp` | `optional` 构造、`value_or`、`transform` |
| `tests/test_exception.cpp` | 异常捕获顺序、`std::nested_exception` |
| `tests/test_exception_safety.cpp` | 异常安全保证验证 |
| `tests/test_noexcept.cpp` | `noexcept` 运算符与函数调用 |
| `tests/test_source_location.cpp` | `source_location` 捕获正确位置 |
| `tests/test_error_code.cpp` | 系统错误码比较与消息 |
| `tests/test_expected.cpp`† | `expected` monadic 操作（`and_then` / `transform` / `or_else`） |

> † 仅在编译器提供 `<expected>` 时构建（libstdc++ 12+ / libc++ 16+ / MSVC 17.4+）。

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    11_error_handling__test_exception

# 跑本模块全部测试
ctest --preset gcc-debug -L 11_error_handling
```
