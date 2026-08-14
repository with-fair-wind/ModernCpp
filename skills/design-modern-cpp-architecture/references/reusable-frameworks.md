# 可复用框架与开源库选型

先按语义选类别，再按证据选库。表中项目是候选，不是默认依赖；采用前检查目标版本的许可证、平台、维护状态和 API。

## 目录

- [信号与进程内事件](#信号与进程内事件)
- [异步 IO 与执行](#异步-io-与执行)
- [Actor 与消息驱动](#actor-与消息驱动)
- [状态机](#状态机)
- [任务图与并行](#任务图与并行)
- [依赖注入](#依赖注入)
- [插件与 ABI](#插件与-abi)
- [跨进程消息](#跨进程消息)
- [自研门槛](#自研门槛)
- [选型记录](#选型记录)

## 信号与进程内事件

### Boost.Signals2

选择条件：需要成熟的 signal/slot、线程安全的连接管理与内部状态保护、scoped connection、对象生命周期跟踪和返回值 combiner。它是 header-only，但模板与 Boost 依赖会影响构建时间。它仍是同步调用模型；慢 slot 会阻塞 signal 调用者。并发调用同一 signal 时，combiner 和 slot 可能并发执行，业务回调不会被自动串行化。

### EnTT dispatcher/sigh

选择条件：已有 EnTT，或需要轻量、类型化的即时触发与按 tick 排队分发。确认当前版本的线程安全、连接 API、分配器和 DSO 边界；不要因只需要一个 observer 就引入完整 ECS 聚合头。

### 自研 typed bus

只在 API 很窄、线程模型简单、依赖政策严格且有能力长期维护时考虑。至少提供 RAII connection、具名消息、无锁外调用、明确的队列/异常/停机语义和完整压力测试。

## 异步 IO 与执行

### Boost.Asio

选择条件：网络、串口、计时器、异步 I/O、executor 和 strand 是核心需求。Asio 提供执行上下文和组合异步操作；协程可以改善表达，但必须继续设计取消、超时、buffer 生命周期和 handler executor。

若只需要通用 CPU 任务池，不要把 I/O 框架当作唯一选择；比较项目已有线程池、Taskflow 或 oneTBB。

## Actor 与消息驱动

### C++ Actor Framework（CAF）

选择条件：系统需要 actor 所有状态隔离、typed actor、邮箱、request/response、监督或分布式扩展。CAF 的语义和运行时明显重于简单事件总线，消息类型还受序列化/inspect 要求影响。

Actor 适合并发状态所有者，不应只是为了避免写一个互斥锁。评估邮箱背压、公平性、远程透明度风险、调试和团队学习成本。

## 状态机

### Boost.ext SML

选择条件：需要编译期状态表、guards/actions、层次/正交状态和较低运行时开销。它是独立项目，不是正式 Boost 库；默认不保证线程安全，应按版本检查 policy、异常与队列行为。

### Boost.Statechart / Boost.MSM

选择条件：项目已依赖 Boost 且需要更传统、功能完备的状态机方案。比较编译时间、错误信息、运行时开销和团队熟悉度。

小型、稳定状态集合优先手写显式表或 `variant` 状态；状态库的价值在于让复杂转换、守卫、层次和测试可见。

## 任务图与并行

### Taskflow

选择条件：工作负载能表达为依赖 DAG，需要动态任务、异构或可视化支持。不要把长期存活的业务服务生命周期硬塞进一次性任务图。

### oneTBB Flow Graph

选择条件：已有 oneTBB，且需要带并发控制的数据流节点。评估 buffer/backpressure 语义和调度可预测性。

简单固定并行算法先使用标准并行算法或项目现有执行设施；C++23 标准库没有通用 executor/结构化并发框架。

## 依赖注入

默认使用手工组合根：构造函数接收清晰依赖，`main`/应用上下文创建并连接对象。这样诊断、生命周期和调试最直观。

Boost.ext DI 可在对象图巨大、绑定组合复杂且团队接受模板诊断时评估。不要让容器泄漏到业务类，也不要以 Service Locator 方式在任意位置查询依赖。

## 插件与 ABI

动态库边界只解决装载与 ABI 契约，不构成安全隔离边界。同进程插件拥有宿主进程的内存访问能力和操作系统权限；对不可信或不同信任级别的插件，优先采用独立进程、操作系统沙箱和基于能力的 IPC 接口，并限制文件、网络、子进程、凭据和资源访问。

Boost.DLL 可辅助动态库加载和符号导入，但不会自动建立稳定 ABI。跨编译器/标准库/版本边界时优先：

- C ABI 工厂函数和显式版本号；
- 不透明句柄与成对创建/销毁；
- 明确分配器和异常不得跨边界；
- capability negotiation 和结构大小字段；
- 插件卸载前清理线程、回调和对象。

同一工具链内部的插件可使用窄 C++ 虚接口，但仍需测试 RTTI、异常、内存分配和依赖库版本。

## 跨进程消息

不要把进程内 `EventBus` 直接序列化后称为分布式总线。根据需求评估：

- **ZeroMQ/libzmq**：提供 socket 模式和传输抽象，不提供完整 broker、持久化或业务交付保证。
- **DDS（Cyclone DDS、Fast DDS）**：适合以数据为中心的实时发布-订阅、QoS、发现和设备/机器人领域；复杂度较高。
- **CAF remote actor**：系统已采用 actor 模型时评估。
- 专业 broker：需要持久化、消费组、确认和运维生态时，按组织基础设施选择，不在核心 C++ 类型中耦合客户端 API。

跨进程契约必须独立定义 schema、版本、身份、认证、大小限制、重试、幂等和观测。

## 自研门槛

满足以下多数条件才抽取公共框架：

- 至少两个独立消费者具有相同语义，而非仅相似类名；
- API 能在不引用首个项目领域类型的情况下表达；
- 已有契约测试、压力测试、基准和 sanitizer 结果；
- 有明确维护者、版本策略、兼容承诺和弃用流程；
- 第三方方案确实不匹配，理由已记录；
- 构建、打包、文档、示例和安全响应成本可承担。

不满足时，把实现留在项目内部，先通过真实使用稳定接口。

## 选型记录

每个候选至少记录：

| 项目 | 内容 |
|---|---|
| 必需能力 | 当前场景不可缺少的语义 |
| 不需要能力 | 防止被功能列表诱导 |
| 契约匹配 | 生命周期、线程、背压、错误、取消 |
| 工程成本 | 依赖、构建、体积、平台、工具链 |
| 生命周期 | 活跃度、发布、问题响应、治理 |
| 风险 | 许可证、供应链、锁定、迁移 |
| 证据 | 原型、基准、测试和失败实验 |

不要用 GitHub star 数替代技术匹配与维护评估。

## 官方资料

- Boost.Signals2：<https://www.boost.org/library/latest/signals2/>
- EnTT：<https://skypjack.github.io/entt/>
- Boost.Asio：<https://www.boost.org/doc/libs/latest/doc/html/boost_asio.html>
- CAF：<https://actor-framework.readthedocs.io/en/latest/>
- Boost.ext SML：<https://boost-ext.github.io/sml/>
- Taskflow：<https://taskflow.github.io/>
- oneTBB Flow Graph：<https://uxlfoundation.github.io/oneTBB/main/tbb_userguide/Flow_Graph.html>
- Boost.ext DI：<https://github.com/boost-ext/di>
- Boost.DLL：<https://www.boost.org/doc/libs/latest/doc/html/boost_dll.html>
- ZeroMQ：<https://zeromq.org/>
- Eclipse Cyclone DDS：<https://cyclonedds.io/>
- eProsima Fast DDS：<https://fast-dds.docs.eprosima.com/>
