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
#ifndef __APPLE__
    // Apple Clang (Xcode ≤ 16) 的 source_location::current() 对 function_name()
    // 返回空字符串，属于已知缺陷，此处跳过。
    EXPECT_GT(std::strlen(loc.function_name()), 0U);
#endif
}

TEST(SourceLocation, LineMatchesCallSite) {
#ifdef __APPLE__
    // Apple Clang 默认参数中的 source_location::current() 捕获的是
    // 函数定义处而非调用处的行号，跳过此断言。
    GTEST_SKIP() << "Apple Clang source_location default-arg line mismatch";
#else
    std::source_location loc;
    loc = capture();
    int const expect_line = __LINE__ - 1;
    EXPECT_EQ(static_cast<int>(loc.line()), expect_line);
#endif
}
