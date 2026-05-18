# 08_templates_basics — 模板基础 / Templates Basics

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/constexpr_basics.cpp` | `constexpr` 函数与变量、编译期计算 |
| `demos/consteval_constinit.cpp` | `consteval` 立即函数、`constinit` 静态初始化 |
| `demos/template_specialization.cpp` | 全特化与偏特化 |
| `demos/overload_resolution.cpp` | 重载决议规则与模板 vs 非模板优先级 |
| `demos/constexpr_if.cpp` | `if constexpr` 编译期分支 |
| `demos/two_phase_lookup.cpp` | 两阶段名查找（dependent / non-dependent） |
| `demos/typename_template.cpp` | `typename` 与 `template` 消歧义 |
| `demos/concepts_basics.cpp` | C++20 concepts 基础：requires 子句与约束 |
| `demos/concept_subsumption.cpp` | concept 包含（subsumption）与约束排序 |
| `demos/universal_ref.cpp` | 万能引用（转发引用）与完美转发 |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_constexpr.cpp` | `constexpr` 函数编译期 / 运行期行为 |
| `tests/test_specialization.cpp` | 全特化与偏特化选择 |
| `tests/test_constexpr_if.cpp` | `if constexpr` 分支验证 |
| `tests/test_concepts.cpp` | concept 约束匹配与拒绝 |
| `tests/test_concept_subsumption.cpp` | concept 包含关系与重载选择 |
| `tests/test_universal_ref.cpp` | 万能引用推导与 `std::forward` |
| `tests/test_ref_collapsing.cpp` | 引用折叠规则 |

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    08_templates_basics__test_constexpr

# 跑本模块全部测试
ctest --preset gcc-debug -L 08_templates_basics
```
