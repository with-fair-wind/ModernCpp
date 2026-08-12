# 06 · 模板、概念与多态

## 适用范围与核心原则

用约束表达调用方必须满足的语义，而不是为每个模板附加形式化但无价值的条件。选择编译期或运行时多态应由替换需求决定。

## `concept` 与 `requires`

- 应优先使用标准概念（standard concept）。只有项目语义无法由标准概念准确表达时才定义新 `concept`。
- 不要求所有模板都带 `concept`。对确实接受任意满足表达式要求的内部辅助模板，无约束模板可以更清晰。
- **必须**让约束与实现使用的操作一致，并避免只检查语法而忽略语义要求。
- 使用 `requires` 表达式检查操作、返回类型和异常保证；C++23 新代码不应以 `std::void_t` 检测惯用法
  （detection idiom）为首选。

```cpp
template<typename T>
concept NothrowSwappable = std::swappable<T> && requires(T& lhs, T& rhs) {
    { std::ranges::swap(lhs, rhs) } noexcept;
};
```

## 常量求值与分支

- 使用 `if constexpr` 按模板条件丢弃不适用分支；非依赖名称仍必须在模板定义处合法。
- 使用 `if consteval` 区分当前调用是否正在常量求值。`std::is_constant_evaluated()` 在普通 `if` 中仍有用途，
  不要把它放入 `if constexpr` 后误以为可以区分调用环境。
- 使用 `consteval` 要求每次潜在求值调用都产生常量表达式。普通函数可以调用立即函数，只要该调用满足此要求。
- 使用 `constinit` 保证静态或线程存储期对象执行静态初始化；它不使对象成为 const。

```cpp
constexpr auto executionMode() -> int
{
    if consteval {
        return 1;
    } else {
        return 0;
    }
}

static_assert(executionMode() == 1);
```

## 泛型实现与多态选择

- 转发引用必须与 `std::forward<T>` 配套，并防止转发构造函数意外吞掉拷贝/移动构造重载。
- 使用折叠表达式处理变参归约；对空参数包明确选择单位元或施加非空约束。
- 使用类模板实参推导（CTAD）时确认推导结果符合公开语义；必要时提供推导指引，不要让窄化或引用类型意外进入
  类模板参数。
- 使用 `concept`、重载或 CRTP 表达编译期多态；使用虚函数或类型擦除表达运行时替换。
- 值语义且无需复制的运行时可调用对象可使用 `std::move_only_function`。不要把类型擦除称为编译期多态。

## 审查重点

- 检查 `concept` 是否描述语义，而非仅复述实现中的表达式。
- 检查转发模板的重载集合、引用折叠、约束和生命周期。
- 检查模板错误是否在接口约束处暴露，而不是延迟到深层实现。
