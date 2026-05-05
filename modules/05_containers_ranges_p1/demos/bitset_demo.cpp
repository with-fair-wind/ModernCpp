// std::bitset：编译期固定长度的位数组，支持位运算与字符串/整数互转。
//
// 关键点：
//   - bitset 的尺寸是模板参数，在编译期确定。
//   - 与 vector<bool> 不同，bitset 不暴露迭代器；它把"位串"作为一等公民，
//     直接提供 `&|^~`、`<<` / `>>`、count / any / all / none、to_string / to_ullong。
//   - operator[] 同样返回代理（const 版本返回 bool）。
//   - 位下标 0 是最低位（"右边"），与 to_string 的输出顺序相反。

#include <bitset>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>

int main() {
    constexpr std::size_t kN = 8;
    std::bitset<kN> a{0b1010'1100U};

    std::cout << "a            = " << a << " (count=" << a.count() << ")\n";
    std::cout << "a[0]=" << a[0] << " a[7]=" << a[7] << " (位 0 是最低位，与字符串显示左右相反)\n";

    // 位运算
    std::bitset<kN> b{0b0000'1111U};
    std::cout << "a & b        = " << (a & b) << '\n';
    std::cout << "a | b        = " << (a | b) << '\n';
    std::cout << "a ^ b        = " << (a ^ b) << '\n';
    std::cout << "~a           = " << (~a) << '\n';
    std::cout << "a << 2       = " << (a << 2) << '\n';

    // 单位修改
    a.set(0);    // 将位 0 置 1
    a.reset(7);  // 将位 7 置 0
    a.flip(1);   // 翻转位 1
    std::cout << "after set/reset/flip: " << a << '\n';

    // 整体查询
    std::cout << "any=" << a.any() << " all=" << a.all() << " none=" << a.none() << '\n';

    // 边界检查：test 抛 out_of_range；operator[] 不会
    try {
        (void)a.test(99);
    } catch (const std::out_of_range& e) {
        std::cout << "test(99) threw: " << e.what() << '\n';
    }

    // 互转
    std::cout << "to_string()  = " << a.to_string() << '\n';
    std::cout << "to_ullong()  = " << a.to_ullong() << '\n';

    // 由字符串构造（'0'/'1' 可自定义）
    std::bitset<8> c{std::string{"xx11oo11"}, 0, std::string::npos, 'o', 'x'};
    std::cout << "c (from str) = " << c << '\n';

    return 0;
}
