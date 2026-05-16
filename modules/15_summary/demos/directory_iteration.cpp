// directory_iterator / recursive_directory_iterator：遍历目录项。

#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <system_error>

namespace fs = std::filesystem;

namespace {

fs::path makeSandboxRoot() {
    static std::mt19937_64 rng{std::random_device{}()};
    auto const salt = static_cast<unsigned long long>(rng());
    return fs::temp_directory_path() /
           ("mcpp_summary_dir_demo_" + std::to_string(salt));
}

void describeEntry(fs::directory_entry const& e) {
    std::error_code ec;
    auto const status = e.status(ec);
    char const* kind = "其他";
    if (ec) {
        kind = "状态未知";
    } else if (fs::is_directory(status)) {
        kind = "目录";
    } else if (fs::is_regular_file(status)) {
        kind = "普通文件";
    } else if (fs::is_symlink(status)) {
        kind = "符号链接";
    }
    std::cout << "  [" << kind << "] " << e.path().filename() << '\n';
}

void walkFlat(fs::path const& root) {
    std::cout << "directory_iterator @ " << root << ":\n";
    std::error_code ec;
    if (!fs::exists(root, ec) || ec) {
        std::cout << "  （路径不存在或不可访问）\n";
        return;
    }
    for (fs::directory_entry const& e : fs::directory_iterator(root, ec)) {
        if (ec) {
            std::cout << "  （迭代出错: " << ec.message() << "）\n";
            break;
        }
        describeEntry(e);
    }
}

void walkRecursive(fs::path const& root) {
    std::cout << "\nrecursive_directory_iterator @ " << root << ":\n";
    std::error_code ec;
    for (fs::directory_entry const& e : fs::recursive_directory_iterator(root, ec)) {
        if (ec) {
            std::cout << "  （迭代出错: " << ec.message() << "）\n";
            break;
        }
        describeEntry(e);
    }
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    fs::path const sandbox = makeSandboxRoot();
    struct Cleaner {
        fs::path path_;
        explicit Cleaner(fs::path p) : path_(std::move(p)) {}
        ~Cleaner() {
            std::error_code ec;
            fs::remove_all(path_, ec);
        }
        Cleaner(Cleaner const&) = delete;
        Cleaner& operator=(Cleaner const&) = delete;
        Cleaner(Cleaner&&) = delete;
        Cleaner& operator=(Cleaner&&) = delete;
    } guard{sandbox};

    fs::create_directories(sandbox / "inner");
    {
        std::ofstream top(sandbox / "readme.txt");
        top << "demo";
    }
    {
        std::ofstream leaf(sandbox / "inner" / "leaf.log");
        leaf << "nested";
    }

    std::cout << "临时沙箱目录（演示用）:\n";
    walkFlat(sandbox);
    walkRecursive(sandbox);

    std::cout << "\n系统临时目录（仅浅层遍历，避免递归整盘）:\n";
    walkFlat(fs::temp_directory_path());

    std::cout << "\n当前工作目录:\n";
    walkFlat(fs::current_path());

    return 0;
}
