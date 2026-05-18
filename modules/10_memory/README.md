# 10_memory — 内存管理 / Memory Management

## 文档 / Docs

- 中文: [`docs/zh-CN.md`](docs/zh-CN.md)
- English: [`docs/en-US.md`](docs/en-US.md)

## 内容 / Contents

### Demos

| 文件 | 主题 |
| ---- | ---- |
| `demos/object_layout.cpp` | 对象内存布局、成员对齐、填充 |
| `demos/alignment_demo.cpp` | `alignof` / `alignas`、过对齐类型 |
| `demos/new_delete.cpp` | `operator new` / `delete` 重载与类域分配 |
| `demos/unique_ptr_demo.cpp` | `std::unique_ptr` 独占所有权、自定义删除器 |
| `demos/shared_ptr_demo.cpp` | `std::shared_ptr` 共享所有权、`make_shared` |
| `demos/weak_ptr_demo.cpp` | `std::weak_ptr` 打破循环引用 |
| `demos/pimpl_demo.cpp` | PImpl 惯用法（`unique_ptr` 实现） |
| `demos/allocator_basics.cpp` | `std::allocator` 接口与自定义分配器 |
| `demos/pmr_demo.cpp` | `std::pmr` 多态内存资源 |
| `demos/smart_ptr_adaptors.cpp` | `enable_shared_from_this`、`owner_less`、指针类型转换 |
| `demos/false_sharing_demo.cpp` | 伪共享与 `hardware_destructive_interference_size` |

### Tests

| 文件 | 覆盖点 |
| ---- | ---- |
| `tests/test_layout.cpp` | 对象布局、sizeof、offsetof |
| `tests/test_alignment.cpp` | 对齐要求与过对齐分配 |
| `tests/test_new_delete.cpp` | 类域 `operator new` / `delete` 拦截 |
| `tests/test_unique_ptr.cpp` | `unique_ptr` 所有权转移、自定义删除器 |
| `tests/test_shared_ptr.cpp` | `shared_ptr` 引用计数、别名构造 |
| `tests/test_weak_ptr.cpp` | `weak_ptr` 过期检测、`lock()` |
| `tests/test_allocator.cpp` | 自定义分配器与容器适配 |
| `tests/test_pmr.cpp` | PMR 资源切换与内存池 |

## 运行 / Run

```bash
# 构建本模块的某个测试（任选一个 preset）
cmake --build --preset gcc-debug --target \
    10_memory__test_unique_ptr

# 跑本模块全部测试
ctest --preset gcc-debug -L 10_memory
```
