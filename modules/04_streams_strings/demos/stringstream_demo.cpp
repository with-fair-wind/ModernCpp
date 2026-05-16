// 内存字符串流：istringstream / ostringstream / stringstream 与定位 API。

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace {

void printSep() { std::cout << "---\n"; }

void demoIostringstreamBasics() {
    std::ostringstream builder{};
    builder << "coeff=" << 3 << " name=" << "rho";
    std::cout << "ostringstream.str(): \"" << builder.str() << "\"\n";

    std::istringstream parser{"coeff = 3"};
    std::string label{};
    char eq = '?';
    int number = {};
    parser >> label >> eq >> number;
    std::cout << "istringstream 解构: \"" << label << "\" '" << eq << "' " << number << '\n';
}

void demoStringstreamStrAccessor() {
    std::stringstream editor{};
    editor << "baseline";
    std::cout << "初始 str: \"" << editor.str() << "\"\n";
    editor.str("reset");
    std::cout << "赋值 str 后: \"" << editor.str() << "\"\n";
}

#if __cplusplus >= 202002L

void demoStringstreamViewAccessor() {
    std::stringstream bucket{};
    bucket << "lightweight-handle";
    std::string_view const peek = bucket.view();
    std::cout << "stringstream.view()：" << peek << '\n';
}

#else

void demoStringstreamViewAccessor() { std::cout << "视图 API 仅在 C++20 及以上展示。\n"; }

#endif

void demoTellSeekReadWritePointers() {
    std::stringstream cursor{"012345"};
    cursor.seekg(static_cast<std::streamoff>(2), std::ios::beg);
    char snippet = '?';
    cursor.get(snippet);
    std::cout << "tellg（读指针）截取字符 '" << snippet << "', 偏移="
              << static_cast<std::intmax_t>(cursor.tellg()) << '\n';

    cursor.seekp(static_cast<std::streamoff>(1), std::ios::beg);
    cursor.put('!');
    std::cout << "seekp+tellp 补丁后=\"" << cursor.str() << "\" tellp="
              << static_cast<std::intmax_t>(cursor.tellp()) << '\n';
}

void demoAteOpensAtEndOfInitialBuffer() {
    std::ostringstream tail{"seed", std::ios::ate};
    tail << "-tail";
    std::cout << "ostringstream + ate 追加: \"" << tail.str() << "\"\n";
}

void demoInMemoryFormattingPipeline() {
    std::ostringstream stage_one{};
    stage_one << "items=" << 4;
    std::istringstream stage_two{stage_one.str()};
    int items = {};
    char eq = '?';
    std::string keyword{};
    stage_two >> keyword >> eq >> items;
    std::cout << "管道化解析: keyword=\"" << keyword << "\" count=" << items << '\n';
}

}  // namespace

int main() {
    demoIostringstreamBasics();
    printSep();
    demoStringstreamStrAccessor();
    printSep();
    demoStringstreamViewAccessor();
    printSep();
    demoTellSeekReadWritePointers();
    printSep();
    demoAteOpensAtEndOfInitialBuffer();
    printSep();
    demoInMemoryFormattingPipeline();
    return 0;
}
