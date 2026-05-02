// Smoke demo for module 11: std::expected as a value-based error channel.
//
// std::expected<T, E> models "either a T or an E"; unlike exceptions, the
// failure path is part of the function signature and zero-cost on success.

#include <expected>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

namespace {

std::expected<int, std::string> parse_positive(std::string_view s) {
    if (s.empty()) {
        return std::unexpected("empty input");
    }
    constexpr int kMax = std::numeric_limits<int>::max();
    int value = 0;
    for (char const c : s) {
        if (c < '0' || c > '9') {
            return std::unexpected(std::string{"non-digit: "} + c);
        }
        int const digit = c - '0';
        // Reject before the multiply/add can overflow into UB.
        if (value > (kMax - digit) / 10) {
            return std::unexpected("overflow");
        }
        value = (value * 10) + digit;
    }
    if (value == 0) {
        return std::unexpected("zero is not positive");
    }
    return value;
}

}  // namespace

int main() {
    for (std::string_view const s : {"42", "", "12a", "0", "7", "9999999999"}) {
        auto const r = parse_positive(s);
        if (r) {
            std::cout << "ok    \"" << s << "\" -> " << *r << '\n';
        } else {
            std::cout << "error \"" << s << "\" -> " << r.error() << '\n';
        }
    }
    return 0;
}
