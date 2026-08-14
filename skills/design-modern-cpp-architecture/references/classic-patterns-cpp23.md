# 经典设计模式的 C++23 实现

GoF 23 种经典模式描述反复出现的变化、协作和扩展问题，不规定必须使用继承，也不会因新语言特性出现而失效。先说明模式意图，再判断类型集合开放性、操作和类型的变化频率、替换发生在编译期还是运行期、对象是否跨 ABI，最后选择实现机制。参考仓库还收录了不属于 GoF 23 的 Null Object；将其与其他扩展惯用法分开处理。将参考仓库用作模式索引和教学素材，不把其中的双重检查单例、自制智能指针或早期协程示例直接当作生产模板；按目标 C++23 工具链、标准保证和项目约束重新设计。

## 目录

- [创建型模式](#创建型模式)
- [结构型模式](#结构型模式)
- [行为型模式](#行为型模式)
- [补充模式与 C++ 惯用法](#补充模式与-c-惯用法)
- [C++23 特性映射](#c23-特性映射)
- [常见误用](#常见误用)
- [来源](#来源)

## 创建型模式

| 模式 | 现代实现 | 使用条件 | 优先检查的替代方案 |
|---|---|---|---|
| Builder | 配置值对象、分阶段 builder、具名工厂；最终 `build()` 返回值或 `expected` | 参数多、存在构建阶段或跨字段不变量 | 聚合类型、指定初始化器、强类型参数 |
| Factory Method | 具名自由函数/静态函数，返回具体值、`unique_ptr<Base>` 或 `expected<T,E>` | 构造策略需要命名、验证、缓存或隐藏实现 | 直接构造、重载构造函数 |
| Abstract Factory | 一组相关产品的注入接口；开放集合用虚接口/类型擦除，封闭集合用 `variant` | 产品族必须一致替换 | 在组合根直接组装对象图 |
| Prototype | 值语义复制；开放层次提供 `clone()` 并返回 `unique_ptr` | 构造昂贵且模板实例确实能简化创建 | 工厂、不可变共享对象 |
| Singleton | 默认避免；在组合根拥有唯一实例并注入 | 进程唯一是外部资源约束，而非访问便利 | 显式所有权、函数参数、作用域服务 |

Builder 不应让无效对象长期存在。若构建失败是正常输入结果，使用 `std::expected<Product, BuildError>`；若 builder 只是给三个可选字段增加链式 setter，配置聚合通常更清晰。

Singleton 的函数局部静态初始化自 C++11 起线程安全，但这不解决隐式依赖、销毁顺序、测试替换和进程级可变状态问题。不要用双重检查锁定展示“现代”实现；优先由组合根决定实例数和生命周期。

## 结构型模式

| 模式 | 现代实现 | 关键权衡 |
|---|---|---|
| Adapter | 小型值包装器、自由函数、ranges view、受 `concept` 约束的泛型适配 | 明确所有权与借用；不要缓存悬空 `span/string_view` |
| Bridge | 组合 + 虚接口/类型擦除；Pimpl 隔离实现 | Bridge 表达两个独立变化维度；Pimpl 主要解决编译依赖/ABI |
| Composite | `unique_ptr` 拥有树；封闭节点用递归 `variant`；遍历用 ranges/generator | 防止父子所有权环和无界递归 |
| Decorator | callable 组合、动态包装器、policy/mixin、ranges adaptor | 明确顺序、异常、状态与重复装饰语义 |
| Facade | 稳定用例 API、模块接口或薄适配层 | Facade 不应成为全能 Service Locator |
| Flyweight | 不可变共享值、intern pool、稳定 ID | 并发访问、回收、内存上限和字符串视图生命周期 |
| Proxy | 拥有/非拥有句柄、延迟加载、缓存、远程端口 | 让隐藏的 I/O、阻塞、失败和一致性成本可见 |

Pimpl 通常使用 `std::unique_ptr<Impl>`，并在实现文件定义析构函数以确保 `Impl` 完整。若公共类型需要值语义，明确深复制、共享或禁止复制；不要默认用 `shared_ptr` 掩盖所有权。

Decorator 若在编译期固定且性能敏感，可用 policy/mixin；若需运行时重组，用类型擦除或抽象接口。对数据变换流水线，ranges view 往往比手写装饰器层次更直接。

## 行为型模式

| 模式 | C++23 首选表达 | 注意事项 |
|---|---|---|
| Chain of Responsibility | 明确的 handler pipeline，返回 `expected`/枚举表示处理、拒绝或继续 | 顺序、短路、错误聚合与观测必须可见 |
| Command | 具名值类型、封闭 `variant`，或 `move_only_function` 保存一次性任务 | 区分领域命令与任意回调；定义幂等、撤销和重试 |
| Interpreter | tokenizer/parser + `variant` AST + visitor | 复杂语法优先成熟解析器；错误需含位置和恢复策略 |
| Iterator | ranges、view、标准 iterator concepts、`std::generator` | `generator` 的工具链支持需验证；明确借用和失效 |
| Mediator | 显式应用服务/协调器，必要时加窄消息端口 | 全局总线会隐藏依赖和调用链 |
| Memento | 不可变值快照、版本化序列化、事件日志 | 控制快照大小、敏感数据和兼容性 |
| Observer | 作用域连接、弱生命周期跟踪、明确同步/异步派发 | 线程安全、重入、取消订阅与异常是核心契约 |
| State | 状态对象、`variant`/枚举 + 迁移函数、显式状态表或状态机库 | 区分 GoF State、有限/分层状态机和持久化工作流 |
| Strategy | 编译期 `concept`/policy；运行时 callable、类型擦除或虚接口 | 替换频率与 ABI 决定机制，不按偏好选择 |
| Template Method | 非虚接口（NVI）或受保护 hook；通常优先策略组合 | 继承耦合不应只为复用几行算法骨架 |
| Visitor | 虚 Visitor、`variant` + `visit`、重载/CPO 或类型擦除 | 保留操作扩展与双分派意图；依据类型集合开放性选择 |

命令、事件和任务不是同义词：命令表达期望执行的动作，事件表达已发生的事实，任务是执行单元。名称与 API 应保留这个差异。

### State 模式与状态机实现选择

不要把 GoF State 与所有状态机实现混为一谈：

- **GoF State**：状态对象封装状态相关行为，适合各状态拥有明显不同的行为、资源或局部数据，并需要在运行时切换委托对象。
- **有限状态机**：状态和事件集合封闭，迁移关系比状态内部算法更重要；优先枚举/`variant` 加显式迁移函数或迁移表。
- **分层状态机**：需要父子状态、进入/退出动作、历史状态或正交区域；优先采用经过验证的状态机库。
- **持久化工作流**：跨进程、长时间运行、需要恢复、补偿或人工介入；不要用普通进程内 State 对象假装满足持久化语义。

按下列顺序选择：

1. 写出状态、事件、守卫、动作、未处理事件和终止状态。
2. 确认状态是否拥有不同数据或资源，以及数据在转换时如何移动和销毁。
3. 状态和事件很少时使用 `enum class`/`variant` 与纯迁移函数，保持转换可穷举测试。
4. 行为随状态显著变化且状态对象需要独立封装时使用经典 State。
5. 迁移、守卫、层次或正交区域（orthogonal regions）较多时评估 Boost.SML、Boost.Statechart 或 MSM，不继续扩大手写 `switch`。
6. 明确副作用是在转换前、转换中还是转换后发生；让失败、取消、超时和重入语义可测试。

`std::variant` 只是表示封闭状态集合的工具，不自动提供状态机语义。协程也只改变等待过程的表达方式，不替代事件模型、迁移规则和持久化。

### Visitor 模式的实现选择

Visitor 的核心价值是把操作从对象结构中分离，并在需要时表达双分派。先比较“元素类型变化频率”和“操作变化频率”：

| 条件 | 合适实现 |
|---|---|
| 元素类型集合封闭，操作经常增加 | `std::variant` + `std::visit`，利用穷举检查 |
| 已有稳定虚对象层次，操作经常增加 | 经典虚 Visitor，集中新增操作 |
| 元素类型经常由第三方扩展 | 虚接口、acyclic visitor、注册机制或重新设计扩展边界；经典 Visitor 增加元素类型成本较高 |
| 算法只需编译期适配，不保存异构对象 | 受 `concept` 约束的重载、自由函数或 CPO |
| 需要隐藏具体类型、值语义或跨模块隔离 | 类型擦除；明确分配、复制和 ABI 成本 |

设计 Visitor 时明确：

- 返回值如何组合，失败是否使用 `expected`，是否允许提前终止；
- 遍历由对象结构、Visitor 还是外部算法负责；
- `const`/可变访问是否分离，是否允许 Visitor 保存跨节点状态；
- 新增元素类型或操作时需要修改哪些文件和重新编译哪些模块；
- 对象是否跨共享库 ABI，虚接口版本如何演进。

不要把 `concept` 本身称为 Visitor；只有当设计仍表达“针对不同元素选择相应操作”时，它才是 Visitor 意图的一种静态实现。对于 AST、序列化、代码生成和图形对象等稳定节点集合，经典 Visitor 或 `variant` Visitor 通常仍是清晰且有价值的选择。

## 补充模式与 C++ 惯用法

参考仓库中的 Null Object 是常用补充模式，但不属于 GoF 23。仅当默认对象具有清晰恒等行为、调用方无需区分“缺失”与“存在但无操作”时使用；若缺失本身影响控制流或错误处理，优先使用 `std::optional`、`std::expected` 或显式状态，避免静默吞掉错误。

Pimpl、policy-based design、CRTP、mixin、类型擦除和 CPO 是 C++ 惯用法或实现机制，也不应计入 GoF 23。使用它们承载已识别的边界或变化轴，不把语言技法重新包装成新的模式目录。

## C++23 特性映射

- `concept`/`requires`：为 policy、strategy、adapter 和 builder 阶段定义语义约束，替代难读的 SFINAE。
- `std::expected<T,E>`：工厂、解析、命令处理和边界适配中的可恢复失败。
- `std::variant`/`std::visit`：封闭状态、命令、AST、产品和访问者。
- `std::move_only_function`：拥有 move-only 捕获的一次性命令、回调或任务；不要需要复制时强行使用。
- 显式对象形参（deducing this）：减少某些 CRTP 样板，用于 mixin/接口转发；工具链与可读性优先。
- ranges/view：Adapter、Iterator、Decorator、Chain/Pipeline 的惰性组合。
- `std::generator`：同步惰性遍历；它不是异步流，也不提供背压。
- `std::span`、`string_view`、`mdspan`：非拥有数据视图；接口必须说明失效条件。
- `std::jthread`/`stop_token`：协作取消与线程所有权；不能代替完整任务系统。
- `std::atomic<std::shared_ptr<T>>`：少数读多写少快照场景；先评估普通锁和所有权简化。

不要为了“用上 C++23”重写稳定模式。新特性应减少非法状态、动态分配、样板或隐式契约，并由构建矩阵验证可用性。

## 常见误用

- 为每个类建立接口、工厂和 builder，却没有第二种实现或真实变化轴。
- 用 Service Locator、全局事件总线或单例隐藏依赖。
- 把异步 API 设计成同步回调加后台线程，却不定义取消和关闭。
- 把 `shared_ptr` 当作默认参数类型，使所有权图不可推理。
- 在公共 ABI 中暴露模板实例、标准容器或第三方框架类型，却声称 ABI 稳定。
- 以模板消除虚调用后未测量性能，却付出代码膨胀和构建时间成本。

## 来源

- 参考项目（C++11/14/17 示例素材）：<https://github.com/liuzengh/design-pattern>
- C++ Core Guidelines：<https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>
- `std::expected` 提案 P0323：<https://wg21.link/P0323>
- `std::move_only_function` 提案 P0288：<https://wg21.link/P0288>
- 显式对象形参提案 P0847：<https://wg21.link/P0847>
- `std::generator` 提案 P2502：<https://wg21.link/P2502>
