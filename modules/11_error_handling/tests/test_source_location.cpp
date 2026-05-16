// 模块 11：std::source_location 各字段可读且与调用点一致。

#include <cstring>
#include <source_location>

#include <gtest/gtest.h>

namespace {

std::source_location capture(std::source_location loc = std::source_location::current()) {
    return loc;
}

}  // namespace

TEST(SourceLocation, FileLineFunctionNonEmpty) {
    auto const loc = capture();
    EXPECT_GT(loc.line(), 0U);
    EXPECT_GT(std::strlen(loc.file_name()), 0U);
    EXPECT_GT(std::strlen(loc.function_name()), 0U);
}

TEST(SourceLocation, LineMatchesCallSite) {
    std::source_location loc;
    loc = capture();
    int const expect_line = __LINE__ - 1;
    EXPECT_EQ(static_cast<int>(loc.line()), expect_line);
}
