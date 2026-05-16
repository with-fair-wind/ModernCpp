// 自定义 POD 接入 std::format：验证 parse/format_to 协议的正确性。
//
// Apple libc++（Xcode ≤ 16）的 make_format_args 不识别自定义 formatter 特化，
// 会触发 static_assert——在该平台上跳过全部测试。

#include <version>

// Apple libc++ 的 make_format_args 尚不识别自定义 formatter 特化
#ifndef __APPLE__

#include <format>
#include <string>

#include <gtest/gtest.h>

namespace {

struct CpuSpec {
    std::string codename;
    unsigned cores{};
    unsigned mhz{};
};

}  // namespace

template <>
struct std::formatter<CpuSpec> {
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static) — std::formatter 协议要求非 static 成员
    constexpr auto parse(std::format_parse_context& ctx) noexcept -> decltype(ctx.begin()) {
        auto it = ctx.begin();  // NOLINT(readability-qualified-auto) — parse 遍历格式串需要就地迭代器类型
        while (it != ctx.end()) {
            ++it;
        }
        return it;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static) — std::formatter 协议要求非 static 成员
    auto format(CpuSpec const& cpu, std::format_context& ctx) const -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "{}:{}c@{}MHz", cpu.codename, cpu.cores, cpu.mhz);
    }
};

TEST(stdUserFormatter, ProducesReadableSummary) {
    CpuSpec const zen{.codename = "Zen4", .cores = 8U, .mhz = 4515U};
    auto text = std::vformat("{}", std::make_format_args(zen));
    EXPECT_NE(text.find("Zen4"), std::string::npos);
    EXPECT_NE(text.find('8'), std::string::npos);
    EXPECT_NE(text.find("MHz"), std::string::npos);
}

TEST(stdUserFormatter, HonorsReuseAcrossCalls) {
    CpuSpec const zen{.codename = "Zen4-lite", .cores = 6U, .mhz = 3900U};
    auto const left = std::vformat("{}", std::make_format_args(zen));
    auto const right = std::vformat("{}", std::make_format_args(zen));
    auto const first = left + std::string(" / ") + right;
    auto const first_hit = first.find("Zen4-lite");
    ASSERT_NE(first_hit, std::string::npos);
    EXPECT_NE(first.find("Zen4-lite", first_hit + 1), std::string::npos);
}

TEST(stdUserFormatter, PropagatesUnderlyingFields) {
    CpuSpec const zen{.codename = "Zen3", .cores = 12U, .mhz = 3650U};
    auto payload = std::vformat("{}", std::make_format_args(zen));
    EXPECT_NE(payload.find("12"), std::string::npos);
    EXPECT_NE(payload.find("3650"), std::string::npos);
}

#endif
