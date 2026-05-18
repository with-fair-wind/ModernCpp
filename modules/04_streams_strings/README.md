# 04_streams_strings — 流与字符串 / Streams & Strings

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/string_basics.cpp` | `std::string` 构造、拼接、查找、子串 |
| `demos/string_view_demo.cpp` | `std::string_view` 非拥有视图、生命周期注意事项 |
| `demos/charconv_demo.cpp` | `std::from_chars` / `std::to_chars` 高性能数值转换 |
| `demos/user_defined_literals.cpp` | 用户自定义字面量 |
| `demos/format_basics.cpp` | `std::format` 基础用法 |
| `demos/format_ranges.cpp` | `std::format` 格式化 ranges（C++23） |
| `demos/print_demo.cpp` | `std::print` / `std::println`（C++23） |
| `demos/user_defined_format.cpp` | 自定义 `std::formatter` 特化 |
| `demos/stream_basics.cpp` | `iostream` 基础：`cin` / `cout` / `cerr` |
| `demos/stream_buffer.cpp` | `streambuf` 与流缓冲 |
| `demos/spanstream_demo.cpp` | `std::spanstream`（C++23 固定缓冲区流） |
| `demos/io_manipulators.cpp` | IO 操纵器：`setw` / `setprecision` / `hex` 等 |
| `demos/regex_demo.cpp` | `std::regex` 正则表达式匹配与搜索 |
| `demos/fstream_demo.cpp` | `std::fstream` 文件读写 |
| `demos/stringstream_demo.cpp` | `std::stringstream` 字符串流 |
| `demos/osyncstream_demo.cpp` | `std::osyncstream` 线程安全输出 |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_string.cpp` | `std::string` 构造、修改、查找、比较 |
| `tests/test_string_view.cpp` | `string_view` 构造、子视图、与 `string` 互操作 |
| `tests/test_charconv.cpp` | `from_chars` / `to_chars` 整数与浮点往返 |
| `tests/test_format.cpp` | `std::format` 格式化字符串、对齐、填充 |
| `tests/test_format_ranges.cpp` | ranges 格式化（C++23） |
| `tests/test_print.cpp` | `std::print` / `std::println`（C++23） |
| `tests/test_user_format.cpp` | 自定义 `formatter` 特化 |
| `tests/test_stream.cpp` | `iostream` 状态、格式标志、`getline` |
| `tests/test_regex.cpp` | `regex_match` / `regex_search` / `regex_replace` |
| `tests/test_fstream.cpp` | 文件读写、`tellg` / `seekg` |
| `tests/test_stringstream.cpp` | `stringstream` 读写、`str()` 提取 |

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    04_streams_strings__test_string

# 跑本模块全部测试
ctest --preset gcc-debug -L 04_streams_strings
```
