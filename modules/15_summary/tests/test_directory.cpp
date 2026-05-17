// 目录迭代：在临时目录树中创建若干文件并校验遍历结果。

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

namespace {

fs::path makeUniqueRoot(std::string_view prefix) {
    static std::mt19937_64 rng{std::random_device{}()};
    auto const salt = static_cast<unsigned long long>(rng());
    return fs::temp_directory_path() / (std::string{prefix} + "_" + std::to_string(salt));
}

void touchFile(fs::path const& file, std::string_view payload) {
    fs::create_directories(file.parent_path());
    std::ofstream out(file);
    ASSERT_TRUE(out.is_open());
    out << payload;
}

int countFlat(fs::path const& dir) {
    int n = 0;
    std::error_code ec;
    for ([[maybe_unused]] fs::directory_entry const& ignored : fs::directory_iterator(dir, ec)) {
        ++n;
        (void)ignored;
    }
    EXPECT_FALSE(ec);
    return n;
}

int countRecursive(fs::path const& dir) {
    int n = 0;
    std::error_code ec;
    for ([[maybe_unused]] fs::directory_entry const& ignored :
         fs::recursive_directory_iterator(dir, ec)) {
        ++n;
        (void)ignored;
    }
    EXPECT_FALSE(ec);
    return n;
}

}  // namespace

TEST(DirectoryIteration, ListsCreatedEntries) try {
#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
    fs::path const root = makeUniqueRoot("mcpp_summary_dir_test");
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
    } cleaner{root};

    touchFile(root / "readme.txt", "x");
    touchFile(root / "nested" / "data.bin", "y");

    EXPECT_EQ(countFlat(root), 2);
    EXPECT_EQ(countRecursive(root), 3);

    std::vector<std::string> names;
    for (fs::directory_entry const& e : fs::directory_iterator(root)) {
        names.push_back(e.path().filename().string());
    }
    std::ranges::sort(names);
    ASSERT_EQ(names.size(), 2U);
    EXPECT_EQ(names[0], "nested");
    EXPECT_EQ(names[1], "readme.txt");
#else
    GTEST_SKIP() << "未启用 std::filesystem";
#endif
} catch (...) {
    FAIL() << "目录迭代测试异常终止";
}
