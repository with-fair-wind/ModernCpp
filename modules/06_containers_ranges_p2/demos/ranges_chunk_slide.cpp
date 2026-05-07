// 滑动窗口与分块：chunk / chunk_by / slide / adjacent。
//
// 关键点：
//   - chunk(width) 按宽度切片，最后一块可能更短。
//   - chunk_by(pred2) 在 pred2(*it, *next(it)) 为 false 时切开。
//   - slide(W) 与 adjacent<W> 都给出滑动窗口；区别在元素类型：
//     · slide  → 元素是 subrange<T>（仍是 range），需嵌套 for。
//     · adjacent<W> → 元素是 tuple<T&, ..., T&>，可结构化绑定，但不是 range。
//   - 每个产生 view 的 view 都得用嵌套 for：先取出子 view，再遍历它。

#include <functional>
#include <iostream>
#include <ranges>
#include <utility>
#include <vector>

namespace stdv = std::views;

namespace {

template <std::ranges::range Outer>
void dump2d(const char* tag, Outer&& outer) {
    std::cout << tag << ":\n";
    for (auto&& part : std::forward<Outer>(outer)) {
        std::cout << "  [";
        for (auto&& x : part) {
            std::cout << x << ' ';
        }
        std::cout << "]\n";
    }
}

}  // namespace

int main() {
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7};

    // 1) chunk(2)：每 2 个一段（最后一段可能不足）
    dump2d("chunk(2)         ", v | stdv::chunk(2));

    // 2) chunk_by(less)：当 a < b 不成立（这里指序列开始下降）时切开
    std::vector<int> w{1, 2, 5, 4, 3, 4};
    dump2d("chunk_by(less)   ", w | stdv::chunk_by(std::less<>{}));

    // 3) slide(3)：宽度为 3 的滑动窗口；每个窗口是 subrange
    dump2d("slide(3)         ", v | stdv::slide(3));

    // 4) adjacent<3>：滑动窗口宽度 3，但元素是 tuple<T&, T&, T&>
    std::cout << "adjacent<3>      :\n";
    for (auto const& [a, b, c] : v | stdv::adjacent<3>) {
        std::cout << "  (" << a << ',' << b << ',' << c << ")\n";
    }

    // 5) chunk + transform：每段求和——一种"批量归约"模式
    auto block_sums = v | stdv::chunk(2) | stdv::transform([](auto chunk_view) {
                          int s = 0;
                          for (auto x : chunk_view) {
                              s += x;
                          }
                          return s;
                      });
    std::cout << "block sums(2)    : ";
    for (auto s : block_sums) {
        std::cout << s << ' ';
    }
    std::cout << '\n';

    return 0;
}
