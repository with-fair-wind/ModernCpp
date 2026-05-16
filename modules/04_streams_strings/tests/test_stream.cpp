// ostringstream / istringstream 解析与可选的 std::filesystem 临时文件回放。

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

TEST(stdStreams, StringStreamCompose) {
    std::ostringstream builder;
    builder << "sum=" << 4 + 5;
    EXPECT_EQ(builder.str(), "sum=9");
    EXPECT_TRUE(builder.good());
}

TEST(stdStreams, InvalidIntegerParseSetsFailBit) {
    std::istringstream broken{"oops"};
    int candidate = -1;
    broken >> candidate;
    EXPECT_TRUE(broken.fail());
    EXPECT_FALSE(broken.good());
}

TEST(stdStreams, ParsesIntegerThenAdjacentToken) {
    std::istringstream digits{"42 answers"};
    int value = 0;
    ASSERT_TRUE(static_cast<bool>(digits >> value));
    std::string word;
    ASSERT_TRUE(static_cast<bool>(digits >> word));
    EXPECT_EQ(value, 42);
    EXPECT_EQ(word, "answers");
}

TEST(stdStreams, StringStreamSeekAndPatch) {
    std::stringstream editor{"-----"};
    editor.seekp(2);
    editor << "##";
    EXPECT_EQ(editor.str(), "--##-");
}

TEST(stdStreams, TempFileRoundTrip) try {
#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
    auto path = fs::temp_directory_path() / "mcpp_test_stream.tmp";
    {
        std::ofstream writer(path);
        ASSERT_TRUE(writer.is_open());
        writer << "alpha\n42 3.5\n";
    }
    {
        std::ifstream reader(path);
        ASSERT_TRUE(reader.is_open());
        std::string header;
        std::getline(reader, header);
        int iv = 0;
        double dv = 0.0;
        reader >> iv >> dv;
        EXPECT_EQ(header, "alpha");
        EXPECT_EQ(iv, 42);
        EXPECT_DOUBLE_EQ(dv, 3.5);
    }
    std::error_code remove_error;
    fs::remove(path, remove_error);
#else
    GTEST_SKIP() << "未启用 std::filesystem";
#endif
} catch (...) {
    FAIL() << "临时文件演示失败";
}
