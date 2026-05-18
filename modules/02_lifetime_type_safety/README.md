# 02_lifetime_type_safety — 生命周期与类型安全 / Lifetime & Type Safety

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/storage_duration.cpp` | 自动 / 静态 / 线程 / 动态存储期 |
| `demos/lifetime_extension.cpp` | const 引用 / 右值引用延长临时对象生命周期 |
| `demos/dangling_pitfalls.cpp` | 悬空引用 / 指针的常见陷阱 |
| `demos/placement_new.cpp` | placement new、显式析构、`std::launder` |
| `demos/storage_reuse.cpp` | 存储复用、`start_lifetime_as`（C++23） |
| `demos/byte_storage.cpp` | `std::byte` / `unsigned char` 缓冲区与严格别名 |
| `demos/slicing.cpp` | 对象切片问题与 `clone()` 模式 |
| `demos/multiple_inheritance.cpp` | 多重继承、`using` 声明消歧义 |
| `demos/virtual_inheritance.cpp` | 菱形继承与虚继承 |
| `demos/mixin_pattern.cpp` | CRTP 混入模式 |
| `demos/casts_overview.cpp` | `static_cast` / `const_cast` / `reinterpret_cast` / `std::bit_cast` |
| `demos/dynamic_cast_rtti.cpp` | `dynamic_cast` 与 RTTI（`typeid` / `type_index`） |
| `demos/variant_visit.cpp` | `std::variant` + `std::visit` 类型安全联合 |
| `demos/any_demo.cpp` | `std::any` 类型擦除容器 |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_lifetime.cpp` | 存储期、生命周期开始 / 结束规则 |
| `tests/test_lifetime_extension.cpp` | 临时对象延寿场景与失效场景 |
| `tests/test_placement_new.cpp` | placement new 正确性、`std::launder` 必要性 |
| `tests/test_storage_reuse.cpp` | 存储复用与隐式生命周期类型 |
| `tests/test_aliasing.cpp` | 严格别名规则、`std::byte` / `memcpy` 安全访问 |
| `tests/test_slicing.cpp` | 切片行为验证、值语义 vs 引用语义 |
| `tests/test_inheritance.cpp` | 多重继承、虚继承、混入模式 |
| `tests/test_casts.cpp` | 四种 cast 正确使用与 `bit_cast` |
| `tests/test_dynamic_cast.cpp` | `dynamic_cast` 成功 / 失败（指针返回 null / 引用抛异常） |
| `tests/test_variant.cpp` | `variant` 构造、`visit`、`get_if`、`valueless_by_exception` |
| `tests/test_any.cpp` | `any` 存取、`any_cast`、类型擦除边界 |

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    02_lifetime_type_safety__test_lifetime

# 跑本模块全部测试
ctest --preset gcc-debug -L 02_lifetime_type_safety
```
