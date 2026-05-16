// if constexpr：在实例化期丢弃分支；C++23 起 if consteval 区分立即与混合上下文。
//
// 典型用途：在单一模板里对不同类型走不同实现，而无需真正的运行期分支开销。

#include <iostream>
#include <string>
#include <type_traits>

namespace {

template <typename T>
constexpr auto storageBytes() {
    if constexpr (std::is_same_v<T, void>) {
        return std::size_t{0};
    } else if constexpr (std::is_integral_v<T>) {
        return sizeof(T);
    } else {
        return sizeof(T) * 2;  // 演示用的占位策略
    }
}

#if __cplusplus >= 202302L
// 常量求值语境走 consteval 分支；运行期调用走 else。
constexpr int scaleOrIdentity(int x) {
    if consteval {
        return x * 10;
    }
    return x;
}
#endif

}  // namespace

int main() {
    std::cout << "storageBytes<int> = " << storageBytes<int>() << '\n';
    std::cout << "storageBytes<std::string> = " << storageBytes<std::string>() << '\n';

    constexpr auto kVoidBytes = storageBytes<void>();
    static_assert(kVoidBytes == 0);
    std::cout << "storageBytes<void> (constexpr) = " << kVoidBytes << '\n';

#if __cplusplus >= 202302L
    static_assert(scaleOrIdentity(3) == 30);
    int runtime = 5;
    std::cout << "scaleOrIdentity(constexpr 2) compiled as " << 30
              << " vs runtime(5)=" << scaleOrIdentity(runtime) << '\n';
#endif

    return 0;
}
