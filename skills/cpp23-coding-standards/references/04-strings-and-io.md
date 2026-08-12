# 04 · 字符串与输入输出

## 适用范围与核心原则

区分文本的所有权、编码、格式化和输出同步。`std::string`、`std::string_view` 与字符编码是不同维度的问题。

## 字符串接口

- 当函数仅在调用期间读取字符序列、不保存引用且不要求空字符结尾时，**应**接受 `std::string_view`。
- 当函数保存文本时，应取得 `std::string` 所有权或复制内容。保存 `string_view` 只有在生命周期由更高层契约保证时才可行。
- 当接口必须接收以 `\0` 结尾的 C 字符串时，使用能表达该前提的接口，并在边界验证空指针与编码约定。
- `std::string_view` 不保证空字符结尾；不要把 `view.data()` 直接传给要求 C 字符串的 API。
- 不要把 `std::string` 等同于 UTF-8。**必须**在系统、文件、网络和 UI 边界明确编码及错误处理策略。

## 格式化与流

- 工具链完整支持时，应使用 `std::format`、`std::print` 或 `std::println` 表达结构化格式化；简单流式读写仍可使用 iostream。
- 若受支持工具链不一致，应在集中兼容层检查 `__cpp_lib_format`、`__cpp_lib_print` 等宏并提供替代实现，
  不要在每个调用点重复条件编译。
- 为项目拥有的类型特化 `std::formatter<T, CharT>` 时，**必须**正确解析格式说明并保持 `parse` 与 `format` 契约一致。
- 多线程向同一流写完整记录时可使用 `std::osyncstream` 防止字符交错；它不保护被格式化的数据，也不代替互斥量。
- 在调用方提供的固定字符缓冲区上进行流式读写时可以考虑 `<spanstream>`，并检查写入范围和截断行为。
- 不需要立即刷新时，应使用 `'\n'` 而不是 `std::endl`。

## 示例

```cpp
void logMessage(std::string_view category, std::string_view message);

struct Point
{
    int x{};
    int y{};
};

template<>
struct std::formatter<Point>
{
    constexpr auto parse(std::format_parse_context& context)
    {
        return context.begin();
    }

    auto format(const Point& point, std::format_context& context) const
    {
        return std::format_to(context.out(), "({}, {})", point.x, point.y);
    }
};
```

## 审查重点

- 检查视图是否从临时字符串、会重分配的缓冲区或短生命周期局部变量取得。
- 检查空字符结尾、编码、locale 和错误替换策略是否在边界明确。
- 检查并发输出只解决输出交错，没有掩盖数据竞争。
