// C++23 std::out_ptr / std::inout_ptr：把智能指针与“输出参数式”C API 对接。

#include <version>
#ifdef __cpp_lib_out_ptr

#include <cstdint>
#include <iostream>
#include <memory>

namespace {

// 模拟常见 C API：通过二级指针返回堆上资源。
void legacyOpen(std::int32_t** out_handle) {
    *out_handle = new std::int32_t{42};  // NOLINT(cppcoreguidelines-owning-memory)
}

void legacyRealloc(std::int32_t** inout_handle) {
    if (inout_handle != nullptr && *inout_handle != nullptr) {
        **inout_handle += 1;
    }
}

}  // namespace

int main() {
    std::unique_ptr<std::int32_t> holder;

    legacyOpen(std::out_ptr(holder));

    std::cout << "out_ptr 载入值: " << *holder << '\n';

    legacyRealloc(std::inout_ptr(holder));

    std::cout << "inout_ptr 调整之后: " << *holder << '\n';

    return 0;
}

#else

#include <iostream>

int main() {
    std::cout << "当前编译器标准库不提供 __cpp_lib_out_ptr（out_ptr/inout_ptr），跳过演示。\n";
    return 0;
}

#endif
