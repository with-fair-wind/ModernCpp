// 验证变参模板模板形参与严格单参模板模板形参（duplicateValue / duplicateStrict + Bag）。

#include <deque>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

namespace {

template <template <typename...> class Outer, typename T>
[[nodiscard]] Outer<T> duplicateValue(T value) {
    Outer<T> out;
    out.push_back(value);
    out.push_back(value);
    return out;
}

template <typename Elem>
struct Bag {
    std::vector<Elem> store_{};
    void pushBack(Elem v) {
        store_.push_back(std::move(v));
    }
};

template <template <typename> class Container, typename T>
[[nodiscard]] Container<T> duplicateStrict(T value) {
    Container<T> out;
    out.pushBack(value);
    out.pushBack(value);
    return out;
}

constexpr int kSeed = 7;

}  // namespace

TEST(TemplateTemplateParam, DuplicateValueVectorAndDeque) {
    auto vec = duplicateValue<std::vector, int>(kSeed);
    auto deq = duplicateValue<std::deque, int>(kSeed);

    ASSERT_EQ(vec.size(), 2U);
    ASSERT_EQ(deq.size(), 2U);
    EXPECT_EQ(vec[0], kSeed);
    EXPECT_EQ(vec[1], kSeed);
    EXPECT_EQ(deq[0], kSeed);
    EXPECT_EQ(deq[1], kSeed);

    static_assert(std::is_same_v<decltype(vec), std::vector<int>>);
    static_assert(std::is_same_v<decltype(deq), std::deque<int>>);
}

TEST(TemplateTemplateParam, DuplicateStrictWithBagAndStaticAssertions) {
    auto bag = duplicateStrict<Bag, int>(kSeed);

    ASSERT_EQ(bag.store_.size(), 2U);
    EXPECT_EQ(bag.store_[0], kSeed);
    EXPECT_EQ(bag.store_[1], kSeed);

    static_assert(std::is_same_v<decltype(bag), Bag<int>>);
}
