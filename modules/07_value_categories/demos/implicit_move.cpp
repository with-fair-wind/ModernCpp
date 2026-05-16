// C++20/23 下的隐式移动：按值返回函数体中的局部自动对象时，
// 若未禁止复制/移动且未标记为 NRVO 候选失败，可把 return name; 视作
// std::move(name) 的语义（在值类别仍是 xvalue）。

#include <iostream>
#include <utility>

namespace {

struct Instrument {
    Instrument() = default;

    Instrument(Instrument const& other) : id_{other.id_} {
        std::cout << "copy ctor\n";
    }

    Instrument(Instrument&& other) noexcept : id_{other.id_} {
        std::cout << "move ctor\n";
    }

    Instrument& operator=(Instrument const&) = default;
    Instrument& operator=(Instrument&&) noexcept = default;

    ~Instrument() = default;

    int id_ = 0;
};

Instrument splitpath(bool left_branch) {
    Instrument lane{};
    lane.id_ = left_branch ? 1 : 2;
    // 双分支各自返回局部变量 → NRVO 往往不可用；编译器改用隐式移动。
    if (left_branch) {
        return lane;
    }
    Instrument other{};
    other.id_ = 3;
    return other;
}

}  // namespace

int main() {
    std::cout << "left_branch = true:\n";
    (void)splitpath(true);

    std::cout << "left_branch = false:\n";
    (void)splitpath(false);

    std::cout << "提示：典型输出会出现 move ctor（也可能进一步优化掉）。\n";
    return 0;
}
