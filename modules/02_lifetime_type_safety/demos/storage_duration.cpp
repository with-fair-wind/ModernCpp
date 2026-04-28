// Smoke demo for module 02: storage durations and lifetime printing.
//
// Demonstrates static-vs-automatic storage and the construction order
// guaranteed by the language: static is initialised before main() runs,
// automatics in declaration order on entry to their block.

#include <iostream>
#include <string>
#include <utility>

namespace {

struct Tag {
    std::string name;
    explicit Tag(std::string n) : name(std::move(n)) {
        std::cout << "  ctor " << name << '\n';
    }
    Tag(Tag const&) = delete;
    Tag(Tag&&) = delete;
    Tag& operator=(Tag const&) = delete;
    Tag& operator=(Tag&&) = delete;
    ~Tag() {
        std::cout << "  dtor " << name << '\n';
    }
};

Tag const g_static{"static"};

}  // namespace

int main() {
    std::cout << "main entered\n";
    Tag const local{"automatic"};
    std::cout << "main leaving\n";
    return 0;
}
