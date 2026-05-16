// 为自定义类型特化 std::formatter，接入 std::format 格式化管线。
//
// 关键点：
//   C++20 起用 parse/format 成员实现协议；format 应写入 ctx.out() 迭代器；
//   parse 遍历 format-spec（冒号后到 `}` 的子串）。

#include <cstdint>
#include <format>
#include <iostream>
#include <string>

namespace mcpp_demo {

class RgbColor {
public:
    constexpr RgbColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept
        : red_(red), green_(green), blue_(blue) {}

    [[nodiscard]] constexpr std::uint8_t red() const noexcept { return red_; }
    [[nodiscard]] constexpr std::uint8_t green() const noexcept { return green_; }
    [[nodiscard]] constexpr std::uint8_t blue() const noexcept { return blue_; }

private:
    std::uint8_t red_;
    std::uint8_t green_;
    std::uint8_t blue_;
};

}  // namespace mcpp_demo

template <>
struct std::formatter<mcpp_demo::RgbColor> {
    bool hex_mode_{false};

    constexpr auto parse(std::format_parse_context& ctx) {
        hex_mode_ = false;
        const auto *it = ctx.begin();
        for (; it != ctx.end(); ++it) {
            if (*it == 'h' || *it == 'H') {
                hex_mode_ = true;
            }
        }
        return it;
    }

    auto format(mcpp_demo::RgbColor const& color, std::format_context& ctx) const {
        auto out = ctx.out();
        if (hex_mode_) {
            return std::format_to(out, "#{:02X}{:02X}{:02X}", static_cast<unsigned>(color.red()),
                                  static_cast<unsigned>(color.green()), static_cast<unsigned>(color.blue()));
        }
        return std::format_to(out, "rgb({}, {}, {})", static_cast<unsigned>(color.red()),
                              static_cast<unsigned>(color.green()), static_cast<unsigned>(color.blue()));
    }
};

namespace {

void printSep() { std::cout << "---\n"; }

void demoRgb() {
    mcpp_demo::RgbColor teal{32, 178, 170};
    // GCC libstdc++ 会在编译期扫描自定义 formatter；改用运行时 vformat 避免误判花括号匹配。
    std::cout << "默认：" << std::vformat("{}\n", std::make_format_args(teal));
    std::cout << std::vformat("{:h}\n", std::make_format_args(teal));  // h 令牌切换十六进制短格式
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    demoRgb();
    printSep();
    std::cout << "可把 parse 扩展到支持 alpha（{:ha}）等团队约定令牌。\n";
    return 0;
}
