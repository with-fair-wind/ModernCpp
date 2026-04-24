// auto / decltype(auto) deduction rules and a taste of structured bindings.
//
// Key takeaway:
//   auto strips top-level const and references by default, so to actually bind
//   a const reference you must spell it out: `const auto&`. `decltype(auto)`
//   on the other hand preserves the exact expression type.

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

namespace {

std::string const& source() {
    static std::string const s = "hello";
    return s;
}

}  // namespace

int main() {
    auto        a = source();  // std::string          (copy)
    auto const& b = source();  // std::string const&   (no copy)
    decltype(auto) c = source();  // std::string const&

    static_assert(std::is_same_v<decltype(a), std::string>);
    static_assert(std::is_same_v<decltype(b), std::string const&>);
    static_assert(std::is_same_v<decltype(c), std::string const&>);

    std::cout << "auto a       : " << a << '\n'
              << "const auto& b: " << b << '\n'
              << "decltype(auto) c: " << c << '\n';

    // Structured bindings unpack aggregates/tuples.
    std::pair p{42, std::string{"answer"}};
    auto const& [value, label] = p;
    std::cout << label << " = " << value << '\n';

    return 0;
}
