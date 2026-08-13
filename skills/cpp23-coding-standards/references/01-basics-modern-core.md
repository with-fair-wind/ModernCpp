# 01 · 现代 C++ 基础

## 适用范围与核心原则

用类型、初始化和语言属性直接表达意图。只在语义匹配时采用新语法，不把“更新”本身当作收益。

## 规则

- **应**默认使用 `enum class`，并在布局、序列化或外部协议需要时指定底层类型。位标志应提供受约束的位运算，
  不要依赖隐式整数转换。C ABI 或既有协议边界可以使用无作用域枚举，但应在边界转换。
- 可从枚举获得整数值时使用 `std::to_underlying`。不要用 C 风格转换。
- **应**让可由一个实参调用、但不表示自然隐式转换的构造函数使用 `explicit`；有条件转换可使用
  `explicit(condition)`。
- 涉及可能窄化的数值转换时，**必须**使用能触发窄化诊断的初始化形式，或执行显式的范围检查和转换。
  不要通过改用 `=` 初始化绕过诊断。
- 可在比较语义与成员逐项比较一致时默认生成 `operator==` 或 `operator<=>`。浮点、大小写折叠、规范化值和
  跨类型比较通常需要显式定义语义。
- 可对聚合类型使用指定初始化。**必须**按成员声明顺序书写，并接受它会把调用方与聚合布局耦合。
- 应在类型明显且不会隐藏重要转换、所有权或单位时使用 `auto`。不要仅为缩短类型而牺牲接口可读性。
- 应使用 `const`、`constexpr`、`consteval` 和 `constinit` 表达各自语义；不要把它们视为可互换的“编译期”标记。
- 仅当忽略返回值通常表示缺陷时使用 `[[nodiscard]]`。观察者函数（observer function）通常适合，
  但不要求机械地为所有访问函数添加。
- 仅在有测量依据且分支概率稳定时使用 `[[likely]]` / `[[unlikely]]`；错误提示和性能回归都可能来自错误标注。
- **应**用 `[[fallthrough]]` 标记有意贯穿；项目将相关警告视为错误时，它属于项目强制规则。可用
  `[[maybe_unused]]` 处理条件编译导致的未使用实体，但不要用属性掩盖本应删除的代码。
- 应使用具名常量、`constexpr` 函数或类型安全封装代替对象式宏。仅在预处理确有必要时使用宏。

## 示例

```cpp
enum class FilePermission : std::uint8_t { read = 1, write = 2, execute = 4 };

[[nodiscard]] constexpr auto operator|(FilePermission lhs, FilePermission rhs) noexcept
    -> FilePermission
{
    return static_cast<FilePermission>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

struct Point
{
    int x{};
    int y{};
    auto operator<=>(const Point&) const = default;
};
```

## 审查重点

- 检查隐式转换是否改变单位、精度、符号或所有权。
- 检查指定初始化是否依赖不稳定的聚合布局。
- 检查属性是否表达真实契约，而不是用于压制尚未理解的警告。
