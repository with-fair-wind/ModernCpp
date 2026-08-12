# 07 · 内存、资源与所有权

## 适用范围与核心原则

优先使用值语义和 RAII。让类型系统表达谁负责释放资源，不让异常、提前返回或并发路径绕过清理。

## 所有权与智能指针

- **禁止**用裸 `new` / `delete` 管理应用层所有权。优先直接成员、标准容器、`std::make_unique` 或工厂函数。
- 使用 `std::unique_ptr<T>` 表达动态独占所有权。仅在多个对象确实共同决定生命周期时使用 `std::shared_ptr<T>`。
- **必须**识别并打破 `std::shared_ptr` 环；观察关系通常用 `std::weak_ptr` 或受生命周期约束的裸指针/引用。
- 裸指针和引用默认非拥有。指针用于可空或可重绑定观察，引用用于必须存在且不可重绑定的借用。
- `std::make_unique_for_overwrite` 主要用于随后会完整写入的标量或平凡数组。对类类型仍会执行默认初始化；
  在标量或平凡数组的元素被明确写入前，不得读取其不确定值。

## 非内存资源与 C 边界

- 对 `FILE*`、库对象指针等指针型 C 句柄，可使用带自定义删除器的 `std::unique_ptr`。
- 对文件描述符、套接字整数、复合句柄或带特殊无效值的资源，应定义专用 RAII 类型；不要为了使用 `unique_ptr`
  扭曲句柄表示。
- 使用 `std::out_ptr` / `std::inout_ptr` 前，确认 C API 的输出、替换、失败和释放语义与智能指针匹配。
  适配器通常沿用智能指针现有删除器；额外实参会转发给智能指针的 `reset` 操作，必须符合对应智能指针的接口。

## PImpl 与分配资源

- 只有在稳定 ABI、隐藏第三方依赖或显著降低重编译成本时才应采用 PImpl；普通内部实现不必默认付出动态分配和间接访问成本。
- `std::unique_ptr<Impl>` 的宿主类型应在 `Impl` 完整的翻译单元中定义析构函数。需要显式移动操作时也应考虑在该处定义。
- 需要复制的 PImpl 类型必须明确深拷贝、共享实现或禁止复制；不要让接口使用者猜测。
- 对可证明的分配热点可以使用多态内存资源（PMR）。**必须**保证 `std::pmr::memory_resource` 比使用它的容器和
  分配对象存活更久。
- `std::pmr::monotonic_buffer_resource` 适合批量释放场景；它不会逐个回收分配。不要把 PMR 当成通用性能开关。

## 示例

```cpp
struct FileCloser
{
    void operator()(std::FILE* file) const noexcept
    {
        if (file != nullptr) {
            std::fclose(file);
        }
    }
};

using File = std::unique_ptr<std::FILE, FileCloser>;
```

## 审查重点

- 为每个资源标出创建、转移、共享、借用和释放位置。
- 检查自定义删除器、分配器（allocator）和内存资源（memory resource）的状态与生命周期。
- 检查智能指针是否只是用来掩盖不清晰的对象关系。
