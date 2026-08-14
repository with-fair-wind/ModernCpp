# 事件、消息与组件通信

先选择通信语义，再选择 API。信号、领域事件、命令总线、actor 邮箱和网络消息队列解决的问题不同。

## 目录

- [通信机制分类](#通信机制分类)
- [最小契约](#最小契约)
- [类型与所有权](#类型与所有权)
- [订阅生命周期](#订阅生命周期)
- [同步派发](#同步派发)
- [异步队列与背压](#异步队列与背压)
- [线程安全与重入](#线程安全与重入)
- [异常、取消和停机](#异常取消和停机)
- [请求响应与返回值](#请求响应与返回值)
- [1mZGG 案例评估](#1mzgg-案例评估)
- [测试矩阵](#测试矩阵)
- [来源](#来源)

## 通信机制分类

| 机制 | 典型语义 | 适用范围 |
|---|---|---|
| 直接调用 | 同步、单接收者、错误直接返回 | 依赖明确的组件协作 |
| Signal/Observer | 一对多通知，通常同步 | UI、对象状态通知、局部扩展 |
| 进程内事件总线 | 按类型/topic 解耦发布者与多个订阅者 | 应用内低耦合通知 |
| Command/Query Bus | 单个逻辑处理者，请求执行或查询 | 用例分发、装饰中间件 |
| 异步 mailbox/actor | 有队列、单元串行处理、生命周期监督 | 并发隔离、状态拥有者 |
| Broker/DDS/传输 | 跨进程/网络，序列化和交付策略 | 分布式系统、设备通信 |

能直接调用时优先直接调用。事件总线适合“不需要发布者知道接收者”的事实通知，不应替代所有应用服务调用。

## 最小契约

任何总线或 signal API 都必须回答：

- `emit/trigger` 是同步还是排队，在哪个线程调用处理器？
- 处理器按何种顺序运行，是否允许递归发布？
- 发布期间订阅/取消订阅何时生效？
- 处理器抛异常时停止、聚合、记录还是隔离？
- 消息是复制、移动、共享不可变对象还是借用？借用到何时有效？
- 队列是否有界，满时阻塞、拒绝、丢新、丢旧还是合并？
- 取消订阅是否等待正在执行的处理器结束？
- 总线关闭后 `post`、`flush` 和连接析构如何表现？

无法回答这些问题的“线程安全 EventBus”仍然没有可用契约。

## 类型与所有权

优先使用具名、强类型、不可变事实消息。事件名称使用过去式或完成语义，例如 `FrameAcquired`；命令使用祈使语义，例如 `StartCapture`。避免 `void*`、字符串 topic + `any` 作为默认接口。

- 小型消息按值传递。
- 大型不可变 payload 可用 `shared_ptr<const T>`，但记录分配与保留成本。
- `span`/`string_view` 只适合同步、调用期借用；不得进入异步队列，除非背后所有权另有保证。
- 跨线程消息尽量不可变，避免共享可变对象图。
- 跨进程消息使用稳定 schema、显式字段编号/版本，不依赖 C++ 对象布局或 RTTI。

## 订阅生命周期

返回 move-only 的 connection token，并支持 RAII 取消订阅。订阅者应持有 token；丢弃临时 token 导致立即退订应有测试和文档。

捕获裸 `this` 只在订阅生命周期严格短于对象时安全。跨线程派发时，简单 `weak_ptr.lock()` 仍需定义对象销毁与回调并发关系。可参考 Boost.Signals2 的 tracked slot：调用前取得临时强引用，过期则断开。

取消订阅通常不保证已经运行的处理器停止。若需要“disconnect 返回后绝不再调用”，必须增加执行计数/等待语义，并防止处理器内自退订造成死锁。

## 同步派发

同步 signal 的优点是顺序和错误传播直观，无队列延迟。缺点是慢订阅者直接阻塞发布者，并可能产生深度重入。

使用快照或 copy-on-write 处理派发期间连接变化时，明确语义：快照取得后取消的处理器是否仍可能执行。不要在持有订阅表锁时调用用户代码，否则容易死锁、优先级反转或阻塞所有连接操作。

## 异步队列与背压

“`post()` 后 `flush()`”只是延迟批处理，不自动等于异步；只有明确执行器/消费线程后才是异步系统。设计队列时决定：

- 每类型独立队列还是统一队列；
- FIFO 是每发布者、每 topic 还是全局保证；
- 容量和内存核算；
- 溢出策略是否按消息类型不同；
- 公平性、优先级、批量大小和饥饿；
- 慢消费者指标和告警。

高频状态更新常适合 coalescing/latest-value，而审计、交易或控制命令通常不能丢弃。不要用无界 `vector` 队列处理生产者可能快于消费者的路径。

## 线程安全与重入

分别声明：连接管理是否线程安全、多个发布者能否并发、同一处理器是否会并发、消息队列是否多生产者/多消费者。锁策略模板只能改变互斥实现，不能自动定义回调并发语义。

若组件状态只允许串行访问，考虑 executor/strand 或 actor mailbox，让同一逻辑单元的处理器不并发，而不是给每个字段加锁。Boost.Asio strand 保证绑定处理器串行执行，但所有相关 handler 都必须使用同一 strand。

## 异常、取消和停机

同步通知可选择首异常、聚合异常或隔离并记录；不要静默吞异常。异步系统不能把异常抛回发布调用栈，应通过错误事件、结果通道、监督策略或日志/指标处理。

停机顺序通常是：停止接受新消息 → 请求生产者停止 → 排空或按策略丢弃 → 等待处理器完成 → 断开订阅 → 销毁依赖。定义超时与强制终止路径。使用 `stop_token` 传递协作取消，但不要假设它能中断阻塞的第三方调用。

## 请求响应与返回值

多订阅者事件返回值常产生含糊语义。若需要一个回答，优先直接调用、query handler 或 actor request。确需 signal combiner 时说明：空订阅结果、处理顺序、短路、异常以及多个回答如何合并。

命令处理返回 `expected<Result, Error>` 时，不要把 retry、timeout 和 cancellation 都压缩成同一个错误码；这些语义影响调用方策略。

## 1mZGG 案例评估

参考实现位于 `include/dss/core/event_bus.h` 与 `detail/`，具备值得保留的设计点：

- 按消息类型分通道，公共事件使用具名 struct；
- move-only `ScopedConnection` 管理退订；
- 处理器列表采用快照/COW，调用用户代码时不持有列表写锁；
- 支持同步 `emit`、延迟 `post/flush`、锁策略和返回值 combiner；
- 用测试覆盖多个应用组件的发布/订阅。

抽取为跨项目框架前需重新确认：

- `NoLock` 默认是否与真实多线程使用一致；
- `std::function` 是否需要支持 move-only handler；
- `post` 队列无界增长、每通道 flush 顺序和公平性；
- 取消订阅与快照中的并发回调语义；
- 异常策略目前是继续调用后重抛首异常，是否适合所有消息；
- RTTI/`type_index` 跨 DSO 和禁用 RTTI 场景；
- Qt `emit` 宏兼容是否应留在核心库；
- 返回值事件、锁 policy 和自定义 delegate 是否增加了不必要的公共复杂度。

若只需要线程安全 signal，优先评估 Boost.Signals2；若需要轻量类型 dispatcher，评估 EnTT；若需要真正异步并发隔离，评估 Boost.Asio executor/strand 或 CAF，而不是继续扩展本地 `flush()`。

## 测试矩阵

- 无订阅、单订阅、多订阅及稳定顺序。
- 处理器内订阅、退订、自退订、递归发布和销毁总线。
- handler 抛异常、多个 handler 抛异常和异常后继续策略。
- move-only payload/callback、巨大 payload 和借用生命周期。
- 多生产者、多消费者、队列满、慢消费者和取消。
- disconnect 与正在执行 handler 竞争。
- 停机时新消息、排空超时、未消费消息和资源释放。
- sanitizer、压力测试、故障注入和延迟/内存基准。

## 来源

- 1mZGG 类型化事件框架：<https://github.com/with-fair-wind/1mZGG/tree/master/include/dss/core>
- Boost.Signals2 教程：<https://www.boost.org/doc/libs/latest/doc/html/signals2/tutorial.html>
- Boost.Signals2 设计理由：<https://www.boost.org/doc/libs/latest/doc/html/signals2/rationale.html>
- EnTT dispatcher：<https://skypjack.github.io/entt/classentt_1_1basic__dispatcher.html>
- Boost.Asio strand：<https://www.boost.org/doc/libs/latest/doc/html/boost_asio/overview/core/strands.html>
- CAF 消息传递：<https://actor-framework.readthedocs.io/en/latest/core/MessagePassing.html>
