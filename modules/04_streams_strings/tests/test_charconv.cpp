// <charconv> 整数/浮点格式化往返与典型错误枚举。

#include <array>
#include <charconv>
#include <cstring>
#include <string_view>

#include <gtest/gtest.h>

TEST(stdCharConv, IntegerRoundtripBase10) {
    int constexpr kValue = 1337;
    std::array<char, 24> buf{};
    auto written = std::to_chars(buf.data(), buf.data() + buf.size(), kValue, 10);
    ASSERT_EQ(written.ec, std::errc{});

    int parsed = 0;
    auto parsed_view =
        std::string_view(buf.data(), static_cast<std::size_t>(written.ptr - buf.data()));
    auto read =
        std::from_chars(parsed_view.data(), parsed_view.data() + parsed_view.size(), parsed, 10);
    ASSERT_EQ(read.ec, std::errc{});
    EXPECT_EQ(parsed, kValue);
    EXPECT_EQ(read.ptr, parsed_view.data() + static_cast<std::ptrdiff_t>(parsed_view.size()));
}

TEST(stdCharConv, PartialParsePropagatesTrailingPointer) {
    std::string_view text{"12z34"};
    int value = 0;
    auto result = std::from_chars(text.data(), text.data() + text.size(), value, 10);
    ASSERT_EQ(result.ec, std::errc{});
    EXPECT_EQ(value, 12);
    EXPECT_EQ(result.ptr - text.data(), 2);
}

TEST(stdCharConv, BufferTooSmallForOutput) {
    std::array<char, 3> narrow{};
    int big = 9'999;
    auto r = std::to_chars(narrow.data(), narrow.data() + narrow.size(), big, 10);
    EXPECT_NE(r.ec, std::errc{});
}

TEST(stdCharConv, SupportsHexadecimalBase) {
    std::array<char, 30> encoded{};
    int value = 0xbeef;
    auto to = std::to_chars(encoded.data(), encoded.data() + encoded.size(), value, 16);
    ASSERT_EQ(to.ec, std::errc{});

    int roundtrip = 0;
    std::string_view slice(encoded.data(), static_cast<std::size_t>(to.ptr - encoded.data()));
    auto from = std::from_chars(slice.data(), slice.data() + slice.size(), roundtrip, 16);
    ASSERT_EQ(from.ec, std::errc{});
    EXPECT_EQ(roundtrip, value);
}

#if defined(__cpp_lib_to_chars) && (__cpp_lib_to_chars >= 201611L)

TEST(stdCharConv, FloatingWritesWithinBuffer) {
    double constexpr kTau = 6.28318530717958647692;
    std::array<char, 96> numeric{};
    auto r = std::to_chars(numeric.data(), numeric.data() + numeric.size(), kTau);
    ASSERT_EQ(r.ec, std::errc{});
    auto len = static_cast<std::size_t>(r.ptr - numeric.data());
    EXPECT_GT(len, 3U);

    double parsed_value = 0.0;
    std::string_view view(numeric.data(), len);
    auto f = std::from_chars(view.data(), view.data() + view.size(), parsed_value,
                             std::chars_format::general);
    ASSERT_EQ(f.ec, std::errc{});
    EXPECT_NEAR(parsed_value, kTau, 1e-3);
}

#endif
