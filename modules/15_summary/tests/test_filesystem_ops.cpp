// filesystem 高层操作：创建、复制、存在性、删除。

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

namespace {

fs::path makeUniqueRoot(std::string_view prefix) {
    static std::mt19937_64 rng{std::random_device{}()};
    auto const salt = static_cast<unsigned long long>(rng());
    return fs::temp_directory_path() /
           (std::string{prefix} + "_" + std::to_string(salt));
}

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
};

#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
void writeTextFile(fs::path const& file_path, std::string_view text) {
    std::ofstream writer(file_path);
    ASSERT_TRUE(writer.is_open());
    writer << text;
}

void assertExistsNonEmpty(fs::path const& file_path) {
    ASSERT_TRUE(fs::exists(file_path));
    ASSERT_GT(fs::file_size(file_path), static_cast<std::uintmax_t>(0));
}

void assertCopyPreservesSize(fs::path const& source, fs::path const& destination) {
    fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
    ASSERT_TRUE(fs::exists(destination));
    EXPECT_EQ(fs::file_size(source), fs::file_size(destination));
}

void removeAndExpectGone(fs::path const& file_path) {
    fs::remove(file_path);
    EXPECT_FALSE(fs::exists(file_path));
}

void runCopyThenRemoveLifecycle() {
    fs::path const root = makeUniqueRoot("mcpp_summary_fs_ops_test");
    Cleaner const cleaner{root};

    fs::path const original = root / "note.txt";
    fs::path const duplicate = root / "note_dup.txt";

    fs::create_directories(root);

    writeTextFile(original, "artifact");

    assertExistsNonEmpty(original);

    assertCopyPreservesSize(original, duplicate);

    removeAndExpectGone(duplicate);

    removeAndExpectGone(original);
}
#endif

}  // namespace

TEST(FileSystemOps, CopyThenRemoveLifecycle) try {
#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
    runCopyThenRemoveLifecycle();
#else
    GTEST_SKIP() << "未启用 std::filesystem";
#endif
} catch (...) {
    FAIL() << "filesystem 操作测试异常终止";
}
