// 常用 filesystem 操作：创建目录树、复制、重命名、权限、查询大小与时间戳。

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>

namespace fs = std::filesystem;

namespace {

fs::path makeScratchRoot() {
    static std::mt19937_64 rng{std::random_device{}()};
    auto const stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    auto const suffix = static_cast<unsigned long long>(rng());
    return fs::temp_directory_path() /
           ("mcpp_summary_fs_demo_" + std::to_string(stamp) + "_" + std::to_string(suffix));
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    fs::path const root = makeScratchRoot();
    fs::path const nested = root / "nested";
    fs::path const src = nested / "hello.txt";
    fs::path const dst = nested / "hello_copy.txt";
    fs::path const renamed = nested / "hello_renamed.txt";

    fs::create_directories(nested);
    {
        std::ofstream out(src);
        out << "filesystem demo payload";
    }

    std::cout << "create_directories + 写入文件:\n  " << src << '\n';

    fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
    std::cout << "copy_file -> " << dst << '\n';

    fs::rename(dst, renamed);
    std::cout << "rename -> " << renamed << '\n';

    auto perm = fs::status(renamed).permissions();
    fs::permissions(renamed, perm | fs::perms::owner_write, fs::perm_options::add);

    std::cout << "file_size: " << fs::file_size(renamed) << " bytes\n";

    fs::file_time_type const t_before = fs::last_write_time(renamed);
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    {
        std::ofstream touch(renamed, std::ios::app);
        touch.put('\n');
    }
    fs::file_time_type const t_after = fs::last_write_time(renamed);
    std::cout << "last_write_time 写入后可观察到更新: "
              << (t_after != t_before ? "是" : "否（文件系统粒度可能合并）") << '\n';

    fs::remove(renamed);
    fs::remove(src);
    fs::remove(nested);
    fs::remove(root);
    std::cout << "清理临时目录完成。\n";

    return 0;
}
