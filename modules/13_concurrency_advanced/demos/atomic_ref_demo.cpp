// 模块 13：std::atomic_ref（C++20）——对「非原子」对象施加原子操作。
//
// 典型用途：复用已有布局的结构体字段做原子访问，或对接不支持原子类型的遗留代码。
// 调用方需保证同一对象的不同 atomic_ref 访问不与其他非原子访问交错数据竞争。

#include <atomic>
#include <iostream>

#if defined(__cpp_lib_atomic_ref) && __cpp_lib_atomic_ref >= 201806L

namespace {

struct Point {
    int x;
    int y;
};

void demoAtomicRefMember() {
    Point p = {.x = 0, .y = 0};
    std::atomic_ref<int> ref_x{p.x};
    ref_x.fetch_add(1, std::memory_order_relaxed);
    ref_x.fetch_add(2, std::memory_order_relaxed);
    std::cout << "atomic_ref 递增后 p.x = " << p.x << '\n';
}

}  // namespace

int main() {
    demoAtomicRefMember();
    return 0;
}

#else

int main() {
    std::cout << "当前工具链未提供 std::atomic_ref（缺 __cpp_lib_atomic_ref），跳过演示。\n";
    return 0;
}

#endif
