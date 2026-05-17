// CTAD：构造实参与推导指引应得到期望的 Tracker 推导结果。

#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>

#include <gtest/gtest.h>

namespace {

template <typename Head, typename Tail>
struct Tracker {
    Head head{};
    Tail tail{};

    Tracker(Head head_in, Tail tail_in) : head(std::move(head_in)), tail(std::move(tail_in)) {}
};

template <typename Unified>
Tracker(Unified, Unified) -> Tracker<Unified, Unified>;

}  // namespace

TEST(Ctad, PairLikeTripleDeducesHomogeneousTuple) {
    std::tuple values{42, -1, 7};
    static_assert(std::is_same_v<decltype(values), std::tuple<int, int, int>>);
    EXPECT_EQ(std::tuple_size_v<decltype(values)>, 3U);
}

TEST(Ctad, CustomGuideProducesSameUnifiedType) {
    Tracker deduced_same{std::byte{8}, std::byte{12}};
    static_assert(std::is_same_v<decltype(deduced_same), Tracker<std::byte, std::byte>>);

    Tracker mixed{std::string{"demo"}, std::byte{9}};
    static_assert(std::is_same_v<decltype(mixed.head), std::string>);

    EXPECT_EQ(static_cast<unsigned>(mixed.tail), 9U);
}
