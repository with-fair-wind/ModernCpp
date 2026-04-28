// Smoke test for module 02: object lifetime and reference rebinding rules.

#include <string>

#include <gtest/gtest.h>

namespace {

struct Counter {
    int* live;
    explicit Counter(int* l) : live(l) {
        ++*live;
    }
    Counter(Counter const& other) : live(other.live) {
        ++*live;
    }
    Counter(Counter&& other) noexcept : live(other.live) {
        ++*live;
    }
    Counter& operator=(Counter const&) = default;
    Counter& operator=(Counter&&) noexcept = default;
    ~Counter() {
        --*live;
    }
};

}  // namespace

TEST(Lifetime, ScopedObjectsAreDestroyedAtScopeExit) {
    int live = 0;
    {
        Counter const a{&live};
        EXPECT_EQ(live, 1);
        {
            Counter const b{&live};
            EXPECT_EQ(live, 2);
        }
        EXPECT_EQ(live, 1);
    }
    EXPECT_EQ(live, 0);
}

TEST(Lifetime, ReferenceBindsToInitializerNotLater) {
    std::string a{"first"};
    std::string b{"second"};
    std::string& ref = a;
    ref = b;  // assigns through ref into a; does NOT rebind ref to b
    EXPECT_EQ(a, "second");
    EXPECT_EQ(&ref, &a);
}
