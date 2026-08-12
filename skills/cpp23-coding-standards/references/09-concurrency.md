# 09 · 并发与协作取消

## 适用范围与核心原则

数据竞争是未定义行为。优先减少共享可变状态，再选择互斥、消息传递或原子操作；不要把无锁等同于更快或更安全。

## 线程、任务与取消

- 新建可汇合（join）的线程时应优先使用 `std::jthread`。其析构会对仍可汇合的线程先请求停止再汇合，因此可能阻塞。
  若接口承诺协作取消，任务应在有界时间内检查停止请求，或使用支持 `std::stop_token` 的等待操作；有界任务可以自然结束。
- 使用 `std::stop_token`、`std::stop_source` 和 `std::stop_callback` 表达协作取消。停止请求不会强制中断线程。
- 使用 `std::future` / `std::promise`、`std::packaged_task` 或 `std::async` 时明确所有权、异常传播和等待位置。
  调用 `std::async` 时显式选择执行策略，避免意外的延迟执行（deferred execution）。
- 协程本身不是线程或调度器；`co_await` 是否切换执行上下文由等待器（awaiter）和运行时决定。

## 锁与同步设施

- 对共享不变量应优先使用 `std::mutex` 与具名 RAII 锁对象。不要手工配对 `lock()` / `unlock()`。
- 同时获取多把互斥量时使用 `std::scoped_lock` 或 `std::lock` 避免锁顺序死锁，并保持全项目一致的锁层级。
- 读操作远多于写操作且测量表明确有收益时，可使用 `std::shared_mutex` 与 `std::shared_lock`。
- 条件变量等待**必须**使用谓词或等价循环处理虚假唤醒，并检查停止、超时和对象析构条件。
- 根据同步阶段使用 `std::counting_semaphore`、`std::latch`、`std::barrier` 和 `std::call_once`，不要用轮询循环重复实现。
- 不要在持锁期间调用未知回调、虚函数、用户代码或可能长时间阻塞的操作。

## 原子量与内存序

- 原子量适合独立状态、计数器和经过证明的同步协议；涉及多个字段的不变量通常仍需互斥量。
- 默认使用 `std::memory_order_seq_cst`。只有在建立清晰的先行发生（happens-before）证明并有测量收益后，
  才使用更弱内存序。
- 单独使用 `std::memory_order_relaxed` 只保证该原子对象的原子性，不建立线程间同步（synchronizes-with）或
  先行发生关系；高级协议仍可能通过 release sequence 或内存栅栏让 relaxed 操作参与同步。
- 比较并交换（CAS）循环通常使用 `compare_exchange_weak`；单次尝试可使用 `compare_exchange_strong`。
  必须正确处理 `expected` 实参被改写。
- **禁止**使用 `volatile` 进行线程同步；它主要用于特定硬件访问和实现约定。

## 审查重点

- 为每个共享对象标出同步方式、锁顺序、线程归属和销毁条件。
- 检查 `jthread` 析构、future 析构和条件等待是否可能造成未预期阻塞。
- 检查伪共享、争用和无锁算法回收问题；没有基准和证明时优先简单同步。
