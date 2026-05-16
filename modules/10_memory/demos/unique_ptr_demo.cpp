// std::unique_ptr：独占所有权、自定义删除器、std::make_unique。

#include <cstdio>
#include <iostream>
#include <memory>
#include <utility>

namespace {

void demonstrateFileHandle() {
    if (FILE* raw = std::tmpfile(); raw != nullptr) {  // NOLINT(cppcoreguidelines-owning-memory)
        std::unique_ptr<FILE, int (*)(FILE*)> file(raw, &std::fclose);
        std::cout << "已获得临时 FILE*，离开作用域时将调用 fclose。\n";
    } else {
        std::cout << "tmpfile 创建失败（沙箱环境偶发），跳过句柄封装演示。\n";
    }
}

void demonstrateMove() {
    auto owner = std::make_unique<int>(2026);
    std::cout << "转移前: " << *owner << '\n';
    auto moved = std::move(owner);
    std::cout << "转移后原指针为空? " << (owner == nullptr ? "是" : "否") << '\n';
    std::cout << "新所有者: " << *moved << '\n';
}

}  // namespace

int main() {
    demonstrateFileHandle();
    demonstrateMove();

    auto array_owner = std::make_unique_for_overwrite<int[]>(4);
    for (int i = 0; i < 4; ++i) {
        array_owner[static_cast<std::size_t>(i)] = i * i;
    }
    std::cout << "make_unique_for_overwrite[0]=" << array_owner[0] << '\n';

    return 0;
}
