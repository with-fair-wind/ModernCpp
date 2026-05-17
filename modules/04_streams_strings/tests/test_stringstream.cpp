// StringStream：覆盖 stringstream_demo.cpp 所列行为的小型测试。

#include <sstream>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

TEST(StringStream, ConstructsFromLiteralAndParsesInts) {
    std::istringstream source{"coeff = 42 remainder"};
    std::string label{};
    char eq = '?';
    int value = {};
    ASSERT_TRUE(static_cast<bool>(source >> label >> eq >> value));
    EXPECT_EQ(label, "coeff");
    EXPECT_EQ(eq, '=');
    EXPECT_EQ(value, 42);
}

TEST(StringStream, StrGetterSetterResetsBackingStore) {
    std::stringstream editor{};
    editor << "draft";
    EXPECT_EQ(editor.str(), "draft");
    editor.str("final");
    EXPECT_EQ(editor.str(), "final");
}

#if __cplusplus >= 202002L

TEST(StringStream, ViewReflectsBackingStringWithoutAllocation) {
    std::stringstream bucket{};
    bucket << "peekable";
    std::string_view const window = bucket.view();
    EXPECT_EQ(window, "peekable");
}

#endif

TEST(StringStream, SeekAndTellPatchMiddleCharacter) {
    std::stringstream surface{"@@@@@"};
    surface.seekp(static_cast<std::streamoff>(2), std::ios::beg);
    surface << "##";
    EXPECT_EQ(surface.str(), "@@##@");
}

TEST(StringStream, AteOpensWriteCursorAtExistingSuffix) {
    std::ostringstream tail{"seed", std::ios::ate};
    tail << "+more";
    EXPECT_EQ(tail.str(), "seed+more");
}

TEST(StringStream, FormatThenParseActsAsPortablePipeline) {
    std::ostringstream formatted{};
    formatted << "n=" << 7;
    std::istringstream reader{formatted.str()};
    char key = '?';
    char delimiter = '?';
    int parsed = {};
    ASSERT_TRUE(static_cast<bool>(reader >> key >> delimiter >> parsed));
    EXPECT_EQ(key, 'n');
    EXPECT_EQ(delimiter, '=');
    EXPECT_EQ(parsed, 7);
}
