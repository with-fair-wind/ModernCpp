// std::filesystem::path：构造、分解、拼接、规范化。
//
// 展示 path 如何用 / 运算符组合、如何用 parent_path / filename /
// extension / stem 拆解，以及 lexically_normal 的纯词法规范化。

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {

void printParts(fs::path const& p) {
    std::cout << "path        : " << p << '\n'
              << "parent_path : " << p.parent_path() << '\n'
              << "filename    : " << p.filename() << '\n'
              << "stem        : " << p.stem() << '\n'
              << "extension   : " << p.extension() << '\n';
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    fs::path const dir = fs::path("demo") / "data";
    fs::path p = dir / "report.final.txt";

    printParts(p);

    fs::path q = "scratch";
    q /= "out";
    q.append("item.log");
    std::cout << "\noperator/= + append:\n  " << q << '\n';

    fs::path const messy = fs::path("x") / ".." / "." / "y" / "z";
    std::cout << "\nlexically_normal:\n  raw: " << messy << '\n'
              << "  -> " << messy.lexically_normal() << '\n';

    fs::path const abs_style = fs::path("C:") / "tmp" / "a";
    std::cout << "\n词法拼接（示意）:\n  " << abs_style << '\n';

    return 0;
}
