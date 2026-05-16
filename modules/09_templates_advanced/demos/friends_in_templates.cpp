// 类模板中的友元：`friend` 可声明函数模板、嵌套友元或整个类模板，
// 控制对私有实现的访问或实现 ADL 钩子。

#include <iostream>
#include <utility>

namespace {

template <typename T>
class SafeBox {
private:
    T value_{};

    // 友元函数模板：每个 `SafeBox<T>` 都有自己这一组友元实例，
    // 不是「一个友元管所有 T」。
    template <typename U>
    friend void reveal(SafeBox<U> const& box, std::ostream& out);

    template <typename U>
    friend class Auditor;

public:
    explicit SafeBox(T init) : value_(std::move(init)) {}
};

template <typename U>
void reveal(SafeBox<U> const& box, std::ostream& out) {
    out << box.value_;
}

template <typename T>
class Auditor {
public:
    static void peek(SafeBox<T> const& box) { std::cout << box.value_ << '\n'; }
};

}  // namespace

int main() {
    SafeBox<int> box{1337};
    std::cout << "ADL 友元模板: ";
    reveal(box, std::cout);
    std::cout << '\n';

    std::cout << "友元类模板: ";
    Auditor<int>::peek(box);

    return 0;
}
