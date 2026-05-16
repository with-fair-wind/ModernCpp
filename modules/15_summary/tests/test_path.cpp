// path 分解、拼接与 lexically_normal 的可观测行为验证。

#include <filesystem>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

namespace {

fs::path makeSamplePath() {
    return fs::path("segment") / "nested" / "archive.tar.gz";
}

}  // namespace

TEST(PathBasics, Decomposition) {
    fs::path const p = makeSamplePath();
    EXPECT_EQ(p.parent_path(), fs::path("segment") / "nested");
    EXPECT_EQ(p.filename(), "archive.tar.gz");
    EXPECT_EQ(p.extension(), ".gz");
    EXPECT_EQ(p.stem(), "archive.tar");
}

TEST(PathBasics, ConcatOperators) {
    fs::path base = fs::path("var") / "log";
    base /= "app.log";
    EXPECT_EQ(base, fs::path("var") / "log" / "app.log");

    fs::path pieced = fs::path("a");
    pieced.append("b");
    pieced.append("c.txt");
    EXPECT_EQ(pieced, fs::path("a") / "b" / "c.txt");
}

TEST(PathBasics, LexicallyNormalCollapsesDots) {
    fs::path const messy = fs::path("alpha") / ".." / "." / "beta" / "gamma";
    EXPECT_EQ(messy.lexically_normal(), fs::path("beta") / "gamma");
}
