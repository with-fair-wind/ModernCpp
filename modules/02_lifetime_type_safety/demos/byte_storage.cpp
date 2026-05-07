// std::byte / unsigned char 缓冲区与 C++23 std::start_lifetime_as(_array)。
//
// 关键点：
//   - std::byte（C++17，<cstddef>）是强类型枚举类，专门表示"一个字节"，比
//     unsigned char 更不易与算术类型混用。
//   - 通过 std::byte 或 unsigned char 指针 "看" 任何对象都是合法（严格别名规则
//     的明确豁免），但反过来不一定。
//   - C++20 给 std::malloc / std::calloc / 分配器加了 "隐式开始生命周期" 的口子；
//     但写自己的内存池时，仍可能需要显式调用 C++23 的
//     std::start_lifetime_as<T>(void*) 或 std::start_lifetime_as_array<T>(void*, n)
//     才能合法访问。
//   - 本 demo 在编译期检测 __cpp_lib_start_lifetime_as；不可用则跳过那一段。

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <version>

#if defined(__cpp_lib_start_lifetime_as) && __cpp_lib_start_lifetime_as >= 202207L
#include <memory>
#endif

namespace {

struct Pixel {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;
};

}  // namespace

int main() {
    std::cout << "[1] std::byte 的强类型语义\n";
    {
        std::byte b{0x3F};
        b |= std::byte{0x40};  // 按位运算保持 std::byte 类型
        std::cout << "  b = 0x" << std::hex << std::to_integer<int>(b) << std::dec << "\n";
    }

    std::cout << "\n[2] 用 std::byte* 观察任意对象（严格别名豁免）\n";
    {
        Pixel const px{.r = 0x12, .g = 0x34, .b = 0x56, .a = 0x78};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto const* bytes = reinterpret_cast<std::byte const*>(&px);
        std::cout << "  raw bytes = ";
        for (std::size_t i = 0; i < sizeof(Pixel); ++i) {
            std::cout << std::hex << std::to_integer<int>(bytes[i]) << ' ';
        }
        std::cout << std::dec << "\n";
    }

    std::cout << "\n[3] memcpy 在平凡可拷贝类型间做按位等价\n";
    {
        Pixel const src{.r = 1, .g = 2, .b = 3, .a = 4};
        std::uint32_t packed = 0;
        std::memcpy(&packed, &src, sizeof(packed));
        Pixel dst{};
        std::memcpy(&dst, &packed, sizeof(dst));
        std::cout << "  round-trip dst = (" << +dst.r << "," << +dst.g << "," << +dst.b << ","
                  << +dst.a << ")\n";
    }

    std::cout << "\n[4] C++23 std::start_lifetime_as_array （可用时）\n";
#if defined(__cpp_lib_start_lifetime_as) && __cpp_lib_start_lifetime_as >= 202207L
    {
        constexpr std::size_t kCount = 4;
        auto raw = std::make_unique<std::byte[]>(sizeof(Pixel) * kCount);
        Pixel const seed[kCount]{
            {.r = 1, .g = 2, .b = 3, .a = 4},
            {.r = 5, .g = 6, .b = 7, .a = 8},
            {.r = 9, .g = 10, .b = 11, .a = 12},
            {.r = 13, .g = 14, .b = 15, .a = 16},
        };
        std::memcpy(raw.get(), seed, sizeof(seed));

        // 直接 reinterpret_cast 后访问是 UB（无 Pixel 对象处于生命周期内）；
        // start_lifetime_as_array 让 kCount 个 Pixel 隐式开始生命周期。
        Pixel* arr = std::start_lifetime_as_array<Pixel>(raw.get(), kCount);
        for (std::size_t i = 0; i < kCount; ++i) {
            std::cout << "  arr[" << i << "] = (" << +arr[i].r << "," << +arr[i].g << ","
                      << +arr[i].b << "," << +arr[i].a << ")\n";
        }
    }
#else
    std::cout << "  当前 stdlib 未实现 std::start_lifetime_as —— 跳过\n";
#endif

    return 0;
}
