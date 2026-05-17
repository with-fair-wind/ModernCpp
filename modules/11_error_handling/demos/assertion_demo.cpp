// 模块 11 的小演示：运行时 assert、编译期 static_assert 与 C++23 [[assume]]。

#include <cassert>
#include <iostream>

namespace {

constexpr int kBufferCap = 16;

void demoAssume(int x) {
#if defined(__has_cpp_attribute) && __has_cpp_attribute(assume)
    // [[assume]] 向优化器提示谓词为真；若违反则 UB —— 仅用于已知不变式。
    [[assume(x >= 0)]];
    std::cout << "[[assume]] 可用：假定 x>=0 后继续使用 x=" << x << '\n';
#else
    (void)x;
    std::cout << "当前编译器未声明 assume 属性，跳过 [[assume]] 演示。\n";
#endif
}

}  // namespace

int main() {
    std::cout << "--- static_assert（编译期）---\n";
    static_assert(kBufferCap > 0, "容量必须为正");
    std::cout << "kBufferCap = " << kBufferCap << " 已通过 static_assert。\n";

    std::cout << "\n--- assert（运行时，NDEBUG 下为空操作）---\n";
#ifndef NDEBUG
    std::cout << "当前非 NDEBUG：以下 assert 会检查条件。\n";
#else
    std::cout << "当前为 NDEBUG：assert 宏已禁用。\n";
#endif
    [[maybe_unused]] int n = 3;
    assert(n > 0 && "n 必须为正");

    std::cout << "\n--- [[assume]] ---\n";
    demoAssume(42);

    return 0;
}
