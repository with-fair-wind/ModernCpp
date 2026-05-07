// 严格别名规则下哪些"类型双关"是合法的：
//   - std::byte / unsigned char 指针看任意对象。
//   - std::bit_cast / std::memcpy 在 trivially copyable 类型间做按位重读。
//   - 反例：用 reinterpret_cast<float&>(int) 读浮点位 = UB（这里只用文字提示）。
// 同时覆盖 C++23 std::start_lifetime_as_array（可用时）。

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <version>

#if defined(__cpp_lib_start_lifetime_as) && __cpp_lib_start_lifetime_as >= 202207L
#include <memory>
#endif

#include <gtest/gtest.h>

namespace {

struct Pixel {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;
};
static_assert(std::is_trivially_copyable_v<Pixel>);
static_assert(sizeof(Pixel) == 4);

}  // namespace

TEST(Aliasing, ByteViewIsAlwaysAllowed) {
    Pixel const px{.r = 0x12, .g = 0x34, .b = 0x56, .a = 0x78};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto const* bytes = reinterpret_cast<std::byte const*>(&px);
    EXPECT_EQ(std::to_integer<int>(bytes[0]), 0x12);
    EXPECT_EQ(std::to_integer<int>(bytes[1]), 0x34);
    EXPECT_EQ(std::to_integer<int>(bytes[2]), 0x56);
    EXPECT_EQ(std::to_integer<int>(bytes[3]), 0x78);
}

TEST(Aliasing, BitCastBetweenSameSizedTrivialTypes) {
    Pixel src{.r = 1, .g = 2, .b = 3, .a = 4};
    auto packed = std::bit_cast<std::uint32_t>(src);
    auto round = std::bit_cast<Pixel>(packed);
    EXPECT_EQ(round.r, 1);
    EXPECT_EQ(round.g, 2);
    EXPECT_EQ(round.b, 3);
    EXPECT_EQ(round.a, 4);
}

TEST(Aliasing, MemcpyIsTheSafePortableEquivalent) {
    Pixel src{.r = 0xAA, .g = 0xBB, .b = 0xCC, .a = 0xDD};
    std::uint32_t packed = 0;
    std::memcpy(&packed, &src, sizeof(packed));
    Pixel dst{};
    std::memcpy(&dst, &packed, sizeof(dst));
    EXPECT_EQ(dst.r, 0xAA);
    EXPECT_EQ(dst.g, 0xBB);
    EXPECT_EQ(dst.b, 0xCC);
    EXPECT_EQ(dst.a, 0xDD);
}

#if defined(__cpp_lib_start_lifetime_as) && __cpp_lib_start_lifetime_as >= 202207L
TEST(Aliasing, StartLifetimeAsArrayMakesObjectsLive) {
    constexpr std::size_t kCount = 3;
    auto raw = std::make_unique<std::byte[]>(sizeof(Pixel) * kCount);

    Pixel const seed[kCount]{
        {.r = 1, .g = 2, .b = 3, .a = 4},
        {.r = 5, .g = 6, .b = 7, .a = 8},
        {.r = 9, .g = 10, .b = 11, .a = 12},
    };
    std::memcpy(raw.get(), seed, sizeof(seed));

    Pixel* arr = std::start_lifetime_as_array<Pixel>(raw.get(), kCount);
    EXPECT_EQ(arr[0].r, 1);
    EXPECT_EQ(arr[1].g, 6);
    EXPECT_EQ(arr[2].b, 11);

    // 写入也合法
    arr[0].r = 99;
    EXPECT_EQ(arr[0].r, 99);
}
#endif
