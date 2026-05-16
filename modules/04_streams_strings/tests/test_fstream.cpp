// FileStream：临时文件读写与二进制 round-trip 的一致性。

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <type_traits>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

namespace {

constexpr std::uint32_t kMagic = 0xC0FFEEUL;
constexpr double kScalar = 1.5625;

struct Payload {
    std::uint32_t tag{};
    double value{};
};
static_assert(std::is_trivially_copyable_v<Payload>);

void wipe(fs::path const& path) noexcept {
    std::error_code ec{};
    fs::remove(path, ec);
}

}  // namespace

TEST(FileStream, TextLinesRoundTrip) try {
#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
    auto const path = fs::temp_directory_path() / "mcpp_test_fstream_lines.tmp";
    wipe(path);
    {
        std::ofstream writer(path, std::ios::out | std::ios::trunc);
        ASSERT_TRUE(writer.is_open());
        writer << "alpha\n42\n";
        writer.flush();
        ASSERT_TRUE(writer.good());
        writer.close();
    }
    {
        std::ifstream reader(path, std::ios::in);
        ASSERT_TRUE(reader.is_open());
        std::string headline{};
        std::getline(reader, headline);
        int boxed = {};
        ASSERT_TRUE(static_cast<bool>(reader >> boxed));
        EXPECT_EQ(headline, "alpha");
        EXPECT_EQ(boxed, 42);
    }
    wipe(path);
#else
    GTEST_SKIP() << "未启用 std::filesystem";
#endif
} catch (...) {
    FAIL();
}

TEST(FileStream, BinaryPodRoundTrip) try {
#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
    auto const path = fs::temp_directory_path() / "mcpp_test_fstream_pod.bin";
    wipe(path);

    Payload const outbound{.tag = kMagic, .value = kScalar};
    {
        std::ofstream writer(path, std::ios::binary | std::ios::trunc);
        ASSERT_TRUE(writer.is_open());
        // iostream 二进制写入需要字节视图；reinterpret_cast 与 NOLINT 必须在同一诊断行
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        writer.write(reinterpret_cast<char const*>(&outbound),
                     static_cast<std::streamsize>(sizeof(outbound)));
        ASSERT_TRUE(writer.good());
    }
    Payload inbound{};
    {
        std::ifstream reader(path, std::ios::binary);
        ASSERT_TRUE(reader.is_open());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reader.read(reinterpret_cast<char*>(&inbound),
                    static_cast<std::streamsize>(sizeof(inbound)));
        ASSERT_TRUE(reader.good());
    }
    EXPECT_EQ(inbound.tag, kMagic);
    EXPECT_DOUBLE_EQ(inbound.value, kScalar);
    wipe(path);
#else
    GTEST_SKIP() << "未启用 std::filesystem";
#endif
} catch (...) {
    FAIL();
}

TEST(FileStream, ReopenSameObjectClearsState) try {
#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
    auto const path = fs::temp_directory_path() / "mcpp_test_fstream_reopen.tmp";
    wipe(path);
    std::fstream bridge{};
    bridge.open(path, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(bridge.is_open());
    bridge << "ping";
    bridge.close();
    ASSERT_FALSE(bridge.is_open());

    bridge.open(path, std::ios::in);
    ASSERT_TRUE(bridge.is_open());
    std::string token{};
    bridge >> token;
    EXPECT_EQ(token, "ping");
    bridge.close();
    wipe(path);
#else
    GTEST_SKIP() << "未启用 std::filesystem";
#endif
} catch (...) {
    FAIL();
}

#if defined(__cpp_lib_ios_noreplace) || (__cplusplus >= 202302L)

TEST(FileStream, NoreplaceRefusesExistingFile) try {
#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
    auto const path = fs::temp_directory_path() / "mcpp_test_fstream_noreplace.tmp";
    wipe(path);
    {
        std::ofstream seeded(path, std::ios::out | std::ios::trunc);
        seeded << "seed\n";
    }
    std::ofstream challenger(path, std::ios::out | std::ios::noreplace);
    EXPECT_FALSE(challenger.is_open());
    wipe(path);
#else
    GTEST_SKIP() << "未启用 std::filesystem";
#endif
} catch (...) {
    FAIL();
}

#endif
