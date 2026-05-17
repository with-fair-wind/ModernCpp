// 惰性实例化（lazy instantiation）：模板「成员」仅在 ODR 使用或需要完整类型时实例化。
// 未用到的成员可以包含「当前 T 下不成立」的代码，只要不触发实例化。

#include <iostream>

namespace {

// 普通平凡类型：未重载 operator+，`add` 仅在 ODR 使用时才会实例化并产生错误。
struct NoAdd {
    int tag{};
};

template <typename T>
struct MaybeMath {
    // 总是可用：不要求 T 具备 operator+。
    [[nodiscard]] static constexpr int sizeOf() noexcept {
        return static_cast<int>(sizeof(T));
    }

    // 成员函数模板：未调用时不会对 `U` 做返回类型代入，因而可安全与
    // 无 operator+ 的 `T` 共存；一旦对 `NoAdd` 调用 `add` 即会编译失败。
    template <typename U = T>
    [[nodiscard]] static auto add(U const& lhs, U const& rhs) -> decltype(lhs + rhs) {
        return lhs + rhs;
    }
};

}  // namespace

int main() {
    constexpr int kSz = MaybeMath<int>::sizeOf();
    std::cout << "sizeof(int) through lazy member: " << kSz << '\n';
    std::cout << "int add (成员被使用时才实例化): " << MaybeMath<int>::add(2, 3) << '\n';

    // NoAdd 不支持 operator+：若实例化 `add`，会在该成员定义处失败。
    // 仅调用 `sizeOf` 不会触发 `add` 的实例化（惰性）。
    constexpr int kPlainBytes = MaybeMath<NoAdd>::sizeOf();
    std::cout << "sizeof(NoAdd) without instantiating add: " << kPlainBytes << '\n';

    return 0;
}
