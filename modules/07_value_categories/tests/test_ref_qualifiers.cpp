// 验证成员函数引用限定符（& / && / const& / const&&）的重载决议结果。

#include <cstdint>
#include <utility>

#include <gtest/gtest.h>

namespace {

enum class Channel : std::uint8_t { None, LvalueMut, RvalueMut, LvalueConst, RvalueConst };

class Dispatcher {
public:
    void mark() & {  // NOLINT(readability-convert-member-functions-to-static) —
                     // 引用限定符演示必须保留非 static 成员
        kind_ = Channel::LvalueMut;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void mark() && {
        kind_ = Channel::RvalueMut;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void mark() const& {
        kind_ = Channel::LvalueConst;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void mark() const&& {
        kind_ = Channel::RvalueConst;
    }

    static Channel last() {
        return kind_;
    }

    static void reset() {
        kind_ = Channel::None;
    }

private:
    // mutable static inline 计数通道；尾部下划线与私有成员约定一致
    // NOLINTNEXTLINE(readability-identifier-naming)
    static inline Channel kind_ = Channel::None;
};

}  // namespace

TEST(RefQualifiers, MutLvalueSelectsAmpersandOverload) {
    Dispatcher::reset();
    Dispatcher box{};
    box.mark();
    EXPECT_EQ(Dispatcher::last(), Channel::LvalueMut);
}

TEST(RefQualifiers, MutPrvalueSelectsDoubleAmpersandOverload) {
    Dispatcher::reset();
    Dispatcher{}.mark();
    EXPECT_EQ(Dispatcher::last(), Channel::RvalueMut);
}

TEST(RefQualifiers, MutXvalueSelectsDoubleAmpersandOverload) {
    Dispatcher::reset();
    Dispatcher box{};
    std::move(box).mark();
    EXPECT_EQ(Dispatcher::last(), Channel::RvalueMut);
}

TEST(RefQualifiers, ConstLvalueSelectsConstAmpersandOverload) {
    Dispatcher::reset();
    Dispatcher const frozen{};
    frozen.mark();
    EXPECT_EQ(Dispatcher::last(), Channel::LvalueConst);
}

TEST(RefQualifiers, ConstPrvalueSelectsConstDoubleAmpersandOverload) {
    Dispatcher::reset();
    static_cast<Dispatcher const&&>(Dispatcher{}).mark();
    EXPECT_EQ(Dispatcher::last(), Channel::RvalueConst);
}
