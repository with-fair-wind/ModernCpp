// 迭代器：concept 满足关系、traits、advance/distance、反向/插入/流迭代器。

#include <forward_list>
#include <iterator>
#include <list>
#include <sstream>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

TEST(IteratorConcepts, CategoriesMatchContainer) {
    static_assert(std::contiguous_iterator<std::vector<int>::iterator>);
    static_assert(std::random_access_iterator<std::vector<int>::iterator>);
    static_assert(std::bidirectional_iterator<std::list<int>::iterator>);
    static_assert(!std::random_access_iterator<std::list<int>::iterator>);
    static_assert(std::forward_iterator<std::forward_list<int>::iterator>);
    static_assert(!std::bidirectional_iterator<std::forward_list<int>::iterator>);

    // 指针也是 contiguous 迭代器
    static_assert(std::contiguous_iterator<int*>);
}

TEST(IteratorTraits, ValueAndReference) {
    using It = std::vector<int>::iterator;
    static_assert(std::is_same_v<std::iter_value_t<It>, int>);
    static_assert(std::is_same_v<std::iter_reference_t<It>, int&>);
    static_assert(std::is_same_v<std::iter_difference_t<It>, std::ptrdiff_t>);
}

TEST(IteratorOps, AdvanceAndDistance) {
    std::list<int> lst{1, 2, 3, 4, 5};

    auto it = lst.begin();
    std::advance(it, 3);
    EXPECT_EQ(*it, 4);

    EXPECT_EQ(*std::next(lst.begin(), 2), 3);
    EXPECT_EQ(*std::prev(lst.end(), 1), 5);

    // 对非随机访问迭代器，distance 是 O(n) 但仍正确
    EXPECT_EQ(std::distance(lst.begin(), lst.end()), 5);
}

TEST(ReverseIterator, BaseRelation) {
    std::vector<int> v{1, 2, 3, 4};

    EXPECT_EQ(v.rbegin().base(), v.end());
    EXPECT_EQ(v.rend().base(), v.begin());

    // ++rit 物理上等同 --base()
    auto rit = v.rbegin();
    ++rit;
    EXPECT_EQ(*rit, 3);
    EXPECT_EQ(*rit.base(), 4);  // base() 指向"当前所引用元素之后"
}

TEST(BackInserter, AppendsViaPushBack) {
    std::vector<int> dst;
    auto out = std::back_inserter(dst);
    *out = 10;
    *out = 20;
    *out = 30;
    EXPECT_EQ(dst, (std::vector{10, 20, 30}));
}

TEST(StreamIterator, ReadsUntilParseFails) {
    std::istringstream iss{"1 2 3 stop 4"};
    std::vector<int> got;
    std::copy(std::istream_iterator<int>{iss}, std::istream_iterator<int>{},
              std::back_inserter(got));
    EXPECT_EQ(got, (std::vector{1, 2, 3}));
}

TEST(StreamIterator, OstreamIteratorWritesWithSeparator) {
    std::ostringstream oss;
    std::vector<int> v{1, 2, 3};
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>{oss, "-"});
    EXPECT_EQ(oss.str(), "1-2-3-");
}
