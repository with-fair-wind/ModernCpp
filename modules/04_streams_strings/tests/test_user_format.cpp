// 自定义 POD 接入 std::format：验证 parse/format_to 协议的正确性。

#include <format>
#include <string>

#include <gtest/gtest.h>

namespace {

struct CpuSpec {
    std::string codename;
    unsigned cores{};
    double ghz{};
};

}  // namespace

template <>
struct std::formatter<CpuSpec> {
    constexpr auto parse(std::format_parse_context& ctx) noexcept -> decltype(ctx.begin()) {  // NOLINT(readability-convert-member-functions-to-static)
        const auto *it = ctx.begin();
        // 教学版：当前不识别附加格式说明，但需完整消费 format-spec 以满足 constexpr 诊断。
        while (it != ctx.end()) {
            ++it;
        }
        return it;
    }

    auto format(CpuSpec const& cpu, std::format_context& ctx) const -> decltype(ctx.out()) {  // NOLINT(readability-convert-member-functions-to-static)
        return std::format_to(ctx.out(), "{}:{}c@{:.2f}GHz", cpu.codename, cpu.cores, cpu.ghz);
    }
};

TEST(stdUserFormatter,ProducesReadableSummary) {
    CpuSpec const zen{.codename = "Zen4", .cores = 8U, .ghz = 4.515};
    auto text = std::vformat("{}", std::make_format_args(zen));
    EXPECT_NE(text.find("Zen4"), std::string::npos);
    EXPECT_NE(text.find('8'), std::string::npos);
    EXPECT_NE(text.find("GHz"), std::string::npos);
}

TEST(stdUserFormatter,HonorsReuseAcrossCalls) {
    CpuSpec const zen{.codename = "Zen4-lite", .cores = 6U, .ghz = 3.900};
    auto const left = std::vformat("{}", std::make_format_args(zen));
    auto const right = std::vformat("{}", std::make_format_args(zen));
    auto const first = left + std::string(" / ") + right;
    auto const first_hit = first.find("Zen4-lite");
    ASSERT_NE(first_hit, std::string::npos);
    EXPECT_NE(first.find("Zen4-lite", first_hit + 1), std::string::npos);
}

TEST(stdUserFormatter,PropagatesUnderlyingFields) {
    CpuSpec const zen{.codename = "Zen3", .cores = 12U, .ghz = 3.650};
    auto payload = std::vformat("{}", std::make_format_args(zen));
    EXPECT_NE(payload.find("12"), std::string::npos);
    EXPECT_NE(payload.find("3.65"), std::string::npos);
}
