# 12_threading — 多线程（新） / Threading (Modern)

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

> 整个模块受 `MCPP_HAS_STD_JTHREAD` 守卫（需要 libstdc++ 11+ / MSVC 17.0+ / libc++ 18+）。

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/thread_basics.cpp` | `std::thread` 创建、`join` / `detach` |
| `demos/jthread_basics.cpp` | `std::jthread` 自动 join、协作取消 |
| `demos/stop_token_demo.cpp` | `std::stop_token` / `stop_source` / `stop_callback` |
| `demos/mutex_lock_demo.cpp` | `std::mutex` / `lock_guard` / `unique_lock` / `scoped_lock` |
| `demos/shared_mutex_demo.cpp` | `std::shared_mutex` 读写锁 |
| `demos/condition_variable_demo.cpp` | `std::condition_variable` 等待通知 |
| `demos/semaphore_demo.cpp` | `std::counting_semaphore` / `binary_semaphore`（C++20） |
| `demos/latch_barrier_demo.cpp` | `std::latch` / `std::barrier`（C++20） |
| `demos/future_promise_demo.cpp` | `std::future` / `std::promise` / `std::async` |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_thread.cpp` | 线程创建与参数传递 |
| `tests/test_jthread.cpp` | `jthread` 自动 join 与 stop 请求 |
| `tests/test_stop_token.cpp` | stop 协议、`stop_callback` 触发 |
| `tests/test_mutex.cpp` | 互斥量与 RAII 锁 |
| `tests/test_condition_variable.cpp` | 条件变量唤醒与虚假唤醒防护 |
| `tests/test_semaphore.cpp` | 信号量计数与限流 |
| `tests/test_latch_barrier.cpp` | latch 一次性屏障 / barrier 可重用屏障 |
| `tests/test_future_promise.cpp` | `future` / `promise` 值传递与异常传播 |

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    12_threading__test_jthread

# 跑本模块全部测试
ctest --preset gcc-debug -L 12_threading
```
