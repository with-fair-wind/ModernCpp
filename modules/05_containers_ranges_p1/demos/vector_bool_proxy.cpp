// vector<bool> 的代理引用（proxy reference）陷阱。
//
// 关键点：
//   - vector<bool> 是被压缩的位数组特化，operator[] 不能返回 bool&；
//     它返回的是一个代理对象，可以读、可以赋值，但它本身不是普通引用。
//   - 因此 `auto x = vb[i];` 推导出"代理"而非 bool，对它赋值会写回原 vector！
//     这是与 vector<int> 完全不同的行为，新手很容易踩坑。
//   - 相反，`auto& x = vb[i];` 不合法（代理是临时值，不能绑左值引用）。
//   - 一般建议：要修改用 `auto`（保留代理），要只读用 `bool` 或 `const auto&`
//     之外的方法（这里 `bool` 最稳）。
//   - vector<bool> 的迭代器也不是 contiguous_iterator。

#include <iostream>
#include <type_traits>
#include <vector>

int main() {
    std::vector<bool> vb{false, false, false};

    // 对 vector<int> 而言 auto 推导出 int，赋值不影响容器
    std::vector<int> vi{0, 0, 0};
    {
        auto x = vi[1];
        x = 42;
        std::cout << "vector<int>  vi[1]=" << vi[1] << " (auto 是值副本，写不回去)\n";
    }

    // 对 vector<bool>，auto 推导出代理；赋值"穿透"到位数组
    {
        auto bit = vb[1];  // 代理对象
        bit = true;
        std::cout << "vector<bool> vb[1]=" << vb[1] << " (auto 是代理，写回了)\n";
    }

    // 推导可视化：vb[i] 的类型不是 bool&
    using BoolRef = decltype(vb[0]);
    std::cout << "decltype(vb[0]) is bool& ? " << std::is_same_v<BoolRef, bool&> << '\n';

    // 想"只读"，最稳的做法是写明 bool（强制拷贝出真值）
    bool ro = vb[1];
    ro = false;
    std::cout << "after writing local bool, vb[1]=" << vb[1] << " (确实没改)\n";

    // flip 翻转所有位，代理也支持 ~/flip()
    vb.flip();
    std::cout << "after flip(): vb={" << vb[0] << ',' << vb[1] << ',' << vb[2] << "}\n";

    return 0;
}
