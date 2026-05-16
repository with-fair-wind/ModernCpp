// 模块 11：异常捕获、重抛与自定义异常类型。

#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

namespace {

class AppError : public std::runtime_error {
public:
    explicit AppError(std::string const& what) : std::runtime_error(what) {}
};

void throwAppError() {
    throw AppError{"应用错误"};
}

void nestedRethrow() {
    try {
        throw std::invalid_argument{"内层"};
    } catch (std::exception const&) {
        throw;
    }
}

}  // namespace

TEST(Exception, CatchStdRuntimeError) {
    EXPECT_THROW(throw std::runtime_error{"x"}, std::runtime_error);
}

TEST(Exception, CatchDerivedAsBase) {
    EXPECT_THROW(throwAppError(), std::runtime_error);
}

TEST(Exception, CatchExactDerived) {
    try {
        throwAppError();
    } catch (AppError const& ex) {
        EXPECT_EQ(std::string{ex.what()}, "应用错误");
        return;
    }
    FAIL() << "未捕获 AppError";
}

TEST(Exception, RethrowPreservesType) {
    EXPECT_THROW(nestedRethrow(), std::invalid_argument);
}

TEST(Exception, NoThrowWhenNothingThrown) {
    EXPECT_NO_THROW(static_cast<void>(42));
}
