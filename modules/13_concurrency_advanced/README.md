# 13_concurrency_advanced — 并发进阶 / Advanced Concurrency

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/memory_order_seq_cst.cpp` | `memory_order_seq_cst` 全序一致性 |
| `demos/memory_order_acq_rel.cpp` | `acquire` / `release` 语义 |
| `demos/memory_order_relaxed.cpp` | `memory_order_relaxed` 宽松序 |
| `demos/atomic_operations.cpp` | `std::atomic` 基础操作：load / store / exchange / CAS |
| `demos/atomic_flag_spinlock.cpp` | `std::atomic_flag` 实现自旋锁 |
| `demos/atomic_ref_demo.cpp` | `std::atomic_ref`（C++20）非侵入式原子访问 |
| `demos/fence_demo.cpp` | `std::atomic_thread_fence` 内存栅栏 |
| `demos/coroutine_basics.cpp` | C++20 协程基础：`co_await` / `co_yield` / `co_return` |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_atomic_basic.cpp` | 原子操作正确性、CAS 循环 |
| `tests/test_atomic_flag.cpp` | `atomic_flag` 自旋锁互斥性 |
| `tests/test_atomic_ref.cpp` | `atomic_ref` 对普通变量的原子访问 |
| `tests/test_memory_order.cpp` | acquire-release 与 relaxed 内存序 |
| `tests/test_fence.cpp` | 栅栏同步验证 |
| `tests/test_coroutine.cpp` | 协程挂起 / 恢复 / 结果提取 |

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    13_concurrency_advanced__test_atomic_basic

# 跑本模块全部测试
ctest --preset gcc-debug -L 13_concurrency_advanced
```
