// `<system_error>`：error_code（平台相关）与 error_condition（语义/可移植）及 system_error。

#include <filesystem>
#include <iostream>
#include <system_error>

namespace fs = std::filesystem;

namespace {

void printDivider(char const* title) {
    std::cout << "\n=== " << title << " ===\n";
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    printDivider("构造与默认值");
    std::error_code ec_default;
    std::cout << "默认构造: bool(ec)=" << static_cast<bool>(ec_default)
              << " value=" << ec_default.value()
              << " message=\"" << ec_default.message() << "\"\n";

    auto ec_enoent = std::make_error_code(std::errc::no_such_file_or_directory);
    std::cout << "make_error_code(errc::no_such_file_or_directory): "
              << "bool=" << static_cast<bool>(ec_enoent) << " value=" << ec_enoent.value()
              << " message=\"" << ec_enoent.message() << "\"\n";

    auto ec_bad =
        std::error_code(static_cast<int>(std::errc::invalid_argument), std::generic_category());
    std::cout << "从 errc + generic_category: value=" << ec_bad.value()
              << " category=" << ec_bad.category().name() << '\n';

    printDivider("类别：generic_category 与 system_category");
    auto const& gen = std::generic_category();
    auto const& sys = std::system_category();
    std::cout << "generic_category.name(): \"" << gen.name() << "\"\n";
    std::cout << "system_category.name():  \"" << sys.name() << "\"\n";
    auto ec_gen = std::make_error_code(std::errc::no_such_file_or_directory);
    std::cout << "make_error_code(errc::...) 与 generic_category 相等："
              << (ec_gen.category() == gen) << '\n';

    printDivider("error_condition（平台无关语义）");
    std::error_condition cond =
        std::make_error_condition(std::errc::no_such_file_or_directory);
    std::cout << "cond.value=" << cond.value() << " cond.category=" << cond.category().name()
              << " cond.message=\"" << cond.message() << "\"\n";
    std::cout << "ec_gen == cond: " << (ec_gen == cond) << '\n';

    printDivider("比较与相等");
    std::error_code ec_copy = ec_gen;
    std::cout << "ec_gen == ec_copy: " << (ec_gen == ec_copy) << '\n';
    std::cout << "ec_gen != ec_default: " << (ec_gen != ec_default) << '\n';

    printDivider("system_error 异常");
    try {
        throw std::system_error{ec_gen, "附加上下文"};
    } catch (std::system_error const& ex) {
        std::cout << "捕获 system_error: what=\"" << ex.what() << "\" code=" << ex.code().value()
                  << '\n';
    }

    printDivider("与 std::filesystem 结合");
    fs::path const ghost = fs::temp_directory_path() / "mcpp_error_code_demo_nonexistent";
    std::error_code ec_fs;
    bool const removed = fs::remove(ghost, ec_fs);
    std::cout << "fs::remove(不存在的路径, ec): removed=" << removed
              << " bool(ec)=" << static_cast<bool>(ec_fs) << " value=" << ec_fs.value()
              << " message=\"" << ec_fs.message() << "\"\n";
    if (ec_fs) {
        std::cout << "可与 portable 条件比较: (ec_fs == cond)=" << (ec_fs == cond) << '\n';
    }

    printDivider("为何仍需了解这些工具");
    std::cout << "errc + generic_category 描述可移植错误语义；平台 API 常通过 error_code + "
                 "system_category 返回；两者可用 == 在语义层对齐。\n";

    return 0;
}
