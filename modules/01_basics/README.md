# 01_basics — 基础复习与扩展 / Basics Review & Extensions

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/auto_deduction.cpp` | `auto` 类型推导规则、`decltype(auto)` |
| `demos/integer_safety.cpp` | 定长整型、安全整数比较（`std::cmp_*`） |
| `demos/bit_ops.cpp` | `<bit>` 辅助函数：`popcount`、`countl_zero`、`bit_ceil` 等 |
| `demos/literals_and_init.cpp` | 字面量后缀、聚合初始化、指定初始化器 |
| `demos/enum_scoped.cpp` | 作用域枚举（`enum class`）与底层类型 |
| `demos/lambdas.cpp` | Lambda 捕获、泛型 lambda、`mutable`、IIFE |
| `demos/polymorphism.cpp` | 虚函数、`override`、`final`、纯虚 |
| `demos/spaceship.cpp` | 三路比较运算符（`<=>`）与自动生成的关系运算 |
| `demos/attributes.cpp` | `[[nodiscard]]`、`[[maybe_unused]]`、`[[deprecated]]`、`[[likely]]` |
| `demos/control_flow.cpp` | `if constexpr`、`if` 初始化语句、结构化绑定 |
| `demos/floating_point.cpp` | 浮点精度、`<cfloat>`、NaN / Inf 处理 |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_basics.cpp` | auto 推导、整数安全比较、bit 操作、字面量初始化 |
| `tests/test_lambda.cpp` | Lambda 捕获语义、泛型 lambda、返回值推导 |
| `tests/test_enum.cpp` | `enum class` 类型安全、底层类型、`std::to_underlying` |
| `tests/test_polymorphism.cpp` | 虚函数分派、`override` / `final` 保护 |
| `tests/test_spaceship.cpp` | `<=>` 运算符、`strong_ordering` / `partial_ordering` |
| `tests/test_floating_point.cpp` | 浮点比较、特殊值（NaN / Inf）行为 |

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    01_basics__test_basics

# 跑本模块全部测试
ctest --preset gcc-debug -L 01_basics
```
