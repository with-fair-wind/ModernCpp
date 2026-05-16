// 验证类模板友元函数模板 / 友元类模板对 SafeBox 私有成员的访问。

#include <sstream>
#include <string>
#include <utility>

#include <gtest/gtest.h>

namespace {

template <typename T>
class SafeBox {
private:
    T value_{};

    template <typename U>
    friend void reveal(SafeBox<U> const& box, std::ostream& out);

    template <typename U>
    friend class Auditor;

public:
    explicit SafeBox(T init) : value_(std::move(init)) {}
};

template <typename U>
void reveal(SafeBox<U> const& box, std::ostream& out) {
    out << box.value_;
}

template <typename T>
class Auditor {
public:
    static std::string peek(SafeBox<T> const& box) {
        std::ostringstream oss;
        oss << box.value_;
        return oss.str();
    }
};

}  // namespace

TEST(FriendsInTemplates, RevealOutputsPrivateValueForIntAndString) {
    SafeBox<int> box_i{1337};
    std::ostringstream oss_i;
    reveal(box_i, oss_i);
    EXPECT_EQ(oss_i.str(), "1337");

    SafeBox<std::string> box_s{std::string("hi")};
    std::ostringstream oss_s;
    reveal(box_s, oss_s);
    EXPECT_EQ(oss_s.str(), "hi");
}

TEST(FriendsInTemplates, AuditorPeekReadsPrivateThroughFriendClassTemplate) {
    SafeBox<int> box{42};
    EXPECT_EQ(Auditor<int>::peek(box), "42");

    SafeBox<std::string> sbox{std::string("audit")};
    EXPECT_EQ(Auditor<std::string>::peek(sbox), "audit");
}
