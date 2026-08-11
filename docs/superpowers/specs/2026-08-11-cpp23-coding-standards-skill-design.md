# C++23 编码规范 Skill —— 设计文档

- **日期**：2026-08-11
- **参考来源**：`ModernCpp` 学习仓库（`D:\code\cpp\ModernCpp\`）的 15 个模块及其 `.clang-format` / `.clang-tidy` / `cmake/` 工具链
- **交付物**：一个用户级 Claude Code skill，编码 C++23（基线）项目的"判断层"编码规范
- **状态**：待用户评审

---

## 1. 目标与定位

创建一个**通用、便携**的 skill，在编写/评审/重构 C++ 代码时，给出"机器（formatter/linter）管不到、但工程师需要判断"的设计与惯用法决策指引。它以 **C++23 为基线**，覆盖 C++20/23 的现代特性取舍。

**核心定位声明**（写进 SKILL.md 开头）：

> 本 skill 是"判断层"，回答"该用哪个特性、怎么设计接口、所有权/错误处理/并发该怎么选"。它**不重复** `.clang-format`（格式）和 `.clang-tidy`（命名/静态检查）已经机器强制的内容——那些以项目本地 `.clang-*` 配置为唯一权威。本 skill 与二者互补。

### 范围内

- C++23 特性取舍（`std::expected` / `std::print` / `std::ranges::to` / `std::generator` / `std::out_ptr` / `std::source_location` / `std::stacktrace` / `if consteval` / deducing-this 等）
- 惯用法偏好（rule-of-five-or-zero、PImpl、copy-and-swap、`string_view`/`span` 只读形参、匿名命名空间内部链接、feature-test 宏门控 + fallback 范式）
- 设计决策（expected vs optional vs 异常、concept vs SFINAE、内存序、何时类型擦除/CRTP）
- 跨主题通用的"必守规则"

### 范围外（显式排除，避免与现有工具重复）

- 缩进/列宽/大括号/指针对齐等格式细节（`.clang-format` 已强制）
- 标识符大小写/前后缀等命名细节（`.clang-tidy readability-identifier-naming` 已强制）—— SKILL.md 仅给一行指针 + 速查
- 构建系统/CI/preset 细节（项目级，不通用）
- 第三方库 API 细节

---

## 2. 已确认决策

| 维度 | 决策 |
| --- | --- |
| Skill 位置/范围 | **用户级** `~/.claude/skills/cpp-coding-standards/`，便携，跨所有 C++ 项目可用 |
| 正文语言 | **中文** |
| 组织方式 | **按 C++ 领域主题**（以本仓库模块划分为素材/示例来源） |
| 与工具分工 | **聚焦判断/设计规则**，不重复 `.clang-*` 机器强制项 |
| 结构 | **方案 B：渐进式**——`SKILL.md`（核心）+ `references/`（按主题拆分） |
| 内容深度 | **惯用法级**——每主题：推荐 → 避免对照（短示例）→ 陷阱 → 何时偏离 |

### 便携性原则（重要）

skill 面向"任何 C++23 项目"，因此 ModernCpp 仓库的**特定**选择只作**示例**，不写成强制规则：

- 头文件守卫：通用规则"全项目统一一种保护方式（include guard 或 `#pragma once` 二选一）"；ModernCpp 的 `MCPP_<MODULE>_<FILE>_H` 方案作为示例引用
- 构建辅助：不纳入（`mcpp_add_demo` 等是项目私有）
- feature-test 宏门控范式：作为**范式**推荐（写法通用），具体的宏名（`__cpp_lib_print` 等）是标准、可通用

---

## 3. 文件结构

```text
~/.claude/skills/cpp-coding-standards/
├── SKILL.md                                  # 始终加载：定位 + 全局规则 + 决策速查 + 主题索引
└── references/
    ├── 01-basics-modern-core.md              # 现代 C++ 基础（enum/位运算/spaceship/缩写模板/指定初始化/属性）
    ├── 02-lifetime-type-safety.md            # 生命周期与类型安全（variant+visit/bit_cast/mixin/安全转型）
    ├── 03-headers-and-linkage.md             # 头文件与链接（守卫/前置声明/extern "C"/inline 变量/ODR/匿名命名空间）
    ├── 04-strings-and-io.md                  # 字符串与 IO（string_view/format+print/formatter 特化/osyncstream/spanstream）
    ├── 05-containers-and-ranges.md           # 容器与 ranges（span/map 惯用法/管道/ranges::to/fold/zip·chunk·slide/generator）
    ├── 06-templates-and-concepts.md          # 模板与 concept（requires/if consteval·consteval·constinit/CRTP/类型擦除/fold/CTAD/void_t）
    ├── 07-memory-and-ownership.md            # 内存与所有权（make_unique/自定义删除器/PImpl/PMR/out_ptr·inout_ptr）
    ├── 08-error-handling.md                  # 错误处理（expected/optional 单子/source_location/stacktrace/noexcept/copy-and-swap）
    ├── 09-concurrency.md                     # 并发（jthread/stop_token/原子量与内存序/协程）
    └── 10-move-semantics-and-values.md       # 移动语义与值类别（五之则/零之则/noexcept move/copy-and-swap/ref-qualifier/decltype(auto)）
```

10 个 `references/` 文件 = 把仓库 15 个模块按"关注点"聚合（而非按课时号），更贴合规范手册的检索习惯。映射关系：

| references 文件 | 对应仓库模块 |
| --- | --- |
| 01-basics-modern-core | 01_basics |
| 02-lifetime-type-safety | 02_lifetime_type_safety |
| 03-headers-and-linkage | 03_multi_file |
| 04-strings-and-io | 04_streams_strings |
| 05-containers-and-ranges | 05 + 06（containers_ranges_p1/p2） |
| 06-templates-and-concepts | 08 + 09（templates_basics/advanced） |
| 07-memory-and-ownership | 10_memory |
| 08-error-handling | 11_error_handling |
| 09-concurrency | 12 + 13（threading/concurrency_advanced） |
| 10-move-semantics-and-values | 07 + 14（value_categories/move_semantics） |
| （15_summary 的 chrono/filesystem/numbers/random 内容分散归入相关主题作为示例） | 15_summary |

---

## 4. SKILL.md 内容蓝图

### 4.1 Frontmatter

```yaml
---
name: cpp-coding-standards
description: 在编写、评审、重构 C++（C++20/23，以 C++23 为基线）代码时使用——尤其是需要在语言特性之间做选择、设计 API、做所有权/生命周期/错误处理/并发决策、或判断代码是否符合现代 C++ 惯用法时。本 skill 是"判断层"，覆盖 .clang-format / .clang-tidy 管不到的设计与惯用法决策；格式与命名细节以项目本地 .clang-* 配置为准。
---
```

> `description` 是触发命中的关键，须明确"何时用"并列出触发场景关键词（C++、API 设计、所有权、错误处理、并发、惯用法、重构、评审）。

### 4.2 正文章节

1. **定位声明**（范围内外，见上 §1）
2. **全局必守规则**（跨主题精华，~10 条；每条 1-2 句 + 必要时 1 行示例）：
   - C++23 基线；用到的 C++20/23 特性一律 feature-test 宏门控并提供 fallback
   - 只读字符串形参用 `std::string_view`；只读连续数组形参用 `std::span<const T>`
   - getter 与"忽略返回值即 bug"的函数加 `[[nodiscard]]`（必要时带理由 `[[nodiscard("reason")]])
   - 单参构造函数加 `explicit`
   - move 操作与叶子/constexpr 函数加 `noexcept`
   - 文件内部链接helper 用匿名 `namespace { }`，不用文件级 `static`
   - 禁用 `using namespace std;`；ranges 可用别名 `namespace stdv = std::views;`
   - 资源管理类遵循"五之则或零之则"，二选一，不要半套
   - 全项目统一一种头文件保护方式（include guard 或 `#pragma once`）
   - 头文件最小包含：能前置声明就不 `#include`
3. **决策速查表**（"遇到 X → 用 Y"，密度高、可扫读）：

   | 场景 | 首选 | 避免 |
   | --- | --- | --- |
   | 可失败且有错误信息的返回 | `std::expected<T,E>` | 出参指针 / 抛异常（业务错误）/ 裸 `bool`+out |
   | 可能为空的值 | `std::optional<T>` | 裸指针 / 哨兵值 |
   | 只读字符串形参 | `std::string_view` | `std::string const&`（无所有权时） |
   | 只读连续数组形参 | `std::span<const T>` | `(T*, n)` / `std::vector const&`（不需拷贝时） |
   | 格式化输出 | `std::format` / `std::print` | iostream 操纵符 / `printf` |
   | 惰性序列 | ranges 管道 / `std::generator` | 物化中间 `vector` |
   | 把 view 物化成容器 | `std::ranges::to<C>()` | 手写循环 `push_back` |
   | 折叠/累加 | `std::ranges::fold_left` | `std::accumulate` |
   | 跨 TU 共享全局 | `inline` 变量 | 头文件中定义非 inline 全局 |
   | C 资源持有 | `unique_ptr` + 自定义删除器 | 裸 `new`/`delete` / 手写 RAII 外壳 |
   | 智能 ptr 桥接 C out 形参 | `std::out_ptr` / `std::inout_ptr` | `.release()` + 手动重置 |
   | 隐藏实现、降编译依赖 | PImpl | 头文件暴露私有成员 |
   | 多态无虚函数开销 | CRTP / 类型擦除 | 模板爆炸 / 不必要的 `virtual` |
   | 线程 | `std::jthread` | `std::thread`（需手 join） |
   | 协作取消 | `std::stop_token` | 自制 atomic flag（首选标准设施） |
   | 跨线程日志输出 | `std::osyncstream` | 直接并发写 `std::cout` |
   | 编译期 vs 运行期分支 | `if consteval` / `if constexpr` | 运行时 `if` + trait |

4. **主题索引**（指示模型何时加载哪个 reference）：
   - 做生命周期/类型安全/`variant`/安全转型 → `references/02-lifetime-type-safety.md`
   - 写头文件、处理 include 耦合、跨 TU、`extern "C"` → `references/03-headers-and-linkage.md`
   - ……（每个 reference 一条触发描述）

5. **末尾指针**：提醒格式/命名以项目 `.clang-format` / `.clang-tidy` 为准；提醒"改了 C++ 源码后跑 format-check / tidy-check"作为可选 CI 指引（标注为"若项目有此约定"）。

---

## 5. references/*.md 统一模板

每个主题文件遵循同一结构（惯用法级深度）：

````text
# <主题名>

## 基线与心智模型
1-2 段：该主题在 C++23 下的核心取舍原则。

## ✅ 推荐 → ❌ 避免（对照）
- ✅ <推荐写法>  ❌ <避免写法>
  ```cpp
  <1-3 行示例>
  ```
  <一句 why>
（5-8 组对照）

## 常见陷阱
- <陷阱> → <正确做法>

## 何时偏离
- <例外情形>：<为何可以破例>
````

### 各文件要点（来自 Explore 模块惯用法调查）

**01-basics-modern-core**：`enum class`+显式底层类型+自由函数位运算符（`constexpr operator|`）、`using enum`、`<=>` 默认、缩写函数模板 `auto f(auto x)`、泛型 lambda、指定位分隔符 `'`、指定初始化 `{.x=}`、属性 `[[nodiscard]]`/`[[likely]]`/`[[maybe_unused]]`/`[[fallthrough]]`、`std::to_underlying`。

**02-lifetime-type-safety**：`std::variant`+`std::visit`+`Overloaded` helper（pack-expansion `using Fs::operator()...;`）、`std::monostate` 空态、`std::holds_alternative`/`std::get_if`、`std::bit_cast` 替代 `reinterpret_cast`、`dynamic_cast` 能力查询、纯接口 mixin 组合。

**03-headers-and-linkage**：守卫统一（include guard vs `#pragma once`）、前置声明降耦合（持不完整类型指针成员）、`extern "C"` + `#ifdef __cplusplus`、C++17 `inline` 变量跨 TU、Meyer 单例（函数局部 `static` + 删拷贝/移动）、匿名 `namespace` 内部链接（优于文件级 `static`）、声明与定义分离、默认参数只在声明、显式模板实例化。

**04-strings-and-io**：`std::string_view` 只读形参（生命周期注意）、`std::format`/`std::print`/`std::println`（C++23）、自定义 `std::formatter<T>` 特化（`parse`+`format`+`format_to` 到 `ctx.out()`）、`std::osyncstream` 并发写、`<spanstream>`（`std::span<char>` 上的流）、feature-test 门控 + fallback `main` 范式、`#elifndef`。

**05-containers-and-ranges**：`std::span`（`const T`/可写/静态 extent/`first·last·subspan`/`as_bytes`）、map 惯用法（`contains`/`try_emplace`/`insert_or_assign`/`extract`+改 key+重插）、结构化绑定遍历、`if (auto it=…; …)` init-statement、ranges 管道 `|`、`std::ranges::to<C>()`（C++23）、`fold_left` 家族、`zip`/`zip_transform`/`cartesian_product`/`pairwise`/`adjacent<N>`/`chunk`/`chunk_by`/`slide`/`stride`、`std::generator<T>` 协程惰性序列。

**06-templates-and-concepts**：自定义 concept + `requires` 表达式、约束形参 `template <Addable T>`、复合 `requires` 子句、`if constexpr` + `if consteval`（C++23）、`consteval`/`constinit`、万能引用 + `std::forward`、CRTP（`friend Derived;` + `static_cast<Derived*>(this)`）、手写类型擦除（concept/model + `unique_ptr<Concept>`）、折叠表达式（一元/二元、左/右）、CTAD + deduction guide、检测惯用法 `std::void_t`、C++20 NTTP（浮点/类类型/无状态 lambda）。

**07-memory-and-ownership**：`std::make_unique` / `make_unique_for_overwrite<T[]>(n)`（C++20）、自定义删除器 `unique_ptr`（C 资源如 `FILE*`+`fclose`）、PImpl（不完整类型 + `unique_ptr<Impl>` + 删拷贝 + `noexcept` defaulted move）、PMR（`monotonic_buffer_resource` / `unsynchronized_pool_resource` + `pmr::vector`）、`std::out_ptr`/`std::inout_ptr`（C++23）桥接智能 ptr 与 out 形参。

**08-error-handling**：`std::expected<T,E>`（`return std::unexpected(e)`；`if(r){*r}else{r.error()}`）、`std::optional` 单子链（`and_then`/`transform`/`or_else`，C++23）、`std::source_location` 默认实参惯用法、`std::stacktrace`（C++23，feature-test 门控）、`noexcept` 说明符 + `noexcept(expr)` 查询、copy-and-swap 赋值、五之则类。

**09-concurrency**：`std::jthread`（RAII 自动 join）+ `std::stop_token` 协作取消（`while(!st.stop_requested())`）、`stop_source`/`stop_callback`、`std::atomic` + 显式内存序（`relaxed` 只用于无发布语义的计数器；禁止用 relaxed 发布非原子数据）、`compare_exchange` 模式、协程（`promise_type` 全套 + `coroutine_handle` + RAII move-only wrapper + 自定义 Iterator/Sentinel；优先用 `std::generator` 而非手写）。

**10-move-semantics-and-values.md**：五之则（手写 dtor+copy+move+`swap()`+`using std::swap;`）vs 零之则（成员包进 `unique_ptr`）并陈、`noexcept` move 让 `vector` 重分配走 move（用 `MaybeThrowMove` 对照）、copy-and-swap 赋值、自赋值保护、ref-qualifier 重载（`& &`/`&&`/`const&`/`const&&`）、`decltype(auto)` 保留精确类型 vs `auto` 剥离、`static inline` 计数器探针。

---

## 6. 内容来源与可信度

- **风格/命名/检查**：直接来自仓库 `.clang-format`（6.3 KB）与 `.clang-tidy`（9.5 KB）——SKILL.md 仅引用，不重述。
- **惯用法素材**：来自 Explore agent 对 15 个模块的源码调查（76 次工具调用），每条都有对应仓库文件佐证（如 `expected_basics.cpp`、`pimpl_demo.cpp`、`coroutine_basics.cpp`、`rule_of_five_zero.cpp` 等）。
- **C++23 特性清单**：来自仓库实际使用的、经 feature-test 宏确认的特性（`std::expected`、`std::print`、`std::ranges::to`、`std::generator`、`std::out_ptr`、`std::source_location`、`std::stacktrace`、`if consteval`、`#elifndef`）。

---

## 7. 非目标 / 不做

- 不替代或重述 `.clang-format` / `.clang-tidy`
- 不绑定 ModernCpp 仓库特定约定为强制规则（便携）
- 不写构建系统/CI/包管理规范（项目级，已有 `docs/*-guide.md`）
- 不做完整的 C++ 教程（是规范，不是教材；示例只够说明规则）

---

## 8. 验收标准

1. skill 安装到 `~/.claude/skills/cpp-coding-standards/`，frontmatter 合法（`name`+`description`），可被 Skill 工具列出与触发
2. SKILL.md 控制在精简体量（目标 < ~350 行），始终加载不臃肿
3. 10 个 `references/*.md` 各自聚焦单一主题，遵循统一模板
4. 便携：不出现"必须用 `MCPP_*` 守卫 / `mcpp_add_demo`"等仓库私有强制项
5. 每条规则可追溯到仓库既有写法（示例）或 C++23 标准
6. 中文正文，代码示例标识符英文

---

## 9. 后续步骤

1. 用户评审本 spec
2. （文档类交付物建议跳过单独的 writing-plans 阶段，直接按本 spec 撰写）——或按 brainstorming 流程进入 writing-plans，由用户决定
3. 撰写 SKILL.md + 10 个 references/*.md
4. 自检：frontmatter 合法性、模板一致性、便携性、与 `.clang-*` 无重复
