// std::print / std::println 写入 std::ostringstream 的可观察行为。
//
// MinGW + libstdc++：虽有特性测试宏，但链接阶段缺少终端辅助符号，此处自动跳过测试。

#if defined(__MINGW32__) && defined(__GLIBCXX__)
#define MCPP_LIBSTDCPP_STREAM_PRINT_UNSUPPORTED 1
#endif

#if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L) && !defined(MCPP_LIBSTDCPP_STREAM_PRINT_UNSUPPORTED)
#define MCPP_HAVE_STREAM_PRINT 1
#endif

#include <sstream>
#include <string>

#include <gtest/gtest.h>

#ifdef MCPP_HAVE_STREAM_PRINT
#include <print>
#endif

#ifdef MCPP_HAVE_STREAM_PRINT

TEST(stdPrint,OstreamOverloadCapturesInterpolation) {
    std::ostringstream capture;
    ASSERT_TRUE(capture.good());
    std::print(capture, "n={}", 8);
    EXPECT_EQ(capture.str(), "n=8");
    EXPECT_TRUE(capture.good());

    capture.str("");
    std::print(capture, "{}+{}={}", 2, 3, 5);
    EXPECT_EQ(capture.str(), "2+3=5");
    EXPECT_FALSE(capture.str().empty());

    capture.str("");
    std::print(capture, "empty{{}} brace");
    EXPECT_NE(capture.str().find("empty{} brace"), std::string::npos);
}

TEST(stdPrint,PrintlnAddsNewlineCharacter) {
    std::ostringstream capture;
    std::println(capture, "{}", "done");
    auto const payload = capture.str();
    ASSERT_FALSE(payload.empty());
    EXPECT_EQ(payload.back(), '\n');
    EXPECT_NE(payload.find("done"), std::string::npos);
    EXPECT_GE(payload.size(), 5U);
}

TEST(stdPrint,SupportsFormatSpecifiers) {
    std::ostringstream capture;
    std::print(capture, "{:+05}", 42);
    EXPECT_EQ(capture.str(), "+0042");

    capture.str("");
    std::print(capture, "{:#x}", 255);
    EXPECT_EQ(capture.str(), "0xff");

    capture.str("");
    std::print(capture, "{:>6}", -3);
    EXPECT_EQ(capture.str(), "    -3");
}

#else

TEST(stdPrint,UnavailableEnvironment) {
#ifdef MCPP_LIBSTDCPP_STREAM_PRINT_UNSUPPORTED
    GTEST_SKIP() << "MinGW libstdc++ 尚未提供完整的 stream print 链接符号";
#elifndef __cpp_lib_print
    GTEST_SKIP() << "当前翻译单元缺失 __cpp_lib_print";
#else
    GTEST_SKIP() << "当前组合不支持 stream print";
#endif
}

#endif
