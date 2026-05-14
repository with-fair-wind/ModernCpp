// 命名空间测试：嵌套命名空间、inline namespace 版本控制、
// using 声明、命名空间别名。

#include <string>

#include <gtest/gtest.h>

// ========== 测试用命名空间定义 ==========

namespace outer::inner {
int compute(int x) { return x * 2; }
}  // namespace outer::inner

// inline namespace 版本控制
namespace library {

inline namespace v2 {
int process(int x) { return x + 100; }
std::string version() { return "2.0"; }
}  // namespace v2

namespace v1 {
int process(int x) { return x + 10; }
std::string version() { return "1.0"; }
}  // namespace v1

}  // namespace library

// 同名命名空间扩展
namespace outer::inner {
int negate(int x) { return -x; }
}  // namespace outer::inner

// 命名空间别名
namespace oi = outer::inner;

// ========== 测试 ==========

TEST(Namespace, NestedCpp17Syntax) {
    // C++17 嵌套命名空间语法 namespace A::B { }
    EXPECT_EQ(outer::inner::compute(5), 10);
}

TEST(Namespace, ExtensionAddsToSameNamespace) {
    // 命名空间可多次打开，扩展内容属于同一命名空间
    EXPECT_EQ(outer::inner::negate(3), -3);
    EXPECT_EQ(outer::inner::compute(3), 6);
}

TEST(Namespace, Alias) {
    // 命名空间别名简化长名字
    EXPECT_EQ(oi::compute(7), 14);
    EXPECT_EQ(oi::negate(7), -7);
}

TEST(Namespace, UsingDeclaration) {
    using outer::inner::compute;
    // using 声明后可不写命名空间前缀
    EXPECT_EQ(compute(10), 20);
}

TEST(Namespace, InlineNamespaceDefaultVersion) {
    // 未加限定名时默认使用 inline namespace（v2）
    EXPECT_EQ(library::process(5), 105);
    EXPECT_EQ(library::version(), "2.0");
}

TEST(Namespace, InlineNamespaceExplicitVersion) {
    // 显式指定旧版本
    EXPECT_EQ(library::v1::process(5), 15);
    EXPECT_EQ(library::v1::version(), "1.0");

    // 显式指定新版本（与默认相同）
    EXPECT_EQ(library::v2::process(5), 105);
    EXPECT_EQ(library::v2::version(), "2.0");
}

TEST(Namespace, InlineNamespaceIsNotAmbiguous) {
    // 通过 v2 或直接通过 library 访问得到相同结果
    EXPECT_EQ(library::process(0), library::v2::process(0));
}
