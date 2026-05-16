// std::regex 示例：构造、匹配、搜索、捕获、替换、sregex_iterator；贪婪 / 懒惰、icase。

#include <iostream>
#include <regex>
#include <string>

namespace {

void printSep() { std::cout << "---\n"; }

void demoConstructionAndMatch() {
    std::regex const digits{R"(^\d{3}$)", std::regex_constants::ECMAScript};
    std::string const ok{"007"};
    std::string const bad{"40a"};
    std::cout << "regex_match 整段吻合: \"" << ok << "\" -> " << std::regex_match(ok, digits) << '\n';
    std::cout << "regex_match 整段吻合: \"" << bad << "\" -> " << std::regex_match(bad, digits) << '\n';
}

void demoSearchAndCmatch() {
    std::regex const word{R"(\w+)", std::regex_constants::ECMAScript};
    std::string const buf{" x y "};
    std::cmatch matched{};
    [[maybe_unused]] bool const hit = std::regex_search(buf.data(), buf.data() + buf.size(),
                                                         matched, word);
    std::cout << "regex_search (cmatch) 第一段: \"" << matched.str() << "\" 位置="
              << matched.position() << '\n';
}

void demoCaptureGroupsAndSmatch() {
    std::regex const path_re{R"(/usr/([^/]+)/([^/]+))"};
    std::string const uri{"/usr/local/bin"};
    std::smatch captures{};
    if (std::regex_search(uri, captures, path_re)) {
        std::cout << "smatch 全匹配: \"" << captures[0].str() << "\"\n";
        std::cout << "  捕获1: \"" << captures[1].str() << "\" 捕获2: \"" << captures[2].str()
                  << "\"\n";
    }
}

void demoReplace() {
    std::regex const spaces{R"(\s+)", std::regex_constants::ECMAScript};
    std::string noisy{" aa   bb \t cc "};
    std::string const cleaned = std::regex_replace(noisy, spaces, " ");
    std::cout << "regex_replace 压空白: \"" << cleaned << "\"\n";
}

void demoSregexIteratorAllMatches() {
    std::regex const token{R"(\d+)"};
    std::string const haystack{"hits: 12 and 348 plus 9"};
    std::cout << "sregex_iterator 列出全部数字: ";
    for (auto it = std::sregex_iterator(haystack.begin(), haystack.end(), token);
         it != std::sregex_iterator(); ++it) {
        std::cout << '[' << (*it).str() << "] ";
    }
    std::cout << '\n';
}

void demoGreedyVsLazyStar() {
    std::regex const greedy{R"(<.*>)"};
    std::regex const lazy{R"(<.*?>)"};
    std::string const html{"<tag>nested</tag>"};
    std::smatch g{};
    std::smatch l{};
    [[maybe_unused]] bool const greedy_hit = std::regex_search(html, g, greedy);
    [[maybe_unused]] bool const lazy_hit = std::regex_search(html, l, lazy);
    std::cout << "贪婪 .*：\"" << g.str() << "\"\n";
    std::cout << "懒惰 .*?：\"" << l.str() << "\"\n";
}

void demoIcaseRawString() {
    std::regex const hello{R"(^Hello WORLD$)", std::regex_constants::ECMAScript
                                                  | std::regex_constants::icase};
    std::string variant{"HELLO woRLd"};
    std::cout << "icase + 原始字符串字面量: \"" << variant << "\" 整匹配 -> "
              << std::regex_match(variant, hello) << '\n';
}

}  // namespace

int main() {  // NOLINT(bugprone-exception-escape)
    demoConstructionAndMatch();
    printSep();
    demoSearchAndCmatch();
    printSep();
    demoCaptureGroupsAndSmatch();
    printSep();
    demoReplace();
    printSep();
    demoSregexIteratorAllMatches();
    printSep();
    demoGreedyVsLazyStar();
    printSep();
    demoIcaseRawString();
    return 0;
}
