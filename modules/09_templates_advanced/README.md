# 09_templates_advanced — 模板进阶 / Advanced Templates

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/template_template_param.cpp` | 模板模板形参 |
| `demos/nttp_demo.cpp` | 非类型模板形参（NTTP）：整数、浮点、字面量类 |
| `demos/type_deduction.cpp` | 模板参数推导规则（值 / 引用 / 万能引用） |
| `demos/ctad_demo.cpp` | 类模板参数推导（CTAD）与自定义推导指引 |
| `demos/friends_in_templates.cpp` | 模板中的友元声明 |
| `demos/lazy_instantiation.cpp` | 惰性实例化与未使用成员不实例化 |
| `demos/variadic_templates.cpp` | 可变参数模板与参数包展开 |
| `demos/fold_expressions.cpp` | 折叠表达式（C++17） |
| `demos/sfinae_demo.cpp` | SFINAE 与 `std::enable_if` |
| `demos/crtp_type_erasure.cpp` | CRTP 静态多态与类型擦除 |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_template_template_param.cpp` | 模板模板形参匹配 |
| `tests/test_nttp.cpp` | NTTP 推导与特化 |
| `tests/test_type_deduction.cpp` | 模板参数推导验证 |
| `tests/test_ctad.cpp` | CTAD 推导结果与自定义推导指引 |
| `tests/test_friends.cpp` | 模板友元的可见性与访问 |
| `tests/test_lazy_instantiation.cpp` | 惰性实例化行为验证 |
| `tests/test_variadic.cpp` | 参数包展开与递归终止 |
| `tests/test_fold.cpp` | 折叠表达式（一元 / 二元、左折 / 右折） |
| `tests/test_sfinae.cpp` | SFINAE 替换失败与 `enable_if` |
| `tests/test_crtp.cpp` | CRTP 静态分派 |
| `tests/test_type_erasure.cpp` | 类型擦除（`std::function` 风格） |

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    09_templates_advanced__test_variadic

# 跑本模块全部测试
ctest --preset gcc-debug -L 09_templates_advanced
```
