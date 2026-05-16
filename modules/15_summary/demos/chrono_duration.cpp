// std::chrono::duration：多精度时长、算术与 duration_cast / 格式化输出。

#include <chrono>
#include <format>
#include <iostream>
#include <numbers>

namespace {

void printScaled(auto duration_value) {
    using namespace std::chrono_literals;
    auto const ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration_value);
    auto const sec =
        std::chrono::duration_cast<std::chrono::seconds>(duration_value);
    std::cout << std::format("  原始表示 {}, 折算毫秒 {}, 折算整秒 {}\n",
                             duration_value, ms.count(), sec.count());
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    using namespace std::chrono_literals;

    auto const two_hours = std::chrono::hours{2};
    auto const quarter_min = std::chrono::minutes{15};
    auto const pi_seconds = std::chrono::duration<double>{std::numbers::pi};
    auto const millis = 375ms;

    std::cout << "duration 示例:\n";
    printScaled(two_hours + quarter_min);
    printScaled(pi_seconds + millis);

    auto mixed = 90s + 500ms;
    mixed *= 2;
    mixed -= 10s;
    std::cout << "\n算术后 mixed（应为 170500ms）:\n";
    std::cout << std::format("  {}\n", mixed);

    std::cout << "\nstd::format 直接格式化 duration:\n";
    std::cout << std::format("  {}\n", 12min + 34s);

    return 0;
}
