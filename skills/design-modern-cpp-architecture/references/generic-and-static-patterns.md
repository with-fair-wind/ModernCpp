# 泛型、静态多态与类型擦除模式

模板是建立零开销抽象和编译期扩展点的工具，不是架构层次的替代品。先确定替换时机、类型集合、ABI 边界和性能证据。

## 目录

- [机制选择](#机制选择)
- [Concept 与语义约束](#concept-与语义约束)
- [Policy-Based Design](#policy-based-design)
- [CRTP 与显式对象形参](#crtp-与显式对象形参)
- [Mixin 与技能组合](#mixin-与技能组合)
- [Tag Dispatch 与定制点](#tag-dispatch-与定制点)
- [类型擦除](#类型擦除)
- [Variant 多态](#variant-多态)
- [编译期注册与反射边界](#编译期注册与反射边界)
- [验证成本](#验证成本)

## 机制选择

| 机制 | 集合 | 替换时机 | 主要成本 | 适合 |
|---|---|---|---|---|
| 普通重载/自由函数 | 开放 | 编译期 | 低 | 算法与值类型扩展 |
| `concept` + 模板 | 开放 | 编译期 | 实例化、代码膨胀 | 泛型算法、policy |
| CRTP/mixin | 开放 | 编译期 | 耦合、错误信息 | 静态接口复用、能力注入 |
| `variant` + visitor | 封闭 | 编译期定义，运行时选择 | variant 大小、分支 | 状态、AST、命令 |
| 类型擦除 | 开放 | 运行时 | 间接调用、可能分配 | 值语义插件点、callback |
| 虚接口 | 开放 | 运行时 | 间接调用、对象生命周期 | 稳定 OO 接口、跨模块替换 |

模板参数出现在公共类型中会传播实现选择。若调用方无需知道策略类型，考虑在边界处做类型擦除或 Pimpl，保持接口稳定。

## Concept 与语义约束

用标准 concept 组合项目语义，避免只检测某个表达式能否编译：

```cpp
template <class T>
concept EventMessage = std::movable<T> && requires(const T& value) {
    { eventName(value) } -> std::convertible_to<std::string_view>;
};
```

concept 应说明契约仍需文档化的语义，例如线程安全、幂等和生命周期不能仅靠表达式验证。不要给所有内部模板添加重复 `requires`；约束放在公共入口和能改善诊断的位置。

## Policy-Based Design

把正交、编译期固定的行为作为 policy，例如锁、分配、重试或组合策略：

```cpp
template <class Queue, class OverflowPolicy>
requires MessageQueue<Queue> && OverflowHandler<OverflowPolicy, Queue>
class Mailbox;
```

要求 policy 之间真正正交。模板参数超过少量、组合存在非法搭配或频繁传遍业务类型时，改用配置对象、策略聚合或运行时接口。不要让用户理解实现细节才能实例化公共组件。

## CRTP 与显式对象形参

CRTP 适合静态接口、返回派生类型的 fluent API 和空基类优化，但会把基类与派生类紧密耦合。优先让 concept 检查派生契约，避免在基类中随意 `static_cast`。

C++23 显式对象形参可在某些场景替代 CRTP：

```cpp
struct Fluent {
    template <class Self>
    auto&& named(this Self&& self, std::string value) {
        self.name = std::move(value);
        return std::forward<Self>(self);
    }
};
```

仅在目标编译器支持且团队能维护时使用。它减少样板，但不改变继承、封装和生命周期问题。

## Mixin 与技能组合

Mixin 应提供单一、正交能力，并避免假设派生类的隐藏布局。常见用途包括可比较、可序列化、可观测或带统计能力。检查：

- 多个 mixin 是否产生同名成员和模糊调用；
- 构造/销毁顺序是否成为隐式协议；
- 是否因继承暴露了不应公开的 API；
- 是否能用自由函数、组合成员或 decorator 更清晰实现。

不要建立“万能实体基类 + 数十个 mixin”的隐式组件系统。

## Tag Dispatch 与定制点

优先普通重载、成员函数或标准库既有定制机制。只有需要 ADL 驱动、可扩展且防止名称冲突的通用库接口时才设计定制点对象（CPO）。

定制点应有：稳定名称、明确优先级、fallback、`noexcept`/返回类型契约和不允许的扩展方式。不要在应用代码中引入复杂 `tag_invoke` 体系来替代一个直接接口。

## 类型擦除

类型擦除适合“调用方按值持有、实现类型开放、无需继承”的运行时抽象。实现选择：

- 单个 callable：`std::function` 或 C++23 `std::move_only_function`；
- 小型概念对象：自定义 `AnyX`，明确复制/移动、SBO、分配和异常保证；
- 稳定跨模块接口：Pimpl 或传统虚接口通常更易审计。

不要假设 `std::function` 能保存 move-only callable。自定义类型擦除必须测试空状态、移动后状态、异常安全、对齐、SBO 阈值和析构跨模块行为。

## Variant 多态

当备选集合封闭时，`std::variant` 把分派完整性转为编译期检查，适合状态、AST、协议内部消息和命令：

```cpp
using Command = std::variant<Start, Stop, Reconfigure>;

std::expected<void, CommandError> handle(const Command& command) {
    return std::visit(overloaded{
        [](const Start& c) { return handleStart(c); },
        [](const Stop& c) { return handleStop(c); },
        [](const Reconfigure& c) { return handleReconfigure(c); }
    }, command);
}
```

注意最大 alternative 决定对象大小；异常赋值可能进入 `valueless_by_exception`；跨版本持久化或网络协议不能依赖 `variant.index()`，应使用稳定显式标识。

## 编译期注册与反射边界

模板 typelist、constexpr 表和编译期字符串可替代部分运行时注册，但不等于通用反射。适合封闭协议表、状态转移和静态插件集合。动态插件、配置驱动类型或跨共享库发现仍需稳定的运行时注册协议。

避免依赖 `typeid(T).hash_code()`、编译器名称修饰或模板实例地址作为持久化/网络标识。使用显式版本化 ID，并检测重复。

## 验证成本

- 记录模板实例数、构建时间和二进制体积基线。
- 对 concepts 编写正/负编译测试。
- 对 policy 组合测试非法组合是否在入口给出清晰诊断。
- 对类型擦除测试复制/移动/分配/异常和 ABI 边界。
- 对 `variant` 访问者使用穷尽处理，避免吞掉新增类型的泛型 catch-all。
- 性能结论以基准、profile 和生成代码为依据，不以“静态多态”标签为依据。

## 上游依据

- C++ Core Guidelines 模板章节：<https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-templates>
- EnTT 的静态多态和类型擦除实践：<https://skypjack.github.io/entt/>
- 显式对象形参 P0847：<https://wg21.link/P0847>
- `move_only_function` P0288：<https://wg21.link/P0288>
