// Smoke demo for module 11: std::expected as a value-based error channel.
//
// std::expected<T, E> models "either a T or an E"; unlike exceptions, the
// failure path is part of the function signature and zero-cost on success.

#include <expected>
#include <iostream>
#include <string>
#include <string_view>

namespace {

std::expected<int, std::string> parse_positive(std::string_view s) {
    if (s.empty()) {
        return std::unexpected("empty input");
    }
    int value = 0;
    for (char const c : s) {
        if (c < '0' || c > '9') {
            return std::unexpected(std::string{"non-digit: "} + c);
        }
        value = (value * 10) + (c - '0');
    }
    if (value == 0) {
        return std::unexpected("zero is not positive");
    }
    return value;
}

}  // namespace

int main() {
    for (std::string_view const s : {"42", "", "12a", "0", "7"}) {
        auto const r = parse_positive(s);
        if (r) {
            std::cout << "ok    \"" << s << "\" -> " << *r << '\n';
        } else {
            std::cout << "error \"" << s << "\" -> " << r.error() << '\n';
        }
    }
    return 0;
}
